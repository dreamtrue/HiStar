
// HiStarDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HiStar.h"
#include "afxdialogex.h"
#include "BasicPage.h"
#include "EClientSocket.h"   // C:\JTS\SocketClient\include must be added to include path
#include "global.h"
#include "UserMsg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
extern HANDLE g_hEvent;
extern BOOL g_bLoginCtpT;
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
	ON_BN_CLICKED(IDCANCEL, &CBasicPage::OnBnClickedCancel)
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
	ON_BN_CLICKED(IDC_BUTTON7, &CBasicPage::OnStart)
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
	m_csS1P.SetBkColor(WHITE);
	m_csS1P.SetWindowText(_T("0.0"),GREEN);
	m_csLastP.SetBkColor(WHITE);
	m_csLastP.SetWindowText(_T("0.0"),GREEN);
	m_csB1P.SetBkColor(WHITE);
	m_csB1P.SetWindowText(_T("0.0"),GREEN);

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

void CBasicPage::OnBnClickedCancel(){

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
}

void CBasicPage::OnStart()
{
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	TThostFtdcCombOffsetFlagType kpp;
	kpp[0] = THOST_FTDC_OF_Open;
	if(g_bLoginCtpT){
		if(pApp->m_cT){
			for(int i = 0;i < 100;i++){
				pApp->m_cT->ReqOrdLimit("IF1406", THOST_FTDC_D_Buy,kpp,2291,1);
			}
		}
	}
	//pApp->m_cT->ReqOrdAny("IF1406", THOST_FTDC_D_Buy,kpp,1);
}
