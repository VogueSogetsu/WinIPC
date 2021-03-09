// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include <atlstr.h>
#include <atltime.h>
#include "MainDlg.h"

typedef HRESULT (WINAPI *pFn_CreateIPCPeer)(INT nProductID, 
                     BOOL bAsSender,
                     BOOL bAsReceiver,
                     INT nAcceptSenderProductID,
                     INT nAcceptReceiverProductID,
                     BOOL bExitIfSameProductExist,
                     IWinIPCPeer** ppOut);


LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

    LOG_INIT_EX(_T("rs_log4cplus.cfg"));

	m_listboxRecv.Attach(GetDlgItem(IDC_RECV_CONTEXT));
	return TRUE;
}

LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CSimpleDialog<IDD_ABOUTBOX, FALSE> dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add validation code     
	EndDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{    
	EndDialog(wID);
	return 0;
}

LRESULT CMainDlg::OnBnClickedInit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nID = GetDlgItemInt(IDC_ID);
	int nTargetID = GetDlgItemInt(IDC_TARGETID);
	TargetProductIDVec vecIDs;
	vecIDs.push_back(nTargetID);
	CreateIPCPeer(nID, vecIDs, this);
	::EnableWindow(GetDlgItem(IDC_INIT), FALSE);
	return 0;
}

LRESULT CMainDlg::OnBnClickedSend(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(m_spIPCPeer)
	{
		CString sText;
		GetDlgItemText(IDC_EDIT_SEND, sText);

		CStringA saText = CT2A(sText);
		m_spIPCPeer->SendData((LPBYTE)saText.GetBuffer(), saText.GetLength(), VARIANT_FALSE);

		//Send_DataTest();
	}

	return 0;
}

LRESULT CMainDlg::OnBnClickedSendTarget(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: 在此添加控件通知处理程序代码

	if(m_spIPCPeer)
	{
		CString sText;
		GetDlgItemText(IDC_EDIT_SEND, sText);
		CStringA saText = CT2A(sText);

		GetDlgItemText(IDC_TARGETID2, sText);
		int targId = _ttoi(sText);

		return m_spIPCPeer->SendDataToSpecialProduct(targId, (LPBYTE)saText.GetBuffer(), saText.GetLength(), VARIANT_FALSE);
	}
	return 0;
}


HRESULT STDMETHODCALLTYPE CMainDlg::OnReceiveData(  /* [in] */ ULONG lSenderID, /* [in] */ BYTE * lBuffer, /* [in] */ ULONG lSize )
{
	SYSTEMTIME tm;
	GetSystemTime(&tm);

	CString sText;
	CString sBuffer = CA2T((char*)lBuffer);
	sText.Format(_T("[%d][%d-%d-%d %d:%d:%d.%03d](%d) %s"), lSenderID, 
		tm.wYear, tm.wMonth, tm.wDay,
		tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds,
		lSize,
		(LPCTSTR)sBuffer);

	m_listboxRecv.AddString(sText);
	return S_OK;
}



LRESULT CMainDlg::OnBnClickedAddproduct(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(m_spIPCPeer)
	{
		int nID = GetDlgItemInt(IDC_ADDTARGETID);
		m_spIPCPeer->AddReceiverProductID(nID);
		m_spIPCPeer->AddSenderProductID(nID);

		SetDlgItemText(IDC_ADDTARGETID, _T(""));
	}
	return 0;
}

LRESULT CMainDlg::OnBnClickedRemoveproduct(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(m_spIPCPeer)
	{
		int nID = GetDlgItemInt(IDC_REMOVETARGETID);
		m_spIPCPeer->RemoveReceiverProductID(nID);
		m_spIPCPeer->RemoveSenderProductID(nID);

		SetDlgItemText(IDC_REMOVETARGETID, _T(""));
	}
	return 0;
}

int CMainDlg::GenerateCmdID()
{
	static int nCmdID = 0;
	return ++nCmdID;
}

#define PLAYER_CMD(_x_) (#_x_)
#define PlayerCmd_SetProp       PLAYER_CMD(SetProp)


HRESULT STDMETHODCALLTYPE CMainDlg::OnConnect( /* [in] */ VARIANT_BOOL vbIsSender, /* [in] */ ULONG lTargetID )
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CMainDlg::OnReceiverDisconnect( /* [in] */ ULONG lTargetID )
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CMainDlg::OnSendFinish( /* [in] */ ULONG lOffset, /* [in] */ ULONG lTargetID, /* [in] */ LONG lRet )
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CMainDlg::OnReceiveMsg( /* [in] */ ULONG lSenderID, /* [in] */ LONG lParam1, /* [in] */ LONG lParam2 )
{
	return S_OK;
}
