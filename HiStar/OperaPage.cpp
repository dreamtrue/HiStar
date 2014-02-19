
// HiStarDlg.cpp : ʵ���ļ�
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
END_MESSAGE_MAP()


// COperaPage �Ի���

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
	DDX_Control(pDX, IDC_LIST3, m_LstOnRoad);
	DDX_Control(pDX, IDC_LIST4, m_LstOrdInf);
	DDX_Control(pDX, IDC_LIST5, m_LstTdInf);
	DDX_Control(pDX, IDC_LIST6, m_LstInvPosInf);
	DDX_Control(pDX, IDC_LIST7, m_LstAllInsts);
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
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &COperaPage::OnTcnSelchangeTab1)
END_MESSAGE_MAP()


// COperaPage ��Ϣ�������

BOOL COperaPage::OnInitDialog()
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
	m_csS1P.SetBkColor(WHITE);
	m_csS1P.SetWindowText(_T("0.0"),GREEN);
	m_csLastP.SetBkColor(WHITE);
	m_csLastP.SetWindowText(_T("0.0"),GREEN);
	m_csB1P.SetBkColor(WHITE);
	m_csB1P.SetWindowText(_T("0.0"),GREEN);

	m_TabOption.InsertItem( 0, _T("�ҵ�") );
	m_TabOption.InsertItem( 1, _T("ί��") );
	m_TabOption.InsertItem( 2, _T("�ֲ�") );
	m_TabOption.InsertItem( 3, _T("�ɽ�") );
	m_TabOption.InsertItem( 4, _T("��Լ") );
	m_TabOption.InsertItem( 5, _T("����") );
	m_TabOption.SetCurSel(0);
	m_LstTdInf.ShowWindow( SW_HIDE );
	m_LstOrdInf.ShowWindow( SW_HIDE );
	m_LstInvPosInf.ShowWindow( SW_HIDE );
	m_LstAllInsts.ShowWindow( SW_HIDE );
	m_LstOnRoad.ShowWindow( SW_SHOW );
	InitAllHdrs();

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void COperaPage::OnPaint()
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
	//���ε���OK����Ӧ
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
	TCHAR* lpHdrs0[ONROAD_ITMES] = {_T("����"),_T("��Լ"),_T("����"),_T("��ƽ"),_T("δ��"),_T("�۸�"),_T("ʱ��"),_T("���к�"),_T("�����")};
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
	TCHAR* lpHdrs1[ORDER_ITMES] = {_T("����"),_T("��Լ"),_T("����"),_T("��ƽ"),_T("״̬"),_T("�۸�"),_T("����"),_T("δ��"),
						_T("�ѳ�"),_T("����"),_T("ʱ��"),_T("���к�"),_T("����֤��"),_T("��������"),_T("��ϸ״̬"),};
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
	TCHAR* lpHdrs2[TRADE_ITMES] = {_T("��Լ"),_T("����"),_T("��ƽ"),_T("�۸�"),_T("����"),_T("ʱ��"),_T("������"),
		_T("Ͷ��"),_T("�ɽ�����"),_T("������"),_T("�ɽ����"),_T("�������")};
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
	TCHAR* lpHdrs3[INVPOS_ITMES] = {_T("��Լ"),_T("����"),_T("�ֲܳ�"),_T("��ƽ��"),_T("�ֲ־���"),_T("�ֲ�ӯ��"),_T("ռ��֤��"),_T("��ӯ��")};
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
	
	TCHAR* lpHdrs4[ALLINST_ITMES] = {_T("����"),_T("��Լ"),_T("��Լ��"),_T("������"),_T("����"),_T("���"),
	_T("����"),_T("�������"),_T("��֤����"),_T("��������")};
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

void COperaPage::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
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
