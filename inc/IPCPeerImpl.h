#include <vector>
#include <xl_data/xl_data_sim.h>
#include "WinIPC.h"
#include <atlpath.h>

using namespace xl::sim_class;

typedef std::vector<INT> TargetProductIDVec;

typedef HRESULT (WINAPI *pFn_CreateIPCPeerArray)(INT nProductID, 
											     BOOL bAsSender,
												 BOOL bAsReceiver,
												 INT* nAcceptSenderProductID,
												 INT nSenderCount,
												 INT* nAcceptReceiverProductID,
												 INT nReceiverCount,
												 BOOL bExitIfSameProductExist,
												 IWinIPCPeer** ppOut);

template <typename T>
class ATL_NO_VTABLE CIPCPeerImpl
{
public:
	CIPCPeerImpl()
	{

	}

	~CIPCPeerImpl()
	{

	}

public:
	BOOL CreateIPCPeer(INT nProductID, TargetProductIDVec& vecTargetId, IUnknown* pEventHandler)
	{
		//LOG_DEBUG(_T("IPCPeerWrapper::CreateIPCPeer - productid=") << nProductID <<
		//	_T(", target productid size=") << vecTargetId.size());

		if(vecTargetId.empty())
		{
			return TRUE;
		}

		if(m_spIPCPeer)
		{
			//LOG_INFO(_T("IPCPeerWrapper::CreateIPCPeer - IPC peer already in running"));
			return TRUE;
		}

		TCHAR szPath[MAX_PATH] = {0};
		GetModuleFileName((HMODULE)&__ImageBase, szPath, MAX_PATH);

		ATLPath::Combine(szPath, szPath, _T("..\\WinIPC.dll"));

		HMODULE hModule = LoadLibrary(szPath);
		if(NULL == hModule)
		{
			//LOG_ERROR(_T("IPCPeerWrapper::CreateIPCPeer - LoadLibrary(WinIPC.dll) failed!"));
			return FALSE;        
		}

		INT nCount = (INT)vecTargetId.size();
		INT	*pIDs = new(std::nothrow)INT[nCount];
		if (pIDs == NULL)
		{
			//LOG_ERROR("IPCPeerWrapper::CreateIPCPeer - Alloc Memory(pIDs) failed! Size:" << nCount);
			OutputDebugString(_T("IPCPeerWrapper::CreateIPCPeer - Alloc Memory(pIDs) failed!"));
			return FALSE;
		}

		TargetProductIDVec::iterator it;

		INT i = 0;
		for(it = vecTargetId.begin(); it != vecTargetId.end() && i < nCount; it++)
		{
			pIDs[i++] = *it;
		}

		do 
		{
			pFn_CreateIPCPeerArray pFunc = (pFn_CreateIPCPeerArray)GetProcAddress(hModule, "CreateIPCPeerArray");
			if(!pFunc)
			{
				//LOG_ERROR(_T("IPCPeerWrapper::CreateIPCPeer - GetProcAddress(CreateIPCPeerArray) failed!"));				
				return FALSE;
			}

			HRESULT hr = pFunc(nProductID, TRUE, TRUE, pIDs, nCount, pIDs, nCount, FALSE, &m_spIPCPeer);
			if(FAILED(hr) || !m_spIPCPeer)
			{
				//LOG_ERROR(_T("IPCPeerWrapper::CreateIPCPeer - Call CreateIPCPeer return hr=") << hr);
				return FALSE;
			}

			hr = m_spIPCPeer->AddEventHandler(pEventHandler);
			if(FAILED(hr))
			{
				//LOG_ERROR(_T("IPCPeerWrapper::CreateIPCPeer - AddEventHandler failed! hr=") << hr);
				m_spIPCPeer = NULL;
				return FALSE;
			}

			m_spEventHandler = pEventHandler;			
			//LOG_INFO(_T("IPCPeerWrapper::CreateIPCPeer - Succeeded!"));
		} while (false);

		delete[] pIDs;
		//FreeLibrary(hModule);
		
		return TRUE;
	}

	VOID CloseIPCPeer()
	{
		//LOG_METHOD();
		if(m_spIPCPeer)
		{
			if(m_spEventHandler)
			{
				m_spIPCPeer->RemoveEventHandler(m_spEventHandler);
				m_spEventHandler.Release();
			}
			m_spIPCPeer->ClosePeer();
			m_spIPCPeer = NULL;
		}
	}

	BOOL SendCmd(INT nCmdID, xl_data& data, INT nTargetProductID = -1)
	{
		if(!m_spIPCPeer)
		{
			//LOG_WARN(_T("IPCPeer not ready"));
			return FALSE;
		}

		//LOG_DEBUG(_T("IPCPeerWrapper::SendCmd - cmdid=") << nCmdID);

		data["CmdID"] = nCmdID;			
		data.set_format(xl_data::FORMAT_BEN);
		string buffer = data.encode();

		if(buffer.empty())
		{
			//LOG_ERROR(_T("Encode buffer failed"));
			return FALSE;
		}

		HRESULT hr = E_FAIL;
		if(nTargetProductID == -1)
		{
			hr = m_spIPCPeer->SendData((BYTE*)buffer.c_str(), buffer.length(), FALSE);
		}
		else
		{
			hr = m_spIPCPeer->SendDataToSpecialProduct(nTargetProductID, (BYTE*)buffer.c_str(), buffer.length(), FALSE);
		}

        return SUCCEEDED(hr);
	}	

protected:	
	CComPtr<IWinIPCPeer>    m_spIPCPeer;
	CComQIPtr<IWinIPCEvent> m_spEventHandler;

public:
	//LOG_CLS_DEC();
};

template <typename T>
class ATL_NO_VTABLE CIPCEvent : public IWinIPCEvent
{
public:
	STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject)   
	{   
		if (iid == __uuidof(IWinIPCEvent))   
		{   
			*ppvObject = (IWinIPCEvent *)this;   
			AddRef();   
			return S_OK;   
		}   
		if (iid == IID_IUnknown)   
		{   
			*ppvObject = (IUnknown *)this;   
			AddRef();   
			return S_OK;   
		}   
		return E_NOINTERFACE;   
	}   
	ULONG STDMETHODCALLTYPE AddRef()   
	{  		 
		return 1;   
	}   
	ULONG STDMETHODCALLTYPE Release()   
	{		
		return 1;   
	}

	virtual HRESULT STDMETHODCALLTYPE OnConnect( 
		/* [in] */ VARIANT_BOOL vbIsSender,
		/* [in] */ ULONG lTargetID){return S_OK;}

	virtual HRESULT STDMETHODCALLTYPE OnReceiverDisconnect( 
		/* [in] */ ULONG lTargetID){return S_OK;}

	virtual HRESULT STDMETHODCALLTYPE OnSendFinish( 
		/* [in] */ ULONG lOffset,
		/* [in] */ ULONG lTargetID,
		/* [in] */ LONG lRet){return S_OK;}

	virtual HRESULT STDMETHODCALLTYPE OnReceiveData( 
		/* [in] */ ULONG lSenderID,
		/* [in] */ BYTE* lBuffer,
		/* [in] */ ULONG lSize)
	{
		xl_data dx;
		dx["Buffer"]        = (long)lBuffer;
		dx["BufferSize"]    = lSize;
		dx["SenderID"]      = lSenderID;
		return OnIPCEvent(IPC_ET_RECEIVER, _T("OnReceiveData"), dx);
	}

	virtual HRESULT STDMETHODCALLTYPE OnReceiveMsg( 
		/* [in] */ ULONG lSenderID,
		/* [in] */ LONG lParam1,
		/* [in] */ LONG lParam2){return S_OK;}

	virtual HRESULT STDMETHODCALLTYPE OnSendFailed( 
		/* [in] */ ULONG lOffset,
		/* [in] */ ULONG lTargetID,
		/* [in] */ LONG lRet)
	{
		xl_data dx;
		dx["Offet"]     = lOffset;
		dx["TargetID"]  = lTargetID;
		dx["Reture"]    = lRet;
		return OnIPCEvent(IPC_ET_SENDER, _T("OnSendFailed"), dx);
	}

	virtual HRESULT STDMETHODCALLTYPE OnIPCEvent( 
		/* [in] */ enumIPCEvenType type,
		/* [in] */ BSTR bstrEvent,
		/* [in] */ VARIANT varRet){return 0;}
};

class IPCCallBack
{
public:
	virtual void updateIPCData(xl_data& data) = 0;
};

enum enumIPCCmd
{
	IPC_CMD_ADD_GAME				= 10001,
	IPC_CMD_REMOVE_GAME				= 10002,
	IPC_CMD_GAME_OPERATION			= 10003,
	IPC_CMD_GAME_EXIT				= 10004,
	IPC_CMD_RET_GAMEPROCINFO		= 10005,
	IPC_CMD_BOSS_KEY				= 10006,
	IPC_CMD_GET_GAME_SEVED			= 10007,
	IPC_CMD_BOSSKEY_RESULT			= 10008,
	IPC_CMD_CLOSE_ALL_GAME			= 10009,
	IPC_CMD_TO_GAMEBOX				= 10010,
	IPC_CMD_STONE_CHANGED			= 10011,
	IPC_CMD_COIN_CHANGED			= 10012,
	IPC_CMD_GET_COIN_LIMIT			= 10013,
	IPC_CMD_MATCH_ACTION			= 10014,
	IPC_CMD_TO_MINIGAMEHALL			= 10015,
	IPC_CMD_ADDIN_GET_DATA			= 10016,
	IPC_CMD_ADDIN_DATA_NOTIFY		= 10017,
	IPC_CMD_GET_XLUU_STATUS			= 10018,
	IPC_CMD_NOTIFY_XLUU_STATUS		= 10019,
	IPC_CMD_GET_XL7ADDIN_STATUS		= 10020,
	IPC_CMD_NOTIFY_XL7ADDIN_STATUS	= 10021,
	IPC_CMD_CONNECTFLASH			= 10022,
	IPC_CMD_COMMON_FUNCTION			= 20000,
};

enum enumIPCTarget
{
	IPC_TARGET_BEGIN	= 100,
	IPC_TARGET_ADRBOX	= 101,
	IPC_TARGET_XCPLAY	= 102,
	IPC_TARGET_COMMON_DOWNLOAD,
	IPC_TARGET_END		= 104,
};

enum enumGameOperation
{
	GAME_OP_START = 1,
};

