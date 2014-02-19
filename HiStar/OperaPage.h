
// OperaPage.h : ͷ�ļ�
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
#include "e:\�½��ļ���\trunk\diyctrl\colorlistctrl.h"
#include "e:\�½��ļ���\trunk\diyctrl\colorlistctrl.h"
#include "e:\�½��ļ���\trunk\diyctrl\colorlistctrl.h"
#include "e:\�½��ļ���\trunk\diyctrl\colorlistctrl.h"
#include "e:\�½��ļ���\trunk\diyctrl\colorlistctrl.h"
#include "e:\�½��ļ���\trunk\diyctrl\colorlistctrl.h"

// COperaPage �Ի���
class COperaPage : public CDialogEx
{
// ����
public:
	COperaPage(CWnd* pParent = NULL);	// ��׼���캯��
// ����
	~COperaPage();
// �Ի�������
	enum { IDD = IDD_OPERA_PAGE };

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
	afx_msg void OnStart();
	CColorListCtrl m_LstOnRoad;
	CColorListCtrl m_LstOrdInf;
	CColorListCtrl m_LstTdInf;
	CColorListCtrl m_LstInvPosInf;
	CColorListCtrl m_LstAllInsts;
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
};
//��������
const int N = 5;
