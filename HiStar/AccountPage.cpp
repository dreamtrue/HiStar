// AccountPage.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "HiStar.h"
#include "AccountPage.h"
#include "afxdialogex.h"


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
END_MESSAGE_MAP()


// CAccountPage ��Ϣ�������
