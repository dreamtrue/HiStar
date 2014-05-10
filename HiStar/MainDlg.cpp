// MainDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "HiStar.h"
#include "MainDlg.h"
#include "afxdialogex.h"
#include "UserMsg.h"
extern BOOL g_bLoginCtpT;
// CMainDlg �Ի���
extern double AvailCtp;
IMPLEMENT_DYNAMIC(CMainDlg, CDialogEx)

CMainDlg::CMainDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMainDlg::IDD, pParent)
{
}

CMainDlg::~CMainDlg()
{
	if(((CHiStarApp*)AfxGetApp())->m_pIBClient){
		((CHiStarApp*)AfxGetApp())->m_pIBClient->eDisconnect();
		delete ((CHiStarApp*)AfxGetApp())->m_pIBClient;
		((CHiStarApp*)AfxGetApp())->m_pIBClient = NULL;
	}
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, m_tab);
}


BEGIN_MESSAGE_MAP(CMainDlg, CDialogEx)
	ON_MESSAGE(WM_ORDER_STATUS, &CMainDlg::OnOrderStatus)
	ON_MESSAGE(WM_ERRORS, &CMainDlg::OnErrors)
	ON_MESSAGE_VOID(WM_KICKIDLE,OnKickIdle)
	ON_MESSAGE(WM_CONNECT_IB,OnConnectIB)
	ON_MESSAGE(WM_DISCONNECT_IB,OnDisconnectIB)
	ON_MESSAGE(WM_LOGIN_CTP,LoginCtp)
	ON_MESSAGE(WM_LOGOUT_CTP,LogoutCtp)
	ON_MESSAGE(WM_UPDATE_HEDGEHOLD,UpdateHedgeHold)
	ON_MESSAGE(WM_UPDATE_LSTCTRL,OnUpdateLstCtrl)
	ON_MESSAGE(WM_REQ_MSHQ,OnReqMshq)
	ON_MESSAGE(WM_MD_REFRESH,OnHedgeLooping)
	ON_MESSAGE(WM_CONNECT_SQL,OnConnectSql)
	ON_MESSAGE(WM_INI,OnInitialize)
	ON_MESSAGE(WM_LOGIN_TD,LoginCtpTD)
	ON_MESSAGE(WM_LOGIN_MD,LoginCtpMD)
	ON_MESSAGE(WM_SYNCHRONIZE_MARKET,OnSynchronizeMarket)
	ON_MESSAGE(WM_REQACCOUNTUPDATES,OnReqAccountUpdates)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDOK, &CMainDlg::OnOk)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CMainDlg::OnTcnSelchangeTab)
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()


// CMainDlg ��Ϣ�������


BOOL CMainDlg::OnInitDialog(void)
{
	CDialogEx::OnInitDialog();
	//�����ʼ������
	m_tab.InsertItem(0, _T("����"));
	m_tab.InsertItem(1, _T("�˻�"));
	m_tab.InsertItem(2,_T("״̬"));
	//���������Ի���
	m_accountPage.Create(IDD_ACCOUNT_PAGE, &m_tab);
	m_basicPage.Create(IDD_BASIC_PAGE, &m_tab);
	m_statusPage.Create(IDD_STATUS_PAGE, &m_tab);

	//�趨��Tab����ʾ�ķ�Χ
	CRect rc;
	m_tab.GetClientRect(rc);
	rc.top += 20;
	rc.bottom -= 0;
	rc.left += 0;
	rc.right -= 0;
	m_basicPage.MoveWindow(&rc);
	m_accountPage.MoveWindow(&rc);
	m_statusPage.MoveWindow(&rc);
	//�ѶԻ������ָ�뱣������
	m_pDialog[0] = &m_basicPage;
	m_pDialog[1] = &m_accountPage;
	m_pDialog[2] = &m_statusPage;
	//��ʾ��ʼҳ��
	m_pDialog[0]->ShowWindow(SW_SHOW);
	m_pDialog[1]->ShowWindow(false);
	m_pDialog[2]->ShowWindow(false);
	m_tab.SetCurSel(0);
	//���浱ǰѡ��
	m_CurSelTab = 0;
	return 0;
}

afx_msg LRESULT CMainDlg::OnOrderStatus(WPARAM wParam, LPARAM lParam)
{
	int i = m_basicPage.m_orderStatus.AddString(*(CString *)wParam);
	int top = i - N < 0 ? 0 : i - N;
	m_basicPage.m_orderStatus.SetTopIndex(top);
	delete (CString*)wParam;
	return 0;
}
afx_msg LRESULT CMainDlg::OnErrors(WPARAM wParam, LPARAM lParam)
{
	int i = m_basicPage.m_orderStatus.AddString(*(CString *)wParam);
	int top = i - N < 0 ? 0 : i - N;
	m_basicPage.m_orderStatus.SetTopIndex(top);
	delete (CString*)wParam;
	return 0;
}

void CMainDlg::OnOk()
{
	CDialogEx::OnOK();
}

void CMainDlg::addCombInst(void)
{
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	BOOL bRes = FALSE;
	m_basicPage.m_CombInst.ResetContent();//��ֵ�������
	AcquireSRWLockShared(&g_srwLock_Insinf);
	for (UINT i=0; i < pApp->m_cT->m_InsinfVec.size();i++)
	{
		if(pApp->m_cT->m_InsinfVec[i].iinf.InstrumentID[0] == 'I' 
			&& pApp->m_cT->m_InsinfVec[i].iinf.InstrumentID[1] == 'F'){
				m_basicPage.m_CombInst.AddString(CString(pApp->m_cT->m_InsinfVec[i].iinf.InstrumentID));
				m_basicPage.m_CombInst.SetWindowText(CString(pApp->m_cT->m_InsinfVec[i].iinf.InstrumentID));
		}
	}
	ReleaseSRWLockShared(&g_srwLock_Insinf);
}

void CMainDlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	int CurSel = m_tab.GetCurSel();
	switch(CurSel)
	{
	case 0:
		m_CurSelTab = 0;
		m_basicPage.ShowWindow(true);
		m_accountPage.ShowWindow(false);
		m_statusPage.ShowWindow(false);
		break;
	case 1:
		m_CurSelTab = 1;
		m_accountPage.ShowWindow(true);
		m_basicPage.ShowWindow(false);
		m_statusPage.ShowWindow(false);
		break;
	case 2:
		m_CurSelTab = 2;
		m_statusPage.ShowWindow(true);
		m_accountPage.ShowWindow(false);
		m_basicPage.ShowWindow(false);
	}
	*pResult = 0;
}

afx_msg LRESULT CMainDlg::OnRefreshMdPane(WPARAM wParam, LPARAM lParam)
{
	m_basicPage.RefreshMdPane();
	m_accountPage.UpdateAccountPage();
	return 0;
}

void CMainDlg::OnKickIdle()
{

}

#define FILL_MESSAGE_FUNCTION(OnMessage) afx_msg LRESULT CMainDlg::##OnMessage##(WPARAM wParam, LPARAM lParam){\
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();\
	if(pApp){\
	pApp->##OnMessage##(wParam,lParam);\
	}\
	return 0;\
}
FILL_MESSAGE_FUNCTION(OnConnectIB)
	FILL_MESSAGE_FUNCTION(OnDisconnectIB)
	FILL_MESSAGE_FUNCTION(LoginCtp)
	FILL_MESSAGE_FUNCTION(LogoutCtp)
	FILL_MESSAGE_FUNCTION(UpdateHedgeHold)
	FILL_MESSAGE_FUNCTION(OnUpdateLstCtrl)
	FILL_MESSAGE_FUNCTION(OnReqMshq)
	FILL_MESSAGE_FUNCTION(OnHedgeLooping)
	FILL_MESSAGE_FUNCTION(OnConnectSql)
	FILL_MESSAGE_FUNCTION(OnInitialize)
	FILL_MESSAGE_FUNCTION(LoginCtpTD)
	FILL_MESSAGE_FUNCTION(LoginCtpMD)
	FILL_MESSAGE_FUNCTION(OnSynchronizeMarket)
	FILL_MESSAGE_FUNCTION(OnReqAccountUpdates)
#undef FILL_MESSAGE_FUNCTION

	void CMainDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	CDialogEx::OnSysCommand(nID, lParam);
}
