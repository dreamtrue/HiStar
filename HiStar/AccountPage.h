#pragma once


// CAccountPage �Ի���

class CAccountPage : public CDialogEx
{
	DECLARE_DYNAMIC(CAccountPage)

public:
	CAccountPage(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CAccountPage();

// �Ի�������
	enum { IDD = IDD_ACCOUNT_PAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual void OnOK(void);
};
