#pragma once


// CDemoDlg 对话框

class CDemoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDemoDlg)

public:
	CDemoDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDemoDlg();

// 对话框数据
	enum { IDD = IDD_DEMO_PAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnOpenDB();
};
