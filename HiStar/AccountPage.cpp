// AccountPage.cpp : 实现文件
//

#include "stdafx.h"
#include "HiStar.h"
#include "AccountPage.h"
#include "afxdialogex.h"


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
END_MESSAGE_MAP()


// CAccountPage 消息处理程序
