#pragma once

//////////////////////////////////////////////////////////////////////////
// 在这里添加自定义消息代码
enum
{
	WMTMC__BEGIN = WM_APP+1,	// Base
    WMTMC_IDLE,
	WMTMC_EXIT,					// 通知线程退出
	WMTMC_SYN,					// 发送方请求连接		S->R
	WMTMC_SYNACK,				// 接收方响应			S<-R
	WMTMC_ACK,					// 发送方确认			S->R
	WMTMC_DATA_REQ,				// 发送方发送数据		S->R
	WMTMC_MSG_REQ,              // 发送方发送消息		S->R        
	WMTMC_DATA_RESP,			// 接收方收到数据		S<-R
	WMTMC_DATA_INVALID,			// 接收方收到无效数据	S<-R
	WMTMC_RECEIVER_CREATED,		// 一个新的接收方创建	S<-R
    WMTMC_RECEIVER_CREATED_2,	// 一个新的接收方创建	S<-R
	WMTMC_RECEIVER_EXIT,		// 接收方退出			S<-R
	WMTMC_FILEMAPPING_CHANGED,  // 发送方内存映射改变   S->R
    WMTMC__END,
};

#define MAX_SENDER_COUNT 100
#define MAX_RECEIVER_COUNT 100
#define MAX_FILEMAPPING_BUFFER (4*1024*1024)

//////////////////////////////////////////////////////////////////////////
// 线程状态
enum
{
	SYN_SEND,
	SYN_RECV,
	ESTABLISHED,
	CLOSED,
	BROKEN,
};