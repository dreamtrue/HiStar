
#include "stdafx.h"
#include "ColorStatic.h"
#include <float.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorStatic

CColorStatic::CColorStatic()
{
	m_crBkColor = BLACK; // Initializing the Background Color to the system face color.
	//m_crTextColor = YELLOW; // Initializing the text to Black
	//m_uAlign = DT_CENTER;

	SetFont(_T("Arial Black"),17,FW_SEMIBOLD);
}

CColorStatic::~CColorStatic()
{
}

BEGIN_MESSAGE_MAP(CColorStatic, CStatic)
	//{{AFX_MSG_MAP(CColorStatic)
	//ON_WM_CTLCOLOR_REFLECT()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CColorStatic::OnEraseBkgnd(CDC* pDC)
{

	return TRUE;
}

void CColorStatic::OnPaint()
{
	CPaintDC dc(this);

	CDC memDC;
  CRect rc;
	CBitmap memBitmap;
	CBitmap* oldBitmap;
  CFont *oldFont;
	
  GetClientRect(&rc);
	
	memDC.CreateCompatibleDC(&dc);
	memBitmap.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());
	oldBitmap = (CBitmap *)memDC.SelectObject(&memBitmap);
	
  m_brBkgnd.CreateSolidBrush(m_crBkColor);
  memDC.FillRect(&rc, &m_brBkgnd);
  m_brBkgnd.DeleteObject();

  memDC.SetBkMode(TRANSPARENT);
  memDC.SetBkColor(m_crBkColor);
  memDC.SetTextColor(m_crTextColor);

	oldFont = memDC.SelectObject(&m_cFont);

  memDC.DrawText(m_szText,-1,&rc,m_uAlign);
	
  memDC.SelectObject(oldFont);
  dc.BitBlt(rc.left, rc.top,rc.right-rc.left, rc.bottom-rc.top,&memDC,0, 0,SRCCOPY);
	
  memDC.SelectObject(oldBitmap);
  memBitmap.DeleteObject();
  memDC.DeleteDC();

}

void CColorStatic::SetBkColor(COLORREF crColor)
{
	m_crBkColor = crColor; // Passing the value passed by the dialog to the member varaible for Backgound Color

}

void CColorStatic::SetWindowText(LPCTSTR lpszString,COLORREF crColor,UINT uAlign)
{
	m_szText = lpszString;
	m_crTextColor = crColor;
	m_uAlign = uAlign;

	Invalidate(TRUE);
}

void CColorStatic::SetInt(int iVal,COLORREF crColor,UINT uAlign)
{
	TCHAR szTemp[MAX_PATH];
	_stprintf(szTemp,_T("%d"),iVal);

	SetWindowText(szTemp, crColor,uAlign);
}


void CColorStatic::SetDouble(double dVal,COLORREF crColor,UINT uAlign)
{
	CString szTemp;
	if (dVal==DBL_MAX)
	{
		szTemp = _T("¨D");
	}
	else
	{
		szTemp.Format(_T("%.02lf"),dVal);	
		//szTemp.TrimRight(_T("0"));
		int iLen = szTemp.GetLength();
		if (szTemp.Mid(iLen-1,1)==_T(".")) {szTemp.TrimRight(_T("."));}
	}
	SetWindowText(szTemp, crColor,uAlign);
}

BOOL CColorStatic::SetFont(LPCTSTR szFont,int iHeight,int iWeight)
{
	LOGFONT oldFt;
	if (m_cFont.m_hObject != NULL)
	{ 
		GetFont(&oldFt);
		if (!_tcsicmp(oldFt.lfFaceName,szFont) && (oldFt.lfHeight == iHeight) && (oldFt.lfWeight==iWeight))
		{
			return TRUE;
		}
		else
		{
			m_cFont.DeleteObject(); 
		}
		
	}
	
	return	m_cFont.CreateFont(iHeight, 0, 0, 0, iWeight,
		FALSE, FALSE, 0, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, 
		CLIP_DEFAULT_PRECIS,
		CLEARTYPE_NATURAL_QUALITY, 
		DEFAULT_PITCH|FF_SWISS, szFont);
}

int CColorStatic::GetFont(LOGFONT* pLogFont)
{
	return m_cFont.GetLogFont(pLogFont);
}