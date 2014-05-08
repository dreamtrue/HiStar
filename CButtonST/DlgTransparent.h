#ifndef _DLGTRANSPARENT_H_
#define _DLGTRANSPARENT_H_

#include "BkDialog.h"
#include "BtnST.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDlgTransparent : public CBkDialog
{
public:
	CDlgTransparent(CWnd* pParent = NULL);

	//{{AFX_DATA(CDlgTransparent)
	enum { IDD = IDD_TRANSPARENT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgTransparent)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

protected:
	// Generated message map functions
	//{{AFX_MSG(CDlgTransparent)
	virtual BOOL OnInitDialog();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG

private:
	void OnCancel();
	void OnOK();

	CButtonST	m_btnLogOff;
	CButtonST	m_btnWorkgroup;
	CButtonST	m_btnOpen;
	CButtonST	m_btnSearch;
	CButtonST	m_btnExplorer;
	CButtonST	m_btnHelp;
	CButtonST	m_btnAbout;
	CButtonST	m_btnCannibal;
	CButtonST	m_chkCheckbox;

	BOOL		m_bFirstTime;

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
