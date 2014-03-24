
// BasicPage.h : ͷ�ļ�
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

// CBasicPage �Ի���
class CBasicPage : public CDialogEx
{
// ����
public:
	CBasicPage(CWnd* pParent = NULL);	// ��׼���캯��
// ����
	~CBasicPage();
// �Ի�������
	enum { IDD = IDD_BASIC_PAGE };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	//��Ϣ����
	afx_msg void OnConnectIB();
//�ؼ�����
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
	double MarginA50;
};
//��������
const int N = 5;
