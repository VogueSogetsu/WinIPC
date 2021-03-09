#include "StdAfx.h"
#include "IPCReceiverImpl.h"

/*
 *=============================================================================
 *  CIPCReceiverImpl
 *=============================================================================
 */

CIPCReceiverImpl::CIPCReceiverImpl(CBaseDefaultMsgListener* pListener /* = NULL */)
: CBaseMsgCommThread(pListener)
, m_bRunning(FALSE)
, m_hFileMappingReceiver(NULL)
, m_pReceiverListener(NULL)
{
}

CIPCReceiverImpl::~CIPCReceiverImpl()
{
}

BOOL CIPCReceiverImpl::Create(INT nProductID,
                              DWORD dwBindThreadID, 
                              BOOL bExitIfSameProductExist /* = FALSE */)
{
	LOG_METHOD();

	if(m_bRunning)
	{
		LOG_WARN(_T("Receiver already on running!"));
		return TRUE;
	}

	HANDLE hMutexReceiver = ::CreateMutex(NULL, TRUE, _T("{4F67B3FD-5DAF-45ca-A42B-FC4597D1757C}-ReceiverMutex"));
	if(NULL == hMutexReceiver)
	{
		LOG_ERROR(_T("CreateMutex(Receiver) failed!"));
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

		m_bRunning = TRUE;		

		if(!AddReceiverToList(nProductID, bExitIfSameProductExist))
		{		
			LOG_ERROR(_T("AddReceiverToList failed!"));
			break;
		}
		
		m_nProductID		= nProductID;
		m_bRunning			= TRUE;
		bRet = TRUE;
	} while(false);

	::ReleaseMutex(hMutexReceiver);

    NotifyAllSender();
	return bRet;
}

VOID CIPCReceiverImpl::Close()
{
    LOG_METHOD();

    if(!m_bRunning || NULL == m_hFileMappingReceiver)
    {
        return;
    }

    // 从接收者列表中删除
    HANDLE hMutexReceiver = ::CreateMutex(NULL, TRUE, _T("{4F67B3FD-5DAF-45ca-A42B-FC4597D1757C}-ReceiverMutex"));
    if(NULL == hMutexReceiver)
    {
        LOG_ERROR(_T("CreateMutex(Receiver) failed!"));
    }
    else
    {
        RemoveReceiverFromList();
        ::ReleaseMutex(hMutexReceiver);
		CloseHandle(hMutexReceiver);
    }
	
    CloseHandle(m_hFileMappingReceiver);
    m_hFileMappingReceiver = NULL;

    // 通知所有Sender自己退出
    {
        CAutoLock lock(&m_lockSenders);

        TargetSNode* pNode = NULL;
        TargetSenderMap::iterator it;
        for(it = m_mapSenders.begin(); it != m_mapSenders.end(); it++)
        {
            pNode = it->second;
            if(pNode)
            {
                if(pNode->hFileMapping)
                {
                    CloseHandle(pNode->hFileMapping);
                    pNode->hFileMapping = NULL;
                }

                PostTargetMessage(pNode->node.dwThreadId, WMTMC_RECEIVER_EXIT, (WPARAM)m_dwThreadID);                
                delete pNode;
                pNode = NULL;
            }
        }
        m_mapSenders.clear();
    }    

    m_bRunning = FALSE;
}

BOOL CIPCReceiverImpl::AddReceiverToList(INT nProductID, BOOL bExitIfSameProductExist)
{
	LOG_METHOD();

	BOOL bRet = FALSE;
	LPBYTE pMemory = NULL;

	do
	{
		m_hFileMappingReceiver = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, 
			PAGE_READWRITE | SEC_COMMIT, 0, 4+MAX_RECEIVER_COUNT*sizeof(IPCSenderNode), 
			_T("{4F67B3FD-5DAF-45ca-A42B-FC4597D1757C}-Receiverlist"));

		if(!m_hFileMappingReceiver)
		{
			LOG_ERROR(_T("CreateFileMapping(Receiver) failed!"));
			break;
		}

		BOOL bFirstReceiver = FALSE;
		if(GetLastError() == ERROR_SUCCESS)
		{
			// 第一个发送线程
			LOG_INFO(_T("First receiver, threadid=") << m_dwThreadID);
			bFirstReceiver = TRUE;
		}

		pMemory = (LPBYTE)MapViewOfFile(m_hFileMappingReceiver, FILE_MAP_WRITE, 0, 0, 0);
		if(pMemory == NULL)
		{		
			LOG_ERROR(_T("MapViewOfFile(Receiver) failed!"));
			break;
		}

		IPCReceiverNode newReceiverNode;
		newReceiverNode.dwThreadId = m_dwThreadID;
		newReceiverNode.nProductId = nProductID;
        newReceiverNode.dwProcessId = GetCurrentProcessId();

		if(bFirstReceiver)
		{
			*((LPINT)pMemory) = 1;
			CopyMemory(pMemory+sizeof(INT), &newReceiverNode, sizeof(IPCReceiverNode));			
			bRet = TRUE;
			break;
		}
		else
		{
			INT nReceiverCount = *((LPINT)pMemory);
			if(nReceiverCount < 0 || nReceiverCount >= MAX_RECEIVER_COUNT)
			{	
				LOG_ERROR(_T("Receiver count(") << nProductID << _T(") not invalid"));
				return FALSE;
			}

			INT i = 0;
			LPIPCRECEIVERNODE pNode = NULL;

			if(bExitIfSameProductExist)
			{
				// 检查是否有同一产品的接收线程
				// 有则退出				
				pNode = (LPIPCRECEIVERNODE)(pMemory+sizeof(INT));
				for(i = 0; i < nReceiverCount; i++)
				{
					if(pNode->nProductId == nProductID)
					{
						LOG_ERROR(_T("Found same product id(") << nProductID << _T(")"));
						break;
					}
					pNode++;
				}	

				if(i >= nReceiverCount)
				{
					break;
				}
			}

			{
				// 删除无效线程
				pNode = (LPIPCRECEIVERNODE)(pMemory+sizeof(INT));
				for(i = 0; i < nReceiverCount; )
				{
					if( !IsThreadValid(pNode->dwThreadId) )
					{
						LOG_WARN(_T("Found invalid thread=") << pNode->dwThreadId << _T(" productid=") << nProductID);
						CopyMemory(pNode, pNode+1, (MAX_RECEIVER_COUNT-i-1)*sizeof(IPCReceiverNode));
						nReceiverCount--;
						*((LPINT)pMemory) = nReceiverCount;
						continue;
					}
					i++;
					pNode++;
				}
			}

			CopyMemory(pMemory+sizeof(INT)+nReceiverCount*sizeof(IPCReceiverNode), &newReceiverNode, sizeof(IPCReceiverNode));
            *((LPINT)pMemory) = nReceiverCount+1;
			bRet = TRUE;
		}
	} while(false);

	if(m_hFileMappingReceiver)
	{
		if(pMemory)
		{
			UnmapViewOfFile(pMemory);
		}		
	}

	return bRet;
}

VOID CIPCReceiverImpl::RemoveReceiverFromList()
{
    LOG_METHOD();

    LPBYTE pMemory = (LPBYTE)MapViewOfFile(m_hFileMappingReceiver, FILE_MAP_WRITE, 0, 0, 0);
    if(pMemory == NULL)
    {		
        LOG_ERROR(_T("MapViewOfFile(Receiver) failed!"));
        return;
    }

    INT nReceiverCount = *((LPINT)pMemory);

    INT i = 0;
    LPIPCRECEIVERNODE pNode = (LPIPCRECEIVERNODE)(pMemory+sizeof(INT));
    for(i = 0; i < nReceiverCount; i++)
    {
        if( pNode->dwThreadId == m_dwThreadID &&
            pNode->nProductId == m_nProductID &&
            pNode->dwProcessId == GetCurrentProcessId())
        {
            LOG_INFO(_T("Found receiver, ready to delete."));
            CopyMemory(pNode, pNode+1, (MAX_SENDER_COUNT-i-1)*sizeof(IPCReceiverNode));
            *((LPINT)pMemory) = nReceiverCount-1;
            break;
        }

        pNode++;
    }
}

VOID CIPCReceiverImpl::NotifyAllSender(int nTargetProductID)
{
	LOG_METHOD();

	if(m_setAcceptProductID.empty())
	{
		LOG_WARN(_T("accept product id set is empty!"));
		return;
	}

	HANDLE hMutexReceiver = ::CreateMutex(NULL, TRUE, _T("{4F67B3FD-5DAF-45ca-A42B-FC4597D1757C}-SenderMutex"));
	if(NULL == hMutexReceiver)
	{
		LOG_ERROR(_T("CreateMutex(Sender) failed!"));
		return;
	}

	BOOL bRet = FALSE;
	HANDLE hFileMappingSender = NULL;
	LPBYTE pMemory = NULL;

	// 读取发送线程列表
	do
	{
		hFileMappingSender = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, 
			PAGE_READWRITE | SEC_COMMIT, 0, 4+MAX_SENDER_COUNT*sizeof(IPCSenderNode), 
			_T("{4F67B3FD-5DAF-45ca-A42B-FC4597D1757C}-Senderlist"));		

		if(!hFileMappingSender)
		{
			break;
		}

		pMemory = (LPBYTE)MapViewOfFile(hFileMappingSender, FILE_MAP_WRITE, 0, 0, 0);

		INT nSenderCount = *((LPINT)pMemory);
		if(nSenderCount < 0)
		{	
			LOG_ERROR(_T("Sender count(") << nSenderCount << _T(") not invalid"));
			return;
		}

        // 通知所有发送线程 
		LOG_DEBUG(_T("Try to notify sender count=") << nSenderCount);
        {
            CAutoLock lock(&m_lockSenders);

		    LPIPCSENDERNODE pNode = (LPIPCSENDERNODE)(pMemory+sizeof(INT));
		    for(INT i = 0; i < nSenderCount; i++)
		    {
				if( -1 == nTargetProductID )
				{
					if( m_setAcceptProductID.find(pNode->nProductId) != m_setAcceptProductID.end() &&
						pNode->dwThreadId != m_dwThreadID )
					{
						LOG_INFO(_T("Try to notify sender [") <<
							pNode->dwThreadId << _T(", ") << pNode->nProductId << _T("]"));
						NotifyOneSender(pNode);
					}
					else
					{
						LOG_INFO(_T("reject sender [") <<
							pNode->dwThreadId << _T(", ") << pNode->nProductId << _T("]"));
					}
				}
				else
				{
					if( pNode->nProductId == nTargetProductID && pNode->dwThreadId != m_dwThreadID )
					{
						LOG_INFO(_T("Try to notify sender [") <<
							pNode->dwThreadId << _T(", ") << pNode->nProductId << _T("]"));
						NotifyOneSender(pNode);
						break;
					}
				}

			    pNode++;
		    }
        }
	} while(false);

	if(hFileMappingSender)
	{
		if(pMemory)
		{
			::UnmapViewOfFile(pMemory);
		}
		CloseHandle(hFileMappingSender);
	}
	::ReleaseMutex(hMutexReceiver);
	CloseHandle(hMutexReceiver);
}

VOID CIPCReceiverImpl::NotifyOneSender(LPIPCSENDERNODE pSenderNode)
{
	LOG_METHOD();  

	if(m_mapSenders.find(pSenderNode->dwThreadId) != m_mapSenders.end())
	{
		LOG_WARN(_T("sender (") << pSenderNode->dwThreadId << _T(") already in map!"));
		return;
	}	

	PostTargetMessage(pSenderNode->dwThreadId, WMTMC_RECEIVER_CREATED, (WPARAM)m_dwThreadID, m_nProductID);
    PostTargetMessage(pSenderNode->dwThreadId, WMTMC_RECEIVER_CREATED_2, (WPARAM)m_dwThreadID, GetCurrentProcessId());
}

VOID CIPCReceiverImpl::AddAcceptProductID(INT nProductID)
{
	LOG_METHOD();
	if( m_setAcceptProductID.find(nProductID) != m_setAcceptProductID.end() )
	{
		return;
	}
	m_setAcceptProductID.insert(nProductID);

    if(m_bRunning)
    {
        NotifyAllSender(nProductID);
    }    
}

VOID CIPCReceiverImpl::RemoveAcceptProductID(INT nProductID)
{
	if( m_setAcceptProductID.find(nProductID) == m_setAcceptProductID.end() )
	{
		return;
	}
	m_setAcceptProductID.erase(nProductID);

    if(m_bRunning)
    {
		CAutoLock lock(&m_lockSenders);

		TargetSNode* pNode = NULL;
		TargetSenderMap::iterator it;
		for(it = m_mapSenders.begin(); it != m_mapSenders.end(); )
		{
			pNode = it->second;
			if( pNode && (pNode->node.nProductId == nProductID) )
			{
				if(pNode->hFileMapping)
				{
					CloseHandle(pNode->hFileMapping);
					pNode->hFileMapping = NULL;
				}

				PostTargetMessage(pNode->node.dwThreadId, WMTMC_RECEIVER_EXIT, (WPARAM)m_dwThreadID);                
				delete pNode;
				pNode = NULL;

				m_mapSenders.erase(it++);
				continue;
			}
			it++;
		}
    }    
}

//////////////////////////////////////////////////////////////////////////
// 消息循环
BEGIN_THREAD_MSG_MAP(CIPCReceiverImpl)
	ON_THREAD_MSG(WMTMC_SYN, OnSyn)
	ON_THREAD_MSG(WMTMC_ACK, OnAck)
    ON_THREAD_MSG(WMTMC_DATA_REQ, OnDataReq)
	ON_THREAD_MSG(WMTMC_MSG_REQ, OnMsgReq)
	ON_THREAD_MSG(WMTMC_FILEMAPPING_CHANGED, OnFileMappingChanged)
END_THREAD_MSG_MAP

/*
 * wParam - sender thread id 
 * lParam - sender product id
 */
BOOL CIPCReceiverImpl::OnSyn(WPARAM wParam, LPARAM lParam)
{
	DWORD dwTargetID = (DWORD)wParam;
	INT nProductID = (INT)lParam;

	if(m_setAcceptProductID.find(nProductID) == m_setAcceptProductID.end())
	{
		LOG_INFO(_T("Reject sender (") << dwTargetID << _T(", ") << nProductID << _T(")"));
		PostTargetMessage(dwTargetID, WMTMC_SYNACK, 1, m_dwThreadID);
		return TRUE;
	}

	// accept
	TargetSNode* pNewNode = new TargetSNode;
	if(!pNewNode)
	{
		return TRUE;
	}

	pNewNode->node.dwThreadId	= dwTargetID;
	pNewNode->node.nProductId	= nProductID;
	pNewNode->status			= SYN_RECV;
	pNewNode->hFileMapping		= NULL;

    CAutoLock lock(&m_lockSenders);
	m_mapSenders.insert(TargetSenderMap::value_type(dwTargetID, pNewNode));

	LOG_INFO(_T("Accept sender (") << dwTargetID << _T(", ") << nProductID << _T(")"));
	PostTargetMessage(dwTargetID, WMTMC_SYNACK, 0, m_dwThreadID);
	return TRUE;
}

/*
 * wParam - filemapping handle 
 * lParam - receiver thread id
 */
BOOL CIPCReceiverImpl::OnAck(WPARAM wParam, LPARAM lParam)
{
	HANDLE hFileMapping = (HANDLE)wParam;
	DWORD dwThreadID = (DWORD)lParam;

	LOG_INFO(_T("threadid=") << dwThreadID << _T(", filemapping=") << hFileMapping);

	int nProductID;
	{
		CAutoLock lock(&m_lockSenders);

		TargetSenderMap::iterator it = m_mapSenders.find(dwThreadID);
		if(it == m_mapSenders.end())
		{
			LOG_ERROR(_T("Can't find sender!"));
			return TRUE;
		}

		LPTARGETSNODE pNode = it->second;

		pNode->hFileMapping = hFileMapping;
		pNode->status = ESTABLISHED;
		nProductID = (pNode->node).nProductId;
	}
	if(m_pReceiverListener)
	{
		m_pReceiverListener->OnSenderConnect(nProductID);
	}
	LOG_INFO(_T("!!! An connection has builded !!!"));
	return TRUE;
}

/*
 * wParam - sender thread id 
 * lParam - data block offset
 */
BOOL CIPCReceiverImpl::OnDataReq(WPARAM wParam, LPARAM lParam)
{
	LOG_METHOD();

    DWORD dwThreadID = (DWORD)wParam;
    DWORD dwOffset = (DWORD)lParam;

	LOG_DEBUG(m_dwThreadID << _T(" <<< ") << dwThreadID << _T(" [:") << dwOffset << _T("]"));

    BOOL bRecvSuccess = FALSE;
    do 
    {
        CAutoLock lock(&m_lockSenders);
        TargetSenderMap::iterator it = m_mapSenders.find(dwThreadID);
        if(it == m_mapSenders.end())
        {
            LOG_ERROR(_T("Can't find sender!"));
            break;
        }

        TargetSNode* pNode = it->second;
        if(NULL == pNode || NULL == pNode->hFileMapping)
        {
            LOG_ERROR(_T("Filemapping handle is null!"));
            break;
        }

        if(pNode->status != ESTABLISHED)
        {
            LOG_ERROR(_T("Connection not ready, is on status=") << pNode->status);
            break;
        }

        LPBYTE pBuffer = (LPBYTE)::MapViewOfFile(pNode->hFileMapping, FILE_MAP_READ, 0, 0, 0);

        if(NULL == pBuffer)
        {
            LOG_ERROR(_T("MapViewOfFile failed! hr=") << GetLastError());
            break;
        }

        DWORD dwBuffSize = *((LPDWORD)(pBuffer+dwOffset));

        LOG_INFO(_T("!!! Data recv !!! len=") << dwBuffSize);
        //////////////////////////////////////////////////////////////////////////
        // TODO
        //LOG_INFO( _T("Data=") << (LPCTSTR)(pBuffer+dwOffset+sizeof(DWORD)) );
        if(m_pReceiverListener)
        {
            m_pReceiverListener->OnReceiveData(pNode->node.nProductId, pBuffer+dwOffset+sizeof(DWORD), dwBuffSize);
        }

        UnmapViewOfFile(pBuffer);
        bRecvSuccess = TRUE;
    } while (false);

    PostTargetMessage(dwThreadID, (bRecvSuccess ? WMTMC_DATA_RESP : WMTMC_DATA_INVALID), (WPARAM)m_dwThreadID, dwOffset);
    return TRUE;
}

/*
 * wParam - 
 * lParam - 
 */
BOOL CIPCReceiverImpl::OnMsgReq(WPARAM wParam, LPARAM lParam)
{
	DWORD dwThreadID = (DWORD)wParam;
	DWORD dwOffset = (DWORD)lParam;

	LOG_DEBUG(_T("threadid=") << dwThreadID << _T(", offset=") << dwOffset);

	BOOL bRecvSuccess = FALSE;
	do 
	{
		CAutoLock lock(&m_lockSenders);
		TargetSenderMap::iterator it = m_mapSenders.find(dwThreadID);
		if(it == m_mapSenders.end())
		{
			LOG_ERROR(_T("Can't find sender!"));
			break;
		}

		TargetSNode* pNode = it->second;
		if(NULL == pNode || NULL == pNode->hFileMapping)
		{
			LOG_ERROR(_T("Filemapping handle is null!"));
			break;
		}

		if(pNode->status != ESTABLISHED)
		{
			LOG_ERROR(_T("Connection not ready, is on status=") << pNode->status);
			break;
		}

		if(m_pReceiverListener)
		{
			m_pReceiverListener->OnReceiveMsg(pNode->node.nProductId, 
				((int)(short)LOWORD(lParam)), ((int)(short)HIWORD(lParam)));
		}

		bRecvSuccess = TRUE;
	} while(false);

	return bRecvSuccess;
}

BOOL CIPCReceiverImpl::IsSenderConnected( INT nProductID )
{
	if( m_setAcceptProductID.find(nProductID) == m_setAcceptProductID.end() )
	{
		return FALSE;
	}

	CAutoLock lock(&m_lockSenders);

	TargetSNode* pNode = NULL;
	TargetSenderMap::iterator it;
	for(it = m_mapSenders.begin(); it != m_mapSenders.end(); )
	{
		pNode = it->second;
		if( pNode && pNode->status == ESTABLISHED && (pNode->node.nProductId == nProductID) )
		{
			if( IsThreadValid(it->first) )
			{
				return TRUE;
			}
			else
			{
				m_mapSenders.erase(it++);
			}
		}		
		else
		{
			it++;
		}
	}
	return FALSE;
}

/*
 * wParam - filemapping handle 
 * lParam - receiver thread id
 */
BOOL CIPCReceiverImpl::OnFileMappingChanged(WPARAM wParam, LPARAM lParam)
{
	LOG_METHOD();
	HANDLE hFileMapping = (HANDLE)wParam;
	DWORD dwThreadID = (DWORD)lParam;

	LOG_INFO(_T("threadid=") << dwThreadID << _T(", filemapping=") << hFileMapping);

    CAutoLock lock(&m_lockSenders);

	TargetSenderMap::iterator it = m_mapSenders.find(dwThreadID);
	if(it == m_mapSenders.end())
	{
		LOG_ERROR(_T("Can't find sender!"));
		return TRUE;
	}

	LPTARGETSNODE pNode = it->second;
	if(pNode->status != ESTABLISHED)
	{
		LOG_ERROR(_T("stat is not ESTABLISHED"));
		return TRUE;
	}

	pNode->hFileMapping = hFileMapping;
	return TRUE;
}
