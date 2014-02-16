// AccountPage.cpp : 实现文件
//

#include "stdafx.h"
#include "HiStar.h"
#include "AccountPage.h"
#include "afxdialogex.h"
#include "UserMsg.h"

// CAccountPage 对话框

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


// CAccountPage 消息处理程序

void CAccountPage::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
}
