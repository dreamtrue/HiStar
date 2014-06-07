#pragma once
#include "HiStar.h"
#include "mysql.h"
#include "afxwin.h"
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
	afx_msg void OnOpenDB();
	CComboBox m_tableList;
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnBnClickedRunDemo();
	CString m_tableName;
	afx_msg void OnBnClickedReadTable();
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
};
