#pragma once
#include "HiStar.h"
#include "mysql.h"
#include "afxwin.h"
#include "afxdtctl.h"
// CDemoDlg �Ի���

class CDemoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDemoDlg)

public:
	CDemoDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDemoDlg();

// �Ի�������
	enum { IDD = IDD_DEMO_PAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CHiStarApp* m_pHiStarApp;
	MYSQL* conndemo;
	MYSQL_RES * res_set;
	MYSQL_ROW row;
	double A50IndexDemo,a50Bid1Demo,a50Ask1Demo,HS300IndexDemo,ifBid1Demo,ifAsk1Demo;
public:
	afx_msg void OnOpenDB01();
	CComboBox m_tableList;
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnBnClickedRunDemo();
	CString m_tableName;
	CButton m_runDemo;
	long maxIdHoldDemo;
	double datumDiffDemo;
	int PositionAimDemo[22];
	int MultiPosDemo;
	double premiumDemo;
	double premiumHighDemo;
	double premiumLowDemo;
	double deviationDemo;
	double DeviationSellDemo;
	double DeviationBuyDemo;
	double MaxProfitAim;
	double MinProfitAim;
	long maxIdHold;
	int m_MultiA50;
	double fee;
	double profit;
	double NetProfit;
	void DemoTaskRun(CString datetime);
	void DemoTaskRun02(CString datetime);
	afx_msg void OnBnClickedClear();
	CStatic m_demoShow;
	CListBox m_demoList;
	CStatic m_positionShow;
	void PrintPosition(void);
	int hedgenum;
	long numif;
	long numA50;
	afx_msg void OnBnClickedButton5();
	CString datetime;
	void PrintProfit(void);
	afx_msg void OnBnClickedOpenDb02();
	int maxhedgehold;
	afx_msg void OnBnClickedUpdateDemo();
	CEdit m_bDatumdiff;
	CEdit m_bMaxhold;
	virtual BOOL OnInitDialog();
	std::vector<CString> m_csTableList;
	void ColseAllDemo(void);
	CListBox m_profitList;
	void OpenDB(char *filter);
	double m_mLadder;
	CEdit m_bMLadder;
	CDateTimeCtrl m_dateBegin;
	CDateTimeCtrl m_dateEnd;
	CEdit m_A50Multi;
	CEdit m_max;
	CEdit m_min;
	afx_msg void OnRunDemo02();
	CButton m_runDemo02;
};
