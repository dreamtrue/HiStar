// DemoDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "HiStar.h"
#include "DemoDlg.h"
#include "afxdialogex.h"


// CDemoDlg �Ի���

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


// CDemoDlg ��Ϣ�������

void CDemoDlg::OnOpenDB()
{
}
