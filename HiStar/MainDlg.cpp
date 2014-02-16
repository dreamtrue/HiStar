// MainDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HiStar.h"
#include "MainDlg.h"
#include "afxdialogex.h"
#include "UserMsg.h"
extern BOOL g_bLoginCtpT;
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
	ON_MESSAGE(WM_UPDATE_ACC_CTP,&CMainDlg::OnUpdateAccCtp)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDOK, &CMainDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CMainDlg::OnKillTimer)
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
	m_pDialog[0] = &m_pageOpera;
	m_pDialog[1] = &m_AccountPage;
	//显示初始页面
	m_pDialog[0]->ShowWindow(SW_SHOW);
	m_pDialog[1]->ShowWindow(false);
	m_tab.SetCurSel(0);
	//保存当前选择
	m_CurSelTab = 0;
	//设定定时器
	m_timerID = SetTimer(1,10000,NULL);
	return 0;
}

void CMainDlg::OnTcnSelchangingTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	int CurSel = m_tab.GetCurSel();
	switch(CurSel)
	{
	case 0:
		m_CurSelTab = 0;
		m_AccountPage.ShowWindow(true);
		m_pageOpera.ShowWindow(false);
		break;
	case 1:
		m_CurSelTab = 1;
		m_pageOpera.ShowWindow(true);
		m_AccountPage.ShowWindow(false);
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


void CMainDlg::OnTimer(UINT_PTR nIDEvent)
{
	//账户登录了才能查询信息
	PostThreadMessage(GetCurrentThreadId(),WM_QRY_ACC_CTP,NULL,NULL);
	CDialogEx::OnTimer(nIDEvent);
}


void CMainDlg::OnBnClickedOk()
{
	CDialogEx::OnOK();
}


afx_msg LRESULT CMainDlg::OnUpdateAccCtp(WPARAM wParam, LPARAM lParam)
{
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	memcpy(pApp->m_accountCtp.m_pTdAcc,(CThostFtdcTradingAccountField*)lParam,sizeof(CThostFtdcTradingAccountField));
	delete (CThostFtdcTradingAccountField*)lParam;
	CString szCPft,szPPft,szTdFee;
	szTdFee.Format(_T("%d"),D2Int(pApp->m_accountCtp.m_pTdAcc->Commission)); outStrAs4(szTdFee);
	szCPft.Format(_T("%d"),D2Int(pApp->m_accountCtp.m_pTdAcc->CloseProfit)); outStrAs4(szCPft);
	szPPft.Format(_T("%d"),D2Int(pApp->m_accountCtp.m_pTdAcc->PositionProfit)); outStrAs4(szPPft);

	CString szLine = _T(""),szTemp = _T("");

	szLine += FormatLine(_T(""),_T(""),_T("="),42);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->PreBalance); outStrAs4(szTemp);
	szLine += FormatLine(_T("  上次结算准备金:"),szTemp,_T(" "),40);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->PreCredit); outStrAs4(szTemp);
	szLine += FormatLine(_T("- 上次信用额度:"),szTemp,_T(" "),40);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->PreMortgage); outStrAs4(szTemp);
	szLine += FormatLine(_T("- 上次质押金额:"),szTemp,_T(" "),40);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->Mortgage); outStrAs4(szTemp);
	szLine += FormatLine(_T("+ 质押金额:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->Withdraw); outStrAs4(szTemp);
	szLine += FormatLine(_T("- 今日出金:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->Deposit); outStrAs4(szTemp);
	szLine += FormatLine(_T("+ 今日入金:"),szTemp,_T(" "),41);

	szLine += FormatLine(_T(""),_T(""),_T("-"),42);

	double dStatProf = pApp->m_accountCtp.m_pTdAcc->PreBalance - pApp->m_accountCtp.m_pTdAcc->PreCredit
		- pApp->m_accountCtp.m_pTdAcc->PreMortgage + pApp->m_accountCtp.m_pTdAcc->Mortgage
		- pApp->m_accountCtp.m_pTdAcc->Withdraw + pApp->m_accountCtp.m_pTdAcc->Deposit;
	szTemp.Format(_T("%.2f"),dStatProf); outStrAs4(szTemp);
	szLine += FormatLine(_T("= 静态权益:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->CloseProfit); outStrAs4(szTemp);
	szLine += FormatLine(_T("+ 平仓盈亏:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->PositionProfit); outStrAs4(szTemp);
	szLine += FormatLine(_T("+ 持仓盈亏:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->Commission); outStrAs4(szTemp);
	szLine += FormatLine(_T("- 手续费:"),szTemp,_T(" "),42);

	szLine += FormatLine(_T(""),_T(""),_T("-"),42);

	double dDymProf = dStatProf+pApp->m_accountCtp.m_pTdAcc->CloseProfit+pApp->m_accountCtp.m_pTdAcc->PositionProfit-pApp->m_accountCtp.m_pTdAcc->Commission;
	szTemp.Format(_T("%.2f"),dDymProf); outStrAs4(szTemp);
	szLine += FormatLine(_T("= 动态权益:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->CurrMargin); outStrAs4(szTemp);
	szLine += FormatLine(_T("- 占用保证金:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->FrozenMargin); outStrAs4(szTemp);
	szLine += FormatLine(_T("- 冻结保证金:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->FrozenCommission); outStrAs4(szTemp);
	szLine += FormatLine(_T("- 冻结手续费:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->DeliveryMargin); outStrAs4(szTemp);
	szLine += FormatLine(_T("- 交割保证金:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->Credit); outStrAs4(szTemp);
	szLine += FormatLine(_T("+ 信用金额:"),szTemp,_T(" "),41);

	szLine += FormatLine(_T(""),_T(""),_T("-"),42);

	double dValidProf = dDymProf-pApp->m_accountCtp.m_pTdAcc->CurrMargin-pApp->m_accountCtp.m_pTdAcc->FrozenMargin-pApp->m_accountCtp.m_pTdAcc->FrozenCommission-pApp->m_accountCtp.m_pTdAcc->DeliveryMargin+pApp->m_accountCtp.m_pTdAcc->Credit;
	szTemp.Format(_T("%.2f"),dValidProf); outStrAs4(szTemp);
	szLine += FormatLine(_T("= 可用金额:"),szTemp,_T(" "),41);

	szLine += FormatLine(_T(""),_T(""),_T("="),42);
	szLine += FormatLine(_T(""),_T(""),_T("="),42);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->Reserve); outStrAs4(szTemp);
	szLine += FormatLine(_T("  保底资金:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->WithdrawQuota); outStrAs4(szTemp);
	szLine += FormatLine(_T("  可取资金:"),szTemp,_T(" "),41);

	szLine += FormatLine(_T(""),_T(""),_T("="),42);

	HWND hEdit = ::GetDlgItem(m_AccountPage.m_hWnd,IDC_EDIT1);
	::SendMessage(hEdit,WM_SETTEXT,0,(LPARAM)(LPCTSTR)szLine);
	return 0;
}

void CMainDlg::OnKillTimer()
{
	KillTimer(1);
}
