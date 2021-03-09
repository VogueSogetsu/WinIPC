// WinIPCPeer.h : CWinIPCPeer µÄÉùÃ÷

#pragma once
#include "resource.h"       // Ö÷·ûºÅ

#include "WinIPC.h"
#include "IPCPeer.h"
#include <atlpath.h>
#include <vector>


// CWinIPCPeer

class ATL_NO_VTABLE CWinIPCPeer 
: public CComObjectRootEx<CComSingleThreadModel>
, public CComCoClass<CWinIPCPeer, &CLSID_WinIPCPeer>
, public IDispatchImpl<IWinIPCPeer, &IID_IWinIPCPeer, &LIBID_WinIPCLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
, public CIPCPeer
, public CIPCSenderListener
, public CIPCReceiverListener
{
public:
	CWinIPCPeer()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_WINIPCPEER)


BEGIN_COM_MAP(CWinIPCPeer)
	COM_INTERFACE_ENTRY(IWinIPCPeer)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		m_bExit = FALSE;
		m_hEventThread = NULL;
		m_hSemEvent = NULL;

		TCHAR szLogPath[MAX_PATH] = _T("");
		GetModuleFileName((HMODULE)&__ImageBase, szLogPath, MAX_PATH);

		TCHAR szPath[MAX_PATH] = _T("");
		GetModuleFileName(NULL, szPath, MAX_PATH);
		PathRemoveExtension(szPath);

		CString sExeName = PathFindFileName(szPath);

		CString sLogCfgName;
		sLogCfgName.Format(_T("..\\log4cplus_%s.cfg"), sExeName);
		ATLPath::Combine(szLogPath, szLogPath, sLogCfgName);

		OutputDebugString(szLogPath);
		LOG_INIT_EX(szLogPath);	

        m_Sender.SetSenderListener(this);
        m_Receiver.SetReceiverListener(this);
		return S_OK;
	}
	
	void FinalRelease() 
	{
        Close();
        Clean();
	}

public:
	virtual BOOL			Create(BOOL bAsSender, BOOL bAsReceiver, 
								   INT nProductID, BOOL bSenderCompact = FALSE,
		                           BOOL bExitIfSameProductExist = FALSE);
	virtual VOID			Close();

	virtual VOID			OnSenderConnect(INT nTargetID);
	virtual VOID			OnReceiverConnect(INT nTargetID);
	virtual VOID			OnReceiverDisconnect(INT nTargetID);
    virtual VOID            OnSendFinish(DWORD dwOffset, DWORD dwTargetID, INT nRet);
    virtual VOID            OnReceiveData(INT nSenderID, LPBYTE pBuffer, DWORD dwSize);
	virtual VOID			OnReceiveMsg(INT nSenderID, int p1, int p2);

private:
    VOID                    Clean();
	static DWORD WINAPI		EventHandleThread(LPVOID lpParam);
	VOID					EventMainLoop();

private:
    typedef std::set<IWinIPCEvent*> WinIPCEventHandlerSet;

    CCritSec                m_lockHandler;
    WinIPCEventHandlerSet   m_setEventHandler;

	typedef struct stEventData
	{
		int nType;
		long p1;
		long p2;
		long p3;
	} EventData;
	typedef std::vector<EventData> WinIPCEventDataVec;

	enum enumEventType
	{
		SENDER_CONNECT = 0,
		RECEIVER_CONNECT,
		RECEIVER_DISCONNECT,
		SEND_FINISH,
		RECEIVE_DATA,
		RECEIVE_MSG,
	};

	CCritSec				m_lockEventData;
	HANDLE					m_hEventThread;
	WinIPCEventDataVec		m_vecEventData;
	HANDLE					m_hSemEvent;
	BOOL					m_bExit;

    LOG_CLS_DEC();

public:
    STDMETHOD(get_isSender)(VARIANT_BOOL* pVal);
    STDMETHOD(get_isReceiver)(VARIANT_BOOL* pVal);
    STDMETHOD(get_productID)(LONG* pVal);

    STDMETHOD(AddEventHandler)(IUnknown* pHandler);
    STDMETHOD(RemoveEventHandler)(IUnknown* pEventHandler);

    STDMETHOD(AddSenderProductID)(LONG lProductID);
    STDMETHOD(AddReceiverProductID)(LONG lProductID);
    STDMETHOD(RemoveSenderProductID)(LONG lProductID);
    STDMETHOD(RemoveReceiverProductID)(LONG lProductID);

    STDMETHOD(SendData)(BYTE* pBuffer, ULONG ulSize, VARIANT_BOOL vbSync);
	STDMETHOD(SendDataToSpecialProduct)(LONG lProductID, BYTE* pBuffer, ULONG ulSize, VARIANT_BOOL vbSync);
	STDMETHOD(SendMsgToSpecialProduct)(LONG lProductID, LONG lParam1, LONG lParam2);
    STDMETHOD(ClosePeer)();

	STDMETHOD(IsSenderConnected)(LONG lProductID, VARIANT_BOOL* pVal);
	STDMETHOD(IsReceiverConnected)(LONG lProductID, VARIANT_BOOL* pVal);
};

OBJECT_ENTRY_AUTO(__uuidof(WinIPCPeer), CWinIPCPeer)
