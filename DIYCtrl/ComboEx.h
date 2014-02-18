
#pragma once

/////////////////////////////////////////////////////////////////////////////
// CComboEx window

class CComboEx : public CComboBox
{
// Construction
public:
	CComboEx();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CComboEx)
	//public:
	//virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CComboEx();

	BOOL m_bAutoComplete;

	// Generated message map functions
protected:
	//{{AFX_MSG(CComboEx)
	afx_msg void OnEditUpdate();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

