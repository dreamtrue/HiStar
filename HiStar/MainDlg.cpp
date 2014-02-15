// MainDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HiStar.h"
#include "MainDlg.h"
#include "afxdialogex.h"
#include "UserMsg.h"
// CMainDlg 对话框

IMPLEMENT_DYNAMIC(CMainDlg, CDialogEx)

CMainDlg::CMainDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMainDlg::IDD, pParent)
{

}

CMainDlg::~CMainDlg()
{
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, m_tab);
}


BEGIN_MESSAGE_MAP(CMainDlg, CDialogEx)
	ON_NOTIFY(TCN_SELCHANGING, IDC_TAB, &CMainDlg::OnTcnSelchangingTab)
	ON_MESSAGE(WM_ORDER_STATUS, &CMainDlg::OnOrderStatus)
	ON_MESSAGE(WM_ERRORS, &CMainDlg::OnErrors)
END_MESSAGE_MAP()


// CMainDlg 消息处理程序


BOOL CMainDlg::OnInitDialog(void)
{
	CDialogEx::OnInitDialog();
	//额外初始化代码
	m_tab.InsertItem(0, _T("操作"));
	m_tab.InsertItem(1, _T("账户"));
	//创建两个对话框
	m_AccountPage.Create(IDD_ACCOUNT_PAGE, &m_tab);
	m_pageOpera.Create(IDD_OPERA_PAGE, &m_tab);
	//设定在Tab内显示的范围
	CRect rc;
	m_tab.GetClientRect(rc);
	rc.top += 20;
	rc.bottom -= 0;
	rc.left += 0;
	rc.right -= 0;
	m_pageOpera.MoveWindow(&rc);
	m_AccountPage.MoveWindow(&rc);
	//把对话框对象指针保存起来
	pDialog[0] = &m_pageOpera;
	pDialog[1] = &m_AccountPage;
	//显示初始页面
	pDialog[0]->ShowWindow(SW_SHOW);
	pDialog[1]->ShowWindow(false);
	m_tab.SetCurSel(0);
	//保存当前选择
	m_CurSelTab = 0;

	return 0;
}

void CMainDlg::OnTcnSelchangingTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	int CurSel = m_tab.GetCurSel();
	switch(CurSel)
	{
	case 0:
		m_AccountPage.ShowWindow(true);
		m_pageOpera.ShowWindow(false);
		m_CurSelTab = 0;
		break;
	case 1:
		m_pageOpera.ShowWindow(true);
		m_AccountPage.ShowWindow(false);
		m_CurSelTab = 1;
		break;
	}
	*pResult = 0;
}

afx_msg LRESULT CMainDlg::OnOrderStatus(WPARAM wParam, LPARAM lParam)
{
	int i = m_pageOpera.m_orderStatus.AddString(*(CString *)wParam);
	int top = i - N < 0 ? 0 : i - N;
	m_pageOpera.m_orderStatus.SetTopIndex(top);
	delete (CString*)wParam;
	return 0;
}
afx_msg LRESULT CMainDlg::OnErrors(WPARAM wParam, LPARAM lParam)
{
	int i = m_pageOpera.m_errors.AddString(*(CString *)wParam);
	int top = i - N < 0 ? 0 : i - N;
	m_pageOpera.m_errors.SetTopIndex(top);
	delete (CString*)wParam;
	return 0;
}
