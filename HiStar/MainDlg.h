#pragma once
#include "BasicPage.h"
#include "AccountPage.h"
#include "StatusPage.h"
#include "demodlg.h"
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
	CDemoDlg m_demoPage;
	CDialog* m_pDialog[4];  //用来保存对话框对象指针
protected:
	afx_msg LRESULT OnOrderStatus(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT CMainDlg::OnErrors(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnOk();
public:
	void addCombInst(void);
	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnRefreshMdPane(WPARAM wParam, LPARAM lParam);
	afx_msg void OnKickIdle();
	void UpdateAccount();
	//原先是主线程的消息响应函数,修改为主窗口响应
	afx_msg LRESULT OnConnectIB(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDisconnectIB(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT LoginCtp(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT LogoutCtp(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT UpdateHedgeHold(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateLstCtrl(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReqMshq(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnHedgeLooping(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnConnectSql(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnInitialize(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT LoginCtpTD(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT LoginCtpMD(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSynchronizeMarket(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReqAccountUpdates(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
};
