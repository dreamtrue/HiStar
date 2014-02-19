#pragma once
#include "e:\新建文件夹\trunk\diyctrl\colorlistctrl.h"
#include "e:\新建文件夹\trunk\diyctrl\colorlistctrl.h"
#include "e:\新建文件夹\trunk\diyctrl\colorlistctrl.h"
#include "e:\新建文件夹\trunk\diyctrl\colorlistctrl.h"
#include "e:\新建文件夹\trunk\diyctrl\colorlistctrl.h"
#include "afxcmn.h"
#include "e:\新建文件夹\trunk\diyctrl\colorlistctrl.h"
#include "e:\新建文件夹\trunk\diyctrl\colorlistctrl.h"
#include "e:\新建文件夹\trunk\diyctrl\colorlistctrl.h"
#include "e:\新建文件夹\trunk\diyctrl\colorlistctrl.h"
#include "e:\新建文件夹\trunk\diyctrl\colorlistctrl.h"


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
	CTabCtrl m_tab;
	virtual BOOL OnInitDialog();
	void InitAllHdrs(void);
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);

	//////////////////////////////////////////////////
	////////////////////////////////////////////
	INSINFEX *m_InstInf;
	CThostFtdcDepthMarketDataField *m_pDepthMd;
	std::vector<CThostFtdcOrderField*> m_onRoadVec;
	CThostFtdcTradingAccountField *m_pTdAcc;
	CThostFtdcNotifyQueryAccountField *m_pNotifyBkYe;
	CThostFtdcInvestorField *m_pInvInf;
	std::vector<CThostFtdcOrderField*> m_orderVec;
	std::vector<CThostFtdcTradeField*> m_tradeVec;
	std::vector<CThostFtdcInstrumentFieldEx*> m_InsinfVec;
	std::vector<CThostFtdcInstrumentMarginRateField*> m_MargRateVec;
	std::vector<CThostFtdcSettlementInfoField*> m_StmiVec;
	std::vector<CThostFtdcAccountregisterField*> m_AccRegVec;
	std::vector<CThostFtdcTradingCodeField*> m_TdCodeVec;
	std::vector<CThostFtdcInvestorPositionField*> m_InvPosVec;
	std::vector<CThostFtdcRspTransferField*> m_BfTransVec;
	CThostFtdcInstrumentCommissionRateField m_FeeRateRev;
	std::vector<CThostFtdcInstrumentCommissionRateField*> m_FeeRateVec;
	////////////////////////////////////////////
	CThostFtdcTradingAccountField m_TdAcc;
	void InitAllVecs();
	CTimeSpan m_tsEXnLocal[4];
	CColorListCtrl m_LstOnRoad;
	CColorListCtrl m_LstOrdInf;
	CColorListCtrl m_LstTdInf;
	CColorListCtrl m_LstInvPosInf;
	CColorListCtrl m_LstAllInsts;
};
