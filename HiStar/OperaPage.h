
// OperaPage.h : ͷ�ļ�
//

#pragma once
#include "resource.h"
#include "HScrollListBox.h" 
#include "afxcmn.h"
#include "EWrapper.h" 
#include "hscrolllistbox.h"
#include "afxwin.h"

// COperaPage �Ի���
class COperaPage : public CDialogEx
{
// ����
public:
	COperaPage(CWnd* pParent = NULL);	// ��׼���캯��
// ����
	~COperaPage();
// �Ի�������
	enum { IDD = IDD_OPERA_PAGE };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	//��Ϣ����
	afx_msg void OnConnectIB();
//�ؼ�����
	CHScrollListBox m_orderStatus;
	CHScrollListBox m_errors;
	afx_msg void OnDisconnectIB();
	afx_msg void OnLoginCtp();
	afx_msg void OnLogoutCtp();
	CProgressCtrl m_prgs;
	CStatic m_staInfo;
	void ProgressUpdate(LPCTSTR szMsg, const int nPercentDone);
};
//��������
const int N = 5;
