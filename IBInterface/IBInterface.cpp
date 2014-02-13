#include "stdafx.h"
#include "afxdialogex.h"
#include "HiStarDlg.h"
#include "global.h"
extern HANDLE g_hEvent;
void CHiStarDlg::tickPrice( TickerId tickerId, TickType field, double price, int canAutoExecute){}
void CHiStarDlg::tickSize( TickerId tickerId, TickType field, int size){}
void CHiStarDlg::tickOptionComputation( TickerId tickerId, TickType tickType, double impliedVol, double delta,
	double optPrice, double pvDividend, double gamma, double vega, double theta, double undPrice){}
void CHiStarDlg::tickGeneric(TickerId tickerId, TickType tickType, double value){}
void CHiStarDlg::tickString(TickerId tickerId, TickType tickType, const IBString& value){}
void CHiStarDlg::tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const IBString& formattedBasisPoints,
	double totalDividends, int holdDays, const IBString& futureExpiry, double dividendImpact, double dividendsToExpiry){}
void CHiStarDlg::orderStatus( OrderId orderId, const IBString &status, int filled,
	int remaining, double avgFillPrice, int permId, int parentId,
	double lastFillPrice, int clientId, const IBString& whyHeld){}
void CHiStarDlg::openOrder( OrderId orderId, const Contract&, const Order&, const OrderState&){}
void CHiStarDlg::openOrderEnd(){}
void CHiStarDlg::winError( const IBString &str, int lastError){}
void CHiStarDlg::connectionClosed(){}

void CHiStarDlg::updateAccountValue(const IBString& key, const IBString& val,
	const IBString& currency, const IBString& accountName){
		if(key == "AvailableFunds" && currency == "USD"){
			TRACE("IB可用资金 %.f USD\r\n",atof(val));
		}
		if(key == "NetLiquidation" && currency == "USD"){
		}
}

void CHiStarDlg::updatePortfolio( const Contract& contract, int position,
	double marketPrice, double marketValue, double averageCost,
	double unrealizedPNL, double realizedPNL, const IBString& accountName){
		TRACE("持仓 %d\r\n",position);
}

void CHiStarDlg::updateAccountTime(const IBString& timeStamp){
	CString str;
	str.Format("Account Time: %s", timeStamp);
	m_orderStatus.AddString(str);
	SetEvent(g_hEvent);
}

void CHiStarDlg::accountDownloadEnd(const IBString& accountName){
	CString str;
	str.Format("Account Download End: %s", accountName);
	m_orderStatus.AddString(str);
	SetEvent(g_hEvent);
}

void CHiStarDlg::nextValidId( OrderId orderId){}
void CHiStarDlg::contractDetails( int reqId, const ContractDetails& contractDetails){}
void CHiStarDlg::bondContractDetails( int reqId, const ContractDetails& contractDetails){}
void CHiStarDlg::contractDetailsEnd( int reqId){}
void CHiStarDlg::execDetails( int reqId, const Contract& contract, const Execution& execution){}
void CHiStarDlg::execDetailsEnd( int reqId){}
void CHiStarDlg::error(const int id, const int errorCode, const IBString errorString){}
void CHiStarDlg::updateMktDepth(TickerId id, int position, int operation, int side,
	double price, int size){}
void CHiStarDlg::updateMktDepthL2(TickerId id, int position, IBString marketMaker, int operation,
	int side, double price, int size){}
void CHiStarDlg::updateNewsBulletin(int msgId, int msgType, const IBString& newsMessage, const IBString& originExch){}

void CHiStarDlg::managedAccounts( const IBString& accountsList){
	m_accountName = accountsList;
	CString displayString;
	displayString.Format("Connected : The list of managed accounts are : [%s]", accountsList);
	m_orderStatus.AddString( displayString);
	SetEvent(g_hEvent);
}

void CHiStarDlg::receiveFA(faDataType pFaDataType, const IBString& cxml){}
void CHiStarDlg::historicalData(TickerId reqId, const IBString& date, double open, double high, 
	double low, double close, int volume, int barCount, double WAP, int hasGaps){}
void CHiStarDlg::scannerParameters(const IBString &xml){}
void CHiStarDlg::scannerData(int reqId, int rank, const ContractDetails &contractDetails,
	const IBString &distance, const IBString &benchmark, const IBString &projection,
	const IBString &legsStr){}
void CHiStarDlg::scannerDataEnd(int reqId){}
void CHiStarDlg::realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
	long volume, double wap, int count){}
void CHiStarDlg::currentTime(long time){}
void CHiStarDlg::fundamentalData(TickerId reqId, const IBString& data){}
void CHiStarDlg::deltaNeutralValidation(int reqId, const UnderComp& underComp){}
void CHiStarDlg::tickSnapshotEnd( int reqId){}
void CHiStarDlg::marketDataType( TickerId reqId, int marketDataType){}
void CHiStarDlg::commissionReport( const CommissionReport &commissionReport){}
void CHiStarDlg::position( const IBString& account, const Contract& contract, int position, double avgCost){}
void CHiStarDlg::positionEnd(){}
void CHiStarDlg::accountSummary( int reqId, const IBString& account, const IBString& tag, const IBString& value, const IBString& curency){}
void CHiStarDlg::accountSummaryEnd( int reqId){}
void CHiStarDlg::verifyMessageAPI( const IBString& apiData){}
void CHiStarDlg::verifyCompleted( bool isSuccessful, const IBString& errorText){}
void CHiStarDlg::displayGroupList( int reqId, const IBString& groups){}
void CHiStarDlg::displayGroupUpdated( int reqId, const IBString& contractInfo){}