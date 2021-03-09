/******************************************************************************
 �ļ����ƣ� ThreadMsgComm.cpp

 �汾��ţ� V1.0
  
 ��    �ߣ� DRACULAX
	
 �������ڣ� 2007-12-6
	  
 �ļ������� 
		
 ��Ա�б� 
		  
 �޸���ʷ��
            ---����---           ---����---           ---�޸�---
            2007-12-6            DRACULAX              �����ļ�
******************************************************************************/

#include "stdafx.h"
#include "ThreadMsgComm.h"

/*
 *=============================================================================
 *  CBaseMsgCommThread
 *=============================================================================
 */

/******************************************************************************
 �������� CBaseMsgCommThread
 ��  �룺 ��
 ��  ���� ��
 ����ֵ�� ��
 ��  �ܣ� ���캯��
 ˵  ���� 
******************************************************************************/
CBaseMsgCommThread::CBaseMsgCommThread(CBaseDefaultMsgListener* pListener /* = NULL */)
: m_dwThreadID(0)
, m_hThread(NULL)
, m_evStart(NULL)
, m_pDefaultMsgListener(pListener)
{

}


/******************************************************************************
 �������� ~CUploadCommThread
 ��  �룺 ��
 ��  ���� ��
 ����ֵ�� ��
 ��  �ܣ� ��������
 ˵  ���� 
******************************************************************************/
CBaseMsgCommThread::~CBaseMsgCommThread()
{
	if(m_hThread)
	{
		// ֪ͨ�߳��˳�
		this->PostSelfMessage(WMTMC_EXIT);
		
		// �ȴ�5s
// 		if(WaitForSingleObject(m_hThread, 5000) != WAIT_OBJECT_0)
// 		{
// 			TerminateThread(m_hThread, 0);
// 		}
		
		CloseHandle(m_hThread);
		m_dwThreadID = 0;		
	}	
}


/******************************************************************************
 �������� InitInstance
 ��  �룺 ��
 ��  ���� ��
 ����ֵ�� �ɹ�����TRUE
 ��  �ܣ� ��ʼ���߳�
 ˵  ���� 
******************************************************************************/
BOOL CBaseMsgCommThread::InitInstance()
{
	return TRUE;
}


/******************************************************************************
 �������� ExitInstance
 ��  �룺 ��
 ��  ���� ��
 ����ֵ�� ��
 ��  �ܣ� �����߳���Դ
 ˵  ���� 
******************************************************************************/
INT CBaseMsgCommThread::ExitInstance()
{	
	return 0;	
}


/******************************************************************************
 �������� CreateMsgCommThread
 ��  �룺 ��
 ��  ���� ��
 ����ֵ�� �ɹ�����TRUE
 ��  �ܣ� ������Ϣѭ���߳�
 ˵  ���� 
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
�������� MessageLoop
��  �룺 ��
��  ���� ��
����ֵ�� ��
��  �ܣ� ��Ϣѭ��
˵  ���� 
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
 �������� SendSelfMessage
 ��  �룺 
 ��  ���� ��
 ����ֵ�� ��
 ��  �ܣ� ���̷߳���Ϣ
 ˵  ���� 
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
 �������� SendTargetMessage
 ��  �룺 
 ��  ���� ��
 ����ֵ�� ��
 ��  �ܣ� ��Ŀ���̷߳���Ϣ
 ˵  ���� 
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
// ��Ϣѭ��
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

