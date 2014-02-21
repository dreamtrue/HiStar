// TradePage.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "HiStar.h"
#include "TradePage.h"
#include "afxdialogex.h"
#include <algorithm>

bool cmpInst(const CThostFtdcInstrumentFieldEx* pfirst,const CThostFtdcInstrumentFieldEx* psecond) 
{    
	int iRes = stricmp(pfirst->iinf.InstrumentID,psecond->iinf.InstrumentID);
	return (iRes<=0);
}

// CTradePage �Ի���

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
	ON_NOTIFY(NM_DBLCLK, IDC_LIST7, OnNMDblclkOnroad)
	ON_NOTIFY(NM_CLICK, IDC_LIST7, OnNMClkLstOnroad)
	ON_NOTIFY(NM_RCLICK, IDC_LIST7, OnNMRClkLstOnroad)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST7, OnNMDblclkOnroad)
	ON_NOTIFY(NM_CLICK, IDC_LIST7, OnNMClkLstOnroad)
	ON_NOTIFY(NM_RCLICK, IDC_LIST7, OnNMRClkLstOnroad)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST8, OnNMDblclkOrdInf)
	ON_NOTIFY(NM_CLICK, IDC_LIST8, OnNMClkLstOrdInf)
	ON_NOTIFY(NM_RCLICK, IDC_LIST8, OnNMRClkLstOrdInf)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST9, OnNMDblclkTdInf)
	ON_NOTIFY(NM_CLICK, IDC_LIST9, OnNMClkLstTdInf)
	ON_NOTIFY(NM_RCLICK, IDC_LIST9, OnNMRClkLstTdInf)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST10, OnNMDblclkInvPInf)
	ON_NOTIFY(NM_CLICK, IDC_LIST10, OnNMClkLstInvPInf)
	ON_NOTIFY(NM_RCLICK, IDC_LIST10, OnNMRClkLstInvPInf)
	ON_NOTIFY(NM_CLICK, IDC_LIST11, OnNMClkLstInsts)
	ON_NOTIFY(NM_RCLICK, IDC_LIST11, OnNMRClkLstInsts)
	//��ȡ�����б���Ϣ
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST7, OnGetDispinf1)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST8, OnGetDispinf2)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST10, OnGetDispinf3)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST9, OnGetDispinf4)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST11, OnGetDispinf5)
END_MESSAGE_MAP()


// CTradePage ��Ϣ�������


void CTradePage::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//CDialogEx::OnOK();
}


void CTradePage::OnBnClickedCancel()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//CDialogEx::OnCancel();
}


BOOL CTradePage::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	m_tab.InsertItem( 0, _T("�ҵ�") );
	m_tab.InsertItem( 1, _T("ί��") );
	m_tab.InsertItem( 2, _T("�ֲ�") );
	m_tab.InsertItem( 3, _T("�ɽ�") );
	m_tab.InsertItem( 4, _T("��Լ") );
	m_tab.InsertItem( 5, _T("����") );
	m_tab.SetCurSel(0);
	m_LstTdInf.ShowWindow( SW_HIDE );
	m_LstOrdInf.ShowWindow( SW_HIDE );
	m_LstInvPosInf.ShowWindow( SW_HIDE );
	m_LstAllInsts.ShowWindow( SW_HIDE );
	m_LstOnRoad.ShowWindow( SW_SHOW );
	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}
void CTradePage::InitAllHdrs(void)
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

void CTradePage::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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

void CTradePage::OnNMDblclkOnroad(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;
}

void CTradePage::OnNMClkLstOnroad(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstOnRoad.GetItemCount();
		if (iCount)
		{
			m_LstOnRoad.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			m_szInst=m_LstOnRoad.GetItemText(nItem,1); //��Լ
		}
	}
	*pResult = 0;
}

void CTradePage::OnNMDblclkOrdInf(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;
	
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstOrdInf.GetItemCount();	
		m_LstOrdInf.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	}
	*pResult = 0;
}

void CTradePage::OnNMClkLstOrdInf(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;	
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstOrdInf.GetItemCount();
		if (iCount)
		{
			m_LstOrdInf.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			m_szInst=m_LstOrdInf.GetItemText(nItem,1); //��Լ
		}
	}
	*pResult = 0;
}

void CTradePage::OnNMRClkLstOrdInf(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;	
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstOrdInf.GetItemCount();
		m_LstOrdInf.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		iCount = m_LstOrdInf.GetItemCount();
		if (iCount>0)
		{
			CPoint pt;
			GetCursorPos(&pt);
		}
	}
	*pResult = 0;
}

void CTradePage::OnNMDblclkTdInf(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstTdInf.GetItemCount();	
		m_LstTdInf.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	}
	*pResult = 0;
}
void CTradePage::OnNMClkLstTdInf(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstTdInf.GetItemCount();	
		if (iCount)
		{
			m_LstTdInf.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			m_szInst=m_LstTdInf.GetItemText(nItem,0); //��Լ
		}
	}
	*pResult = 0;
}
void CTradePage::OnNMRClkLstTdInf(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;	
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstTdInf.GetItemCount();	
		m_LstTdInf.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		iCount = m_LstTdInf.GetItemCount();
		if (iCount>0)
		{
			CPoint pt;
			GetCursorPos(&pt);		
			CMenu menu;
			menu.CreatePopupMenu();
		}
	}
	*pResult = 0;
}

void CTradePage::OnNMDblclkInvPInf(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;	
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstInvPosInf.GetItemCount();

		m_LstInvPosInf.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	}
	*pResult = 0;
}
void CTradePage::OnNMClkLstInvPInf(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstInvPosInf.GetItemCount();
		
		m_LstInvPosInf.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		if (iCount)
		{
			m_LstInvPosInf.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			m_szInst=m_LstInvPosInf.GetItemText(nItem,0); //��Լ
		}
	}
	*pResult = 0;
}
void CTradePage::OnNMRClkLstInvPInf(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstInvPosInf.GetItemCount();	
		m_LstInvPosInf.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);	
		iCount =m_LstInvPosInf.GetItemCount();
		if (iCount>0)
		{
			CPoint pt;
			GetCursorPos(&pt);	
			CMenu menu;
			menu.CreatePopupMenu();
		}
	}
	*pResult = 0;
}

void CTradePage::OnNMClkLstInsts(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstAllInsts.GetItemCount();
		
		m_LstAllInsts.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		if (iCount)
		{
			m_LstAllInsts.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			m_szInst=m_LstAllInsts.GetItemText(nItem,1); //��Լ	
		}
	}
	*pResult = 0;
}

void CTradePage::OnNMRClkLstInsts(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstAllInsts.GetItemCount();	
		m_LstAllInsts.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);		
		iCount = m_LstAllInsts.GetItemCount();
		
		if (iCount>0)
		{
			CPoint pt;
			GetCursorPos(&pt);
		}

	}
	*pResult = 0;
}
void CTradePage::OnNMRClkLstOnroad(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstOnRoad.GetItemCount();

		m_LstOnRoad.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		
		if (iCount>0)
		{
			CMenu PopMenu,*pSubMenu;
			CPoint pt;
			GetCursorPos(&pt);
			pSubMenu = PopMenu.GetSubMenu(0);	
			pSubMenu->TrackPopupMenu(TPM_LEFTALIGN,pt.x,pt.y,this);		
		}
	}
	*pResult = 0;
}
 
void CTradePage::OnGetDispinf1(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	LV_ITEM* pItem= &(pDispInfo)->item;
	int iItem= m_onRoadVec.size()-1-pItem->iItem;
	if(iItem < 0)return;
	if(pItem->mask & LVIF_TEXT)
	{
		CString szTemp = _T("");
		int iLen=0;
		switch(pItem->iSubItem)
		{
		case 0:
			lstrcpy(pItem->pszText,m_onRoadVec[iItem]->OrderSysID);
			break;
		case 1: 
			lstrcpy(pItem->pszText,m_onRoadVec[iItem]->InstrumentID);
			break;
		case 2:
			szTemp = JgBsType(m_onRoadVec[iItem]->Direction);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 3:
			szTemp=JgOcType(m_onRoadVec[iItem]->CombOffsetFlag[0]);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 4:
			szTemp.Format(_T("%d"),m_onRoadVec[iItem]->VolumeTotal);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 5:
			szTemp.Format(_T("%f"),m_onRoadVec[iItem]->LimitPrice);
			szTemp.TrimRight('0');
			iLen = szTemp.GetLength();
			if (szTemp.Mid(iLen-1,1)==_T(".")) {szTemp.TrimRight(_T("."));}
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 6:
			lstrcpy(pItem->pszText,m_onRoadVec[iItem]->InsertTime);
			break;
		case 7:
			szTemp.Format(_T("%d"),m_onRoadVec[iItem]->BrokerOrderSeq);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 8:
			lstrcpy(pItem->pszText,UNCOMP);
			break;
		}
	}
	*pResult = 0;
}


void CTradePage::OnGetDispinf2(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	LV_ITEM* pItem= &(pDispInfo)->item;
	int iItem= m_orderVec.size()-1-pItem->iItem;
	if(iItem < 0)return;
	if(pItem->mask & LVIF_TEXT)
	{
		CString szTemp = _T("");
		int iLen=0;
		switch(pItem->iSubItem)
		{
		case 0:
			lstrcpy(pItem->pszText,m_orderVec[iItem]->OrderSysID);
			break;
		case 1:
			lstrcpy(pItem->pszText,m_orderVec[iItem]->InstrumentID);
			break;
		case 2: 
			szTemp = JgBsType(m_orderVec[iItem]->Direction);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 3:
			szTemp = JgOcType(m_orderVec[iItem]->CombOffsetFlag[0]);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 4:
			szTemp=JgOrdStatType(m_orderVec[iItem]->OrderStatus);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
		case 5:
			szTemp.Format(_T("%f"),m_orderVec[iItem]->LimitPrice);
			szTemp.TrimRight('0');
			iLen = szTemp.GetLength();
			if (szTemp.Mid(iLen-1,1)==_T(".")) {szTemp.TrimRight(_T("."));}
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 6:
			szTemp.Format(_T("%d"),m_orderVec[iItem]->VolumeTotalOriginal);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 7:
			szTemp.Format(_T("%d"),m_orderVec[iItem]->VolumeTotal);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 8:
			szTemp.Format(_T("%d"),m_orderVec[iItem]->VolumeTraded);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 9:
			lstrcpy(pItem->pszText,UNCOMP);
			break;
		case 10:
			lstrcpy(pItem->pszText,m_orderVec[iItem]->InsertTime);
			break;
		case 11:
			szTemp.Format(_T("%d"),m_orderVec[iItem]->BrokerOrderSeq);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 12:
			lstrcpy(pItem->pszText,UNCOMP);
			break;
		case 13:
			lstrcpy(pItem->pszText,UNCOMP);
			break;
		case 14:
			szTemp =JgOrdSubmitStat(m_orderVec[iItem]->OrderSubmitStatus);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		}
	}
	*pResult = 0;
}

void CTradePage::OnGetDispinf3(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	LV_ITEM* pItem= &(pDispInfo)->item;
	int iItem= pItem->iItem;
	if(iItem >= m_InvPosVec.size())return;
	if(pItem->mask & LVIF_TEXT)
	{
		CString szTemp = _T("");
		int iLen=0;
		double dAvPrice;
		switch(pItem->iSubItem)
		{
		case 0:
			lstrcpy(pItem->pszText,m_InvPosVec[iItem]->InstrumentID);
			break;
		case 1: 
			szTemp = JgBsType(m_InvPosVec[iItem]->PosiDirection-2);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 2:
			szTemp.Format(_T("%d"),m_InvPosVec[iItem]->YdPosition+m_InvPosVec[iItem]->Position);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 3:
			{
				if (m_InvPosVec[iItem]->PosiDirection == '2')
				{
					iLen = m_InvPosVec[iItem]->YdPosition+m_InvPosVec[iItem]->Position-m_InvPosVec[iItem]->ShortFrozen;
					szTemp.Format(_T("%d"),iLen);
				}
				if (m_InvPosVec[iItem]->PosiDirection == '3')
				{
					iLen = m_InvPosVec[iItem]->YdPosition+m_InvPosVec[iItem]->Position-m_InvPosVec[iItem]->LongFrozen;
					szTemp.Format(_T("%d"),iLen);
				}
			}
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 4:
			dAvPrice = m_InvPosVec[iItem]->PositionCost/(m_InvPosVec[iItem]->YdPosition+m_InvPosVec[iItem]->Position)/(FindInstMul(m_InvPosVec[iItem]->InstrumentID));
			szTemp.Format(_T("%.3f"),dAvPrice);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 5:
			szTemp.Format(_T("%f"),m_InvPosVec[iItem]->PositionProfit);
			szTemp.TrimRight('0');
			iLen = szTemp.GetLength();
			if (szTemp.Mid(iLen-1,1)==_T(".")) {szTemp.TrimRight(_T("."));}
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 6:
			szTemp.Format(_T("%f"),m_InvPosVec[iItem]->UseMargin);
			szTemp.TrimRight('0');
			iLen = szTemp.GetLength();
			if (szTemp.Mid(iLen-1,1)==_T(".")) {szTemp.TrimRight(_T("."));}
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 7:
			lstrcpy(pItem->pszText,UNCOMP);
			break;
		}
	}
	*pResult = 0;
}

void CTradePage::OnGetDispinf4(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);	
	LV_ITEM* pItem= &(pDispInfo)->item;
	int iItemIndex= m_tradeVec.size()-1-pItem->iItem;
	if(iItemIndex < 0)return;//��ֹ����Խ��
	if(pItem->mask & LVIF_TEXT)
	{
		CString szTemp = _T("");
		int iLen=0;
		switch(pItem->iSubItem)
		{
		case 0:
			lstrcpy(pItem->pszText,m_tradeVec[iItemIndex]->InstrumentID);
			break;
		case 1: 
			szTemp = JgBsType(m_tradeVec[iItemIndex]->Direction);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 2:
			szTemp = JgOcType(m_tradeVec[iItemIndex]->OffsetFlag);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 3:
			szTemp.Format(_T("%f"),m_tradeVec[iItemIndex]->Price);
			szTemp.TrimRight('0');
			iLen = szTemp.GetLength();
			if (szTemp.Mid(iLen-1,1)==_T(".")) {szTemp.TrimRight(_T("."));}
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 4:
			szTemp.Format(_T("%d"),m_tradeVec[iItemIndex]->Volume);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 5:
			lstrcpy(pItem->pszText,m_tradeVec[iItemIndex]->TradeTime);
			break;
		case 6:
			lstrcpy(pItem->pszText,UNCOMP);
			break;
		case 7:
			szTemp=JgTbType(m_tradeVec[iItemIndex]->HedgeFlag);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 8:
			szTemp=JgTdType(m_tradeVec[iItemIndex]->TradeType);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 9:
			szTemp=JgExchage(m_tradeVec[iItemIndex]->ExchangeID);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 10:
			lstrcpy(pItem->pszText,m_tradeVec[iItemIndex]->TradeID);
			break;
		case 11:
			lstrcpy(pItem->pszText,m_tradeVec[iItemIndex]->OrderLocalID);
			break;
		}
	}
	*pResult = 0;
}

void CTradePage::OnGetDispinf5(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	LV_ITEM* pItem= &(pDispInfo)->item;
	int iItem= pItem->iItem;
	if(iItem >= m_InsinfVec.size())return;
	CString szTemp;
	if(pItem->mask & LVIF_TEXT)
	{
		int iLen=0;
		switch(pItem->iSubItem)
		{
		case 0:
			strcpy(pItem->pszText, m_InsinfVec[iItem]->iinf.ProductID);
			break;
		case 1: 
			strcpy(pItem->pszText, m_InsinfVec[iItem]->iinf.InstrumentID);
			break;
		case 2:
			strcpy(pItem->pszText,m_InsinfVec[iItem]->iinf.InstrumentName);
			break;
		case 3:
			szTemp = JgExchage(m_InsinfVec[iItem]->iinf.ExchangeID);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 4:
			szTemp.Format(_T("%d"),m_InsinfVec[iItem]->iinf.VolumeMultiple);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 5:
			szTemp.Format(_T("%f"),m_InsinfVec[iItem]->iinf.PriceTick);
			szTemp.TrimRight('0');
			iLen = szTemp.GetLength();
			if (szTemp.Mid(iLen-1,1)==_T(".")) {szTemp.TrimRight(_T("."));}
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 6:
			szTemp = JgProType((BYTE)m_InsinfVec[iItem]->iinf.ProductClass);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 7:
			strcpy(pItem->pszText,m_InsinfVec[iItem]->iinf.ExpireDate);
			break;
		case 8:
			szTemp.Format(_T("%d%%"),D2Int(m_InsinfVec[iItem]->iinf.LongMarginRatio*100));
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		case 9:
			Fee2String(szTemp,m_InsinfVec[iItem]->OpenRatioByMoney,m_InsinfVec[iItem]->OpenRatioByVolume,m_InsinfVec[iItem]->CloseRatioByMoney,
				m_InsinfVec[iItem]->CloseRatioByVolume,m_InsinfVec[iItem]->CloseTodayRatioByMoney,m_InsinfVec[iItem]->CloseTodayRatioByVolume);
			lstrcpy(pItem->pszText,(LPCTSTR)szTemp);
			break;
		}
	}
	*pResult = 0;
}
int CTradePage::FindInstMul(TThostFtdcInstrumentIDType InstID)
{
	bool founded=false;
	int iMul = 1;
	for (UINT i=0; i<m_InsinfVec.size();i++)
	{
		if (!strcmp(InstID,m_InsinfVec[i]->iinf.InstrumentID))
		{
			iMul=m_InsinfVec[i]->iinf.VolumeMultiple;
			founded=true;
			break;
		}
	}
	if (founded) {return iMul;}
	return (-1);
}

int CTradePage::FindOrdInOrderVec(TThostFtdcSequenceNoType BkrOrdSeq)
{
	UINT i=0;
	for(i=0; i<m_orderVec.size(); i++)
	{
		if(m_orderVec[i]->BrokerOrderSeq == BkrOrdSeq) 
		{ return i;}
	}
	return (-1);
}

int CTradePage::FindOrdInOnRoadLst(TThostFtdcSequenceNoType BkrOrdSeq)
{
	int nRow = -1;
	int nItem = m_LstOnRoad.GetItemCount();
	if (nItem<1) return (-1);
	CString szID1,szID2;
	szID1.Format(_T("%d"),BkrOrdSeq);
	//szID1.ReleaseBuffer();
	//szID1.TrimLeft();
	for (nRow = 0; nRow < nItem; nRow++)
	{
		szID2 = m_LstOnRoad.GetItemText(nRow,7);
		if (szID1.Compare(szID2)==0)
		{ return nRow; }
	}
	return (-1);
}

int CTradePage::FindOrdInOnRoadVec(TThostFtdcSequenceNoType BkrOrdSeq)
{	
	UINT i=0;
	for(i=0; i<m_onRoadVec.size(); i++)
	{
		if(m_onRoadVec[i]->BrokerOrderSeq == BkrOrdSeq) 
		{ return i;}
	}
	return (-1);
}
void  CTradePage::OnCancelAll()
{
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	for (UINT i=0;i<m_onRoadVec.size();i++)
	{
		pApp->m_cT->ReqOrderCancel(m_onRoadVec[i]->BrokerOrderSeq);	
		//m_LstOnRoad.DeleteItem(0);
	}
}

void CTradePage::FiltInsList()
{
	VIT_if vif;
	VIT_mr vmr;
	VIT_cf vcf;
	for(vif=m_InsinfVec.begin(); vif!=m_InsinfVec.end();)
	{
		if(((*vif)->iinf.ProductClass > '2') || ((*vif)->iinf.IsTrading==0))
		{vif = m_InsinfVec.erase(vif);}
		else
			++vif;
	}
	for(vif=m_InsinfVec.begin(); vif!=m_InsinfVec.end();vif++)
	{
		for(vmr=m_MargRateVec.begin(); vmr!=m_MargRateVec.end();vmr++)
		{
			//ShowErroTips(MY_TIPS,MY_TIPS);
			if (!strcmp((*vmr)->InstrumentID,(*vif)->iinf.InstrumentID))
			{
				(*vif)->iinf.LongMarginRatio = (*vmr)->LongMarginRatioByMoney;

				(*vif)->iinf.ShortMarginRatio = (*vmr)->ShortMarginRatioByMoney;
			}
		}
	}
	for (vcf=m_FeeRateVec.begin();vcf!=m_FeeRateVec.end();vcf++)
	{	
		for(vif=m_InsinfVec.begin(); vif!=m_InsinfVec.end();vif++)
		{
			if (!strcmp((*vcf)->InstrumentID,(*vif)->iinf.ProductID))
			{
				(*vif)->OpenRatioByMoney = (*vcf)->OpenRatioByMoney;
				(*vif)->OpenRatioByVolume = (*vcf)->OpenRatioByVolume;
				(*vif)->CloseRatioByMoney = (*vcf)->CloseRatioByMoney;
				(*vif)->CloseRatioByVolume = (*vcf)->CloseRatioByVolume;
				(*vif)->CloseTodayRatioByMoney = (*vcf)->CloseTodayRatioByMoney;
				(*vif)->CloseTodayRatioByVolume = (*vcf)->CloseTodayRatioByVolume;
			}
		}
	}
	sort(m_InsinfVec.begin(),m_InsinfVec.end(),cmpInst);	
}