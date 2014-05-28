
// HiStarDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "HiStar.h"
#include "afxdialogex.h"
#include "BasicPage.h"
#include "EClientSocket.h"
#include "global.h"
#include "UserMsg.h"
#include "resource.h"
#include <sstream>
#include "MainDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
extern bool iSell,iBuy;
extern DWORD IndexThreadId;
extern double datumDiff;
extern bool isHedgeLoopingPause;
extern BOOL g_bLoginCtpT;
extern double g_A50Index;
extern double g_HS300Index;
extern double g_a50Bid1,g_a50Ask1,g_a50last;
extern double premiumHigh,premiumLow;
extern double MaxProfitAim,MinProfitAim;
extern int MultiPos;
extern double MarginA50;extern int MultiA50;
extern double g_A50IndexZT,g_HS300IndexZT;
extern bool isReal;
extern CVector<HoldDetail> HedgeHold;
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CBasicPage::OnOK)
	ON_BN_CLICKED(IDCANCEL, &CBasicPage::OnCancel)
END_MESSAGE_MAP()


// CBasicPage �Ի���

CBasicPage::CBasicPage(CWnd* pParent /*=NULL*/)
	: CDialogEx(CBasicPage::IDD, pParent)
{
	memset(&m_depthMd,0,sizeof(m_depthMd));
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CBasicPage::~CBasicPage()
{
}

void CBasicPage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_orderStatus);
	DDX_Control(pDX, IDC_PROGRESS1, m_prgs);
	DDX_Control(pDX, IDC_STATIC1, m_staInfo);
	DDX_Control(pDX, IDC_COMBO2, m_CombInst);
	DDX_Control(pDX, IDC_STATIC01, m_csS1P);
	DDX_Control(pDX, IDC_STATIC03, m_csB1P);
	DDX_Control(pDX, IDC_STATIC02, m_csLastP);
	DDX_Control(pDX, IDC_STATIC_HS300, m_csHs300);
	DDX_Control(pDX, IDC_STATIC_A50, m_csA50);
	DDX_Control(pDX, IDC_STATIC_A50ASK1, m_csA50Ask1);
	DDX_Control(pDX, IDC_STATIC_A50_LAST, m_csA50Last);
	DDX_Control(pDX, IDC_STATIC_A50_BID1, m_csA50Bid1);
	DDX_Control(pDX, IDC_HEDGEPRICE_HIGH, m_csHedgePriceHigh);
	DDX_Control(pDX, IDC_HEDGEPRICE_LOW , m_csHedgePriceLow);
	DDX_Text(pDX, IDC_DATUMDIFF,datumDiff);
	DDX_Text(pDX, IDC_MINPROFIT,MinProfitAim);
	DDX_Text(pDX, IDC_MAXPROFIT,MaxProfitAim);
	DDX_Text(pDX,IDC_MULTI_POS,MultiPos);
	DDX_Control(pDX, IDC_LIST3, m_LstHedgeStatus);
	DDX_Text(pDX, IDC_RICHEDIT26, MultiA50);
	DDX_Control(pDX, IDC_UP_HS300, m_HS300UP);
	DDX_Control(pDX, IDC_UP_A50, m_A50UP);
	DDX_Control(pDX, IDC_ClOSE_PROFIT, m_closeProfit);
	DDX_Control(pDX, IDC_OPEN_PROFIT, m_openProfit);
	DDX_Control(pDX, IDC_MARGIN, m_totalMargin);
	DDX_Control(pDX, IDC_AVAIL_IB, m_availIb);
	DDX_Control(pDX, IDC_BUTTON2, m_btnRun);
	DDX_Control(pDX, IDC_BUTTON13, m_btnIni);
	DDX_Control(pDX, IDC_SELL_OPEN, m_sellbuy);
}

BEGIN_MESSAGE_MAP(CBasicPage, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CBasicPage::OnConnectIB)
	ON_BN_CLICKED(IDC_BUTTON3, &CBasicPage::OnDisconnectIB)
	ON_BN_CLICKED(IDC_BUTTON4, &CBasicPage::OnLoginCtp)
	ON_BN_CLICKED(IDC_BUTTON5, &CBasicPage::OnLogoutCtp)
	ON_CBN_SELCHANGE(IDC_COMBO2, &CBasicPage::OnInsSelchange)
	ON_BN_CLICKED(IDC_BUTTON6, &CBasicPage::OnReqComboSelMarketDepth)
	ON_BN_CLICKED(IDC_BUTTON2, &CBasicPage::OnResumeOrPause)
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CBasicPage::OnBnClickedTest)
	ON_BN_CLICKED(IDC_UPDATE, &CBasicPage::OnBnClickedUpdate)
	ON_BN_CLICKED(IDC_BUTTON7, &CBasicPage::OnIniSql)
	ON_BN_CLICKED(IDC_CHECK1, &CBasicPage::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_BUTTON13, &CBasicPage::OnIni)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST3, OnGetHedgeHold)
	ON_NOTIFY(NM_CLICK, IDC_LIST3, OnNMClkLstHedgeStatus)
	ON_NOTIFY(NM_DBLCLK,IDC_LIST3,OnNMDblclkLstHedgeStatus)
	ON_BN_CLICKED(IDC_BUTTON8, &CBasicPage::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_MSHQ, &CBasicPage::OnBnClickedMshq)
	ON_BN_CLICKED(IDC_UPDATE_INDEXREF, &CBasicPage::OnUpdateIndexref)
	ON_BN_CLICKED(IDC_SELL_OPEN, &CBasicPage::OnBnClickedSellOpen)
	ON_BN_CLICKED(IDC_UPDATE_IB, &CBasicPage::OnBnClickedUpdateIb)
END_MESSAGE_MAP()


// CBasicPage ��Ϣ�������

BOOL CBasicPage::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	ShowWindow(SW_NORMAL);

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	m_btnRun.SetIcon(IDI_HALLOWEEN2, IDI_HALLOWEEN1);
    ((CButton *)GetDlgItem(IDC_CHECK1))->SetCheck(TRUE);
	m_csS1P.SetBkColor(ACC_BG);
	m_csS1P.SetWindowText(_T("0.0"),LITGRAY);
	m_csLastP.SetBkColor(ACC_BG);
	m_csLastP.SetWindowText(_T("0.0"),LITGRAY);
	m_csB1P.SetBkColor(ACC_BG);
	m_csB1P.SetWindowText(_T("0.0"),LITGRAY);
	m_csHs300.SetBkColor(ACC_BG);
	m_csHs300.SetWindowText(_T("0.0"),LITGRAY);
	m_csA50.SetBkColor(ACC_BG);
	m_csA50.SetWindowText(_T("0.0"),LITGRAY);
	m_csA50Bid1.SetBkColor(ACC_BG);
	m_csA50Bid1.SetWindowText(_T("0.0"),LITGRAY);
	m_csA50Ask1.SetBkColor(ACC_BG);
	m_csA50Ask1.SetWindowText(_T("0.0"),LITGRAY);
	m_csA50Last.SetBkColor(ACC_BG);
	m_availIb.SetWindowText(_T("0.0"),LITGRAY);
	m_availIb.SetBkColor(ACC_BG);
	m_csA50Last.SetWindowText(_T("0.0"),LITGRAY);
	m_csHedgePriceHigh.SetBkColor(ACC_BG);
	m_csHedgePriceHigh.SetWindowText(_T("0.0"),LITGRAY);
	m_csHedgePriceLow.SetBkColor(ACC_BG);
	m_csHedgePriceLow.SetWindowText(_T("0.0"),LITGRAY);
	m_A50UP.SetBkColor(ACC_BG);
	m_A50UP.SetWindowText(_T("0.0"),LITGRAY);
	m_HS300UP.SetBkColor(ACC_BG);
	m_HS300UP.SetWindowText(_T("0.0"),LITGRAY);
	m_totalMargin.SetBkColor(ACC_BG);
	m_totalMargin.SetWindowText(_T("0.0"),LITGRAY);
	m_openProfit.SetBkColor(ACC_BG);
	m_openProfit.SetWindowText(_T("0.0"),LITGRAY);
	m_closeProfit.SetBkColor(ACC_BG);
	m_closeProfit.SetWindowText(_T("0.0"),LITGRAY);
	//
	SetDlgItemText(IDC_COST_ADJUST,TEXT(_T("0.0")));
	SetDlgItemText(IDC_DATUMDIFF,TEXT(_T("0.0")));
	SetDlgItemText(IDC_MINPROFIT,TEXT(_T("20.0")));
	SetDlgItemText(IDC_MAXPROFIT,TEXT(_T("20.0")));
	SetDlgItemText(IDC_MULTI_POS,TEXT(_T("1")));
	SetDlgItemText(IDC_RICHEDIT26,TEXT(_T("16")));
	//��ʼ���б�ؼ�
	TCHAR* lpHdrs0[6] = {_T("ID"),_T("����"),_T("��������"),_T("�ɱ�"),_T("IF�ֲ�"),_T("A50�ֲ�")};
	int iWidths0[6] = {32,52,68,68,68,68};
	int i;
	int total_cx = 0;
	LVCOLUMN lvcolumn;
	memset(&lvcolumn, 0, sizeof(lvcolumn));

	for (i = 0;i < 6 ; i++)
	{
		lvcolumn.mask     = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH|LVCFMT_IMAGE;
		lvcolumn.fmt      = LVCFMT_RIGHT;
		lvcolumn.iSubItem = i;
		lvcolumn.pszText  = lpHdrs0[i];
		lvcolumn.cx       = iWidths0[i];

		total_cx += lvcolumn.cx;
		m_LstHedgeStatus.InsertColumn(i, &lvcolumn);
	}
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CBasicPage::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CBasicPage::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CBasicPage::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CBasicPage::OnConnectIB()
{
	if((CHiStarApp*)AfxGetApp()->m_pMainWnd){
		while(::PostMessage(((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->GetSafeHwnd(),WM_CONNECT_IB,NULL,NULL) == 0){
			Sleep(100);
		}
	}
}

void CBasicPage::OnDisconnectIB()
{
	if((CHiStarApp*)AfxGetApp()->m_pMainWnd){
		while(::PostMessage(((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->GetSafeHwnd(),WM_DISCONNECT_IB,NULL,NULL) == 0){
			Sleep(100);
		}
	}
}

void CBasicPage::OnLoginCtp()
{
	if((CHiStarApp*)AfxGetApp()->m_pMainWnd){
		while(::PostMessage(((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->GetSafeHwnd(),WM_LOGIN_CTP,NULL,NULL) == 0){
			Sleep(100);
		}
	}
}

void CBasicPage::OnLogoutCtp()
{
	if((CHiStarApp*)AfxGetApp()->m_pMainWnd){
		while(::PostMessage(((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->GetSafeHwnd(),WM_LOGOUT_CTP,NULL,NULL) == 0){
			Sleep(100);
		}
	}
}

void CBasicPage::ProgressUpdate(LPCTSTR szMsg, const int nPercentDone)
{
	ASSERT (AfxIsValidString(szMsg));
	ASSERT ( nPercentDone >= 0  &&  nPercentDone <= 100 );

	m_staInfo.SetWindowText(szMsg);
	m_prgs.SetPos(nPercentDone);
}

void CBasicPage::OnInsSelchange()
{
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	m_CombInst.GetLBText(m_CombInst.GetCurSel(),pApp->m_accountCtp.m_szInst);
}

void CBasicPage::OnOK(void)
{
	//���ε���OK����Ӧ
}

void CBasicPage::OnCancel(){

}

void CBasicPage::OnReqComboSelMarketDepth()
{
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	char szInst[MAX_PATH];
	uni2ansi(CP_ACP,pApp->m_accountCtp.m_szInst,szInst);
	LPSTR* pInst = new LPSTR;
	pInst[0] = szInst;
	if(pApp->m_cQ){
		pApp->m_cQ->SubscribeMarketData(pInst,1);
	}
	delete []pInst;
}

void CBasicPage::RefreshMdPane(void)
{
	double dPresp=m_depthMd.PreSettlementPrice;
	double dUpD = m_depthMd.LastPrice-dPresp;
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	m_csS1P.SetDouble(m_depthMd.AskPrice1,CmpPriceColor(m_depthMd.AskPrice1,dPresp));
	m_csLastP.SetDouble(m_depthMd.LastPrice,CmpPriceColor(m_depthMd.AskPrice1,dPresp));
	m_csB1P.SetDouble(m_depthMd.BidPrice1,CmpPriceColor(m_depthMd.AskPrice1,dPresp));
	m_csHs300.SetDouble(g_HS300Index,CmpPriceColor(g_HS300Index,g_HS300IndexZT));
	m_csA50.SetDouble(g_A50Index,CmpPriceColor(g_A50Index,g_A50IndexZT));
	m_csA50Bid1.SetDouble(g_a50Bid1,BLACK);
	m_csA50Ask1.SetDouble(g_a50Ask1,BLACK);
	m_csA50Last.SetDouble(g_a50last,BLACK);
	m_availIb.SetDouble(pApp->m_accountvalue.AvailableFunds,BLACK);
	m_csHedgePriceHigh.SetDouble(premiumHigh - datumDiff,BLACK);
	m_csHedgePriceLow.SetDouble(premiumLow - datumDiff,BLACK);
	m_A50UP.SetDouble(g_A50Index - g_A50IndexZT,CmpPriceColor(g_A50Index - g_A50IndexZT,0));
	m_HS300UP.SetDouble(g_HS300Index - g_HS300IndexZT,CmpPriceColor(g_HS300Index - g_HS300IndexZT,0));

	if(pApp && pApp->m_cT){

		AcquireSRWLockShared(&g_srwLock_TradingAccount);
		m_totalMargin.SetDouble(pApp->m_cT->TradingAccount.CurrMargin,CmpPriceColor(pApp->m_cT->TradingAccount.CurrMargin,0));
		m_closeProfit.SetDouble(pApp->m_cT->TradingAccount.CloseProfit,CmpPriceColor(pApp->m_cT->TradingAccount.CloseProfit,0));
		m_openProfit.SetDouble(pApp->m_cT->TradingAccount.PositionProfit,CmpPriceColor(pApp->m_cT->TradingAccount.PositionProfit,0));
		ReleaseSRWLockShared(&g_srwLock_TradingAccount);

	}

	SYSTEMTIME sys;
	GetLocalTime(&sys);
	char data[1000],datetime[100];
	if(((CHiStarApp*)AfxGetApp())->conn){
		sprintf(datetime,"'%d-%d-%d %d:%d:%d',%d,",sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds);
		sprintf(data,"%.02lf,%.02lf,%.02lf,%.02lf,%.02lf,%.02lf,%.02lf,%.02lf",g_A50Index,g_a50Bid1,g_a50Ask1,g_HS300Index,m_depthMd.BidPrice1,m_depthMd.AskPrice1,
			g_a50Ask1 - m_depthMd.BidPrice1 / g_HS300Index * g_A50Index,g_a50Bid1 - m_depthMd.AskPrice1 / g_HS300Index * g_A50Index);
		CString insertdata = "INSERT INTO " + ((CHiStarApp*)AfxGetApp())->m_marketTableName 
			+ " (datetime,millisecond,a50index,a50bid,a50ask,hs300index,hs300bid,hs300ask,preniumHigh,preniumLow) VALUES (" + CString(datetime) + CString(data) +")";
		if(mysql_query(((CHiStarApp*)AfxGetApp())->conn,insertdata.GetBuffer())){
			TRACE("Error %u: %s\n", mysql_errno(((CHiStarApp*)AfxGetApp())->conn), mysql_error(((CHiStarApp*)AfxGetApp())->conn));
		}
	}	
}

void CBasicPage::OnResumeOrPause()
{
	if(isHedgeLoopingPause){
		isHedgeLoopingPause = false;
		m_btnRun.SetWindowText(_T("������..."));
	}
	else{
		isHedgeLoopingPause = true;
		m_btnRun.SetWindowText(_T("��ͣ��..."));
	}
}

void CBasicPage::OnBnClickedTest()
{
	//����
	if((CHiStarApp*)AfxGetApp()->m_pMainWnd){
		while(::PostMessage(((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->GetSafeHwnd(),WM_UPDATE_LSTCTRL,NULL,NULL) == 0){
			Sleep(100);
		}
	}
	while(PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,NULL) == 0){
		Sleep(100);
	}
	((CHiStarApp*)AfxGetApp())->OnHedgeLooping(NULL,NULL);
	SynchronizeHoldDataToView();
	//((CHiStarApp*)AfxGetApp())->m_cT->ReqQryTfSerial("1");
	/*
	while(true){
	((CHiStarApp*)AfxGetApp())->m_cT->ReqQryInstFee("IF1405");
	TRACE("���ͳɹ�\n");
	}
	*/
	//TThostFtdcCombOffsetFlagType kpp;kpp[0] = THOST_FTDC_OF_Open;
	//((CHiStarApp*)AfxGetApp())->m_cT->ReqParkedOrderInsert("IF1405",THOST_FTDC_D_Sell,kpp,2500.0,1);
	//((CHiStarApp*)AfxGetApp())->OnReqAccountUpdates(NULL,NULL);
}


void CBasicPage::OnBnClickedUpdate()
{
	UpdateData();
}

void CBasicPage::OnIniSql()
{
	if((CHiStarApp*)AfxGetApp()->m_pMainWnd){
		while(::PostMessage(((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->GetSafeHwnd(),WM_CONNECT_SQL,NULL,NULL) == 0){
			Sleep(100);
		}
	}
}

void CBasicPage::OnBnClickedCheck1()
{
	if (BST_CHECKED == IsDlgButtonChecked(IDC_CHECK1))
	{
		isReal = true;
	}
	else
	{
		isReal = false;
	}
}

void CBasicPage::OnIni()
{
	if((CHiStarApp*)AfxGetApp()->m_pMainWnd){
		while(::PostMessage(((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->GetSafeHwnd(),WM_INI,NULL,NULL) == 0){
			Sleep(100);
		}
	}
	m_btnIni.EnableWindow(false);
}

void CBasicPage::OnGetHedgeHold(NMHDR *pNMHDR, LRESULT *pResult){
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	LV_ITEM* pItem= &(pDispInfo)->item;
	unsigned int iItem= /*m_hedgeHold.size()-1-*/pItem->iItem;
	if(iItem < 0 || iItem >= m_hedgeHold.size())return;
	if(pItem->mask & LVIF_TEXT)
	{
		CString szTemp = _T("");
		int iLen=0;CString str;
		switch(pItem->iSubItem)
		{
		case 0:
			str.Format("%ld",m_hedgeHold[iItem].id);
			lstrcpy(pItem->pszText,str);
			break;
		case 1: 
			str.Format("%d",m_hedgeHold[iItem].HedgeNum);
			lstrcpy(pItem->pszText,str);
			break;
		case 2:
			str.Format("%d",m_hedgeHold[iItem].HedgeSection);
			lstrcpy(pItem->pszText,str);
			break;
		case 3:
			str.Format("%.02lf",m_hedgeHold[iItem].originalCost);
			lstrcpy(pItem->pszText,str);
			break;
		case 4:
			str.Format("%ld",m_hedgeHold[iItem].numIf);
			lstrcpy(pItem->pszText,str);
			break;
		case 5:
			str.Format("%ld",m_hedgeHold[iItem].numA50);
			lstrcpy(pItem->pszText,str);
			break;
		}
	}
	*pResult = 0;
}

void CBasicPage::OnNMClkLstHedgeStatus(NMHDR *pNMHDR, LRESULT *pResult){

}

void CBasicPage::OnNMDblclkLstHedgeStatus(NMHDR *pNMHDR, LRESULT *pResult){
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstHedgeStatus.GetItemCount();
		if(nItem != -1){
			int res = ::MessageBox(m_hWnd,_T("�Ƿ�ɾ����ѡ�"),_T(""),MB_OKCANCEL|MB_ICONQUESTION);
			if(res == IDOK){
				m_LstHedgeStatus.DeleteItem(nItem);
				m_hedgeHold.erase(m_hedgeHold.begin() + nItem);
				SynchronizeHoldViewToData();
			}
		}
	}
}

void CBasicPage::SynchronizeHoldDataToView(){
	m_hedgeHold = HedgeHold.GetBuffer();
	m_LstHedgeStatus.SetItemCountEx(m_hedgeHold.size());
	m_LstHedgeStatus.Invalidate();
	m_LstHedgeStatus.UpdateWindow();
	SynchronizeSql();	
}

void CBasicPage::SynchronizeHoldViewToData(){
	HedgeHold = m_hedgeHold;
	m_LstHedgeStatus.SetItemCountEx(m_hedgeHold.size());
	m_LstHedgeStatus.Invalidate();
	m_LstHedgeStatus.UpdateWindow();
	SynchronizeSql();
}

void CBasicPage::SynchronizeSql(){
	//��������ݿ�
	if(((CHiStarApp*)AfxGetApp())->conn){
		if(mysql_query(((CHiStarApp*)AfxGetApp())->conn,"truncate " + ((CHiStarApp*)AfxGetApp())->m_positionTableName)){
			TRACE("Error %u: %s\n", mysql_errno(((CHiStarApp*)AfxGetApp())->conn), mysql_error(((CHiStarApp*)AfxGetApp())->conn));
		}
	}
	//���ݿ�ͬ��
	if(((CHiStarApp*)AfxGetApp())->conn){
		for(unsigned int j = 0;j < m_hedgeHold.size();j++){
			char id[100],amou[100],sec[100],price[100],numIf[100],numA50[100];
			memset(id,0,sizeof(id));memset(amou,0,sizeof(amou));memset(sec,0,sizeof(sec));memset(price,0,sizeof(price));memset(numIf,0,sizeof(numIf));memset(numA50,0,sizeof(numA50));
			sprintf(id,"%ld,",m_hedgeHold[j].id);
			sprintf(amou,"%d,",m_hedgeHold[j].HedgeNum);
			sprintf(sec,"%d,",m_hedgeHold[j].HedgeSection);
			sprintf(price,"%.02lf,",m_hedgeHold[j].originalCost);
			sprintf(numIf,"%ld,",m_hedgeHold[j].numIf);
			sprintf(numA50,"%ld",m_hedgeHold[j].numA50);
			CString insertdata = "INSERT INTO " + ((CHiStarApp*)AfxGetApp())->m_positionTableName 
				+ " (ID,amount,section,price,numIf,numA50) VALUES (" + CString(id) + CString(amou) + CString(sec) + CString(price) + CString(numIf)+ CString(numA50) + ")";
			if(mysql_query(((CHiStarApp*)AfxGetApp())->conn,insertdata.GetBuffer())){
				TRACE("Error %u: %s\n", mysql_errno(((CHiStarApp*)AfxGetApp())->conn), mysql_error(((CHiStarApp*)AfxGetApp())->conn));
			}
		}
	}
}

void CBasicPage::OnBnClickedButton8()
{
	char text[100];int id;int num;int section;double price;long numIf;long numA50;memset(text,0,sizeof(text));
	std::stringstream stream;
	GetDlgItemTextA(IDC_RICHEDIT24,text,100);
	if(strcmp(text,"") == 0)return;
	for(int i = 0;i < 100;i++){
		if(text[i] == ','){
			text[i] = ' ';
		}
	}
	stream << text;
	stream >> id >> num >> section >> price >> numIf >> numA50;
	bool idfouned = false;
	for(unsigned int j = 0;j < m_hedgeHold.size();j++){
		if(id == m_hedgeHold[j].id){
			idfouned = true;
			break;
		}
	}
	if(idfouned){
		int res = ::MessageBox(m_hWnd,_T("ID�ظ������������룡"),_T(""),MB_OK|MB_ICONERROR);
		if(res == IDOK){
			SetDlgItemTextA(IDC_RICHEDIT24,_T(""));
			return;
		}
	}
	if(num * numA50 < 0 || num * numIf > 0){
		int res = ::MessageBox(m_hWnd,_T("�ֲ�����ì�ܣ�"),_T(""),MB_OK|MB_ICONERROR);
		if(res == IDOK){
			SetDlgItemTextA(IDC_RICHEDIT24,_T(""));
			return;
		}
	}
	HoldDetail hd;
	hd.id = id;hd.HedgeNum = num;hd.HedgeSection = section;hd.originalCost = price;hd.numIf = numIf;hd.numA50 = numA50;
	m_hedgeHold.push_back(hd);
	SetDlgItemTextA(IDC_RICHEDIT24,_T(""));
	SynchronizeHoldViewToData();
}


void CBasicPage::OnBnClickedMshq()
{
	if((CHiStarApp*)AfxGetApp()->m_pMainWnd){
		while(::PostMessage(((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->GetSafeHwnd(),WM_REQ_MSHQ,NULL,NULL) == 0){
			Sleep(100);
		}
	}
}


void CBasicPage::OnUpdateIndexref()
{
	while(PostThreadMessage(IndexThreadId,WM_UPDATE_INDEX_REF,NULL,NULL) == 0){
		Sleep(100);
	}
}


void CBasicPage::OnBnClickedSellOpen()
{
	static int i = 0;
	if(i % 3 == 0)
	{
		iSell = false;iBuy = true;
		m_sellbuy.SetWindowText("OnlyBuy");
	}
	else if(i % 3 == 1){
		iSell = true;iBuy = false;
		m_sellbuy.SetWindowText("OnlySell");
	}
	else{
		iSell = true;iBuy = true;
		m_sellbuy.SetWindowText("Sell&&Buy");
	}
	i++;
}

void CBasicPage::OnBnClickedUpdateIb()
{
	if((CHiStarApp*)AfxGetApp()->m_pMainWnd){
		while(::PostMessage(((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->GetSafeHwnd(),WM_REQACCOUNTUPDATES,NULL,0) == 0){
			Sleep(100);
		}
		TRACE("REQACCOUNTUPDATES 1\n");
		while(::PostMessage(((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->GetSafeHwnd(),WM_REQACCOUNTUPDATES,NULL,1) == 0){
			Sleep(100);
		}
	}
}
