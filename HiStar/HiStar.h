
// HiStar.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif
#include "global.h"
#include "resource.h"		// 主符号
#include "eclient.h"
#include "EWrapper.h" 
#include "EClientSocket.h" 
#include "HScrollListBox.h" 
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
	virtual void openOrder( OrderId orderId, const Contract&, const Order&, const OrderState&);
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
	void OnConnectIB(UINT wParam,LONG lParam);
	void OnDisconnectIB(UINT wParam,LONG lParam);
	void PostOrderStatus(CString str);
	void PostErrors(CString str);
	bool faError;
	SAcountIB m_accountIB;
};

extern CHiStarApp theApp;