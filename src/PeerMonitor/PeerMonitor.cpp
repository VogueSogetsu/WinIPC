// PeerMonitor.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\\IPCStructs.h"
#include "..\\ThreadMsgDefines.h"

BOOL IsThreadValid( DWORD dwThreadID )
{
	HANDLE hTargetThread = ::OpenThread(THREAD_QUERY_INFORMATION, FALSE, dwThreadID);
	if(!hTargetThread)
	{
		return FALSE;
	}
	CloseHandle(hTargetThread);
	return TRUE;
}

void DumpSenderList()
{
	HANDLE hMutexSender = ::CreateMutex(NULL, TRUE, _T("{4F67B3FD-5DAF-45ca-A42B-FC4597D1757C}-SenderMutex"));
	if(NULL == hMutexSender)
	{		
		return;
	}

	LPBYTE pMemory = NULL;
	HANDLE hFileMappingSender = NULL;
	do
	{
		hFileMappingSender = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, 
			PAGE_READWRITE | SEC_COMMIT, 0, 4+MAX_SENDER_COUNT*sizeof(IPCSenderNode), 
			_T("{4F67B3FD-5DAF-45ca-A42B-FC4597D1757C}-Senderlist"));

		if(!hFileMappingSender)
		{			
			break;
		}

		if(GetLastError() == ERROR_SUCCESS)
		{
			break;
		}

		pMemory = (LPBYTE)MapViewOfFile(hFileMappingSender, FILE_MAP_WRITE, 0, 0, 0);
		if(pMemory == NULL)
		{	
			break;
		}

		INT nSenderCount = *((LPINT)pMemory);
		if(nSenderCount < 0 || nSenderCount >= MAX_SENDER_COUNT)
		{
			break;
		}

		INT i = 0;
		LPIPCSENDERNODE pNode = NULL;
		pNode = (LPIPCSENDERNODE)(pMemory+sizeof(INT));
		for(i = 0; i < nSenderCount; i++)
		{
			if(pNode)
			{
				_tprintf(_T("[S] ProductID:%d ThreadID:%d Valid:%d\n"), 
					pNode->nProductId, pNode->dwThreadId, IsThreadValid(pNode->dwThreadId));
			}
			pNode++;
		}	
	} while(false);

	if(hFileMappingSender)
	{
		if(pMemory)
		{
			UnmapViewOfFile(pMemory);
		}		
	}
	::ReleaseMutex(hMutexSender);
}

void DumpReceiverList()
{
	HANDLE hMutexReceiver = ::CreateMutex(NULL, TRUE, _T("{4F67B3FD-5DAF-45ca-A42B-FC4597D1757C}-ReceiverMutex"));
	if(NULL == hMutexReceiver)
	{
		return;
	}

	LPBYTE pMemory = NULL;
	HANDLE hFileMappingSender = NULL;

	do
	{
		hFileMappingSender = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, 
			PAGE_READWRITE | SEC_COMMIT, 0, 4+MAX_RECEIVER_COUNT*sizeof(IPCSenderNode), 
			_T("{4F67B3FD-5DAF-45ca-A42B-FC4597D1757C}-Receiverlist"));

		if(!hFileMappingSender)
		{
			break;
		}
		if(GetLastError() == ERROR_SUCCESS)
		{
			break;
		}

		pMemory = (LPBYTE)MapViewOfFile(hFileMappingSender, FILE_MAP_WRITE, 0, 0, 0);
		if(pMemory == NULL)
		{
			break;
		}

		INT nReceiverCount = *((LPINT)pMemory);
		if(nReceiverCount < 0 || nReceiverCount >= MAX_RECEIVER_COUNT)
		{	
			break;
		}

		INT i = 0;
		LPIPCRECEIVERNODE pNode = NULL;
		pNode = (LPIPCRECEIVERNODE)(pMemory+sizeof(INT));
		for(i = 0; i < nReceiverCount; i++)
		{
			if(pNode)
			{
				_tprintf(_T("[R] ProductID:%d ThreadID:%d Valid:%d\n"), 
					pNode->nProductId, pNode->dwThreadId, IsThreadValid(pNode->dwThreadId));
			}
			pNode++;
		}	
	} while(false);

	if(hFileMappingSender)
	{
		if(pMemory)
		{
			UnmapViewOfFile(pMemory);
		}		
	}
	::ReleaseMutex(hMutexReceiver);
}

int _tmain(int argc, _TCHAR* argv[])
{
	DumpSenderList();
	DumpReceiverList();
	return 0;
}

