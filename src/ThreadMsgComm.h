/******************************************************************************
 文件名称： ThreadMsgComm.h

 版本序号： V1.0
  
 作    者： DRACULAX
	
 创建日期： 2007-12-6
	  
 文件描述： 
		
 成员列表： 
		  
 修改历史：
            ---日期---           ---作者---           ---修改---
            2007-12-6             DRACULAX             创建文件
******************************************************************************/

#pragma once

#include "ThreadMsgDefines.h"

//////////////////////////////////////////////////////////////////////////
// 消息定义宏
#define DECLARE_THREAD_MSG_HANDLER \
	public: \
        virtual BOOL HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

#define BEGIN_THREAD_MSG_MAP(className) \
BOOL className::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) \
{ \
	BOOL bRet = FALSE; \
	switch(msg) \
    {

#define ON_THREAD_MSG(msg, func) \
	case msg: \
    { \
	    bRet = func(wParam, lParam); \
	} \
    break;

#define END_THREAD_MSG_MAP \
	default: \
	    if(m_pDefaultMsgListener) \
        { \
            bRet = m_pDefaultMsgListener->OnDefaultMsg(msg, wParam, lParam); \
        } \
        else \
            bRet = FALSE; \
        break; \
	} \
	if(!bRet) \
	{ \
		bRet = __super::HandleMessage(msg, wParam, lParam); \
	} \
	return bRet; \
}

#define THREAD_MSG_HANDLER(func) \
    BOOL func(WPARAM wParam, LPARAM lParam)
// 消息定义宏
//////////////////////////////////////////////////////////////////////////

class CBaseDefaultMsgListener
{
public:
    CBaseDefaultMsgListener() {}
    virtual ~CBaseDefaultMsgListener() {}
    virtual BOOL OnDefaultMsg(UINT msg, WPARAM wParam, LPARAM lParam) { return FALSE; }
};


/************************************************************************/
/* 类    名： CCommThread                                               */
/* 主要成员：                                                           */
/* 功    能： CWinThread API实现                                        */
/* 说    明： 通信线程接口定义                                          */
/************************************************************************/
class CCommThread
{
public:
	virtual BOOL InitInstance()                 PURE;
	virtual INT  ExitInstance()                 { return 0; }
	virtual BOOL OnIdle(LONG lCount)            { return FALSE; }
	virtual BOOL PreTranslateMessage(MSG* pMsg) { return FALSE; }
	virtual INT  Run()                          { return 0; }
	virtual BOOL HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) { return FALSE; }
	virtual ~CCommThread() {}
};

typedef CCommThread* LPCOMMTHREAD;


/************************************************************************/
/* 类    名： CBaseMsgCommThread                                        */
/* 主要成员：                                                           */
/* 功    能：                                                           */
/* 说    明：                                                           */
/************************************************************************/
class CBaseMsgCommThread : public CCommThread
{

public:
	CBaseMsgCommThread(CBaseDefaultMsgListener* pListener = NULL);
	virtual ~CBaseMsgCommThread();

	DECLARE_THREAD_MSG_HANDLER

// CCommThread
public:
	// 初始化线程
	// 重载以获得更多的初始化控制
	virtual BOOL InitInstance();

	// 线程退出前清理资源
	// 重载以获得自定义清理权
	virtual INT ExitInstance();	

// 线程通信
public:
	// 向本线程发消息	
	VOID PostSelfMessage(UINT msg, WPARAM wParam = 0, LPARAM lParam = 0);

	// 向目标线程发消息
	BOOL PostTargetMessage(DWORD dwTargetID, UINT msg, WPARAM wParam = 0, LPARAM lParam = 0);

	// 向目标窗口发消息
	VOID PostTargetMessage(HWND hWnd, UINT msg, WPARAM wParam = 0, LPARAM lParam = 0);

public:
	// 创建线程
	BOOL CreateMsgCommThread();

    // 绑定线程
    BOOL BindMsgCommThread(DWORD dwThreadID);

	// 消息循环
	// 线程主函数
	static DWORD WINAPI MessageLoop(LPVOID lpParam);

    DWORD GetThreadID() const { return m_dwThreadID; }

public:
	static BOOL IsThreadValid(DWORD dwThreadID);

// 消息处理函数
public:
	///////////////////////////////////////////
	// 在这里加入自定义消息处理函数

	THREAD_MSG_HANDLER( OnExit );	
    THREAD_MSG_HANDLER( OnWinIdle );

	///////////////////////////////////////////

protected:
	DWORD          m_dwThreadID;          // 本线程ID
	HANDLE         m_hThread;             // 本线程句柄
    HANDLE         m_evStart;
    CBaseDefaultMsgListener* m_pDefaultMsgListener;

private:
    LOG_CLS_DEC();    
};
