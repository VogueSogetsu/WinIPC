#pragma once

#include <set>
#include <hash_map>
#include "IPCLock.h"
#include "IPCStructs.h"
#include "ThreadMsgComm.h"

class CIPCReceiverListener;

class CIPCReceiverImpl : public CBaseMsgCommThread
{
public:
	CIPCReceiverImpl(CBaseDefaultMsgListener* pListener = NULL);
	virtual				~CIPCReceiverImpl();

	DECLARE_THREAD_MSG_HANDLER

public:
	THREAD_MSG_HANDLER( OnSyn );
	THREAD_MSG_HANDLER( OnAck );
    THREAD_MSG_HANDLER( OnDataReq );
	THREAD_MSG_HANDLER( OnMsgReq );
	THREAD_MSG_HANDLER( OnFileMappingChanged );

public:
	BOOL				Create(INT nProductID, DWORD dwBindThreadID = 0, BOOL bExitIfSameProductExist = FALSE);
    VOID                Close();

public:
	BOOL				GetProductID() const { return m_nProductID; }
	VOID				AddAcceptProductID(INT nProductID);
	VOID				RemoveAcceptProductID(INT nProductID);
	BOOL				IsSenderConnected(INT nProductID);

    VOID                SetReceiverListener(CIPCReceiverListener* p) { m_pReceiverListener = p; }

protected:	
	BOOL				AddReceiverToList(INT nProductID, BOOL bExitIfSameProductExist);
    VOID                RemoveReceiverFromList();
	VOID				NotifyAllSender(int nTargetProductID = -1);
	VOID				NotifyOneSender(LPIPCSENDERNODE pSenderNode);

protected:
	typedef struct stTargetSNode
	{
		IPCSenderNode		node;
		INT					status;	
		HANDLE				hFileMapping;
	} TargetSNode, *LPTARGETSNODE;

	typedef std::set<INT> AcceptProductIDSet;
	typedef stdext::hash_map<DWORD, LPTARGETSNODE> TargetSenderMap;

	BOOL				m_bRunning;
	INT					m_nProductID;
	TargetSenderMap		m_mapSenders;
	AcceptProductIDSet	m_setAcceptProductID;
    HANDLE              m_hFileMappingReceiver;

    CCritSec            m_lockSenders;

    CIPCReceiverListener* m_pReceiverListener;

	LOG_CLS_DEC();
};


class CIPCReceiverListener
{
public:
    CIPCReceiverListener() {}
    virtual ~CIPCReceiverListener() {}

public:
	virtual VOID OnSenderConnect(INT nTargetID) {}
    virtual VOID OnReceiveData(INT nSenderID, LPBYTE pBuffer, DWORD dwSize) {}
	virtual VOID OnReceiveMsg(INT nSenderID, int p1, int p2) {}
};
