
// HiStar.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif
#include "resource.h"		// 主符号
#include "EWrapper.h" 
#include "EClientSocket.h" 
#include "HScrollListBox.h" 
#include "mdspi.h"
#include "traderspi.h"
#include "Index.h"
#include "contract.h"
#include "my_global.h"
#include "mysql.h"
#include "HedgePostProcessing.h"
#include "MSHQ.h"
#include "order.h"
#include "ibaccount.h"
extern DWORD MainThreadId;
struct HoldDetail{
	double adjustedCost;//根据datumdiff调整后的价格
	double originalCost;//原始成本
	int HedgeNum;
	int HedgeSection;//开仓时所在的Section
	long id;//序号
};
struct OrderStatus{
	OrderId orderId;IBString status;int filled;
	int remaining;double avgFillPrice;int permId;int parentId;
	double lastFillPrice;int clientId;IBString whyHeld;
};
	// CHiStarApp:
// 有关此类的实现，请参阅 HiStar.cpp
//
class CHiStarApp : public CWinApp,public EWrapper
{
public:
	CHiStarApp();
	~CHiStarApp(void);
// 重写
public:
	virtual BOOL InitInstance();
public:
	// handle events from client
	virtual void tickPrice( TickerId tickerId, TickType field, double price, int canAutoExecute);
	virtual void tickSize( TickerId tickerId, TickType field, int size);
	virtual void tickOptionComputation( TickerId tickerId, TickType tickType, double impliedVol, double delta,
		double optPrice, double pvDividend, double gamma, double vega, double theta, double undPrice);
	virtual void tickGeneric(TickerId tickerId, TickType tickType, double value);
	virtual void tickString(TickerId tickerId, TickType tickType, const IBString& value);
	virtual void tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const IBString& formattedBasisPoints,
		double totalDividends, int holdDays, const IBString& futureExpiry, double dividendImpact, double dividendsToExpiry);
	virtual void orderStatus( OrderId orderId, const IBString &status, int filled,
		int remaining, double avgFillPrice, int permId, int parentId,
		double lastFillPrice, int clientId, const IBString& whyHeld);
	virtual void openOrder(OrderId orderId, const Contract& contract, const Order& order, const OrderState& orderstate);
	virtual void openOrderEnd();
	virtual void winError( const IBString &str, int lastError);
	virtual void connectionClosed();
	virtual void updateAccountValue(const IBString& key, const IBString& val,
		const IBString& currency, const IBString& accountName);
	virtual void updatePortfolio( const Contract& contract, int position,
		double marketPrice, double marketValue, double averageCost,
		double unrealizedPNL, double realizedPNL, const IBString& accountName);
	virtual void updateAccountTime(const IBString& timeStamp);
	virtual void accountDownloadEnd(const IBString& accountName);
	virtual void nextValidId( OrderId orderId);
	virtual void contractDetails( int reqId, const ContractDetails& contractDetails);
	virtual void bondContractDetails( int reqId, const ContractDetails& contractDetails);
	virtual void contractDetailsEnd( int reqId);
	virtual void execDetails( int reqId, const Contract& contract, const Execution& execution);
	virtual void execDetailsEnd( int reqId);
	virtual void error(const int id, const int errorCode, const IBString errorString);
	virtual void updateMktDepth(TickerId id, int position, int operation, int side,
		double price, int size);
	virtual void updateMktDepthL2(TickerId id, int position, IBString marketMaker, int operation,
		int side, double price, int size);
	virtual void updateNewsBulletin(int msgId, int msgType, const IBString& newsMessage, const IBString& originExch);
	virtual void managedAccounts( const IBString& accountsList);
	virtual void receiveFA(faDataType pFaDataType, const IBString& cxml);
	virtual void historicalData(TickerId reqId, const IBString& date, double open, double high, 
		double low, double close, int volume, int barCount, double WAP, int hasGaps);
	virtual void scannerParameters(const IBString &xml);
	virtual void scannerData(int reqId, int rank, const ContractDetails &contractDetails,
		const IBString &distance, const IBString &benchmark, const IBString &projection,
		const IBString &legsStr);
	virtual void scannerDataEnd(int reqId);
	virtual void realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
		long volume, double wap, int count);
	virtual void currentTime(long time);
	virtual void fundamentalData(TickerId reqId, const IBString& data);
	virtual void deltaNeutralValidation(int reqId, const UnderComp& underComp);
	virtual void tickSnapshotEnd( int reqId);
	virtual void marketDataType( TickerId reqId, int marketDataType);
	virtual void commissionReport( const CommissionReport &commissionReport);
	virtual void position( const IBString& account, const Contract& contract, int position, double avgCost);
	virtual void positionEnd();
	virtual void accountSummary( int reqId, const IBString& account, const IBString& tag, const IBString& value, const IBString& curency);
	virtual void accountSummaryEnd( int reqId);
	virtual void verifyMessageAPI( const IBString& apiData);
	virtual void verifyCompleted( bool isSuccessful, const IBString& errorText);
	virtual void displayGroupList( int reqId, const IBString& groups);
	virtual void displayGroupUpdated( int reqId, const IBString& contractInfo);
// 实现
	DECLARE_MESSAGE_MAP()
	//进程消息
	void PostOrderStatus(CString str);
	void PostErrors(CString str);
public:
	//CTP系统
	bool faError;
	SAccountCtp m_accountCtp;
	CThostFtdcMdApi* m_MApi;
	CThostFtdcTraderApi* m_TApi;
	CtpMdSpi* m_cQ;
	CtpTraderSpi* m_cT;
	void SetIFContract(void);
	void CreateCtpClient(void);
	void LoginCtp(WPARAM wParam,LPARAM lParam);
	void LogoutCtp(WPARAM wParam,LPARAM lParam);
	void LoginCtpMD(WPARAM wParam,LPARAM lParam);
	void LoginCtpTD(WPARAM wParam,LPARAM lParam);
	void OnUpdateLstCtrl(WPARAM wParam,LPARAM lParam);
	void UpdateHedgeHold(WPARAM wParam,LPARAM lParam);
	void OnHedgeLooping(WPARAM wParam,LPARAM lParam);
	void OnSynchronizeMarket(WPARAM wParam,LPARAM lParam);
	int FindInstMul(TThostFtdcInstrumentIDType InstID);
	void OnReqMshq(WPARAM wParam,LPARAM lParam);
	CString m_strPath;
	CIndex* m_pIndexThread;
	//IB系统
	long m_id;
	SAccountIB m_accountIB;
	EClient* m_pIBClient;//EClient是一个纯虚类接口,被继承并实现。
	void SetA50Contract();
	void OnConnectIB(WPARAM wParam,LPARAM lParam);
	void OnDisconnectIB(WPARAM wParam,LPARAM lParam);
	void OnConnectSql(WPARAM wParam,LPARAM lParam);
	void OnIni(WPARAM wParam,LPARAM lParam);
	void OnReqAccountUpdates(WPARAM wParam,LPARAM lParam);
	Contract m_A50Contract;
	WORD m_LifeA50;
	TagValueListSPtr m_mktDepthOptions;
	TagValueListSPtr m_mktDataOptions;
	CHedgePostProcessing* m_pHedgePostProcessing;
	int ReqHedgeOrder(HoldDetail *pHD,bool OffsetFlag);
	Order m_IBOrder;
	CMSHQ* m_pMSHQ;
	int FileInput(void);
	void iniSql(void);
	MYSQL *conn; 
	CString m_marketTableName;CString m_positionTableName;
	std::string str,str01,str02,str03,str04,str05,str06,str07,str08;
	virtual BOOL OnIdle(LONG lCount);
	//IB的账户参数
	std::vector<Portfolio> m_portfolio;
	AccountValue m_accountvalue;
};

extern CHiStarApp theApp;