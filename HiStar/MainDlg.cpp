// MainDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "HiStar.h"
#include "MainDlg.h"
#include "afxdialogex.h"
#include "UserMsg.h"
// CMainDlg �Ի���

IMPLEMENT_DYNAMIC(CMainDlg, CDialogEx)

CMainDlg::CMainDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMainDlg::IDD, pParent)
{

}

CMainDlg::~CMainDlg()
{
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, m_tab);
}


BEGIN_MESSAGE_MAP(CMainDlg, CDialogEx)
	ON_NOTIFY(TCN_SELCHANGING, IDC_TAB, &CMainDlg::OnTcnSelchangingTab)
	ON_MESSAGE(WM_ORDER_STATUS, &CMainDlg::OnOrderStatus)
	ON_MESSAGE(WM_ERRORS, &CMainDlg::OnErrors)
END_MESSAGE_MAP()


// CMainDlg ��Ϣ�������


BOOL CMainDlg::OnInitDialog(void)
{
	CDialogEx::OnInitDialog();
	//�����ʼ������
	m_tab.InsertItem(0, _T("����"));
	m_tab.InsertItem(1, _T("�˻�"));
	//���������Ի���
	m_AccountPage.Create(IDD_ACCOUNT_PAGE, &m_tab);
	m_pageOpera.Create(IDD_OPERA_PAGE, &m_tab);
	//�趨��Tab����ʾ�ķ�Χ
	CRect rc;
	m_tab.GetClientRect(rc);
	rc.top += 20;
	rc.bottom -= 0;
	rc.left += 0;
	rc.right -= 0;
	m_pageOpera.MoveWindow(&rc);
	m_AccountPage.MoveWindow(&rc);
	//�ѶԻ������ָ�뱣������
	pDialog[0] = &m_pageOpera;
	pDialog[1] = &m_AccountPage;
	//��ʾ��ʼҳ��
	pDialog[0]->ShowWindow(SW_SHOW);
	pDialog[1]->ShowWindow(false);
	m_tab.SetCurSel(0);
	//���浱ǰѡ��
	m_CurSelTab = 0;

	return 0;
}

void CMainDlg::OnTcnSelchangingTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int CurSel = m_tab.GetCurSel();
	switch(CurSel)
	{
	case 0:
		m_AccountPage.ShowWindow(true);
		m_pageOpera.ShowWindow(false);
		m_CurSelTab = 0;
		break;
	case 1:
		m_pageOpera.ShowWindow(true);
		m_AccountPage.ShowWindow(false);
		m_CurSelTab = 1;
		break;
	}
	*pResult = 0;
}

afx_msg LRESULT CMainDlg::OnOrderStatus(WPARAM wParam, LPARAM lParam)
{
	int i = m_pageOpera.m_orderStatus.AddString(*(CString *)wParam);
	int top = i - N < 0 ? 0 : i - N;
	m_pageOpera.m_orderStatus.SetTopIndex(top);
	delete (CString*)wParam;
	return 0;
}
afx_msg LRESULT CMainDlg::OnErrors(WPARAM wParam, LPARAM lParam)
{
	int i = m_pageOpera.m_errors.AddString(*(CString *)wParam);
	int top = i - N < 0 ? 0 : i - N;
	m_pageOpera.m_errors.SetTopIndex(top);
	delete (CString*)wParam;
	return 0;
}
