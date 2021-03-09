#pragma once

#include "IPCSenderImpl.h"
#include "IPCReceiverImpl.h"

class CIPCPeer : public CBaseDefaultMsgListener
{
public:
    CIPCPeer();
    virtual ~CIPCPeer();

public:
    virtual BOOL        Create(BOOL bAsSender, BOOL bAsReceiver, 
                               INT nProductID, BOOL bSenderCompact = FALSE,
                               BOOL bExitIfSameProductExist = FALSE);
    virtual VOID        Close();
    virtual BOOL        OnDefaultMsg(UINT msg, WPARAM wParam, LPARAM lParam);

public:
    INT 				GetProductID() const { return m_nProductID; }

    VOID				AddAcceptSenderProductID(INT nProductID);
    VOID				RemoveAcceptSenderProductID(INT nProductID);

    VOID				AddAcceptReceiverProductID(INT nProductID);
    VOID				RemoveAcceptReceiverProductID(INT nProductID);

public:
    BOOL				Send(LPBYTE pBuffer, DWORD dwSize, BOOL bSync = FALSE);
    BOOL				Send(LPCTSTR szBuffer, BOOL bSync = FALSE);
	BOOL				Send(INT nProductID, LPBYTE pBuffer, DWORD dwSize, BOOL bSync = FALSE);
	BOOL				Send(INT nProductID, INT nParam1, INT nParam2);

public:
    DWORD               GetThreadID();

protected:
    INT                 m_nProductID;
    BOOL                m_bAsSender;
    BOOL                m_bAsReceiver;
    CIPCSenderImpl      m_Sender;
    CIPCReceiverImpl    m_Receiver;

    LOG_CLS_DEC();
};
