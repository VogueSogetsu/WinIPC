/******************************************************************************
 �ļ����ƣ� ThreadMsgComm.h

 �汾��ţ� V1.0
  
 ��    �ߣ� DRACULAX
	
 �������ڣ� 2007-12-6
	  
 �ļ������� 
		
 ��Ա�б� 
		  
 �޸���ʷ��
            ---����---           ---����---           ---�޸�---
            2007-12-6             DRACULAX             �����ļ�
******************************************************************************/

#pragma once

#include "ThreadMsgDefines.h"

//////////////////////////////////////////////////////////////////////////
// ��Ϣ�����
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
// ��Ϣ�����
//////////////////////////////////////////////////////////////////////////

class CBaseDefaultMsgListener
{
public:
    CBaseDefaultMsgListener() {}
    virtual ~CBaseDefaultMsgListener() {}
    virtual BOOL OnDefaultMsg(UINT msg, WPARAM wParam, LPARAM lParam) { return FALSE; }
};


/************************************************************************/
/* ��    ���� CCommThread                                               */
/* ��Ҫ��Ա��                                                           */
/* ��    �ܣ� CWinThread APIʵ��                                        */
/* ˵    ���� ͨ���߳̽ӿڶ���                                          */
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
/* ��    ���� CBaseMsgCommThread                                        */
/* ��Ҫ��Ա��                                                           */
/* ��    �ܣ�                                                           */
/* ˵    ����                                                           */
/************************************************************************/
class CBaseMsgCommThread : public CCommThread
{

public:
	CBaseMsgCommThread(CBaseDefaultMsgListener* pListener = NULL);
	virtual ~CBaseMsgCommThread();

	DECLARE_THREAD_MSG_HANDLER

// CCommThread
public:
	// ��ʼ���߳�
	// �����Ի�ø���ĳ�ʼ������
	virtual BOOL InitInstance();

	// �߳��˳�ǰ������Դ
	// �����Ի���Զ�������Ȩ
	virtual INT ExitInstance();	

// �߳�ͨ��
public:
	// ���̷߳���Ϣ	
	VOID PostSelfMessage(UINT msg, WPARAM wParam = 0, LPARAM lParam = 0);

	// ��Ŀ���̷߳���Ϣ
	BOOL PostTargetMessage(DWORD dwTargetID, UINT msg, WPARAM wParam = 0, LPARAM lParam = 0);

	// ��Ŀ�괰�ڷ���Ϣ
	VOID PostTargetMessage(HWND hWnd, UINT msg, WPARAM wParam = 0, LPARAM lParam = 0);

public:
	// �����߳�
	BOOL CreateMsgCommThread();

    // ���߳�
    BOOL BindMsgCommThread(DWORD dwThreadID);

	// ��Ϣѭ��
	// �߳�������
	static DWORD WINAPI MessageLoop(LPVOID lpParam);

    DWORD GetThreadID() const { return m_dwThreadID; }

public:
	static BOOL IsThreadValid(DWORD dwThreadID);

// ��Ϣ������
public:
	///////////////////////////////////////////
	// ����������Զ�����Ϣ������

	THREAD_MSG_HANDLER( OnExit );	
    THREAD_MSG_HANDLER( OnWinIdle );

	///////////////////////////////////////////

protected:
	DWORD          m_dwThreadID;          // ���߳�ID
	HANDLE         m_hThread;             // ���߳̾��
    HANDLE         m_evStart;
    CBaseDefaultMsgListener* m_pDefaultMsgListener;

private:
    LOG_CLS_DEC();    
};
