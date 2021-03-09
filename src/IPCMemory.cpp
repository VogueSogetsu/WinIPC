#include "StdAfx.h"
#include "IPCMemory.h"
#include "ThreadMsgDefines.h"
#include "IPCSenderImpl.h"

/*
 *=============================================================================
 *  CIPCMemory
 *=============================================================================
 */

CIPCMemory::CIPCMemory()
: m_hFileMapping(NULL)
, m_pSender(NULL)
, m_dwTotalLen(0)
, m_dwBindReceiverID(0)
{
}

CIPCMemory::~CIPCMemory()
{
}

BOOL CIPCMemory::Create(CIPCSenderImpl* pSender, DWORD dwThreadID)
{	
	LOG_METHOD();

	if(m_hFileMapping)
	{
		LOG_WARN(_T("Memory already created!"));
		return TRUE;
	}

	m_hFileMapping = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, 
		PAGE_READWRITE | SEC_COMMIT, 0, MAX_FILEMAPPING_BUFFER, NULL);

	if(NULL == m_hFileMapping)
	{
		LOG_ERROR(_T("CreateFileMapping failed!"));
		return FALSE;
	}

	IPCMemoryNode node;
	node.dwOffset = 0;
	node.dwLength = MAX_FILEMAPPING_BUFFER;
	m_dwTotalLen  = MAX_FILEMAPPING_BUFFER;

	m_listIdleMemoryNode.push_back(node);
	m_dwBindReceiverID = dwThreadID;
	m_pSender = pSender;
	return TRUE;
}

VOID CIPCMemory::Close()
{
	LOG_METHOD();

	if(NULL == m_hFileMapping)
	{
		return;
	}

    CAutoLock lock(&m_lockMemory);

#ifdef DEBUG
	IPCBusyMemoryMap::iterator it;
	for(it = m_mapBusyMemoryNode.begin(); it != m_mapBusyMemoryNode.end(); it++)
	{
		LOG_WARN(_T("Busy block [") << it->second.dwOffset <<
			_T(", ") << it->second.dwLength << _T("] ref=") << it->second.nRef);
	}
#endif

	m_mapBusyMemoryNode.clear();
	m_listIdleMemoryNode.clear();

	CloseHandle(m_hFileMapping);
	m_hFileMapping = NULL;
}

BOOL CIPCMemory::CopyAndGetMemory(LPBYTE pBuffer, DWORD dwLength, 
								  LPDWORD pdwOffset /* = NULL */, BOOL bWait /* = FALSE */)
{
	LOG_INFO(_T("Try to get block, len=") << dwLength);

	if(NULL == m_hFileMapping || 0 == dwLength || NULL == pBuffer)
	{
		return FALSE;
	}

	if(pdwOffset)
	{
		*pdwOffset = 0;
	}

	// 从空闲列表取第一个满足条件的块
    CAutoLock lock(&m_lockMemory);

	int nRetry = 0;
	do 
	{
		if(dwLength+sizeof(DWORD) <= m_dwTotalLen)
		{		
			IPCIdleMemoryList::iterator it;
			for(it = m_listIdleMemoryNode.begin(); it != m_listIdleMemoryNode.end(); it++)
			{
				IPCMemoryNode& node = (*it);
				if(node.dwLength >= dwLength+sizeof(DWORD))
				{
					LOG_INFO(_T("Found idle block [") << node.dwOffset <<
						_T(", ") << node.dwLength << _T("]"));

					// split it
					IPCMemoryRefNode	nodeBusy;
					IPCMemoryNode		nodeTail;

					nodeBusy.dwOffset = node.dwOffset;
					nodeBusy.dwLength = dwLength+sizeof(DWORD);

					nodeTail.dwOffset = nodeBusy.dwOffset+nodeBusy.dwLength;
					nodeTail.dwLength = node.dwLength-nodeBusy.dwLength;	

					LPBYTE pRetBuffer = (LPBYTE)::MapViewOfFile(m_hFileMapping, FILE_MAP_WRITE, 
						0, 0, 0);

					if(NULL == pRetBuffer)
					{
						LOG_ERROR(_T("MapViewOfFile failed! hr=") << GetLastError());
						continue;
					}

					nodeBusy.nRef = 1;			
					*((LPDWORD)(pRetBuffer+nodeBusy.dwOffset)) = dwLength;
					CopyMemory(pRetBuffer+nodeBusy.dwOffset+sizeof(DWORD), pBuffer, dwLength);
					UnmapViewOfFile(pRetBuffer);

					if(bWait)
					{
						LOG_DEBUG(_T("Create waiting event"));
						nodeBusy.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
					}

					m_listIdleMemoryNode.erase(it);
					m_mapBusyMemoryNode.insert(IPCBusyMemoryMap::value_type(nodeBusy.dwOffset, nodeBusy));
					if(nodeTail.dwLength > 0)
					{
						m_listIdleMemoryNode.push_front(nodeTail);
					}

					if(pdwOffset)
					{
						*pdwOffset = nodeBusy.dwOffset;
					}

					LOG_DEBUG(_T("Succeeded"));
					return TRUE;
				}
			}
		}
		
		if(0 == nRetry)
		{
			// try to merge idle block
			//Dump();
			MergeIdleBlock();
			nRetry++;
		}
		else if(1 == nRetry)
		{				
			// resize memory
			if( ResizeMemoryToFit(dwLength+sizeof(DWORD)) )
			{
				nRetry++;
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
		
	 } while(nRetry <=2 );
	
	LOG_ERROR(_T("Failed"));
	return FALSE;
}

INT CIPCMemory::AddRefMemory(DWORD dwOffset, DWORD dwLength)
{
	LOG_DEBUG(_T("Try to addref memory [") << 
		dwOffset << _T(", ") << dwLength << _T("]"));

	if(0 == dwLength)
	{
		return -1;
	}

    CAutoLock lock(&m_lockMemory);

	IPCBusyMemoryMap::iterator it = m_mapBusyMemoryNode.find(dwOffset);
	if(it == m_mapBusyMemoryNode.end())
	{
		LOG_ERROR(_T("Not found busy block"));
		return -2;
	}

	if(dwLength != it->second.dwLength)
	{
		LOG_ERROR(_T("Block size not right, real block size=") << it->second.dwLength);
		return -3;
	}
	
	return ++(it->second.nRef);
}

INT CIPCMemory::AddRefMemory(DWORD dwOffset)
{
	LOG_DEBUG(_T("Try to addref memory [") << dwOffset);

	CAutoLock lock(&m_lockMemory);

	IPCBusyMemoryMap::iterator it = m_mapBusyMemoryNode.find(dwOffset);
	if(it == m_mapBusyMemoryNode.end())
	{
		LOG_ERROR(_T("Not found busy block"));
		return -2;
	}

	return ++(it->second.nRef);
}

INT CIPCMemory::ReleaseMemory(DWORD dwOffset, DWORD dwLength)
{
	LOG_INFO(_T("Try to free memory [") << 
		dwOffset << _T(", ") << dwLength << _T("]"));

	if(0 == dwLength)
	{
		return -1;
	}

    CAutoLock lock(&m_lockMemory);

	IPCBusyMemoryMap::iterator it = m_mapBusyMemoryNode.find(dwOffset);
	if(it == m_mapBusyMemoryNode.end())
	{
		LOG_ERROR(_T("Not found busy block"));
		return -2;
	}

	if(dwLength != it->second.dwLength)
	{
		LOG_ERROR(_T("Block size not right, real block size=") << it->second.dwLength);
		return -3;
	}

	INT nRef = --(it->second.nRef);
	if(0 == nRef)
	{
		LOG_INFO(_T("Delete busy block from busylist"));
		HANDLE hEvent = it->second.hEvent;
		it->second.hEvent = NULL;
		m_listIdleMemoryNode.push_front(it->second);
		m_mapBusyMemoryNode.erase(dwOffset);

		if(hEvent)
		{
			LOG_DEBUG(_T("Notify sender data is reached"));
			SetEvent(hEvent);
			CloseHandle(hEvent);
		}
	}

	MergeIdleBlock();
	return nRef;
}

INT CIPCMemory::ReleaseMemory(DWORD dwOffset)
{
	LOG_INFO(_T("Try to free memory [:") << dwOffset << _T("]") );

	CAutoLock lock(&m_lockMemory);

	IPCBusyMemoryMap::iterator it = m_mapBusyMemoryNode.find(dwOffset);
	if(it == m_mapBusyMemoryNode.end())
	{
		LOG_ERROR(_T("Not found busy block"));
		return -2;
	}

	INT nRef = --(it->second.nRef);
	if(0 == nRef)
	{
		LOG_INFO(_T("Delete busy block from busylist"));
		HANDLE hEvent = it->second.hEvent;
		it->second.hEvent = NULL;
		m_listIdleMemoryNode.push_front(it->second);
		m_mapBusyMemoryNode.erase(dwOffset);

		if(hEvent)
		{
			LOG_DEBUG(_T("Notify sender data is reached"));
			SetEvent(hEvent);
			CloseHandle(hEvent);
		}
	}

	MergeIdleBlock();
	return nRef;
}

VOID CIPCMemory::MergeIdleBlock()
{
	LOG_METHOD();

    if(NULL == m_hFileMapping)
    {
        return;
    }

    CAutoLock lock(&m_lockMemory);

	if(m_listIdleMemoryNode.empty())
	{
		LOG_DEBUG(_T("No idle block"));
		return;
	}

    if(m_listIdleMemoryNode.size() == 1)
    {
        LOG_DEBUG(_T("Only one block, no need to merge."));
        return;
    }

	m_listIdleMemoryNode.sort();

	DWORD dwRightEnd = 0;
	DWORD dwRightLimit = 0;

	IPCIdleMemoryList::iterator it1 = m_listIdleMemoryNode.begin();
	IPCIdleMemoryList::iterator it2 = m_listIdleMemoryNode.begin();
	it2++;
	
	for(; it2 != m_listIdleMemoryNode.end(); )
	{
		dwRightEnd = (*it1).dwOffset + (*it1).dwLength;
		dwRightLimit = (*it2).dwOffset + (*it2).dwLength;

		if(dwRightEnd < (*it2).dwOffset)
		{
			it1++;
			it2++;
			continue;			
		}
		else
		{
			// merge
            LOG_DEBUG(_T("Merge block [") << (*it1).dwOffset << _T(", ") << (*it1).dwLength <<
                _T("] and [") << (*it2).dwOffset << _T(", ") << (*it2).dwLength << _T("]"));

			dwRightEnd = max(dwRightEnd, dwRightLimit);
			(*it1).dwLength = dwRightEnd-(*it1).dwOffset;

			it2 = m_listIdleMemoryNode.erase(it2);
		}
	}
}

BOOL CIPCMemory::WaitMemoryToRelease( DWORD dwOffset, DWORD dwTimes )
{
	HANDLE hEvent = NULL;
	{
		CAutoLock lock(&m_lockMemory);

		IPCBusyMemoryMap::iterator it = m_mapBusyMemoryNode.find(dwOffset);
		if(it == m_mapBusyMemoryNode.end())
		{
			LOG_ERROR(_T("Not found busy block"));
			return FALSE;
		}
		hEvent = (it->second).hEvent;
	}

	DWORD dwRet = WAIT_TIMEOUT;
	if(hEvent)
	{
		dwRet = WaitForSingleObject(hEvent, dwTimes);
	}
	return (dwRet == WAIT_OBJECT_0);
}

VOID CIPCMemory::Dump()
{
	LOG_TRACE(_T(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"));
	LOG_TRACE(_T("Idle Block:\n"));

	IPCIdleMemoryList::iterator itIdle;
	for(itIdle = m_listIdleMemoryNode.begin(); itIdle != m_listIdleMemoryNode.end(); itIdle++)
	{
		IPCMemoryNode& node = (*itIdle);
		LOG_TRACE(_T("[") << node.dwOffset << _T(", ") << node.dwLength << _T("]"));
	}

	LOG_TRACE(_T(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"));
	LOG_TRACE(_T("Busy Block:\n"));

	IPCBusyMemoryMap::iterator itBusy;
	for(itBusy = m_mapBusyMemoryNode.begin(); itBusy != m_mapBusyMemoryNode.end(); itBusy++)
	{
		IPCMemoryRefNode& node = itBusy->second;
		LOG_TRACE(_T("[") << node.dwOffset << _T(", ") << node.dwLength << _T("](") << node.nRef << _T(")"));		
	}
}

BOOL CIPCMemory::ResizeMemoryToFit( DWORD dwLen )
{
	LOG_METHOD();
	LOG_DEBUG(_T("want to fit ") << dwLen << _T(" bytes"));

	DWORD dwNewLen = m_dwTotalLen+dwLen;
	IPCMemoryNode nodeNewIdle;
	nodeNewIdle.dwOffset = m_dwTotalLen;
	nodeNewIdle.dwLength = dwLen;

	// ASSERT idleblock is sorted
	if( !m_listIdleMemoryNode.empty() )
	{
		IPCIdleMemoryList::reverse_iterator itEnd = m_listIdleMemoryNode.rbegin();
		IPCMemoryNode& node = (*itEnd);

		if(node.dwOffset+node.dwLength == m_dwTotalLen)
		{
			nodeNewIdle.dwOffset = node.dwOffset;
			dwNewLen = m_dwTotalLen+dwLen-node.dwLength;			
		}
	}

	if(dwNewLen <= m_dwTotalLen)
	{
		return TRUE;
	}	

	LOG_DEBUG(_T("Extend memory ") << m_dwTotalLen << _T(" ->") << dwNewLen);

	HANDLE hNewFileMapping = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, 
		PAGE_READWRITE | SEC_COMMIT, 0, dwNewLen, NULL);

	if(NULL == hNewFileMapping)
	{
		LOG_ERROR(_T("CreateFileMapping failed!"));
		return FALSE;
	}

	LPBYTE pBuffer = (LPBYTE)::MapViewOfFile(m_hFileMapping, FILE_MAP_WRITE, 0, 0, 0);
	LPBYTE pNewBuffer = (LPBYTE)::MapViewOfFile(hNewFileMapping, FILE_MAP_WRITE, 0, 0, 0);

	if(NULL == pBuffer || NULL == pNewBuffer)
	{
		LOG_ERROR(_T("MapViewOfFile failed! hr=") << GetLastError());
		return FALSE;
	}

	CopyMemory(pNewBuffer, pBuffer, m_dwTotalLen);
	UnmapViewOfFile(pNewBuffer);
	UnmapViewOfFile(pBuffer);

	CloseHandle(m_hFileMapping);
	m_hFileMapping = hNewFileMapping;	

	// add new idle block
	if(nodeNewIdle.dwOffset != m_dwTotalLen)
	{
		m_listIdleMemoryNode.pop_back();
	}
	m_listIdleMemoryNode.push_back(nodeNewIdle);
	m_dwTotalLen = dwNewLen;

	//Dump();

	// notify sender
	if(m_pSender)
	{
		m_pSender->OnFileMappingChanged(m_dwBindReceiverID);
	}
	return TRUE;
}
