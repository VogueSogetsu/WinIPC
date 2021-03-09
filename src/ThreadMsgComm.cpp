/******************************************************************************
 文件名称： ThreadMsgComm.cpp

 版本序号： V1.0
  
 作    者： DRACULAX
	
 创建日期： 2007-12-6
	  
 文件描述： 
		
 成员列表： 
		  
 修改历史：
            ---日期---           ---作者---           ---修改---
            2007-12-6            DRACULAX              创建文件
******************************************************************************/

#include "stdafx.h"
#include "ThreadMsgComm.h"

/*
 *=============================================================================
 *  CBaseMsgCommThread
 *=============================================================================
 */

/******************************************************************************
 函数名： CBaseMsgCommThread
 输  入： 无
 输  出： 无
 返回值： 无
 功  能： 构造函数
 说  明： 
******************************************************************************/
CBaseMsgCommThread::CBaseMsgCommThread(CBaseDefaultMsgListener* pListener /* = NULL */)
: m_dwThreadID(0)
, m_hThread(NULL)
, m_evStart(NULL)
, m_pDefaultMsgListener(pListener)
{

}


/******************************************************************************
 函数名： ~CUploadCommThread
 输  入： 无
 输  出： 无
 返回值： 无
 功  能： 析构函数
 说  明： 
******************************************************************************/
CBaseMsgCommThread::~CBaseMsgCommThread()
{
	if(m_hThread)
	{
		// 通知线程退出
		this->PostSelfMessage(WMTMC_EXIT);
		
		// 等待5s
// 		if(WaitForSingleObject(m_hThread, 5000) != WAIT_OBJECT_0)
// 		{
// 			TerminateThread(m_hThread, 0);
// 		}
		
		CloseHandle(m_hThread);
		m_dwThreadID = 0;		
	}	
}


/******************************************************************************
 函数名： InitInstance
 输  入： 无
 输  出： 无
 返回值： 成功返回TRUE
 功  能： 初始化线程
 说  明： 
******************************************************************************/
BOOL CBaseMsgCommThread::InitInstance()
{
	return TRUE;
}


/******************************************************************************
 函数名： ExitInstance
 输  入： 无
 输  出： 无
 返回值： 无
 功  能： 清理线程资源
 说  明： 
******************************************************************************/
INT CBaseMsgCommThread::ExitInstance()
{	
	return 0;	
}


/******************************************************************************
 函数名： CreateMsgCommThread
 输  入： 无
 输  出： 无
 返回值： 成功返回TRUE
 功  能： 创建消息循环线程
 说  明： 
******************************************************************************/
BOOL CBaseMsgCommThread::CreateMsgCommThread()
{
    m_evStart = ::CreateEvent(NULL, FALSE, FALSE, 0);
	HANDLE hThread = ::CreateThread(NULL, 0, CBaseMsgCommThread::MessageLoop, this, 0, &m_dwThreadID);
	
	if(INVALID_HANDLE_VALUE == hThread)
	{
		return FALSE;
	}

    ::WaitForSingleObject(m_evStart, 5000);
    CloseHandle(m_evStart);

    PostSelfMessage(WMTMC_IDLE);	
	return TRUE;
}

BOOL CBaseMsgCommThread::BindMsgCommThread(DWORD dwThreadID)
{
    if(m_dwThreadID != 0)
    {
        PostSelfMessage(WMTMC_EXIT);
        m_dwThreadID = 0;
        CloseHandle(m_hThread);
    }

    m_dwThreadID = dwThreadID;
    return TRUE;
}


/******************************************************************************
函数名： MessageLoop
输  入： 无
输  出： 无
返回值： 无
功  能： 消息循环
说  明： 
******************************************************************************/
DWORD WINAPI CBaseMsgCommThread::MessageLoop(IN LPVOID lpParam)
{
	CBaseMsgCommThread* pThis = (CBaseMsgCommThread*)lpParam;
	
	if(!pThis->InitInstance())
	{
		return -1;
	}
	
	BOOL bGotMsg;
    MSG  msg;
    msg.message = WM_NULL;	    
    
	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);
    ::SetEvent(pThis->m_evStart);

    BOOL bIdle = TRUE;
    LONG lIdleCount = 0;
    
    for(;;)
    {        
        while(bIdle && !::PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE))
        {
            if(!pThis->OnIdle(lIdleCount++))
            {
                bIdle = FALSE;
            }
        }
        
        do
        {            
            bGotMsg = GetMessage(&msg, NULL, 0U, 0U);

            if(bGotMsg)
            {
                if(!pThis->PreTranslateMessage(&msg))
                {
                    pThis->HandleMessage(msg.message, msg.wParam, msg.lParam);
                }
            }
            else
            {
                pThis->ExitInstance();
                return 0;
            }
            
            if(msg.message <= WMTMC__BEGIN || msg.message >= WMTMC__END)
            {
                bIdle = TRUE;
                lIdleCount = 0;
            }
        } while(::PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE));
    }
	
	return 0;
}


/******************************************************************************
 函数名： SendSelfMessage
 输  入： 
 输  出： 无
 返回值： 无
 功  能： 向本线程发消息
 说  明： 
******************************************************************************/
VOID CBaseMsgCommThread::PostSelfMessage(UINT msg, 
									     WPARAM wParam /* = 0 */, 
									     LPARAM lParam /* = 0 */)
{
	if(m_dwThreadID)
	{
		BOOL bRet = ::PostThreadMessage(m_dwThreadID, msg, wParam, lParam);
        if(!bRet)
        {
            LOG_ERROR(_T("PostSelfMessage failed! hr=") << GetLastError());
        }
	}
}


/******************************************************************************
 函数名： SendTargetMessage
 输  入： 
 输  出： 无
 返回值： 无
 功  能： 向目标线程发消息
 说  明： 
******************************************************************************/
BOOL CBaseMsgCommThread::PostTargetMessage(DWORD dwTargetID,
										   UINT msg, 
									       WPARAM wParam /* = 0 */, 
									       LPARAM lParam /* = 0 */)
{
	if(dwTargetID)
	{
		BOOL bRet = ::PostThreadMessage(dwTargetID, msg, wParam, lParam);
        if(!bRet)
        {
            LOG_ERROR(_T("PostTargetMessage failed! hr=") << GetLastError());
			return FALSE;
        }
		return TRUE;
	}
	return FALSE;
}

VOID CBaseMsgCommThread::PostTargetMessage(HWND hWnd,
										   UINT msg, 
										   WPARAM wParam /* = 0 */, 
										   LPARAM lParam /* = 0 */)
{
	if(hWnd && IsWindow(hWnd))
	{
		::PostMessage(hWnd, msg, wParam, lParam);
	}
}

//////////////////////////////////////////////////////////////////////////
// 消息循环
BEGIN_THREAD_MSG_MAP(CBaseMsgCommThread)
    ON_THREAD_MSG(WMTMC_EXIT, OnExit)	
    ON_THREAD_MSG(WMTMC_IDLE, OnWinIdle)
END_THREAD_MSG_MAP

BOOL CBaseMsgCommThread::OnExit(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

    LOG_INFO(_T("Thread message loop ready to quit!"));
	PostQuitMessage(0);
	return TRUE;
}

BOOL CBaseMsgCommThread::OnWinIdle(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    return TRUE;
}

BOOL CBaseMsgCommThread::IsThreadValid( DWORD dwThreadID )
{
	HANDLE hTargetThread = ::OpenThread(THREAD_QUERY_INFORMATION, FALSE, dwThreadID);
	if(!hTargetThread)
	{
		LOG_DEBUG(_T("Thread ") << dwThreadID << _T(" not exist"));
		return FALSE;
	}
	CloseHandle(hTargetThread);
	return TRUE;
}

