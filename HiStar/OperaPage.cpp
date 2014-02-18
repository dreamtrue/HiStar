
// HiStarDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HiStar.h"
#include "afxdialogex.h"
#include "OperaPage.h"
#include "EClientSocket.h"   // C:\JTS\SocketClient\include must be added to include path
#include "global.h"
#include "UserMsg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
extern HANDLE g_hEvent;
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
END_MESSAGE_MAP()


// COperaPage 对话框

COperaPage::COperaPage(CWnd* pParent /*=NULL*/)
	: CDialogEx(COperaPage::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

COperaPage::~COperaPage()
{
}

void COperaPage::DoDataExchange(CDataExchange* pDX)
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
	DDX_Control(pDX, IDC_TAB1, m_TabOption);
	//DDX_Control(pDX, IDC_LST_ONROAD, m_LstOnRoad);
	//DDX_Control(pDX, IDC_LST_ODINF, m_LstOrdInf);
	//DDX_Control(pDX, IDC_LST_TRADE, m_LstTdInf);
	//DDX_Control(pDX, IDC_LST_INVPOS, m_LstInvPosInf);
	//DDX_Control(pDX, IDC_LST_ALLINST, m_LstAllInsts);
}

BEGIN_MESSAGE_MAP(COperaPage, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &COperaPage::OnConnectIB)
	ON_BN_CLICKED(IDC_BUTTON3, &COperaPage::OnDisconnectIB)
	ON_BN_CLICKED(IDC_BUTTON4, &COperaPage::OnLoginCtp)
	ON_BN_CLICKED(IDC_BUTTON5, &COperaPage::OnLogoutCtp)
	ON_CBN_SELCHANGE(IDC_COMBO2, &COperaPage::OnInsSelchange)
	ON_BN_CLICKED(IDC_BUTTON6, &COperaPage::OnReqComboSelMarketDepth)
	ON_BN_CLICKED(IDC_BUTTON7, &COperaPage::OnStart)
	ON_NOTIFY(TCN_SELCHANGING, IDC_TAB1, &COperaPage::OnTcnSelchangingTab1)
END_MESSAGE_MAP()


// COperaPage 消息处理程序

BOOL COperaPage::OnInitDialog()
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

	m_TabOption.InsertItem( 0, _T("挂单") );
	m_TabOption.InsertItem( 1, _T("委托") );
	m_TabOption.InsertItem( 2, _T("持仓") );
	m_TabOption.InsertItem( 3, _T("成交") );
	m_TabOption.InsertItem( 4, _T("合约") );
	m_TabOption.InsertItem( 5, _T("其它") );

	//InitAllHdrs();
	/*
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	m_LstOnRoad.SetItemCountEx(4);
	m_LstOrdInf.SetItemCountEx(4);
	m_LstInvPosInf.SetItemCountEx(4);
	m_LstTdInf.SetItemCountEx(4);
	m_LstAllInsts.SetItemCountEx(4);
	*/
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void COperaPage::OnSysCommand(UINT nID, LPARAM lParam)
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

void COperaPage::OnPaint()
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
HCURSOR COperaPage::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void COperaPage::OnConnectIB()
{
	PostThreadMessage(GetCurrentThreadId(),WM_CONNECT_IB,NULL,NULL);
}

void COperaPage::OnDisconnectIB()
{
	PostThreadMessageA(GetCurrentThreadId(),WM_DISCONNECT_IB,NULL,NULL);
}

void COperaPage::OnLoginCtp()
{
	PostThreadMessageA(GetCurrentThreadId(),WM_LOGIN_CTP,NULL,NULL);
}

void COperaPage::OnLogoutCtp()
{
	PostThreadMessageA(GetCurrentThreadId(),WM_LOGOUT_CTP,NULL,NULL);
}


void COperaPage::ProgressUpdate(LPCTSTR szMsg, const int nPercentDone)
{
	ASSERT (AfxIsValidString(szMsg));
	ASSERT ( nPercentDone >= 0  &&  nPercentDone <= 100 );

	m_staInfo.SetWindowText(szMsg);
	m_prgs.SetPos(nPercentDone);
}

void COperaPage::OnInsSelchange()
{
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	m_CombInst.GetLBText(m_CombInst.GetCurSel(),pApp->m_accountCtp.m_szInst);
}

void COperaPage::OnOK(void)
{
	//屏蔽掉对OK的响应
}

void COperaPage::OnReqComboSelMarketDepth()
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

void COperaPage::RefreshMdPane(void)
{
	 double dPresp=m_depthMd.PreSettlementPrice;
	 double dUpD = m_depthMd.LastPrice-dPresp;
	 m_csS1P.SetDouble(m_depthMd.AskPrice1,CmpPriceColor(m_depthMd.AskPrice1,dPresp));
	 m_csLastP.SetDouble(m_depthMd.LastPrice,CmpPriceColor(m_depthMd.AskPrice1,dPresp));
	 m_csB1P.SetDouble(m_depthMd.BidPrice1,CmpPriceColor(m_depthMd.AskPrice1,dPresp));
}

void COperaPage::InitAllHdrs(void)
{
	TCHAR* lpHdrs0[ONROAD_ITMES] = {_T("单号"),_T("合约"),_T("买卖"),_T("开平"),_T("未成"),_T("价格"),_T("时间"),_T("序列号"),_T("冻结金")};
	int iWidths0[ONROAD_ITMES] = {1,46,34,34,34,46,60,60,60};
	int i;
	int total_cx = 0;
	LVCOLUMN lvcolumn;
	memset(&lvcolumn, 0, sizeof(lvcolumn));

	for (i = 0;i<ONROAD_ITMES ; i++)
	{
		lvcolumn.mask     = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH|LVCFMT_IMAGE;
		lvcolumn.fmt      = LVCFMT_RIGHT;
		lvcolumn.iSubItem = i;
		lvcolumn.pszText  = lpHdrs0[i];
		lvcolumn.cx       = iWidths0[i];
		
		total_cx += lvcolumn.cx;
		m_LstOnRoad.InsertColumn(i, &lvcolumn);
	}
/////////////////////////////////////////////////////////////////////////////////////////////
	TCHAR* lpHdrs1[ORDER_ITMES] = {_T("单号"),_T("合约"),_T("买卖"),_T("开平"),_T("状态"),_T("价格"),_T("报量"),_T("未成"),
						_T("已成"),_T("均价"),_T("时间"),_T("序列号"),_T("冻保证金"),_T("冻手续费"),_T("详细状态"),};
	int iWidths1[ORDER_ITMES] = {1,46,34,34,60,46,34,34,34,46,60,60,60,60,120};
	total_cx = 0;
	memset(&lvcolumn, 0, sizeof(lvcolumn));
	
	for (i = 0;i<ORDER_ITMES ; i++)
	{
		lvcolumn.mask     = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH|LVCFMT_IMAGE;
		lvcolumn.fmt      = LVCFMT_RIGHT;
		lvcolumn.iSubItem = i;
		lvcolumn.pszText  = lpHdrs1[i];
		lvcolumn.cx       = iWidths1[i];
		
		total_cx += lvcolumn.cx;
		m_LstOrdInf.InsertColumn(i, &lvcolumn);
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////////
	TCHAR* lpHdrs2[TRADE_ITMES] = {_T("合约"),_T("买卖"),_T("开平"),_T("价格"),_T("手数"),_T("时间"),_T("手续费"),
		_T("投保"),_T("成交类型"),_T("交易所"),_T("成交编号"),_T("报单编号")};
	int iWidths2[TRADE_ITMES] = {46,36,40,50,40,60,50,46,60,46,60,60};
	total_cx = 0;
	//LVCOLUMN lvcolumn;
	memset(&lvcolumn, 0, sizeof(lvcolumn));
	
	for (i = 0;i<TRADE_ITMES ; i++)
	{
		lvcolumn.mask     = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH|LVCFMT_IMAGE;
		lvcolumn.fmt      = LVCFMT_RIGHT;
		lvcolumn.iSubItem = i;
		lvcolumn.pszText  = lpHdrs2[i];
		lvcolumn.cx       = iWidths2[i];
		
		total_cx += lvcolumn.cx;
		m_LstTdInf.InsertColumn(i, &lvcolumn);
	}
	
	///////////////////////////////////////////////////////////////////////////////
	TCHAR* lpHdrs3[INVPOS_ITMES] = {_T("合约"),_T("买卖"),_T("总持仓"),_T("可平量"),_T("持仓均价"),_T("持仓盈亏"),_T("占保证金"),_T("总盈亏")};
	int iWidths3[INVPOS_ITMES] = {46,34,46,46,60,60,60,60};
	total_cx = 0;
	memset(&lvcolumn, 0, sizeof(lvcolumn));
	
	for (i = 0;i<INVPOS_ITMES ; i++)
	{
		lvcolumn.mask     = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH|LVCFMT_IMAGE;
		lvcolumn.fmt      = LVCFMT_RIGHT;
		lvcolumn.iSubItem = i;
		lvcolumn.pszText  = lpHdrs3[i];
		lvcolumn.cx       = iWidths3[i];
		
		total_cx += lvcolumn.cx;
		m_LstInvPosInf.InsertColumn(i, &lvcolumn);
	}
	///////////////////////////////////////////////////////////////////////////////////////
	
	TCHAR* lpHdrs4[ALLINST_ITMES] = {_T("代码"),_T("合约"),_T("合约名"),_T("交易所"),_T("乘数"),_T("点差"),
	_T("类型"),_T("最后日期"),_T("保证金率"),_T("手续费率")};
	int iWidths4[ALLINST_ITMES] = {26,46,80,46,34,34,34,60,60,120};
	total_cx = 0;
	memset(&lvcolumn, 0, sizeof(lvcolumn));
	
	for (i = 0;i<ALLINST_ITMES ; i++)
	{
		lvcolumn.mask     = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH|LVCFMT_IMAGE;
		lvcolumn.fmt      = LVCFMT_RIGHT;
		lvcolumn.iSubItem = i;
		lvcolumn.pszText  = lpHdrs4[i];
		lvcolumn.cx       = iWidths4[i];
		
		total_cx += lvcolumn.cx;
		m_LstAllInsts.InsertColumn(i, &lvcolumn);
	}
	///////////////////////////////////////////////////////////////////////////////////////	
}

void COperaPage::OnStart()
{
}


void COperaPage::OnTcnSelchangingTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	switch ( m_TabOption.GetCurSel() )
	{
	case 0:
		m_LstTdInf.ShowWindow( SW_HIDE );
		m_LstOrdInf.ShowWindow( SW_HIDE );
		m_LstInvPosInf.ShowWindow( SW_HIDE );
		m_LstAllInsts.ShowWindow( SW_HIDE );

		m_LstOnRoad.ShowWindow( SW_SHOW );

		break;
	case 1:
		m_LstOnRoad.ShowWindow( SW_HIDE );
		m_LstTdInf.ShowWindow( SW_HIDE );
		m_LstInvPosInf.ShowWindow( SW_HIDE );
		m_LstAllInsts.ShowWindow( SW_HIDE );

		m_LstOrdInf.ShowWindow( SW_SHOW );
		break;
	case 2:	
		m_LstOnRoad.ShowWindow( SW_HIDE );
		m_LstTdInf.ShowWindow( SW_HIDE );
		m_LstOrdInf.ShowWindow( SW_HIDE );
		m_LstAllInsts.ShowWindow( SW_HIDE );

		m_LstInvPosInf.ShowWindow( SW_SHOW );
		break;
	case 3:
		m_LstOnRoad.ShowWindow( SW_HIDE );
		m_LstOrdInf.ShowWindow( SW_HIDE );
		m_LstInvPosInf.ShowWindow( SW_HIDE );
		m_LstAllInsts.ShowWindow( SW_HIDE );

		m_LstTdInf.ShowWindow( SW_SHOW );
		break;
	case 4:
		m_LstOnRoad.ShowWindow( SW_HIDE );
		m_LstTdInf.ShowWindow( SW_HIDE );
		m_LstOrdInf.ShowWindow( SW_HIDE );
		m_LstInvPosInf.ShowWindow( SW_HIDE );

		m_LstAllInsts.ShowWindow( SW_SHOW );
		break;
	case 5:
		m_LstOnRoad.ShowWindow( SW_HIDE );
		m_LstTdInf.ShowWindow( SW_HIDE );
		m_LstOrdInf.ShowWindow( SW_HIDE );
		m_LstInvPosInf.ShowWindow( SW_HIDE );
		m_LstAllInsts.ShowWindow( SW_HIDE );
		break;
	}
	*pResult = 0;
}
