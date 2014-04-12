// ColorListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "ColorListCtrl.h"
#include "Color.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorListCtrl

CColorListCtrl::CColorListCtrl()
{
	m_iRow = -1;
	m_iCol = -1;
	m_Font = NULL;
}

CColorListCtrl::~CColorListCtrl()
{
	if ( m_Font )    delete m_Font;        //É¾³ý×ÖÌå
}


BEGIN_MESSAGE_MAP(CColorListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CColorListCtrl)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW,   OnNMCustomdraw)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorListCtrl message handlers

void CColorListCtrl::PreSubclassWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
	CListCtrl::PreSubclassWindow();

	DWORD dwStyle, dwExStyle;
	dwStyle = GetWindowLong(GetSafeHwnd(), GWL_STYLE);
	dwStyle |= LVS_REPORT;
	SetWindowLong(GetSafeHwnd(), GWL_STYLE, dwStyle);
	dwExStyle = GetExtendedStyle();
	dwExStyle |= LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT| LVS_EX_HEADERDRAGDROP| LVS_EX_GRIDLINES;
	SetExtendedStyle(dwExStyle);
	
	CImageList m_ImageList; 
	m_ImageList.Create(1,22,ILC_COLOR,1,1); 	 
	SetImageList(&m_ImageList,LVSIL_SMALL);
}

void   CColorListCtrl::OnNMCustomdraw(NMHDR   *pNMHDR,   LRESULT   *pResult)   
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );
	
    // Take the default processing unless we set this to something else below.
    *pResult = CDRF_DODEFAULT;
	
    // First thing - check the draw stage. If it's the control's prepaint
    // stage, then tell Windows we want messages for every item.
	
    if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )
	{
        *pResult = CDRF_NOTIFYITEMDRAW;
	}
    else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )
	{
        // This is the notification message for an item. We'll request
        // notifications before each subitem's prepaint stage.
        *pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
    else if ( (CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage )
	{
        // This is the prepaint stage for a subitem. Here's where we set the
        // item's text and background colors. Our return value will tell 
        // Windows to draw the subitem itself, but it will use the new colors
        // we set here. 
        if ( m_iCol == pLVCD->iSubItem && m_iRow == pLVCD->nmcd.dwItemSpec)
		{
			pLVCD->clrTextBk = m_Color;
			pLVCD->clrText = m_TextColor;
			//SetFont(m_Font, false);
		}
		else
		{
			pLVCD->clrTextBk = WHITE;
			pLVCD->clrText = BLACK;
		}
		//SetFont(m_Font, false);
        // Store the colors back in the NMLVCUSTOMDRAW struct.
        // Tell Windows to paint the control itself.
        *pResult = CDRF_DODEFAULT;
	}
}

void CColorListCtrl::SetColor(int iRow, int iCol, COLORREF color)
{
	m_iRow = iRow;
	m_iCol = iCol;
	m_Color = color;
	/*m_vctRow.push_back(iRow);
	m_vctCol.push_back(iCol);
	m_vctColorValue.push_back(color);*/
}

void CColorListCtrl::SetSubItemFont(LOGFONT font, COLORREF color, long lsize)
{
	if ( m_Font )	delete m_Font;	//É¾³ý¾É×ÖÌå
	m_Font = new CFont;
	m_Font->CreateFontIndirect(&font);
	m_TextColor = color;
	m_TextSize = lsize;
}