#pragma once


#pragma pack(1)

typedef struct stIPCSenderNode
{
	INT		nProductId;
	DWORD	dwThreadId;
} IPCSenderNode, *LPIPCSENDERNODE;

typedef struct stIPCReceiverNode
{
	INT		nProductId;
	DWORD	dwThreadId;
	DWORD	dwProcessId;
} IPCReceiverNode, *LPIPCRECEIVERNODE;

#pragma pack()