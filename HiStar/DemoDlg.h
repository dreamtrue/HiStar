#pragma once


// CDemoDlg �Ի���

class CDemoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDemoDlg)

public:
	CDemoDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDemoDlg();

// �Ի�������
	enum { IDD = IDD_DEMO_PAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnOpenDB();
};
