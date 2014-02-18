
#pragma once


////////////////////////////////////////////////////////////
// NumSpinCtrl.h : header file
//
// Damir Valiulin, Rocscience Inc., 2002-07-03
//
// This simple class allows you to work with doubles for spin control.
//
// 1. Use SetRangeAndDelta to specify your value range and the delta change
//
// 2. Set current position with a call to SetPos
//
// 3. Specify output precision in 3 ways:
//    a) Default is "%g" value formatting. You can achieve the same by calling
//       SetDecimalPlaces (-1); and SetFormatString(NULL);
//    b) You can provide your own formatting string by calling
//       SetFormatString (). Ex: SetFormatString ("%.5f");
//    c) You can specify a number of decimal places after decimal point with a call to
//       SetDecimalPlaces (). If you supply -1 as a parameter, control will
//       use default formatting string: "%g". You can also specify whether you want
//       to trim trailing zeros with a call to SetTrimTrailingZeros()

//
/////////////////////////////////////////////////////////////////////////////
// CNumSpinCtrl window

class CNumSpinCtrl : public CSpinButtonCtrl
{
// Construction
public:
	CNumSpinCtrl();
	virtual ~CNumSpinCtrl();

// Operations
public:
	void GetRangeAndDelta(double &lower, double& upper, double& delta);
	void SetRangeAndDelta(double lower, double upper, double delta);
	
	double GetPos();
	void   SetPos(double val);

	int  GetDecimalPlaces ();
	void SetDecimalPlaces (int num_places);
	void SetFormatString (LPCTSTR lpszFormatString = NULL);
	
	void SetTrimTrailingZeros (BOOL bTrim)		{ m_bTrimTrailingZeros = bTrim; }
	BOOL GetTrimTrailingZeros ()					{ return m_bTrimTrailingZeros; }
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNumSpinCtrl)
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
protected:
	void InitSpinCtrl();
	void SetIntPos (double pos);
	void SetValueForBuddy (double val);
	CString FormatValue (double val);

	// Generated message map functions
protected:
	//{{AFX_MSG(CNumSpinCtrl)
	afx_msg BOOL OnDeltapos (NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

/// Attributes
protected:
	double m_MinVal;
	double m_MaxVal;
	double m_Delta;
	UINT m_IntRange;
	int m_NumDecPlaces;
	CString m_strFormat;
	BOOL m_bTrimTrailingZeros;
	char m_DecSymbol;
};
