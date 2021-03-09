#pragma once

#include <set>
#include <hash_map>
#include "IPCLock.h"
#include "IPCStructs.h"
#include "IPCMemory.h"
#include "ThreadMsgComm.h"

class CIPCSenderListener;

class CIPCSenderImpl : public CBaseMsgCommThread
{
public:
	CIPCSenderImpl(CBaseDefaultMsgListener* pListener = NULL);
	virtual				~CIPCSenderImpl();

	typedef struct stTargetRNode
	{
		IPCReceiverNode		node;
		INT					status;
		CIPCMemory			memory;
	} TargetRNode, *LPTARGETRNODE;

	typedef std::set<INT> AcceptProductIDSet;
	typedef stdext::hash_map<DWORD, TargetRNode*> TargetReceiverMap;

	DECLARE_THREAD_MSG_HANDLER

public:
	THREAD_MSG_HANDLER( OnSynAck );
	THREAD_MSG_HANDLER( OnReceiverCreated );
    THREAD_MSG_HANDLER( OnReceiverCreated2 );
	THREAD_MSG_HANDLER( OnReceiverExit );
	THREAD_MSG_HANDLER( OnDataResp );
	THREAD_MSG_HANDLER( OnDataInvalid );

public:
	BOOL				Create(INT nProductID, DWORD dwBindThreadID = 0, BOOL bCompactMode = FALSE, BOOL bExitIfSameProductExist = FALSE);
	BOOL				IsRunning() const { return m_bRunning; }
	VOID				Close();
    virtual BOOL        OnIdle(LONG lCount);
	BOOL				IsReceiverConnected(INT nProductID);
	BOOL				OnFileMappingChanged(DWORD dwThreadID);

public:
	BOOL				GetProductID() const { return m_nProductID; }
	VOID				AddAcceptProductID(INT nProductID);
	VOID				RemoveAcceptProductID(INT nProductID);

    VOID                SetSenderListener(CIPCSenderListener* p) { m_pSenderListener = p; }

public:
	BOOL				Send(LPBYTE pBuffer, DWORD dwSize, BOOL bSync);
	BOOL				Send(INT nProductID, LPBYTE pBuffer, DWORD dwSize, BOOL bSync);
	BOOL				Send(LPCTSTR szBuffer, BOOL bSync);
	BOOL				Send(INT nProductID, WORD p1, WORD p2);

protected:
	BOOL				AddSenderToList(INT nProductID, BOOL bExitIfSameProductExist);
    VOID                RemoveSenderFromList();
	VOID				ConnectReceivers(int nTargetProductID = -1);
	VOID				ConnectOneReceiver(LPIPCRECEIVERNODE pReceiverNode);
	VOID				SafeReleaseReceiverNode(LPTARGETRNODE pNode);
	HANDLE				DuplicateMemoryToReceiver(DWORD dwThreadID, DWORD dwProcessID, HANDLE hFileMapping);

protected:
	INT					m_nProductID;	
	AcceptProductIDSet	m_setAcceptProductID;
	BOOL				m_bRunning;
	BOOL				m_bCompactMode;	
	TargetReceiverMap	m_mapTargetReceivers;
    HANDLE              m_hFileMappingSender;

	CCritSec			m_lockReceivers;

    CIPCSenderListener* m_pSenderListener;

	CIPCMemory			m_Memory;
	BOOL				m_bMemoryForEachReceiver;

	LOG_CLS_DEC();
};

class CIPCSenderListener
{
public:
    CIPCSenderListener() {}
    virtual ~CIPCSenderListener() {}

public:
	virtual VOID OnReceiverConnect(INT nTargetID) {}
	virtual VOID OnReceiverDisconnect(INT nTargetID) {}
    virtual VOID OnSendFinish(DWORD dwOffset, DWORD dwTargetID, INT nRet) {}
};
