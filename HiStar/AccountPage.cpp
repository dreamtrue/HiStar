// AccountPage.cpp : 实现文件
//

#include "stdafx.h"
#include "HiStar.h"
#include "AccountPage.h"
#include "afxdialogex.h"
#include "UserMsg.h"
extern double AvailCtp;
// CAccountPage 对话框

IMPLEMENT_DYNAMIC(CAccountPage, CDialogEx)

CAccountPage::CAccountPage(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAccountPage::IDD, pParent)
{

}

CAccountPage::~CAccountPage()
{
}

void CAccountPage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_AccountCtp);
}


BEGIN_MESSAGE_MAP(CAccountPage, CDialogEx)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDOK, &CAccountPage::OnOK)
	ON_BN_CLICKED(IDCANCEL, &CAccountPage::OnCancel)
END_MESSAGE_MAP()


// CAccountPage 消息处理程序

void CAccountPage::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
}

void CAccountPage::OnOK(void)
{
	//屏蔽掉对OK的响应
}
void CAccountPage::OnCancel(){
}


void CAccountPage::UpdateAccountPage(void)
{
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	if(pApp && pApp->m_cT){
		AcquireSRWLockShared(&g_srwLock_TradingAccount);
		CString szCPft,szPPft,szTdFee;
		szTdFee.Format(_T("%d"),D2Int(pApp->m_cT->TradingAccount.Commission)); outStrAs4(szTdFee);
		szCPft.Format(_T("%d"),D2Int(pApp->m_cT->TradingAccount.CloseProfit)); outStrAs4(szCPft);
		szPPft.Format(_T("%d"),D2Int(pApp->m_cT->TradingAccount.PositionProfit)); outStrAs4(szPPft);

		CString szLine = _T(""),szTemp = _T("");

		szLine += FormatLine(_T(""),_T(""),_T("="),42);

		szTemp.Format(_T("%.02lf"),pApp->m_cT->TradingAccount.PreBalance); outStrAs4(szTemp);
		szLine += FormatLine(_T("  上次结算准备金:"),szTemp,_T(" "),40);

		szTemp.Format(_T("%.02lf"),pApp->m_cT->TradingAccount.PreCredit); outStrAs4(szTemp);
		szLine += FormatLine(_T("- 上次信用额度:"),szTemp,_T(" "),40);

		szTemp.Format(_T("%.02lf"),pApp->m_cT->TradingAccount.PreMortgage); outStrAs4(szTemp);
		szLine += FormatLine(_T("- 上次质押金额:"),szTemp,_T(" "),40);

		szTemp.Format(_T("%.02lf"),pApp->m_cT->TradingAccount.Mortgage); outStrAs4(szTemp);
		szLine += FormatLine(_T("+ 质押金额:"),szTemp,_T(" "),41);

		szTemp.Format(_T("%.02lf"),pApp->m_cT->TradingAccount.Withdraw); outStrAs4(szTemp);
		szLine += FormatLine(_T("- 今日出金:"),szTemp,_T(" "),41);

		szTemp.Format(_T("%.02lf"),pApp->m_cT->TradingAccount.Deposit); outStrAs4(szTemp);
		szLine += FormatLine(_T("+ 今日入金:"),szTemp,_T(" "),41);

		szLine += FormatLine(_T(""),_T(""),_T("-"),42);

		double dStatProf = pApp->m_cT->TradingAccount.PreBalance - pApp->m_cT->TradingAccount.PreCredit
			- pApp->m_cT->TradingAccount.PreMortgage + pApp->m_cT->TradingAccount.Mortgage
			- pApp->m_cT->TradingAccount.Withdraw + pApp->m_cT->TradingAccount.Deposit;
		szTemp.Format(_T("%.02lf"),dStatProf); outStrAs4(szTemp);
		szLine += FormatLine(_T("= 静态权益:"),szTemp,_T(" "),41);

		szTemp.Format(_T("%.02lf"),pApp->m_cT->TradingAccount.CloseProfit); outStrAs4(szTemp);
		szLine += FormatLine(_T("+ 平仓盈亏:"),szTemp,_T(" "),41);

		szTemp.Format(_T("%.02lf"),pApp->m_cT->TradingAccount.PositionProfit); outStrAs4(szTemp);
		szLine += FormatLine(_T("+ 持仓盈亏:"),szTemp,_T(" "),41);

		szTemp.Format(_T("%.02lf"),pApp->m_cT->TradingAccount.Commission); outStrAs4(szTemp);
		szLine += FormatLine(_T("- 手续费:"),szTemp,_T(" "),42);

		szLine += FormatLine(_T(""),_T(""),_T("-"),42);

		double dDymProf = dStatProf+pApp->m_cT->TradingAccount.CloseProfit+pApp->m_cT->TradingAccount.PositionProfit-pApp->m_cT->TradingAccount.Commission;
		szTemp.Format(_T("%.02lf"),dDymProf); outStrAs4(szTemp);
		szLine += FormatLine(_T("= 动态权益:"),szTemp,_T(" "),41);

		szTemp.Format(_T("%.02lf"),pApp->m_cT->TradingAccount.CurrMargin); outStrAs4(szTemp);
		szLine += FormatLine(_T("- 占用保证金:"),szTemp,_T(" "),41);

		szTemp.Format(_T("%.02lf"),pApp->m_cT->TradingAccount.FrozenMargin); outStrAs4(szTemp);
		szLine += FormatLine(_T("- 冻结保证金:"),szTemp,_T(" "),41);

		szTemp.Format(_T("%.02lf"),pApp->m_cT->TradingAccount.FrozenCommission); outStrAs4(szTemp);
		szLine += FormatLine(_T("- 冻结手续费:"),szTemp,_T(" "),41);

		szTemp.Format(_T("%.02lf"),pApp->m_cT->TradingAccount.DeliveryMargin); outStrAs4(szTemp);
		szLine += FormatLine(_T("- 交割保证金:"),szTemp,_T(" "),41);

		szTemp.Format(_T("%.02lf"),pApp->m_cT->TradingAccount.Credit); outStrAs4(szTemp);
		szLine += FormatLine(_T("+ 信用金额:"),szTemp,_T(" "),41);

		szLine += FormatLine(_T(""),_T(""),_T("-"),42);

		double dValidProf = dDymProf-pApp->m_cT->TradingAccount.CurrMargin-pApp->m_cT->TradingAccount.FrozenMargin-pApp->m_cT->TradingAccount.FrozenCommission-pApp->m_cT->TradingAccount.DeliveryMargin+pApp->m_cT->TradingAccount.Credit;
		szTemp.Format(_T("%.02lf"),dValidProf); outStrAs4(szTemp);
		szLine += FormatLine(_T("= 可用金额:"),szTemp,_T(" "),41);
		AvailCtp = dValidProf;

		szLine += FormatLine(_T(""),_T(""),_T("="),42);
		szLine += FormatLine(_T(""),_T(""),_T("="),42);

		szTemp.Format(_T("%.02lf"),pApp->m_cT->TradingAccount.Reserve); outStrAs4(szTemp);
		szLine += FormatLine(_T("  保底资金:"),szTemp,_T(" "),41);

		szTemp.Format(_T("%.02lf"),pApp->m_cT->TradingAccount.WithdrawQuota); outStrAs4(szTemp);
		szLine += FormatLine(_T("  可取资金:"),szTemp,_T(" "),41);

		szLine += FormatLine(_T(""),_T(""),_T("="),42);
		ReleaseSRWLockShared(&g_srwLock_TradingAccount);
		m_AccountCtp.SetWindowTextA(szLine);
	}
}
