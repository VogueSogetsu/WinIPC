#pragma once

#include <list>
#include <hash_map>
#include <algorithm>
#include "IPCLock.h"

class CIPCSenderImpl;

class CIPCMemory
{
public:
	CIPCMemory();
	~CIPCMemory();

	struct IPCMemoryNode
	{
		DWORD  dwOffset;
		DWORD  dwLength;		

		IPCMemoryNode() : dwOffset(0), dwLength(0) {}
		bool operator < (const IPCMemoryNode& rhs)
		{
			return (dwOffset < rhs.dwOffset);
		}
	};

	struct IPCMemoryRefNode : public IPCMemoryNode
	{
		INT    nRef;
		HANDLE hEvent;
		IPCMemoryRefNode() : nRef(1), hEvent(NULL) {}
	};
	typedef IPCMemoryNode* LPIPCMEMORYNODE;
	typedef std::list<IPCMemoryNode> IPCIdleMemoryList;
	typedef stdext::hash_map<DWORD, IPCMemoryRefNode> IPCBusyMemoryMap;

public:
	BOOL				Create(CIPCSenderImpl* pSender, DWORD dwThreadID);
	VOID				Close();

	BOOL				CopyAndGetMemory(LPBYTE pBuffer, DWORD dwLength, LPDWORD pdwOffset = NULL, BOOL bWait = FALSE);

	INT					AddRefMemory(DWORD dwOffset);
	INT					AddRefMemory(DWORD dwOffset, DWORD dwLength);
	
	INT					ReleaseMemory(DWORD dwOffset);
	INT					ReleaseMemory(DWORD dwOffset, DWORD dwLength);

	BOOL				WaitMemoryToRelease(DWORD dwOffset, DWORD dwTimes = INFINITE);

public:
	HANDLE				GetFileMapping() const { return m_hFileMapping; }
	VOID				MergeIdleBlock();
	VOID				Dump();

private:
	BOOL				ResizeMemoryToFit(DWORD dwLen);

private:
	HANDLE				m_hFileMapping;
	IPCIdleMemoryList	m_listIdleMemoryNode;
	IPCBusyMemoryMap	m_mapBusyMemoryNode;
    CCritSec            m_lockMemory;
	DWORD				m_dwBindReceiverID;
	DWORD				m_dwTotalLen;
	CIPCSenderImpl*		m_pSender;

private:
	LOG_CLS_DEC();
};
