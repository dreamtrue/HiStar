// TradePage.cpp : 实现文件
//

#include "stdafx.h"
#include "HiStar.h"
#include "TradePage.h"
#include "afxdialogex.h"


// CTradePage 对话框

IMPLEMENT_DYNAMIC(CTradePage, CDialogEx)

	CTradePage::CTradePage(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTradePage::IDD, pParent)
{
}

CTradePage::~CTradePage()
{
}

void CTradePage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, m_tab);
	DDX_Control(pDX, IDC_LIST7, m_LstOnRoad);
	DDX_Control(pDX, IDC_LIST8, m_LstOrdInf);
	DDX_Control(pDX, IDC_LIST9, m_LstTdInf);
	DDX_Control(pDX, IDC_LIST10, m_LstInvPosInf);
	DDX_Control(pDX, IDC_LIST11, m_LstAllInsts);
}


BEGIN_MESSAGE_MAP(CTradePage, CDialogEx)
	ON_BN_CLICKED(IDOK, &CTradePage::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CTradePage::OnBnClickedCancel)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CTradePage::OnTcnSelchangeTab1)
END_MESSAGE_MAP()


// CTradePage 消息处理程序


void CTradePage::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
}


void CTradePage::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnCancel();
}


BOOL CTradePage::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_tab.InsertItem( 0, _T("挂单") );
	m_tab.InsertItem( 1, _T("委托") );
	m_tab.InsertItem( 2, _T("持仓") );
	m_tab.InsertItem( 3, _T("成交") );
	m_tab.InsertItem( 4, _T("合约") );
	m_tab.InsertItem( 5, _T("其它") );
	m_tab.SetCurSel(0);
	m_LstTdInf.ShowWindow( SW_HIDE );
	m_LstOrdInf.ShowWindow( SW_HIDE );
	m_LstInvPosInf.ShowWindow( SW_HIDE );
	m_LstAllInsts.ShowWindow( SW_HIDE );
	m_LstOnRoad.ShowWindow( SW_SHOW );
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}
void CTradePage::InitAllHdrs(void)
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


void CTradePage::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	switch ( m_tab.GetCurSel() )
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
void CTradePage::InitAllVecs()
{
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();

	//////////////////////////
	m_orderVec = pApp->m_cT->m_orderVec;
	m_tradeVec = pApp->m_cT->m_tradeVec;
	m_InsinfVec = pApp->m_cT->m_InsinfVec;
	m_MargRateVec = pApp->m_cT->m_MargRateVec;
	m_StmiVec = pApp->m_cT->m_StmiVec;
	m_AccRegVec = pApp->m_cT->m_AccRegVec;
	m_TdCodeVec = pApp->m_cT->m_TdCodeVec;
	m_InvPosVec = pApp->m_cT->m_InvPosVec;
	m_BfTransVec = pApp->m_cT->m_BfTransVec;
	m_FeeRateRev = pApp->m_cT->m_FeeRateRev;
	m_TdAcc = pApp->m_cT->m_TdAcc;
	for (int i=0;i<4;i++)
	{
		m_tsEXnLocal[i] = pApp->m_cT->m_tsEXnLocal[i];
	}
	m_onRoadVec = m_orderVec;
	///////////////////////////////////////////////////
	for(VOrd odIt=m_onRoadVec.begin(); odIt!=m_onRoadVec.end();)
	{
		if((*odIt)->OrderStatus !='1' && (*odIt)->OrderStatus !='3'  )
		{odIt = m_onRoadVec.erase(odIt);}
		else
			++odIt;
	}

	//////////////////////////////////////////////////
	for(VInvP vip=m_InvPosVec.begin(); vip!=m_InvPosVec.end();)
	{
		if((*vip)->YdPosition==0 && (*vip)->Position ==0)
		{vip = m_InvPosVec.erase(vip);}
		else
			++vip;
	}
	///////////////////////////////////////////////////////////
}