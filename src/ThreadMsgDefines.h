#pragma once

//////////////////////////////////////////////////////////////////////////
// ����������Զ�����Ϣ����
enum
{
	WMTMC__BEGIN = WM_APP+1,	// Base
    WMTMC_IDLE,
	WMTMC_EXIT,					// ֪ͨ�߳��˳�
	WMTMC_SYN,					// ���ͷ���������		S->R
	WMTMC_SYNACK,				// ���շ���Ӧ			S<-R
	WMTMC_ACK,					// ���ͷ�ȷ��			S->R
	WMTMC_DATA_REQ,				// ���ͷ���������		S->R
	WMTMC_MSG_REQ,              // ���ͷ�������Ϣ		S->R        
	WMTMC_DATA_RESP,			// ���շ��յ�����		S<-R
	WMTMC_DATA_INVALID,			// ���շ��յ���Ч����	S<-R
	WMTMC_RECEIVER_CREATED,		// һ���µĽ��շ�����	S<-R
    WMTMC_RECEIVER_CREATED_2,	// һ���µĽ��շ�����	S<-R
	WMTMC_RECEIVER_EXIT,		// ���շ��˳�			S<-R
	WMTMC_FILEMAPPING_CHANGED,  // ���ͷ��ڴ�ӳ��ı�   S->R
    WMTMC__END,
};

#define MAX_SENDER_COUNT 100
#define MAX_RECEIVER_COUNT 100
#define MAX_FILEMAPPING_BUFFER (4*1024*1024)

//////////////////////////////////////////////////////////////////////////
// �߳�״̬
enum
{
	SYN_SEND,
	SYN_RECV,
	ESTABLISHED,
	CLOSED,
	BROKEN,
};