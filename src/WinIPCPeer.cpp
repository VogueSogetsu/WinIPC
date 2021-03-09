// WinIPCPeer.cpp : CWinIPCPeer µÄÊµÏÖ

#include "stdafx.h"
#include "WinIPCPeer.h"
#include <atlpath.h>
#include <algorithm>


// CWinIPCPeer

VOID CWinIPCPeer::Clean()
{
	LOG_METHOD();
	m_bExit = TRUE;	

	{
		CAutoLock lock(&m_lockEventData);
		m_vecEventData.clear();
	}

	LOG_DEBUG(_T("Terminate event thread anyway"));
	if(m_hEventThread)
	{			
		if(m_hSemEvent)
		{		
			ReleaseSemaphore(m_hSemEvent, 100, NULL);			
		}

		WaitForSingleObject(m_hEventThread, 100);
		TerminateThread(m_hEventThread, 0);
		CloseHandle(m_hEventThread);
		m_hEventThread = NULL;

		CloseHandle(m_hSemEvent);
		m_hSemEvent = NULL;
	}

	LOG_DEBUG(_T("Clean event handler"));
    CAutoLock lock(&m_lockHandler);
    WinIPCEventHandlerSet::iterator it;
    for(it = m_setEventHandler.begin(); it != m_setEventHandler.end(); it++)
    {
        if(*it)
        {
            (*it)->Release();
        }
    }
    m_setEventHandler.clear();
}

VOID CWinIPCPeer::OnSendFinish(DWORD dwOffset, DWORD dwTargetID, INT nRet)
{
	{
		CAutoLock lock(&m_lockEventData);
		EventData newData;
		newData.nType = SEND_FINISH;
		newData.p1 = dwOffset;
		newData.p2 = dwTargetID;
		newData.p3 = nRet;
		m_vecEventData.push_back(newData);
	}
	ReleaseSemaphore(m_hSemEvent, 1, NULL);
}

VOID CWinIPCPeer::OnReceiveData(INT nSenderID, LPBYTE pBuffer, DWORD dwSize)
{
	IWinIPCEvent* pEventHandler = NULL;
	WinIPCEventHandlerSet::iterator itHandler;
	CAutoLock lock(&m_lockHandler);
	for(itHandler = m_setEventHandler.begin(); itHandler != m_setEventHandler.end(); itHandler++)
	{
		pEventHandler = (*itHandler);
		if(pEventHandler)
		{
			pEventHandler->OnReceiveData(nSenderID, pBuffer, dwSize);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// IWinIPCPeer

STDMETHODIMP CWinIPCPeer::get_isSender(VARIANT_BOOL* pVal)
{
    _ATL_VALIDATE_OUT_POINTER(pVal);
    *pVal = m_bAsSender ? VARIANT_TRUE : VARIANT_FALSE;
    return S_OK;
}

STDMETHODIMP CWinIPCPeer::get_isReceiver(VARIANT_BOOL* pVal)
{
    _ATL_VALIDATE_OUT_POINTER(pVal);
    *pVal = m_bAsReceiver ? VARIANT_TRUE : VARIANT_FALSE;
    return S_OK;
}

STDMETHODIMP CWinIPCPeer::get_productID(LONG* pVal)
{
    _ATL_VALIDATE_OUT_POINTER(pVal);
    *pVal = GetProductID();
    return S_OK;
}

STDMETHODIMP CWinIPCPeer::AddEventHandler(IUnknown* pHandler)
{
    CComQIPtr<IWinIPCEvent> spHandler = pHandler;
    if(!spHandler)
    {
        return E_INVALIDARG;
    }

    CAutoLock lock(&m_lockHandler);
    m_setEventHandler.insert(spHandler.Detach());
    return S_OK;
}

STDMETHODIMP CWinIPCPeer::RemoveEventHandler(IUnknown* pHandler)
{
    CComQIPtr<IWinIPCEvent> spHandler = pHandler;
    if(!spHandler)
    {
        return E_INVALIDARG;
    }

    CAutoLock lock(&m_lockHandler);
    m_setEventHandler.erase(spHandler);
    return S_OK;
}

STDMETHODIMP CWinIPCPeer::AddSenderProductID(LONG lProductID)
{
    AddAcceptSenderProductID(lProductID);
    return S_OK;
}

STDMETHODIMP CWinIPCPeer::AddReceiverProductID(LONG lProductID)
{
    AddAcceptReceiverProductID(lProductID);
    return S_OK;
}

STDMETHODIMP CWinIPCPeer::RemoveSenderProductID(LONG lProductID)
{
    RemoveAcceptSenderProductID(lProductID);
    return S_OK;
}

STDMETHODIMP CWinIPCPeer::RemoveReceiverProductID(LONG lProductID)
{
    RemoveAcceptReceiverProductID(lProductID);
    return S_OK;
}

STDMETHODIMP CWinIPCPeer::SendData(BYTE* pBuffer, ULONG ulSize, VARIANT_BOOL vbSync)
{
	BOOL bRet = Send(pBuffer, ulSize, (vbSync ==VARIANT_TRUE ? TRUE : FALSE));
    return (bRet ? S_OK : E_FAIL);
}

STDMETHODIMP CWinIPCPeer::SendDataToSpecialProduct(LONG lProductID, BYTE* pBuffer, ULONG ulSize, VARIANT_BOOL vbSync)
{
	BOOL bRet = Send(lProductID, pBuffer, ulSize, (vbSync ==VARIANT_TRUE ? TRUE : FALSE));
	return (bRet ? S_OK : E_FAIL);
}

STDMETHODIMP CWinIPCPeer::SendMsgToSpecialProduct(LONG lProductID, LONG lParam1, LONG lParam2)
{
	BOOL bRet = Send(lProductID, lParam1, lParam2);
	return (bRet ? S_OK : E_FAIL);
}

STDMETHODIMP CWinIPCPeer::ClosePeer()
{
    Close();
    return S_OK;
}

STDMETHODIMP CWinIPCPeer::IsSenderConnected(LONG lProductID, VARIANT_BOOL* pVal)
{
	_ATL_VALIDATE_OUT_POINTER(pVal);
	BOOL bRet = m_Receiver.IsSenderConnected(lProductID);	
	*pVal = bRet ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CWinIPCPeer::IsReceiverConnected(LONG lProductID, VARIANT_BOOL* pVal)
{
	_ATL_VALIDATE_OUT_POINTER(pVal);
	BOOL bRet = m_Sender.IsReceiverConnected(lProductID);	
	*pVal = bRet ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}

VOID CWinIPCPeer::OnSenderConnect( INT nTargetID )
{
	{
		CAutoLock lock(&m_lockEventData);
		EventData newData;
		newData.nType = SENDER_CONNECT;
		newData.p1 = nTargetID;
		m_vecEventData.push_back(newData);
	}
	ReleaseSemaphore(m_hSemEvent, 1, NULL);
}

VOID CWinIPCPeer::OnReceiverConnect( INT nTargetID )
{
	{
		CAutoLock lock(&m_lockEventData);
		EventData newData;
		newData.nType = RECEIVER_CONNECT;
		newData.p1 = nTargetID;
		m_vecEventData.push_back(newData);
	}
	ReleaseSemaphore(m_hSemEvent, 1, NULL);
}

VOID CWinIPCPeer::OnReceiveMsg( INT nSenderID, int p1, int p2 )
{
	{
		CAutoLock lock(&m_lockEventData);
		EventData newData;
		newData.nType = RECEIVE_MSG;
		newData.p1 = nSenderID;
		newData.p2 = p1;
		newData.p3 = p2;
		m_vecEventData.push_back(newData);
	}
	ReleaseSemaphore(m_hSemEvent, 1, NULL);
}

VOID CWinIPCPeer::OnReceiverDisconnect( INT nTargetID )
{
	{
		CAutoLock lock(&m_lockEventData);
		EventData newData;
		newData.nType = RECEIVER_DISCONNECT;
		newData.p1 = nTargetID;
		m_vecEventData.push_back(newData);
	}
	
	ReleaseSemaphore(m_hSemEvent, 1, NULL);
}

BOOL CWinIPCPeer::Create(BOOL bAsSender, 
						 BOOL bAsReceiver, 
						 INT nProductID, 
						 BOOL bSenderCompact /*= FALSE*/, 
						 BOOL bExitIfSameProductExist /*= FALSE*/)
{
	m_hSemEvent = CreateSemaphore(NULL, 0, 99999, NULL);
	m_hEventThread = CreateThread(NULL, 0, EventHandleThread, this, 0, NULL);

	BOOL bRet = CIPCPeer::Create(bAsSender, bAsReceiver, nProductID, bSenderCompact, bExitIfSameProductExist);

	if(!bRet)
	{
		Clean();
	}
	return bRet;
}

DWORD WINAPI CWinIPCPeer::EventHandleThread( LPVOID lpParam )
{
	CWinIPCPeer* pThis = (CWinIPCPeer*)lpParam;
	if(pThis)
	{
		pThis->EventMainLoop();
	}
	return 0;
}

VOID CWinIPCPeer::EventMainLoop()
{
	if(!m_hSemEvent)
	{
		return;
	}

	DWORD dwRet;
	IWinIPCEvent* pEventHandler = NULL;
	WinIPCEventDataVec::iterator it;
	WinIPCEventHandlerSet::iterator itHandler;

	while(!m_bExit)
	{
		dwRet = WaitForSingleObject(m_hSemEvent, INFINITE);
		if(dwRet != WAIT_OBJECT_0)
		{
			break;
		}

		if(m_setEventHandler.empty() || m_vecEventData.empty())
		{
			continue;
		}

		WinIPCEventDataVec vecEventData;
		{
			CAutoLock lock(&m_lockEventData);
			vecEventData.resize(m_vecEventData.size());
			std::copy(m_vecEventData.begin(), m_vecEventData.end(), vecEventData.begin());
			m_vecEventData.clear();
		}

		WinIPCEventHandlerSet setEventHandler;
		{
			CAutoLock lock(&m_lockHandler);
			for(itHandler = m_setEventHandler.begin(); itHandler != m_setEventHandler.end(); itHandler++)
			{
				pEventHandler = (*itHandler);
				if(pEventHandler)
				{
					pEventHandler->AddRef();
					setEventHandler.insert(pEventHandler);
				}
			}
		}
		
		for(it = vecEventData.begin(); it != vecEventData.end(); it++)
		{
			EventData& data = (*it);
			for(itHandler = setEventHandler.begin(); itHandler != setEventHandler.end(); itHandler++)
			{
				pEventHandler = (*itHandler);

				if(pEventHandler)
				{
					switch(data.nType)
					{
					case SENDER_CONNECT:
						pEventHandler->OnConnect(VARIANT_TRUE, data.p1);
						break;

					case RECEIVER_CONNECT:
						pEventHandler->OnConnect(VARIANT_FALSE, data.p1);
						break;

					case RECEIVER_DISCONNECT:
						pEventHandler->OnReceiverDisconnect(data.p1);
						break;

					case SEND_FINISH:
						pEventHandler->OnSendFinish(data.p1, data.p2, data.p3);
						break;

					case RECEIVE_DATA:
						//pEventHandler->OnReceiveData(data.p1, data.p2, data.p3);
						ATLASSERT(FALSE);
						break;

					case RECEIVE_MSG:
						pEventHandler->OnReceiveMsg(data.p1, data.p2, data.p3);
						break;

					default:
						break;
					} // end switch
				} // end if
			} // end for handler			
		} // end for event	

		for(itHandler = setEventHandler.begin(); itHandler != setEventHandler.end(); itHandler++)
		{
			pEventHandler = (*itHandler);
			if(pEventHandler)
			{
				pEventHandler->Release();
			}
		}
	}
}

VOID CWinIPCPeer::Close()
{
	LOG_METHOD();
	Clean();
	CIPCPeer::Close();	
}

