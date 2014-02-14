#pragma once
#include "OperaPage.h"
#include "AccountPage.h"
// CMainDlg 对话框
#include "resource.h"
class CMainDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMainDlg)

public:
	CMainDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMainDlg();

// 对话框数据
	enum { IDD = IDD_MAIN_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog(void);
//控件变量
public:
	CTabCtrl m_tab;
	int m_CurSelTab;
	COperaPage m_pageOpera;
	CAccountPage m_AccountPage;
	CDialog* pDialog[2];  //用来保存对话框对象指针
	afx_msg void OnTcnSelchangingTab(NMHDR *pNMHDR, LRESULT *pResult);
};
