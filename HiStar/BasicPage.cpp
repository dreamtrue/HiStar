
// HiStarDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HiStar.h"
#include "afxdialogex.h"
#include "BasicPage.h"
#include "EClientSocket.h"   // C:\JTS\SocketClient\include must be added to include path
#include "global.h"
#include "UserMsg.h"
#include "resource.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
extern double datumDiff;
extern bool isHedgeLoopingPause;
extern HANDLE g_hEvent;
extern BOOL g_bLoginCtpT;
extern double g_A50Index;
extern double g_HS300Index;
extern double g_a50Bid1,g_a50Ask1;
extern double premiumHigh,premiumLow;
extern double MaxProfitAim,MinProfitAim;
extern int MultiPos;
extern double MarginA50;
extern bool isReal;
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	// 实现
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


// CBasicPage 对话框

CBasicPage::CBasicPage(CWnd* pParent /*=NULL*/)
	: CDialogEx(CBasicPage::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CBasicPage::~CBasicPage()
{
}

void CBasicPage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_orderStatus);
	DDX_Control(pDX, IDC_LIST2, m_errors);
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
	DDX_Text(pDX,IDC_RICHEDIT23,MarginA50);
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
	ON_BN_CLICKED(IDC_BUTTON9, &CBasicPage::OnPause)
	ON_BN_CLICKED(IDC_BUTTON2, &CBasicPage::OnResume)
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CBasicPage::OnBnClickedTest)
	ON_BN_CLICKED(IDC_UPDATE, &CBasicPage::OnBnClickedUpdate)
	ON_BN_CLICKED(IDC_BUTTON7, &CBasicPage::OnIniSql)
	ON_BN_CLICKED(IDC_CHECK1, &CBasicPage::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_BUTTON13, &CBasicPage::OnIni)
END_MESSAGE_MAP()


// CBasicPage 消息处理程序

BOOL CBasicPage::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	ShowWindow(SW_NORMAL);

	// TODO: 在此添加额外的初始化代码
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
	m_csA50Last.SetWindowText(_T("0.0"),LITGRAY);
	m_csHedgePriceHigh.SetBkColor(ACC_BG);
	m_csHedgePriceHigh.SetWindowText(_T("0.0"),LITGRAY);
	m_csHedgePriceLow.SetBkColor(ACC_BG);
	m_csHedgePriceLow.SetWindowText(_T("0.0"),LITGRAY);
	//
	SetDlgItemText(IDC_COST_ADJUST,TEXT(_T("0.0")));
	SetDlgItemText(IDC_DATUMDIFF,TEXT(_T("0.0")));
	SetDlgItemText(IDC_MINPROFIT,TEXT(_T("20.0")));
	SetDlgItemText(IDC_MAXPROFIT,TEXT(_T("20.0")));
	SetDlgItemText(IDC_MULTI_POS,TEXT(_T("1")));
	SetDlgItemText(IDC_RICHEDIT23,TEXT(_T("416.0")));
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CBasicPage::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CBasicPage::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CBasicPage::OnConnectIB()
{
	PostThreadMessage(GetCurrentThreadId(),WM_CONNECT_IB,NULL,NULL);
}

void CBasicPage::OnDisconnectIB()
{
	PostThreadMessageA(GetCurrentThreadId(),WM_DISCONNECT_IB,NULL,NULL);
}

void CBasicPage::OnLoginCtp()
{
	PostThreadMessageA(GetCurrentThreadId(),WM_LOGIN_CTP,NULL,NULL);
}

void CBasicPage::OnLogoutCtp()
{
	PostThreadMessageA(GetCurrentThreadId(),WM_LOGOUT_CTP,NULL,NULL);
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
	//屏蔽掉对OK的响应
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
}

void CBasicPage::RefreshMdPane(void)
{
	double dPresp=m_depthMd.PreSettlementPrice;
	double dUpD = m_depthMd.LastPrice-dPresp;
	m_csS1P.SetDouble(m_depthMd.AskPrice1,CmpPriceColor(m_depthMd.AskPrice1,dPresp));
	m_csLastP.SetDouble(m_depthMd.LastPrice,CmpPriceColor(m_depthMd.AskPrice1,dPresp));
	m_csB1P.SetDouble(m_depthMd.BidPrice1,CmpPriceColor(m_depthMd.AskPrice1,dPresp));
	m_csHs300.SetDouble(g_HS300Index,BLACK);
	m_csA50.SetDouble(g_A50Index,BLACK);
	m_csA50Bid1.SetDouble(g_a50Bid1,BLACK);
	m_csA50Ask1.SetDouble(g_a50Ask1,BLACK);
	m_csHedgePriceHigh.SetDouble(premiumHigh - datumDiff,BLACK);
	m_csHedgePriceLow.SetDouble(premiumLow - datumDiff,BLACK);
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	char price[1000],datetime[100];
	if(((CHiStarApp*)AfxGetApp())->conn){
		sprintf(datetime,"'%d-%d-%d %d:%d:%d',%d,",sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds);
		sprintf(price,"%f,%f,%f,%f,%f,%f",g_A50Index,g_a50Bid1,g_a50Ask1,g_HS300Index,m_depthMd.BidPrice1,m_depthMd.AskPrice1);
		CString insertdata = "INSERT INTO " + ((CHiStarApp*)AfxGetApp())->m_marketTableName + " (datetime,millisecond,a50index,a50bid,a50ask,hs300index,hs300bid,hs300ask) VALUES (" + CString(datetime) + CString(price) +")";
		if(mysql_query(((CHiStarApp*)AfxGetApp())->conn,insertdata.GetBuffer())){
			TRACE("Error %u: %s\n", mysql_errno(((CHiStarApp*)AfxGetApp())->conn), mysql_error(((CHiStarApp*)AfxGetApp())->conn));
		}
	}	
}

void CBasicPage::OnPause()
{
	isHedgeLoopingPause = true;
}


void CBasicPage::OnResume()
{
	isHedgeLoopingPause = false;
}

void CBasicPage::OnBnClickedTest()
{
	((CHiStarApp*)AfxGetApp())->OnHedgeLooping(NULL,NULL);
}


void CBasicPage::OnBnClickedUpdate()
{
	UpdateData();
}

void CBasicPage::OnIniSql()
{
	PostThreadMessage(GetCurrentThreadId(),WM_CONNECT_SQL,NULL,NULL);
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
	PostThreadMessage(GetCurrentThreadId(),WM_INI,NULL,NULL);
}
