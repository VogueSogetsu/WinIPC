#include "stdafx.h"
#include "IPCPeer.h"

/*
 *=============================================================================
 * CIPCPeer 
 *=============================================================================
 */

CIPCPeer::CIPCPeer()
: m_Sender(this)
, m_bAsSender(FALSE)
, m_bAsReceiver(FALSE)
, m_nProductID(-1)
{
}

CIPCPeer::~CIPCPeer()
{
}

BOOL CIPCPeer::Create(BOOL bAsSender, 
                      BOOL bAsReceiver, 
                      INT nProductID, 
                      BOOL bSenderCompact /*= FALSE*/, 
                      BOOL bExitIfSameProductExist /*= FALSE*/)
{
    LOG_METHOD();

    BOOL bRet = FALSE;
    if(bAsSender)
    {
		LOG_INFO(_T("Create as sender id=") << nProductID);
        bRet = m_Sender.Create(nProductID, 0, bSenderCompact, bExitIfSameProductExist);
        if(!bRet)
        {
            LOG_ERROR(_T("Create sender failed!!!"));
            return FALSE;
        }

        m_bAsSender = TRUE;
    }

    if(bAsReceiver)
    {
		LOG_INFO(_T("Create as receiver id=") << nProductID);
        DWORD dwThreadID = 0;
        if(bAsSender)
        {
            dwThreadID = m_Sender.GetThreadID();
        }

        bRet = m_Receiver.Create(nProductID, 0, bExitIfSameProductExist);
        if(!bRet)
        {
            LOG_ERROR(_T("Create receiver failed!!!"));
            return FALSE;
        }

        m_bAsReceiver = TRUE;
    }

    m_nProductID = nProductID;
    return TRUE;
}

VOID CIPCPeer::Close()
{
    LOG_METHOD();
    m_Receiver.Close();
    m_Sender.Close();    
}

BOOL CIPCPeer::OnDefaultMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
    if(m_bAsSender && m_bAsReceiver)
    {
        return m_Receiver.HandleMessage(msg, wParam, lParam);
    }

    return FALSE;
}

VOID CIPCPeer::AddAcceptSenderProductID(INT nProductID)
{
    m_Receiver.AddAcceptProductID(nProductID);
}

VOID CIPCPeer::RemoveAcceptSenderProductID(INT nProductID)
{
    m_Receiver.RemoveAcceptProductID(nProductID);
}

VOID CIPCPeer::AddAcceptReceiverProductID(INT nProductID)
{
    m_Sender.AddAcceptProductID(nProductID);
}

VOID CIPCPeer::RemoveAcceptReceiverProductID(INT nProductID)
{
    m_Sender.RemoveAcceptProductID(nProductID);
}

BOOL CIPCPeer::Send(LPBYTE pBuffer, DWORD dwSize, BOOL bSync)
{
    if(!m_bAsSender) 
    {
        return FALSE;
    }

    return m_Sender.Send(pBuffer, dwSize, bSync);
}

BOOL CIPCPeer::Send(INT nProductID, LPBYTE pBuffer, DWORD dwSize, BOOL bSync)
{
	if(!m_bAsSender) 
	{
		return FALSE;
	}

	return m_Sender.Send(nProductID, pBuffer, dwSize, bSync);
}

BOOL CIPCPeer::Send(INT nProductID, INT nParam1, INT nParam2)
{
	if(!m_bAsSender) 
	{
		return FALSE;
	}

	return m_Sender.Send(nProductID, (WORD)nParam1, (WORD)nParam2);
}

BOOL CIPCPeer::Send(LPCTSTR szBuffer, BOOL bSync)
{
    if(!m_bAsSender) 
    {
        return FALSE;
    }

    return m_Sender.Send(szBuffer, bSync);
}

DWORD CIPCPeer::GetThreadID()
{
    return m_Sender.GetThreadID();
}