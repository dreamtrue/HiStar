#pragma once


// CAccountPage 对话框

class CAccountPage : public CDialogEx
{
	DECLARE_DYNAMIC(CAccountPage)

public:
	CAccountPage(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CAccountPage();

// 对话框数据
	enum { IDD = IDD_ACCOUNT_PAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnOK(void);
	afx_msg void OnBnClickedCancel();
};
