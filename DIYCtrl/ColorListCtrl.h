#if !defined(AFX_COLORLISTCTRL_H__3A1C5A98_FD1C_46CE_8580_42FA0B63009B__INCLUDED_)
#define AFX_COLORLISTCTRL_H__3A1C5A98_FD1C_46CE_8580_42FA0B63009B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColorListCtrl.h : header file
//
#include <afxcmn.h>
/////////////////////////////////////////////////////////////////////////////
// CColorListCtrl window

class CColorListCtrl : public CListCtrl
{
// Construction
public:
	CColorListCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorListCtrl)
	public:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CColorListCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CColorListCtrl)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
public:
	void SetColor(int iRow, int iCol, COLORREF color);
	afx_msg   void   OnNMCustomdraw(NMHDR   *pNMHDR,   LRESULT   *pResult);
public:
	void SetSubItemFont(LOGFONT font, COLORREF color, long lsize);
	int m_iRow;
	int m_iCol;
	COLORREF m_Color;
	COLORREF m_TextColor;
	CFont *m_Font;
	long m_TextSize;
	/*std::vector<int> m_vctRow;
	std::vector<int> m_vctCol;
	std::vector<COLORREF> m_vctColorValue;
	std::vector<bool> m_vctFlag;*/
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORLISTCTRL_H__3A1C5A98_FD1C_46CE_8580_42FA0B63009B__INCLUDED_)
