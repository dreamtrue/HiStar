#pragma once
#include "e:\新建文件夹\trunk\diyctrl\colorlistctrl.h"
#include "e:\新建文件夹\trunk\diyctrl\colorlistctrl.h"
#include "e:\新建文件夹\trunk\diyctrl\colorlistctrl.h"
#include "e:\新建文件夹\trunk\diyctrl\colorlistctrl.h"
#include "e:\新建文件夹\trunk\diyctrl\colorlistctrl.h"
#include "afxcmn.h"


// CTradePage 对话框

class CTradePage : public CDialogEx
{
	DECLARE_DYNAMIC(CTradePage)

public:
	CTradePage(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CTradePage();

// 对话框数据
	enum { IDD = IDD_TRADE_PAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CColorListCtrl m_LstOnRoad;
	CColorListCtrl m_LstOrdInf;
	CColorListCtrl m_LstTdInf;
	CColorListCtrl m_LstInvPosInf;
	CColorListCtrl m_LstAllInsts;
	CTabCtrl m_tab;
	virtual BOOL OnInitDialog();
	void InitAllHdrs(void);
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
};
