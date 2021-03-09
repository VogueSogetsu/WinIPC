#include "StdAfx.h"
#include <vector>
#include "IPCSenderImpl.h"

/*
 *=============================================================================
 *  CIPCSenderImpl
 *=============================================================================
 */

#define SEND_TIMEOUT INFINITE
typedef std::vector<DWORD> OffsetVec;

CIPCSenderImpl::CIPCSenderImpl(CBaseDefaultMsgListener* pListener /* = NULL */)
: CBaseMsgCommThread(pListener)
, m_bRunning(FALSE)
, m_bCompactMode(TRUE)
, m_hFileMappingSender(NULL)
, m_pSenderListener(NULL)
, m_bMemoryForEachReceiver(FALSE)
{
}

CIPCSenderImpl::~CIPCSenderImpl()
{
}

BOOL CIPCSenderImpl::Create(INT nProductID, 
                            DWORD dwBindThreadID,
							BOOL bCompactMode /* = FALSE */, 
							BOOL bExitIfSameProductExist /* = FALSE */)
{
	LOG_METHOD();

	if(m_bRunning)
	{
		LOG_WARN(_T("Sender already on running!"));
		return TRUE;
	}

	HANDLE hMutexSender = ::CreateMutex(NULL, TRUE, _T("{4F67B3FD-5DAF-45ca-A42B-FC4597D1757C}-SenderMutex"));
	if(NULL == hMutexSender)
	{
		LOG_ERROR(_T("CreateMutex(Sender) failed!"));
		return FALSE;
	}

	BOOL bRet = FALSE;	

	// 读取发送线程列表
	do
	{
        if(dwBindThreadID > 0)
        {
            BindMsgCommThread(dwBindThreadID);
        }
        else
        {
		    if(!CreateMsgCommThread())
		    {
			    LOG_ERROR(_T("CreateMsgCommThread failed!"));	
			    break;
		    }
        }

		if(!AddSenderToList(nProductID, bExitIfSameProductExist))
		{		
			LOG_ERROR(_T("AddSenderToList failed!"));
			break;
		}
		
		m_nProductID		= nProductID;
		m_bRunning			= TRUE;
		m_bCompactMode		= bCompactMode;
		bRet = TRUE;
	} while(false);
	
	::ReleaseMutex(hMutexSender);

	ConnectReceivers();
	return bRet;
}

VOID CIPCSenderImpl::Close()
{
    LOG_METHOD();

	if(!m_bRunning || NULL == m_hFileMappingSender)
	{
		return;
	}

    // 从发送者列表中删除
    HANDLE hMutexSender = ::CreateMutex(NULL, TRUE, _T("{4F67B3FD-5DAF-45ca-A42B-FC4597D1757C}-SenderMutex"));
    if(NULL == hMutexSender)
    {
        LOG_ERROR(_T("CreateMutex(Sender) failed!"));
    }
    else
    {
        RemoveSenderFromList();
        ::ReleaseMutex(hMutexSender);
		CloseHandle(hMutexSender);
    }

    CloseHandle(m_hFileMappingSender);
    m_hFileMappingSender = NULL;

	// 关闭所有连接
	// 不需要通知接收端
	{
		CAutoLock lock(&m_lockReceivers);
		TargetReceiverMap::iterator it;
		for(it = m_mapTargetReceivers.begin(); it != m_mapTargetReceivers.end(); it++)
		{
			if(it->second)
			{
				LPTARGETRNODE pTarget = it->second;
				SafeReleaseReceiverNode(pTarget);
			}
		}

		m_mapTargetReceivers.clear();
	}	

	m_setAcceptProductID.clear();
	m_bRunning = FALSE;
}

BOOL CIPCSenderImpl::OnIdle(LONG lCount)
{
    if(100 == lCount)
    {
		CAutoLock lock(&m_lockReceivers);
		TargetReceiverMap::iterator it;
		for(it = m_mapTargetReceivers.begin(); it != m_mapTargetReceivers.end(); it++)
		{
			if(it->second)
			{
				LPTARGETRNODE pTarget = it->second;
				pTarget->memory.MergeIdleBlock();
			}
		}
        return FALSE;
    }

    return TRUE;
}

BOOL CIPCSenderImpl::AddSenderToList(INT nProductID, BOOL bExitIfSameProductExist)
{
	LOG_METHOD();

	BOOL bRet = FALSE;
	LPBYTE pMemory = NULL;

	do
	{
		m_hFileMappingSender = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, 
			PAGE_READWRITE | SEC_COMMIT, 0, 4+MAX_SENDER_COUNT*sizeof(IPCSenderNode), 
			_T("{4F67B3FD-5DAF-45ca-A42B-FC4597D1757C}-Senderlist"));

		if(!m_hFileMappingSender)
		{
			LOG_ERROR(_T("CreateFileMapping(Sender) failed!"));
			break;
		}

		BOOL bFirstSender = FALSE;
		if(GetLastError() == ERROR_SUCCESS)
		{
			// 第一个发送线程
			LOG_INFO(_T("First sender, threadid=") << m_dwThreadID);
			bFirstSender = TRUE;
		}

		pMemory = (LPBYTE)MapViewOfFile(m_hFileMappingSender, FILE_MAP_WRITE, 0, 0, 0);
		if(pMemory == NULL)
		{		
			LOG_ERROR(_T("MapViewOfFile(Sender) failed!"));
			break;
		}

		IPCSenderNode newSenderNode;
		newSenderNode.dwThreadId = m_dwThreadID;
		newSenderNode.nProductId = nProductID;

		if(bFirstSender)
		{
			*((LPINT)pMemory) = 1;
			CopyMemory(pMemory+sizeof(INT), &newSenderNode, sizeof(IPCSenderNode));			
			bRet = TRUE;
			break;
		}
		else
		{
			INT nSenderCount = *((LPINT)pMemory);
			if(nSenderCount < 0 || nSenderCount >= MAX_SENDER_COUNT)
			{	
				LOG_ERROR(_T("Sender count(") << nProductID << _T(") not invalid"));
				return FALSE;
			}

			INT i = 0;
			LPIPCSENDERNODE pNode = NULL;

			if(bExitIfSameProductExist)
			{
				// 检查是否有同一产品的发送线程
				// 有则退出	
				pNode = (LPIPCSENDERNODE)(pMemory+sizeof(INT));
				for(i = 0; i < nSenderCount; i++)
				{
					if(pNode->nProductId == nProductID)
					{
						LOG_ERROR(_T("Found same product id(") << nProductID << _T(")"));
						break;
					}
					pNode++;
				}	

				if(i >= nSenderCount)
				{
					break;
				}
			}

			{
				// 删除无效线程
				pNode = (LPIPCSENDERNODE)(pMemory+sizeof(INT));
				for(i = 0; i < nSenderCount; )
				{
					if( !IsThreadValid(pNode->dwThreadId) )
					{
						LOG_WARN(_T("Found invalid thread=") << pNode->dwThreadId << _T(" productid=") << nProductID);
						CopyMemory(pNode, pNode+1, (MAX_SENDER_COUNT-i-1)*sizeof(IPCSenderNode));
						nSenderCount--;
						*((LPINT)pMemory) = nSenderCount;
						continue;
					}
					i++;
					pNode++;
				}
			}

			CopyMemory(pMemory+sizeof(INT)+nSenderCount*sizeof(IPCSenderNode), &newSenderNode, sizeof(IPCSenderNode));	
            *((LPINT)pMemory) = nSenderCount+1;
			bRet = TRUE;
		}
	} while(false);
	
	if(m_hFileMappingSender)
	{
		if(pMemory)
		{
			UnmapViewOfFile(pMemory);
		}		
	}

	return bRet;
}

VOID CIPCSenderImpl::RemoveSenderFromList()
{
    LOG_METHOD();

    LPBYTE pMemory = (LPBYTE)MapViewOfFile(m_hFileMappingSender, FILE_MAP_WRITE, 0, 0, 0);
    if(pMemory == NULL)
    {		
        LOG_ERROR(_T("MapViewOfFile(Sender) failed!"));
        return;
    }
    
    INT nSenderCount = *((LPINT)pMemory);
        
    INT i = 0;
    LPIPCSENDERNODE pNode = (LPIPCSENDERNODE)(pMemory+sizeof(INT));
    for(i = 0; i < nSenderCount; i++)
    {
        if(pNode->dwThreadId == m_dwThreadID &&
           pNode->nProductId == m_nProductID)
        {
            LOG_INFO(_T("Found, ready to delete."));
            CopyMemory(pNode, pNode+1, (MAX_SENDER_COUNT-i-1)*sizeof(IPCSenderNode));
            *((LPINT)pMemory) = nSenderCount-1;
            break;
        }

        pNode++;
    }
}

VOID CIPCSenderImpl::ConnectReceivers(int nTargetProductID)
{
	LOG_METHOD();

	if(m_setAcceptProductID.empty())
	{
		LOG_WARN(_T("accept product id set is empty!"));
		return;
	}

	HANDLE hMutexReceiver = ::CreateMutex(NULL, TRUE, _T("{4F67B3FD-5DAF-45ca-A42B-FC4597D1757C}-ReceiverMutex"));
	if(NULL == hMutexReceiver)
	{
		LOG_ERROR(_T("CreateMutex(Receiver) failed!"));
		return;
	}

	BOOL bRet = FALSE;
	HANDLE hFileMappingReceiver = NULL;
	LPBYTE pMemory = NULL;

	// 读取接收线程列表
	do
	{
		hFileMappingReceiver = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, 
			PAGE_READWRITE | SEC_COMMIT, 0, 4+MAX_RECEIVER_COUNT*sizeof(IPCReceiverNode), 
			_T("{4F67B3FD-5DAF-45ca-A42B-FC4597D1757C}-Receiverlist"));		

		if(!hFileMappingReceiver)
		{
			break;
		}

		pMemory = (LPBYTE)MapViewOfFile(hFileMappingReceiver, FILE_MAP_WRITE, 0, 0, 0);

		INT nReceiverCount = *((LPINT)pMemory);
		if(nReceiverCount < 0)
		{	
			LOG_ERROR(_T("Receiver count(") << nReceiverCount << _T(") not invalid"));
			return;
		}

		// 开始连接Receiver
		{
			CAutoLock lock(&m_lockReceivers);
			LPIPCRECEIVERNODE pNode = (LPIPCRECEIVERNODE)(pMemory+sizeof(INT));
			for(INT i = 0; i < nReceiverCount; i++)
			{
				if(-1 == nTargetProductID)				
				{
					// 连接所有Receiver
					if( m_setAcceptProductID.find(pNode->nProductId) != m_setAcceptProductID.end() &&
						pNode->dwThreadId != m_dwThreadID )
					{
						LOG_INFO(_T("Try to connect receiver [") <<
							pNode->dwThreadId << _T(", ") << pNode->nProductId << _T("]"));
						ConnectOneReceiver(pNode);
					}
					else
					{
						LOG_INFO(_T("reject receiver [") <<
							pNode->dwThreadId << _T(", ") << pNode->nProductId << _T("]"));
					}
				}
				else
				{
					if( pNode->nProductId == nTargetProductID && pNode->dwThreadId != m_dwThreadID )
					{
						LOG_INFO(_T("Try to connect receiver [") <<
							pNode->dwThreadId << _T(", ") << pNode->nProductId << _T("]"));
						ConnectOneReceiver(pNode);
						break;
					}
				}

				pNode++;
			}
		}						
	} while(false);
	
	if(hFileMappingReceiver)
	{
		if(pMemory)
		{
			::UnmapViewOfFile(pMemory);
		}
		CloseHandle(hFileMappingReceiver);
	}
	::ReleaseMutex(hMutexReceiver);
	CloseHandle(hMutexReceiver);
}

VOID CIPCSenderImpl::ConnectOneReceiver(LPIPCRECEIVERNODE pReceiverNode)
{
	LOG_METHOD();

	if(m_mapTargetReceivers.find(pReceiverNode->dwThreadId) != m_mapTargetReceivers.end())
	{
		LOG_WARN(_T("receiver(") << pReceiverNode->dwThreadId << _T(") already in map!"));
		return;
	}

	TargetRNode* pNewNode = new TargetRNode;
	if(!pNewNode)
	{
		return;
	}

	pNewNode->node.dwThreadId	= pReceiverNode->dwThreadId;
	pNewNode->node.nProductId	= pReceiverNode->nProductId;
	pNewNode->node.dwProcessId  = pReceiverNode->dwProcessId;
	pNewNode->status			= SYN_SEND;
	m_mapTargetReceivers.insert(TargetReceiverMap::value_type(pReceiverNode->dwThreadId, pNewNode));

	PostTargetMessage(pReceiverNode->dwThreadId, WMTMC_SYN, (WPARAM)m_dwThreadID, m_nProductID);			
}

VOID CIPCSenderImpl::AddAcceptProductID(INT nProductID)
{
	LOG_METHOD();
	if( m_setAcceptProductID.find(nProductID) != m_setAcceptProductID.end() )
	{
		return;
	}
	m_setAcceptProductID.insert(nProductID);

    if(m_bRunning)
    {
        ConnectReceivers(nProductID);
    }	
}

VOID CIPCSenderImpl::RemoveAcceptProductID(INT nProductID)
{
	if( m_setAcceptProductID.find(nProductID) == m_setAcceptProductID.end() )
	{
		return;
	}
	m_setAcceptProductID.erase(nProductID);

	if(m_bRunning)
    {
		// 关闭相关连接
		CAutoLock lock(&m_lockReceivers);
		TargetReceiverMap::iterator it;
		for(it = m_mapTargetReceivers.begin(); it != m_mapTargetReceivers.end(); )
		{
			if(it->second)
			{
				LPTARGETRNODE pTarget = it->second;
				if( pTarget->node.nProductId == nProductID )
				{
					pTarget->memory.Close();
					delete pTarget;
					pTarget = NULL;
					m_mapTargetReceivers.erase(it++);
					continue;
				}
			}
			it++;
		}
    }	
}

BOOL CIPCSenderImpl::Send(LPBYTE pBuffer, DWORD dwSize, BOOL bSync)
{
	return Send(-1, pBuffer, dwSize, bSync);
}

BOOL CIPCSenderImpl::Send(LPCTSTR szBuffer, BOOL bSync)
{
    if(NULL == szBuffer)
    {
        return FALSE;
    }

    LOG_DEBUG(_T("Send string=") << szBuffer);
    return Send((LPBYTE)szBuffer, DWORD(sizeof(TCHAR)*(_tcslen(szBuffer)+1)), bSync);
}

BOOL CIPCSenderImpl::Send(INT nProductID, LPBYTE pBuffer, DWORD dwSize, BOOL bSync)
{
	LOG_METHOD();

	// 64K max
	if(NULL == pBuffer || 0 == dwSize /*|| dwSize >= 65535*/)
	{
		return FALSE;
	}

	if(m_bCompactMode)
	{
		return FALSE;
	}

	bSync = bSync && (GetCurrentThreadId() != m_dwThreadID);

	CAutoLock lock(&m_lockReceivers);

	if(m_mapTargetReceivers.empty())
	{
		LOG_WARN(_T("No receiver!"));
		return TRUE;
	}   

	if(!m_bMemoryForEachReceiver)
	{
		if( !m_Memory.GetFileMapping() )
		{
			LOG_ERROR(_T("Common memory not ready!!!"));
			return FALSE;
		}
	}

	DWORD dwOffset = 0;
	BOOL bSendOK = FALSE;
	TargetRNode* pNode = NULL;
	TargetReceiverMap::iterator it;

	//dwSize += sizeof(DWORD);
	OffsetVec vecOffset;
	CIPCMemory* pMemory = NULL;

	// 为每个处于就绪状态的接收者发送数据
	for(it = m_mapTargetReceivers.begin(); it != m_mapTargetReceivers.end(); it++)
	{
		pNode = it->second;
		if(!pNode)
		{
			continue;
		}
		if(nProductID == -1 || (pNode->node).nProductId == nProductID)
		{	
			if(ESTABLISHED != pNode->status)
			{
				LOG_WARN(_T("Connection (") << pNode->node.dwThreadId << _T(", ") <<
					pNode->node.nProductId << _T(") not ready"));
				continue;
			}

			if(!m_bMemoryForEachReceiver)
			{
				pMemory = &m_Memory;
			}
			else
			{
				pMemory = &(pNode->memory);
			}

			if(!pMemory->CopyAndGetMemory(pBuffer, dwSize, &dwOffset, bSync))
			{
				LOG_ERROR(_T("CopyAndGetMemory failed!"));
				return FALSE;
			}			

			LOG_DEBUG(m_dwThreadID << _T(" >>> ") << pNode->node.dwThreadId << _T(" [") << (dwSize+sizeof(DWORD)) << _T("]"));
			bSendOK = PostTargetMessage(pNode->node.dwThreadId, WMTMC_DATA_REQ, (WPARAM)m_dwThreadID, dwOffset);
			
			if(bSendOK)
			{
				if(bSync)
				{
					LOG_DEBUG(_T("Wait for dataresp msg"));
					pMemory->WaitMemoryToRelease(dwOffset, SEND_TIMEOUT);
				}
			}
			else
			{
				pNode->status = BROKEN;
			}
		}		
	}	
	LOG_INFO(_T("Total send len=") << dwSize+sizeof(DWORD) << _T(" copies data"));
	return TRUE;
}

BOOL CIPCSenderImpl::Send(INT nProductID, WORD p1, WORD p2)
{
	LOG_METHOD();

	CAutoLock lock(&m_lockReceivers);

	if(m_mapTargetReceivers.empty())
	{
		LOG_WARN(_T("No receiver!"));
		return TRUE;
	}    

	TargetRNode* pNode = NULL;
	TargetReceiverMap::iterator it;

	// 为每个处于就绪状态的接收者发送数据
	for(it = m_mapTargetReceivers.begin(); it != m_mapTargetReceivers.end(); it++)
	{
		pNode = it->second;
		if(pNode && ESTABLISHED == pNode->status && (pNode->node).nProductId == nProductID)
		{	
			if( !PostTargetMessage(pNode->node.dwThreadId, WMTMC_MSG_REQ, m_dwThreadID, MAKELPARAM(p1, p2)) )
			{
				pNode->status = BROKEN;
			}
		}		
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// 消息循环
BEGIN_THREAD_MSG_MAP(CIPCSenderImpl)
	ON_THREAD_MSG(WMTMC_SYNACK, OnSynAck)
	ON_THREAD_MSG(WMTMC_RECEIVER_CREATED, OnReceiverCreated)
    ON_THREAD_MSG(WMTMC_RECEIVER_CREATED_2, OnReceiverCreated2)
	ON_THREAD_MSG(WMTMC_RECEIVER_EXIT, OnReceiverExit)
	ON_THREAD_MSG(WMTMC_DATA_RESP, OnDataResp)
	ON_THREAD_MSG(WMTMC_DATA_INVALID, OnDataInvalid)
END_THREAD_MSG_MAP

/*
 * wParam - accept or not 
 * lParam - receiver thread id
 */
BOOL CIPCSenderImpl::OnSynAck(WPARAM wParam, LPARAM lParam)
{
    LOG_METHOD();

	DWORD dwThreadId = (DWORD)lParam;
	LOG_INFO(_T("receiver: ") << dwThreadId);
	
	CAutoLock lock(&m_lockReceivers);

	if(m_mapTargetReceivers.find(dwThreadId) == m_mapTargetReceivers.end())
	{
		return TRUE;
	}

	TargetRNode* pNode = m_mapTargetReceivers[dwThreadId];
	if(!pNode)
	{
		return TRUE;
	}

	if(wParam != 0)
	{
		SafeReleaseReceiverNode(pNode);
		m_mapTargetReceivers.erase(dwThreadId);

		LOG_WARN(_T("Connect request is rejected by receiver: ") << dwThreadId);
		return TRUE;
	}

	LOG_INFO(_T("Connect accepted by receiver: ") << dwThreadId);

	if(m_bCompactMode)
	{
		return TRUE;
	}

	// 非精简模式创建内存映射
	HANDLE hFileMapping = NULL;
	if(!m_bMemoryForEachReceiver)
	{
		if( !m_Memory.Create(this, 0) )
		{
			LOG_ERROR(_T("Create common memory failed!!!"));
			return TRUE;
		}
		hFileMapping = m_Memory.GetFileMapping();
	}
	else
	{
		if(!pNode->memory.GetFileMapping())
		{
			if(!pNode->memory.Create(this, pNode->node.dwThreadId))
			{
				LOG_ERROR(_T("Create IPC memory for receiver: ") << pNode->node.dwThreadId << _T(" failed!"));
				return TRUE;
			}		
		}
		hFileMapping = pNode->memory.GetFileMapping();
	}

	HANDLE hObjInProcessT = DuplicateMemoryToReceiver(dwThreadId, pNode->node.dwProcessId, hFileMapping);
	if(hObjInProcessT)
	{
		if( PostTargetMessage(dwThreadId, WMTMC_ACK, (WPARAM)hObjInProcessT, m_dwThreadID) )
		{
			pNode->status = ESTABLISHED;
		}
		if(m_pSenderListener)
		{
			m_pSenderListener->OnReceiverConnect(pNode->node.nProductId);
		}
	}
	else
	{
		LOG_ERROR(_T("DuplicateMemoryToReceiver failed! hr=") << GetLastError());
	}
	return TRUE;
}

/*
 * wParam - receiver thread id
 * lParam - receiver product id 
 */
BOOL CIPCSenderImpl::OnReceiverCreated(WPARAM wParam, LPARAM lParam)
{
	DWORD dwThreadID = (DWORD)wParam;
	INT nProductID = (INT)lParam;

	LOG_INFO(_T("Receiver created, threadid=") << dwThreadID << _T(", productid=") << nProductID);

	CAutoLock lock(&m_lockReceivers);

	if(m_setAcceptProductID.find(nProductID) == m_setAcceptProductID.end())
	{
		LOG_INFO(_T("Reject receiver!"));
		return TRUE;
	}

	if(m_mapTargetReceivers.find(dwThreadID) != m_mapTargetReceivers.end())
	{
		LOG_WARN(_T("Receiver already exist!"));
		return TRUE;
	}

	// connect
	TargetRNode* pNewNode = new TargetRNode;
	if(!pNewNode)
	{
		return TRUE;
	}

	pNewNode->node.dwThreadId	= dwThreadID;
	pNewNode->node.nProductId	= nProductID;
	// TODO 获取进程ID
	pNewNode->status			= SYN_SEND;
	m_mapTargetReceivers.insert(TargetReceiverMap::value_type(dwThreadID, pNewNode));	
	return TRUE;
}

BOOL CIPCSenderImpl::OnReceiverCreated2(WPARAM wParam, LPARAM lParam)
{
    DWORD dwThreadID = (DWORD)wParam;
    DWORD dwProcessID = (DWORD)lParam;

    LOG_INFO(_T("Receiver created2, threadid=") << dwThreadID << _T(", processid=") << dwProcessID);

    CAutoLock lock(&m_lockReceivers);
    if(m_mapTargetReceivers.find(dwThreadID) == m_mapTargetReceivers.end())
    {
        LOG_WARN(_T("Receiver not exist!"));
        return TRUE;
    }

    TargetRNode* pNode = m_mapTargetReceivers[dwThreadID];
    pNode->node.dwProcessId = dwProcessID;

    PostTargetMessage(dwThreadID, WMTMC_SYN, (WPARAM)m_dwThreadID, m_nProductID);
    return TRUE;
}

/*
 * wParam - receiver thread id
 * lParam -  
 */
BOOL CIPCSenderImpl::OnReceiverExit(WPARAM wParam, LPARAM lParam)
{
    LOG_INFO(_T("receiver ") << (DWORD)wParam << _T(" exit!"));

	DWORD dwThreadID = (DWORD)wParam;
	//INT nProductID = (INT)lParam;

	CAutoLock lock(&m_lockReceivers);

	if(m_mapTargetReceivers.find(dwThreadID) == m_mapTargetReceivers.end())
	{
		LOG_WARN(_T("Receiver not exist!"));
		return TRUE;
	}

	TargetRNode* pNode = m_mapTargetReceivers[dwThreadID];
	m_mapTargetReceivers.erase(dwThreadID);

	INT nProductID = pNode->node.nProductId;
	SafeReleaseReceiverNode(pNode);

	if(m_pSenderListener)
	{
		m_pSenderListener->OnReceiverDisconnect(nProductID);
	}
	return TRUE;
}

/*
 * wParam - offset
 * lParam - targetid
 */
BOOL CIPCSenderImpl::OnDataResp(WPARAM wParam, LPARAM lParam)
{	
	LOG_METHOD();

	DWORD dwTargetID = (DWORD)wParam; 
    DWORD dwOffset = (DWORD)lParam;

	LOG_INFO(_T("Send data resp: target=") << dwTargetID << _T(" offset=") << dwOffset);

	//CAutoLock lock(&m_lockReceivers);

	if(m_mapTargetReceivers.find(dwTargetID) == m_mapTargetReceivers.end())
	{
		LOG_WARN(_T("Receiver not exist!"));
		return TRUE;
	}

	if(m_bMemoryForEachReceiver)
	{
		TargetRNode* pNode = m_mapTargetReceivers[dwTargetID];
		pNode->memory.ReleaseMemory(dwOffset);
	}
	else
	{
		m_Memory.ReleaseMemory(dwOffset);
	}

	if(m_pSenderListener)
	{
		m_pSenderListener->OnSendFinish(dwOffset, dwTargetID, 0);
	}  
	return TRUE;
}

BOOL CIPCSenderImpl::OnDataInvalid(WPARAM wParam, LPARAM lParam)
{	
	LOG_METHOD();

	DWORD dwTargetID = (DWORD)wParam; 
	DWORD dwOffset = (DWORD)lParam;

	LOG_INFO(_T("Send data invalid resp: target=") << dwTargetID << _T(" offset=") << dwOffset);

	//CAutoLock lock(&m_lockReceivers);

	if(m_mapTargetReceivers.find(dwTargetID) == m_mapTargetReceivers.end())
	{
		LOG_WARN(_T("Receiver not exist!"));
		return TRUE;
	}

	if(m_bMemoryForEachReceiver)
	{
		TargetRNode* pNode = m_mapTargetReceivers[dwTargetID];
		pNode->memory.ReleaseMemory(dwOffset);
	}
	else
	{
		m_Memory.ReleaseMemory(dwOffset);
	}

	if(m_pSenderListener)
	{
		m_pSenderListener->OnSendFinish(dwOffset, dwTargetID, -1);
	}  
	return TRUE;
}

BOOL CIPCSenderImpl::IsReceiverConnected(INT nProductID)
{
	if( m_setAcceptProductID.find(nProductID) == m_setAcceptProductID.end() )
	{
		return FALSE;
	}

	CAutoLock lock(&m_lockReceivers);

	TargetRNode* pNode = NULL;
	TargetReceiverMap::iterator it;
	for(it = m_mapTargetReceivers.begin(); it != m_mapTargetReceivers.end(); )
	{
		pNode = it->second;
		if(pNode && ESTABLISHED == pNode->status && (pNode->node).nProductId == nProductID)
		{	
			if( IsThreadValid(it->first) )
			{
				return TRUE;
			}
			else
			{
				m_mapTargetReceivers.erase(it++);
				SafeReleaseReceiverNode(pNode);
			}
		}	
		else
		{
			it++;
		}
	}
	return FALSE;
}

VOID CIPCSenderImpl::SafeReleaseReceiverNode( LPTARGETRNODE pNode )
{
	if(pNode)
	{
		pNode->memory.Close();
		delete pNode;
		pNode = NULL;
	}
}

BOOL CIPCSenderImpl::OnFileMappingChanged( DWORD dwThreadId )
{
	LOG_METHOD();

	TargetRNode* pNode = NULL;
	HANDLE hObjInProcessT = NULL;

	if(!m_bMemoryForEachReceiver)
	{
		TargetReceiverMap::iterator it;
		for(it = m_mapTargetReceivers.begin(); it != m_mapTargetReceivers.end(); it++)
		{
			pNode = it->second;
			if(!pNode || pNode->status != ESTABLISHED)
			{
				continue;
			}

			hObjInProcessT = DuplicateMemoryToReceiver(
				pNode->node.dwThreadId, 
				pNode->node.dwProcessId,
				m_Memory.GetFileMapping());

			if(hObjInProcessT)
			{
				PostTargetMessage(pNode->node.dwThreadId, 
					WMTMC_FILEMAPPING_CHANGED, (WPARAM)hObjInProcessT, m_dwThreadID);
			}
			else
			{
				LOG_ERROR(_T("DuplicateMemoryToReceiver [") << pNode->node.dwThreadId << _T("] failed!!!"));
			}
		}
	}
	else
	{
		if(m_mapTargetReceivers.find(dwThreadId) == m_mapTargetReceivers.end())
		{
			return FALSE;
		}

		pNode = m_mapTargetReceivers[dwThreadId];
		if(!pNode || pNode->status != ESTABLISHED)
		{
			return FALSE;
		}

		hObjInProcessT = DuplicateMemoryToReceiver(dwThreadId, pNode->node.dwProcessId,
			pNode->memory.GetFileMapping());

		if(hObjInProcessT)
		{
			PostTargetMessage(dwThreadId, WMTMC_FILEMAPPING_CHANGED, (WPARAM)hObjInProcessT, m_dwThreadID);
			return TRUE;
		}
	}

	return FALSE;
}

HANDLE CIPCSenderImpl::DuplicateMemoryToReceiver( DWORD dwThreadID, DWORD dwProcessID, HANDLE hFileMapping )
{
	LOG_DEBUG(_T("DuplicateMemoryToReceiver threadid=") << dwThreadID <<
		_T(", processid=") << dwProcessID << _T(", filemapping=") << hFileMapping);

	HANDLE hTargetThread = ::OpenThread(THREAD_QUERY_INFORMATION, FALSE, dwThreadID);
	if(hTargetThread)
	{
		DWORD dwProcessId = dwProcessID;
		CloseHandle(hTargetThread);

		HANDLE hTargetProcess = ::OpenProcess(PROCESS_DUP_HANDLE, FALSE, dwProcessId);
		if(NULL == hTargetProcess)
		{
			LOG_ERROR(_T("Open target process failed! hr=") << GetLastError());
			return NULL;
		}

		HANDLE hObjInProcessT = NULL;
		BOOL bRet = DuplicateHandle(GetCurrentProcess(), hFileMapping, hTargetProcess,
			&hObjInProcessT, 0, FALSE, DUPLICATE_SAME_ACCESS);
		if(!bRet || NULL == hObjInProcessT)
		{
			LOG_ERROR(_T("DuplicateHandle failed! hr=") << GetLastError());
			return NULL;
		}
		return hObjInProcessT;
	}
	return NULL;
}

