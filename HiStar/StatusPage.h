#pragma once
#include "colorlistctrl.h"
#include "afxcmn.h"

// CStatusPage 对话框

class CStatusPage : public CDialogEx
{
	DECLARE_DYNAMIC(CStatusPage)

public:
	CStatusPage(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CStatusPage();

// 对话框数据
	enum { IDD = IDD_STATUS_PAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnOk();
	afx_msg void OnCancel();
	CTabCtrl m_tab;
	virtual BOOL OnInitDialog();
	void InitAllHdrs(void);
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);

	//////////////////////////////////////////////////
	////////////////////////////////////////////
	INSINFEX *m_InstInf;
	CThostFtdcDepthMarketDataField *m_pDepthMd;
private:
	std::vector<CThostFtdcInputOrderField> m_onRoadVec;
	CThostFtdcTradingAccountField *m_pTdAcc;
	CThostFtdcNotifyQueryAccountField *m_pNotifyBkYe;
	CThostFtdcInvestorField *m_pInvInf;
	//与traderspi.h中的vector同步
	//////////////////////////////////////////////////////
	std::vector<CThostFtdcOrderField> m_orderVec;
	std::vector<CThostFtdcTradeField> m_tradeVec;
	std::vector<CThostFtdcInstrumentFieldEx> m_InsinfVec;
	std::vector<CThostFtdcInvestorPositionField> m_InvPosVec;
	std::vector<CThostFtdcInvestorPositionDetailField> m_InvPosDetailVec;
	CTimeSpan m_tsEXnLocal[4];
	std::vector<CThostFtdcInstrumentMarginRateField> m_MargRateVec;
	std::vector<CThostFtdcSettlementInfoField> m_StmiVec;
	std::vector<CThostFtdcAccountregisterField> m_AccRegVec;
	std::vector<CThostFtdcTradingCodeField> m_TdCodeVec;
	std::vector<CThostFtdcRspTransferField> m_BfTransVec;
	std::vector<CThostFtdcInstrumentCommissionRateField> m_FeeRateVec;
	CThostFtdcInstrumentCommissionRateField m_FeeRateRev;
public:
	////////////////////////////////////////////
	CThostFtdcTradingAccountField m_TdAcc;
	void SynchronizeAllVecs();
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
	int FindOrdInOnRoadVec(TThostFtdcOrderRefType OrderRef);
	void OnCancelAll();
	void FiltInsList();

};
