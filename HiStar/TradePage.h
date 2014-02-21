#pragma once
#include "colorlistctrl.h"
#include "afxcmn.h"
#include "CVector.h"
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
	CVector<CThostFtdcOrderField*> m_onRoadVec;
	CThostFtdcTradingAccountField *m_pTdAcc;
	CThostFtdcNotifyQueryAccountField *m_pNotifyBkYe;
	CThostFtdcInvestorField *m_pInvInf;
	CVector<CThostFtdcOrderField*> m_orderVec;
	CVector<CThostFtdcTradeField*> m_tradeVec;
	CVector<CThostFtdcInstrumentFieldEx*> m_InsinfVec;
	CVector<CThostFtdcInstrumentMarginRateField*> m_MargRateVec;
	CVector<CThostFtdcSettlementInfoField*> m_StmiVec;
	CVector<CThostFtdcAccountregisterField*> m_AccRegVec;
	CVector<CThostFtdcTradingCodeField*> m_TdCodeVec;
	CVector<CThostFtdcInvestorPositionField*> m_InvPosVec;
	CVector<CThostFtdcRspTransferField*> m_BfTransVec;
	CThostFtdcInstrumentCommissionRateField m_FeeRateRev;
	CVector<CThostFtdcInstrumentCommissionRateField*> m_FeeRateVec;
	////////////////////////////////////////////
	CThostFtdcTradingAccountField m_TdAcc;
	void InitAllVecs();
	CTimeSpan m_tsEXnLocal[4];
	CColorListCtrl m_LstOnRoad;
	CColorListCtrl m_LstOrdInf;
	CColorListCtrl m_LstTdInf;
	CColorListCtrl m_LstInvPosInf;
	CColorListCtrl m_LstAllInsts;
public:
	afx_msg void OnNMDblclkOnroad(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClkLstOnroad(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClkLstOnroad(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnNMDblclkOrdInf(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClkLstOrdInf(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClkLstOrdInf(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnNMDblclkTdInf(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClkLstTdInf(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClkLstTdInf(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnNMDblclkInvPInf(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClkLstInvPInf(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClkLstInvPInf(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnNMClkLstInsts(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClkLstInsts(NMHDR *pNMHDR, LRESULT *pResult);

	/////////////////////////////////////////////////////////////////
	afx_msg void OnGetDispinf1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnGetDispinf2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnGetDispinf3(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnGetDispinf4(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnGetDispinf5(NMHDR *pNMHDR, LRESULT *pResult);
public:
	CString m_szInst;
	int FindInstMul(TThostFtdcInstrumentIDType InstID);
	int FindOrdInOrderVec(TThostFtdcSequenceNoType BkrOrdSeq);
	int FindOrdInOnRoadLst(TThostFtdcSequenceNoType BkrOrdSeq);
	int FindOrdInOnRoadVec(TThostFtdcSequenceNoType BkrOrdSeq);
	void OnCancelAll();
	void FiltInsList();

};
