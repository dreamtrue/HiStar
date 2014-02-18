
// OperaPage.h : 头文件
//

#pragma once
#include "resource.h"
#include "HScrollListBox.h" 
#include "afxcmn.h"
#include "EWrapper.h" 
#include "hscrolllistbox.h"
#include "afxwin.h"
#include "colorstatic.h"
#include "colorstatic.h"
#include "colorstatic.h"
#include "thostftdcuserapistruct.h"
#include "ColorListCtrl.h"

// COperaPage 对话框
class COperaPage : public CDialogEx
{
// 构造
public:
	COperaPage(CWnd* pParent = NULL);	// 标准构造函数
// 析构
	~COperaPage();
// 对话框数据
	enum { IDD = IDD_OPERA_PAGE };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	//消息函数
	afx_msg void OnConnectIB();
//控件变量
	CHScrollListBox m_orderStatus;
	CHScrollListBox m_errors;
	afx_msg void OnDisconnectIB();
	afx_msg void OnLoginCtp();
	afx_msg void OnLogoutCtp();
	CProgressCtrl m_prgs;
	CStatic m_staInfo;
	void ProgressUpdate(LPCTSTR szMsg, const int nPercentDone);
	CComboBox m_CombInst;
	afx_msg void OnInsSelchange();
	virtual void OnOK(void);
	afx_msg void OnReqComboSelMarketDepth();
	CColorStatic m_csS1P;
	CColorStatic m_csB1P;
	CColorStatic m_csLastP;
	void RefreshMdPane(void);
	CThostFtdcDepthMarketDataField m_depthMd;
	CTabCtrl m_TabOption;
	void InitAllHdrs(void);
public:
	CColorListCtrl	m_LstOnRoad;
	CColorListCtrl	m_LstOrdInf;
	CColorListCtrl	m_LstTdInf;
	CColorListCtrl	m_LstInvPosInf;
	CColorListCtrl	m_LstAllInsts;
	afx_msg void OnStart();
	afx_msg void OnTcnSelchangingTab1(NMHDR *pNMHDR, LRESULT *pResult);
};
//其他变量
const int N = 5;
