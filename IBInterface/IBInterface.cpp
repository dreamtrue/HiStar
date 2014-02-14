#include "stdafx.h"
#include "afxdialogex.h"
#include "OperaPage.h"
#include "global.h"
extern HANDLE g_hEvent;
void COperaPage::tickPrice( TickerId tickerId, TickType field, double price, int canAutoExecute){}
void COperaPage::tickSize( TickerId tickerId, TickType field, int size){}
void COperaPage::tickOptionComputation( TickerId tickerId, TickType tickType, double impliedVol, double delta,
	double optPrice, double pvDividend, double gamma, double vega, double theta, double undPrice){}
void COperaPage::tickGeneric(TickerId tickerId, TickType tickType, double value){}
void COperaPage::tickString(TickerId tickerId, TickType tickType, const IBString& value){}
void COperaPage::tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const IBString& formattedBasisPoints,
	double totalDividends, int holdDays, const IBString& futureExpiry, double dividendImpact, double dividendsToExpiry){}
void COperaPage::orderStatus( OrderId orderId, const IBString &status, int filled,
	int remaining, double avgFillPrice, int permId, int parentId,
	double lastFillPrice, int clientId, const IBString& whyHeld){}
void COperaPage::openOrder( OrderId orderId, const Contract&, const Order&, const OrderState&){}
void COperaPage::openOrderEnd(){}
void COperaPage::winError( const IBString &str, int lastError){}
void COperaPage::connectionClosed(){}

void COperaPage::updateAccountValue(const IBString& key, const IBString& val,
	const IBString& currency, const IBString& accountName){
		if(key == "AvailableFunds" && currency == "USD"){
			TRACE("IB可用资金 %.f USD\r\n",atof(val));
		}
		if(key == "NetLiquidation" && currency == "USD"){
		}
}

void COperaPage::updatePortfolio( const Contract& contract, int position,
	double marketPrice, double marketValue, double averageCost,
	double unrealizedPNL, double realizedPNL, const IBString& accountName){
		TRACE("持仓 %d\r\n",position);
}

void COperaPage::updateAccountTime(const IBString& timeStamp){
	CString str;
	str.Format("Account Time: %s", timeStamp);
	m_orderStatus.AddString(str);
	SetEvent(g_hEvent);
}

void COperaPage::accountDownloadEnd(const IBString& accountName){
	CString str;
	str.Format("Account Download End: %s", accountName);
	m_orderStatus.AddString(str);
	SetEvent(g_hEvent);
}

void COperaPage::nextValidId( OrderId orderId){}
void COperaPage::contractDetails( int reqId, const ContractDetails& contractDetails){}
void COperaPage::bondContractDetails( int reqId, const ContractDetails& contractDetails){}
void COperaPage::contractDetailsEnd( int reqId){}
void COperaPage::execDetails( int reqId, const Contract& contract, const Execution& execution){}
void COperaPage::execDetailsEnd( int reqId){}
void COperaPage::error(const int id, const int errorCode, const IBString errorString){}
void COperaPage::updateMktDepth(TickerId id, int position, int operation, int side,
	double price, int size){}
void COperaPage::updateMktDepthL2(TickerId id, int position, IBString marketMaker, int operation,
	int side, double price, int size){}
void COperaPage::updateNewsBulletin(int msgId, int msgType, const IBString& newsMessage, const IBString& originExch){}

void COperaPage::managedAccounts( const IBString& accountsList){
	m_accountName = accountsList;
	CString displayString;
	displayString.Format("Connected : The list of managed accounts are : [%s]", accountsList);
	m_orderStatus.AddString( displayString);
	SetEvent(g_hEvent);
}

void COperaPage::receiveFA(faDataType pFaDataType, const IBString& cxml){}
void COperaPage::historicalData(TickerId reqId, const IBString& date, double open, double high, 
	double low, double close, int volume, int barCount, double WAP, int hasGaps){}
void COperaPage::scannerParameters(const IBString &xml){}
void COperaPage::scannerData(int reqId, int rank, const ContractDetails &contractDetails,
	const IBString &distance, const IBString &benchmark, const IBString &projection,
	const IBString &legsStr){}
void COperaPage::scannerDataEnd(int reqId){}
void COperaPage::realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
	long volume, double wap, int count){}
void COperaPage::currentTime(long time){}
void COperaPage::fundamentalData(TickerId reqId, const IBString& data){}
void COperaPage::deltaNeutralValidation(int reqId, const UnderComp& underComp){}
void COperaPage::tickSnapshotEnd( int reqId){}
void COperaPage::marketDataType( TickerId reqId, int marketDataType){}
void COperaPage::commissionReport( const CommissionReport &commissionReport){}
void COperaPage::position( const IBString& account, const Contract& contract, int position, double avgCost){}
void COperaPage::positionEnd(){}
void COperaPage::accountSummary( int reqId, const IBString& account, const IBString& tag, const IBString& value, const IBString& curency){}
void COperaPage::accountSummaryEnd( int reqId){}
void COperaPage::verifyMessageAPI( const IBString& apiData){}
void COperaPage::verifyCompleted( bool isSuccessful, const IBString& errorText){}
void COperaPage::displayGroupList( int reqId, const IBString& groups){}
void COperaPage::displayGroupUpdated( int reqId, const IBString& contractInfo){}