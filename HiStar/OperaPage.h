
// OperaPage.h : 头文件
//

#pragma once
#include "resource.h"
#include "HScrollListBox.h" 
#include "afxcmn.h"
#include "EWrapper.h" 
#include "e:\histar\trunk\cppclient\shared\hscrolllistbox.h"

// COperaPage 对话框
class COperaPage : public CDialogEx
{
// 构造
public:
	COperaPage(CWnd* pParent = NULL);	// 标准构造函数
// 析构
	~COperaPage();
// 对话框数据
	enum { IDD = IDD_OPERA_PAGE };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	//消息函数
	afx_msg void OnConnectIB();
//控件变量
	CHScrollListBox m_orderStatus;
	CHScrollListBox m_errors;
	afx_msg void OnDisconnectIB();
};
//其他变量
const int N = 5;
