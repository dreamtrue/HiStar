// DemoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HiStar.h"
#include "DemoDlg.h"
#include "afxdialogex.h"


// CDemoDlg 对话框

IMPLEMENT_DYNAMIC(CDemoDlg, CDialogEx)

CDemoDlg::CDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDemoDlg::IDD, pParent)
{

}

CDemoDlg::~CDemoDlg()
{
}

void CDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDemoDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CDemoDlg::OnOpenDB)
END_MESSAGE_MAP()


// CDemoDlg 消息处理程序

void CDemoDlg::OnOpenDB()
{
}
