// DlgTransparent.cpp : implementation file
//

#include "stdafx.h"
#include "CButtonST_Demo.h"
#include "DlgTransparent.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgTransparent::CDlgTransparent(CWnd* pParent /*=NULL*/)
	: CBkDialog(CDlgTransparent::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgTransparent)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_bFirstTime = TRUE;
}

void CDlgTransparent::DoDataExchange(CDataExchange* pDX)
{
	CBkDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgTransparent)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

	// Make our buttons a CButtonST buttons
	DDX_Control(pDX, IDC_BTNLOGOFF, m_btnLogOff);
	DDX_Control(pDX, IDC_BTNWORKGROUP, m_btnWorkgroup);
	DDX_Control(pDX, IDC_BTNOPEN, m_btnOpen);
	DDX_Control(pDX, IDC_BTNSEARCH, m_btnSearch);
	DDX_Control(pDX, IDC_BTNEXPLORER, m_btnExplorer);
	DDX_Control(pDX, IDC_BTNHELP, m_btnHelp);
	DDX_Control(pDX, IDC_BTNABOUT, m_btnAbout);
	DDX_Control(pDX, IDC_BTNCANNIBAL, m_btnCannibal);
	DDX_Control(pDX, IDC_CHECK, m_chkCheckbox);
}

BEGIN_MESSAGE_MAP(CDlgTransparent, CBkDialog)
	//{{AFX_MSG_MAP(CDlgTransparent)
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CDlgTransparent::OnInitDialog() 
{
	CBkDialog::OnInitDialog();
	
	//SetBitmap(IDB_SKY);
	SetBitmap(IDB_BMP_TEST);

	// Log off button
	m_btnLogOff.SetIcon(IDI_LOGOFF);
	
	// Workgroup button
	m_btnWorkgroup.SetIcon(IDI_WORKGROUP);

	// Toolbar buttons
	m_btnOpen.SetIcon(IDI_OPEN, (int)BTNST_AUTO_GRAY);
	m_btnOpen.SetTooltipText(_T("Open"));
	m_btnSearch.SetIcon(IDI_SEARCH1, (int)BTNST_AUTO_GRAY);
	m_btnSearch.SetTooltipText(_T("Search"));
	m_btnExplorer.SetIcon(IDI_EXPLORER, (int)BTNST_AUTO_GRAY);
	m_btnExplorer.SetTooltipText(_T("Internet explorer"));
	m_btnHelp.SetIcon(IDI_HELP, (int)BTNST_AUTO_GRAY);
	m_btnHelp.SetTooltipText(_T("Help"));
	m_btnHelp.SetMenu(IDR_MENU, m_hWnd);
 
	m_btnAbout.SetIcon(IDI_ABOUT, (int)BTNST_AUTO_GRAY);
	m_btnAbout.SetTooltipText(_T("About"));

	// Cannibal button
	m_btnCannibal.SetBitmaps(IDB_CANNIBAL, RGB(0, 255, 0));
	// Checkbox
	m_chkCheckbox.SetIcon(IDI_LEDON, IDI_LEDOFF);

	return TRUE;
} // End of OnInitDialog

void CDlgTransparent::OnOK()
{
} // End of OnOK

void CDlgTransparent::OnCancel()
{
} // End of OnCancel

void CDlgTransparent::OnSetFocus(CWnd* pOldWnd) 
{
	CBkDialog::OnSetFocus(pOldWnd);

	// Do only one time
	if (m_bFirstTime == TRUE)
	{
		m_bFirstTime = FALSE;
		m_btnLogOff.DrawTransparent(TRUE);
		m_btnWorkgroup.DrawTransparent(TRUE);
		m_btnOpen.DrawTransparent(TRUE);
		m_btnSearch.DrawTransparent(TRUE);
		m_btnExplorer.DrawTransparent(TRUE);
		m_btnHelp.DrawTransparent(TRUE);
		m_btnAbout.DrawTransparent(TRUE);
		m_btnCannibal.DrawTransparent(TRUE);
		m_chkCheckbox.DrawTransparent(TRUE);
	} // if
} // End of OnSetFocus
