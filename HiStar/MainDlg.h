#pragma once
#include "OperaPage.h"
#include "AccountPage.h"
// CMainDlg �Ի���
#include "resource.h"
class CMainDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMainDlg)

public:
	CMainDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CMainDlg();

// �Ի�������
	enum { IDD = IDD_MAIN_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog(void);
//�ؼ�����
public:
	CTabCtrl m_tab;
	int m_CurSelTab;
	COperaPage m_pageOpera;
	CAccountPage m_AccountPage;
	CDialog* pDialog[2];  //��������Ի������ָ��
	afx_msg void OnTcnSelchangingTab(NMHDR *pNMHDR, LRESULT *pResult);
protected:
	afx_msg LRESULT OnOrderStatus(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT CMainDlg::OnErrors(WPARAM wParam, LPARAM lParam);
};
