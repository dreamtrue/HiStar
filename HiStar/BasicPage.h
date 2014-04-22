
// BasicPage.h : 头文件
//

#pragma once
#include "resource.h"
#include "HScrollListBox.h" 
#include "afxcmn.h"
#include "EWrapper.h" 
#include "hscrolllistbox.h"
#include "afxwin.h"
#include "colorstatic.h"
#include "thostftdcuserapistruct.h"
#include "ColorListCtrl.h"
#include "HiStar.h"
#include "c:\users\administrator\desktop\histar\trunk\diyctrl\colorstatic.h"
#include "c:\users\administrator\desktop\histar\trunk\diyctrl\colorstatic.h"
#include "c:\users\administrator\desktop\histar\trunk\diyctrl\colorstatic.h"
#include "c:\users\administrator\desktop\histar\trunk\diyctrl\colorstatic.h"
#include "c:\users\administrator\desktop\histar\trunk\diyctrl\colorstatic.h"

// CBasicPage 对话框
class CBasicPage : public CDialogEx
{
// 构造
public:
	CBasicPage(CWnd* pParent = NULL);	// 标准构造函数
// 析构
	~CBasicPage();
// 对话框数据
	enum { IDD = IDD_BASIC_PAGE };

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
	afx_msg void OnDisconnectIB();
	afx_msg void OnLoginCtp();
	afx_msg void OnLogoutCtp();
	CProgressCtrl m_prgs;
	CStatic m_staInfo;
	void ProgressUpdate(LPCTSTR szMsg, const int nPercentDone);
	CComboBox m_CombInst;
	afx_msg void OnInsSelchange();
	afx_msg void OnOK(void);
	afx_msg void OnCancel();
	afx_msg void OnReqComboSelMarketDepth();
	CColorStatic m_csS1P;
	CColorStatic m_csB1P;
	CColorStatic m_csLastP;
	void RefreshMdPane(void);
	CThostFtdcDepthMarketDataField m_depthMd;
public:
	CColorStatic m_csHs300;
	CColorStatic m_csA50;
	CColorStatic m_csA50Ask1;
	CColorStatic m_csA50Last;
	CColorStatic m_csA50Bid1;
	afx_msg void OnPause();
	afx_msg void OnResume();
	CColorStatic m_csHedgePriceHigh;
	CColorStatic m_csHedgePriceLow;
	afx_msg void OnBnClickedTest();
	afx_msg void OnBnClickedUpdate();
	afx_msg void OnIniSql();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnIni();
	CColorListCtrl m_LstHedgeStatus;
public:
	void OnGetHedgeHold(NMHDR *pNMHDR, LRESULT *pResult);
	void SynchronizeHoldDataToView();
	void SynchronizeHoldViewToData();
	void SynchronizeSql();
	afx_msg void OnNMClkLstHedgeStatus(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkLstHedgeStatus(NMHDR *pNMHDR, LRESULT *pResult);
	std::vector<HoldDetail> m_hedgeHold;
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedMshq();
	CColorStatic m_HS300UP;
	CColorStatic m_A50UP;
	afx_msg void OnUpdateIndexref();
	CColorStatic m_closeProfit;
	CColorStatic m_openProfit;
	CColorStatic m_totalMargin;
};
//其他变量
const int N = 5;
