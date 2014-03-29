#pragma once
#include "BasicPage.h"
#include "AccountPage.h"
#include "StatusPage.h"
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
	CBasicPage m_basicPage;
	CAccountPage m_accountPage;
	CStatusPage m_statusPage;
	CDialog* m_pDialog[3];  //用来保存对话框对象指针
protected:
	afx_msg LRESULT OnOrderStatus(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT CMainDlg::OnErrors(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//定时器
	UINT_PTR m_timerID;
	afx_msg void OnOk();
protected:
	afx_msg LRESULT OnUpdateAccCtp(WPARAM wParam, LPARAM lParam);
public:
	void addCombInst(void);
	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnRefreshMdPane(WPARAM wParam, LPARAM lParam);
};
