// MainDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "HiStar.h"
#include "MainDlg.h"
#include "afxdialogex.h"
#include "UserMsg.h"
extern BOOL g_bLoginCtpT;
// CMainDlg �Ի���

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
END_MESSAGE_MAP()


// CMainDlg ��Ϣ�������


BOOL CMainDlg::OnInitDialog(void)
{
	CDialogEx::OnInitDialog();
	//�����ʼ������
	m_tab.InsertItem(0, _T("����"));
	m_tab.InsertItem(1, _T("�˻�"));
	//���������Ի���
	m_AccountPage.Create(IDD_ACCOUNT_PAGE, &m_tab);
	m_operaPage.Create(IDD_OPERA_PAGE, &m_tab);
	//�趨��Tab����ʾ�ķ�Χ
	CRect rc;
	m_tab.GetClientRect(rc);
	rc.top += 20;
	rc.bottom -= 0;
	rc.left += 0;
	rc.right -= 0;
	m_operaPage.MoveWindow(&rc);
	m_AccountPage.MoveWindow(&rc);
	//�ѶԻ������ָ�뱣������
	m_pDialog[0] = &m_operaPage;
	m_pDialog[1] = &m_AccountPage;
	//��ʾ��ʼҳ��
	m_pDialog[0]->ShowWindow(SW_SHOW);
	m_pDialog[1]->ShowWindow(false);
	m_tab.SetCurSel(0);
	//���浱ǰѡ��
	m_CurSelTab = 0;
	//�趨��ʱ��
	m_timerID = SetTimer(1,10000,NULL);
	return 0;
}

void CMainDlg::OnTcnSelchangingTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	int CurSel = m_tab.GetCurSel();
	switch(CurSel)
	{
	case 0:
		m_CurSelTab = 0;
		m_AccountPage.ShowWindow(true);
		m_operaPage.ShowWindow(false);
		break;
	case 1:
		m_CurSelTab = 1;
		m_operaPage.ShowWindow(true);
		m_AccountPage.ShowWindow(false);
		break;
	}
	*pResult = 0;
}

afx_msg LRESULT CMainDlg::OnOrderStatus(WPARAM wParam, LPARAM lParam)
{
	int i = m_operaPage.m_orderStatus.AddString(*(CString *)wParam);
	int top = i - N < 0 ? 0 : i - N;
	m_operaPage.m_orderStatus.SetTopIndex(top);
	delete (CString*)wParam;
	return 0;
}
afx_msg LRESULT CMainDlg::OnErrors(WPARAM wParam, LPARAM lParam)
{
	int i = m_operaPage.m_errors.AddString(*(CString *)wParam);
	int top = i - N < 0 ? 0 : i - N;
	m_operaPage.m_errors.SetTopIndex(top);
	delete (CString*)wParam;
	return 0;
}


void CMainDlg::OnTimer(UINT_PTR nIDEvent)
{
	//�˻���¼�˲��ܲ�ѯ��Ϣ
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
	szLine += FormatLine(_T("  �ϴν���׼����:"),szTemp,_T(" "),40);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->PreCredit); outStrAs4(szTemp);
	szLine += FormatLine(_T("- �ϴ����ö��:"),szTemp,_T(" "),40);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->PreMortgage); outStrAs4(szTemp);
	szLine += FormatLine(_T("- �ϴ���Ѻ���:"),szTemp,_T(" "),40);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->Mortgage); outStrAs4(szTemp);
	szLine += FormatLine(_T("+ ��Ѻ���:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->Withdraw); outStrAs4(szTemp);
	szLine += FormatLine(_T("- ���ճ���:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->Deposit); outStrAs4(szTemp);
	szLine += FormatLine(_T("+ �������:"),szTemp,_T(" "),41);

	szLine += FormatLine(_T(""),_T(""),_T("-"),42);

	double dStatProf = pApp->m_accountCtp.m_pTdAcc->PreBalance - pApp->m_accountCtp.m_pTdAcc->PreCredit
		- pApp->m_accountCtp.m_pTdAcc->PreMortgage + pApp->m_accountCtp.m_pTdAcc->Mortgage
		- pApp->m_accountCtp.m_pTdAcc->Withdraw + pApp->m_accountCtp.m_pTdAcc->Deposit;
	szTemp.Format(_T("%.2f"),dStatProf); outStrAs4(szTemp);
	szLine += FormatLine(_T("= ��̬Ȩ��:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->CloseProfit); outStrAs4(szTemp);
	szLine += FormatLine(_T("+ ƽ��ӯ��:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->PositionProfit); outStrAs4(szTemp);
	szLine += FormatLine(_T("+ �ֲ�ӯ��:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->Commission); outStrAs4(szTemp);
	szLine += FormatLine(_T("- ������:"),szTemp,_T(" "),42);

	szLine += FormatLine(_T(""),_T(""),_T("-"),42);

	double dDymProf = dStatProf+pApp->m_accountCtp.m_pTdAcc->CloseProfit+pApp->m_accountCtp.m_pTdAcc->PositionProfit-pApp->m_accountCtp.m_pTdAcc->Commission;
	szTemp.Format(_T("%.2f"),dDymProf); outStrAs4(szTemp);
	szLine += FormatLine(_T("= ��̬Ȩ��:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->CurrMargin); outStrAs4(szTemp);
	szLine += FormatLine(_T("- ռ�ñ�֤��:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->FrozenMargin); outStrAs4(szTemp);
	szLine += FormatLine(_T("- ���ᱣ֤��:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->FrozenCommission); outStrAs4(szTemp);
	szLine += FormatLine(_T("- ����������:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->DeliveryMargin); outStrAs4(szTemp);
	szLine += FormatLine(_T("- ���֤��:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->Credit); outStrAs4(szTemp);
	szLine += FormatLine(_T("+ ���ý��:"),szTemp,_T(" "),41);

	szLine += FormatLine(_T(""),_T(""),_T("-"),42);

	double dValidProf = dDymProf-pApp->m_accountCtp.m_pTdAcc->CurrMargin-pApp->m_accountCtp.m_pTdAcc->FrozenMargin-pApp->m_accountCtp.m_pTdAcc->FrozenCommission-pApp->m_accountCtp.m_pTdAcc->DeliveryMargin+pApp->m_accountCtp.m_pTdAcc->Credit;
	szTemp.Format(_T("%.2f"),dValidProf); outStrAs4(szTemp);
	szLine += FormatLine(_T("= ���ý��:"),szTemp,_T(" "),41);

	szLine += FormatLine(_T(""),_T(""),_T("="),42);
	szLine += FormatLine(_T(""),_T(""),_T("="),42);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->Reserve); outStrAs4(szTemp);
	szLine += FormatLine(_T("  �����ʽ�:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),pApp->m_accountCtp.m_pTdAcc->WithdrawQuota); outStrAs4(szTemp);
	szLine += FormatLine(_T("  ��ȡ�ʽ�:"),szTemp,_T(" "),41);

	szLine += FormatLine(_T(""),_T(""),_T("="),42);

	HWND hEdit = ::GetDlgItem(m_AccountPage.m_hWnd,IDC_EDIT1);
	::SendMessage(hEdit,WM_SETTEXT,0,(LPARAM)(LPCTSTR)szLine);
	return 0;
}

void CMainDlg::addCombInst(void)
{
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	BOOL bRes = FALSE;
	for (UINT i=0; i < pApp->m_cT->m_InsinfVec.size();i++)
	{
		if(pApp->m_cT->m_InsinfVec[i]->iinf.InstrumentID[0] == 'I' 
			&& pApp->m_cT->m_InsinfVec[i]->iinf.InstrumentID[1] == 'F'){
				m_operaPage.m_CombInst.AddString(CString(pApp->m_cT->m_InsinfVec[i]->iinf.InstrumentID));
				m_operaPage.m_CombInst.SetWindowText(CString(pApp->m_cT->m_InsinfVec[i]->iinf.InstrumentID));
		}
	}
}
