#ifndef _IPCPEER_IMPL2_H
#define _IPCPEER_IMPL2_H

#include <vector>
#include "WinIPC.h"
#include <atlpath.h>

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
	CIPCPeerImpl() : m_hModule(NULL)
	{

	}

	~CIPCPeerImpl()
	{

	}

public:
	BOOL CreateIPCPeer(INT nProductID, int nTargetId, IUnknown* pEventHandler)
	{
		TargetProductIDVec vecTarget;
		vecTarget.push_back(nTargetId);
		return CreateIPCPeer(nProductID, vecTarget, pEventHandler);
	}

	BOOL CreateIPCPeer(INT nProductID, TargetProductIDVec& vecTargetId, IUnknown* pEventHandler)
	{
		if(vecTargetId.empty())
		{
			return TRUE;
		}

		if(m_spIPCPeer)
		{			
			return TRUE;
		}

		TCHAR szPath[MAX_PATH] = {0};
		GetModuleFileName((HMODULE)&__ImageBase, szPath, MAX_PATH);
		ATLPath::Combine(szPath, szPath, _T("..\\WinIPC.dll"));

		m_hModule = LoadLibrary(szPath);
		if(NULL == m_hModule)
		{
			return FALSE;        
		}

		INT i = 0;
		INT nCount = (INT)vecTargetId.size();
		INT* pIDs = new(std::nothrow) INT[nCount];
		if (pIDs == NULL)
		{
			//LOG_ERROR("IPCPeerWrapper::CreateIPCPeer - Alloc Memory(pIDs) failed! Size:" << nCount);
			OutputDebugString(_T("IPCPeerWrapper::CreateIPCPeer - Alloc Memory(pIDs) failed!"));
			return FALSE;
		}

		TargetProductIDVec::iterator it;
		for(it = vecTargetId.begin(); it != vecTargetId.end() && i < nCount; it++)
		{
			pIDs[i++] = (*it);
		}

		BOOL bRet = FALSE;
		do 
		{
			pFn_CreateIPCPeerArray pFunc = (pFn_CreateIPCPeerArray)GetProcAddress(m_hModule, "CreateIPCPeerArray");
			if(!pFunc)
			{				
				break;
			}

			HRESULT hr = pFunc(nProductID, TRUE, TRUE, pIDs, nCount, pIDs, nCount, FALSE, &m_spIPCPeer);
			if(FAILED(hr) || !m_spIPCPeer)
			{
				break;
			}

			hr = m_spIPCPeer->AddEventHandler(pEventHandler);
			if(FAILED(hr))
			{
				m_spIPCPeer = NULL;
				break;
			}

			m_spEventHandler = pEventHandler;
			bRet = TRUE;
		} while (false);

		delete[] pIDs;		
		return bRet;
	}

	VOID CloseIPCPeer()
	{
		if(m_spIPCPeer)
		{
			m_spIPCPeer->ClosePeer();
			m_spIPCPeer = NULL;
		}
		if(m_hModule)
		{
			FreeLibrary(m_hModule);
			m_hModule = NULL;
		}
	}

	BOOL IsSenderConnected( int nTargetID )
	{
		if(m_spIPCPeer)
		{
			VARIANT_BOOL vbRet = VARIANT_FALSE;
			m_spIPCPeer->IsSenderConnected(nTargetID, &vbRet);
			return (vbRet == VARIANT_TRUE ? TRUE : FALSE);
		}
		return FALSE;
	}

	BOOL IsReceiverConnected( int nTargetID )
	{
		if(m_spIPCPeer)
		{
			VARIANT_BOOL vbRet = VARIANT_FALSE;
			m_spIPCPeer->IsReceiverConnected(nTargetID, &vbRet);
			return (vbRet == VARIANT_TRUE ? TRUE : FALSE);
		}
		return FALSE;
	}

	BOOL IsTargetConnected( int nTargetID )
	{
		return (IsSenderConnected(nTargetID) && IsReceiverConnected(nTargetID));
	}

protected:	
	HMODULE					m_hModule;
	CComPtr<IWinIPCPeer>    m_spIPCPeer;
	CComQIPtr<IWinIPCEvent> m_spEventHandler;
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
		/* [in] */ ULONG lTargetID){ return S_OK;}

	virtual HRESULT STDMETHODCALLTYPE OnReceiverDisconnect( 
		/* [in] */ ULONG lTargetID){ return S_OK;}

	virtual HRESULT STDMETHODCALLTYPE OnSendFinish( 
		/* [in] */ ULONG lOffset,
		/* [in] */ ULONG lTargetID,
		/* [in] */ LONG lRet){ return S_OK;}

	virtual HRESULT STDMETHODCALLTYPE OnReceiveData( 
		/* [in] */ ULONG lSenderID,
		/* [in] */ LPBYTE lBuffer,
		/* [in] */ ULONG lSize){ return S_OK;}

	virtual HRESULT STDMETHODCALLTYPE OnReceiveMsg( 
		/* [in] */ ULONG lSenderID,
		/* [in] */ LONG lParam1,
		/* [in] */ LONG lParam2){ return S_OK;}
};

#endif // _IPCPEER_IMPL2_H


