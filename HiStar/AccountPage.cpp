// AccountPage.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "HiStar.h"
#include "AccountPage.h"
#include "afxdialogex.h"
#include "UserMsg.h"

// CAccountPage �Ի���

IMPLEMENT_DYNAMIC(CAccountPage, CDialogEx)

CAccountPage::CAccountPage(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAccountPage::IDD, pParent)
{

}

CAccountPage::~CAccountPage()
{
}

void CAccountPage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAccountPage, CDialogEx)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CAccountPage ��Ϣ�������

void CAccountPage::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
}
