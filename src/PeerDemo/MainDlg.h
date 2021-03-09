// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlctrls.h>
#include "IPCPeerImpl2.h"
//#include <json\json.h>

class CMainDlg
	: public CDialogImpl<CMainDlg>
	, public CIPCPeerImpl<CMainDlg>
	, public CIPCEvent<CMainDlg>
{
public:
	enum { IDD = IDD_MAINDLG };

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_HANDLER(IDC_INIT, BN_CLICKED, OnBnClickedInit)
		COMMAND_HANDLER(IDC_SEND, BN_CLICKED, OnBnClickedSend)
		COMMAND_HANDLER(IDC_ADDPRODUCT, BN_CLICKED, OnBnClickedAddproduct)
		COMMAND_HANDLER(IDC_REMOVEPRODUCT, BN_CLICKED, OnBnClickedRemoveproduct)
		COMMAND_HANDLER(IDC_SEND2, BN_CLICKED, OnBnClickedSendTarget)
	END_MSG_MAP()


	virtual HRESULT STDMETHODCALLTYPE OnConnect( 
	/* [in] */ VARIANT_BOOL vbIsSender,
	/* [in] */ ULONG lTargetID);

	virtual HRESULT STDMETHODCALLTYPE OnReceiverDisconnect( 
		/* [in] */ ULONG lTargetID);

	virtual HRESULT STDMETHODCALLTYPE OnSendFinish( 
		/* [in] */ ULONG lOffset,
		/* [in] */ ULONG lTargetID,
		/* [in] */ LONG lRet);

	virtual HRESULT STDMETHODCALLTYPE OnReceiveData( 
		/* [in] */ ULONG lSenderID,
		/* [in] */ BYTE *lBuffer,
		/* [in] */ ULONG lSize);

	virtual HRESULT STDMETHODCALLTYPE OnReceiveMsg( 
		/* [in] */ ULONG lSenderID,
		/* [in] */ LONG lParam1,
		/* [in] */ LONG lParam2);


public:
	CListBox				m_listboxRecv;

public:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedInit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSend(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedAddproduct(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedRemoveproduct(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedSendTarget(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	int GenerateCmdID();
	HRESULT Send_DataTest();
};
