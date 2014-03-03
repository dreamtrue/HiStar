#include "stdafx.h"
#include "afxdialogex.h"
#include "HiStar.h"
#include "global.h"
#include "UserMsg.h"
#define NOT_AN_FA_ACCOUNT_ERROR 321
#define NUM_FA_ERROR_CODES 6
extern double AvailIB;
double g_a50Bid1 = 0.0f,g_a50Ask1 = 0.0f;
int g_a50Bid1Size = 0,g_a50Ask1Size = 0;
static int faErrorCodes[NUM_FA_ERROR_CODES] =
{ 503, 504, 505, 522, 1100, NOT_AN_FA_ACCOUNT_ERROR} ;
extern HANDLE g_hEvent;
DWORD MainThreadId = 0;
//IB交易系统
void CHiStarApp::tickPrice( TickerId tickerId, TickType field, double price, int canAutoExecute){
	TRACE("tickPrice\n");
}
void CHiStarApp::tickSize( TickerId tickerId, TickType field, int size){
	TRACE("tickSize\n");
}
void CHiStarApp::tickOptionComputation( TickerId tickerId, TickType tickType, double impliedVol, double delta,
	double optPrice, double pvDividend, double gamma, double vega, double theta, double undPrice){
		TRACE("tickOptionComputation\n");
}
void CHiStarApp::tickGeneric(TickerId tickerId, TickType tickType, double value){
	TRACE("tickGeneric\n");
}
void CHiStarApp::tickString(TickerId tickerId, TickType tickType, const IBString& value){
	TRACE("tickString\n");
}
void CHiStarApp::tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const IBString& formattedBasisPoints,
	double totalDividends, int holdDays, const IBString& futureExpiry, double dividendImpact, double dividendsToExpiry){
		TRACE("tickEFP\n");
}
void CHiStarApp::orderStatus( OrderId orderId, const IBString &status, int filled,
	int remaining, double avgFillPrice, int permId, int parentId,
	double lastFillPrice, int clientId, const IBString& whyHeld){
		TRACE("orderStatus\n");
}
void CHiStarApp::openOrder( OrderId orderId, const Contract&, const Order&, const OrderState&){
	TRACE("openOrder\n");
}
void CHiStarApp::openOrderEnd(){
	TRACE("openOrderEnd\n");
}
void CHiStarApp::winError( const IBString &str, int lastError){
	TRACE("winError\n");
}
void CHiStarApp::connectionClosed(){
	TRACE("connectionClosed\n");
	CString cStatus;
	cStatus.Format("ConnectionClosed");
	PostOrderStatus(cStatus);
}


void CHiStarApp::updateAccountValue(const IBString& key, const IBString& val,
	const IBString& currency, const IBString& accountName){
	//TRACE("updateAccountValue\n");
	if(key == "AvailableFunds" && currency == "USD"){
		TRACE("IB可用资金 %.f USD\r\n",atof(val));
		AvailIB = atof(val);
	}
	if(key == "NetLiquidation" && currency == "USD"){
	}
}

void CHiStarApp::updatePortfolio( const Contract& contract, int position,
	double marketPrice, double marketValue, double averageCost,
	double unrealizedPNL, double realizedPNL, const IBString& accountName){
		TRACE("updatePortfolio\n");
		TRACE("持仓 %d\r\n",position);
}

void CHiStarApp::updateAccountTime(const IBString& timeStamp){
	TRACE("updateAccountTime\n");
	CString cStatus;
	cStatus.Format("Account Time: %s", timeStamp);
	PostOrderStatus(cStatus);
}

void CHiStarApp::accountDownloadEnd(const IBString& accountName){
	TRACE("accountDownloadEnd\n");
	CString cStatus;
	cStatus.Format("Account Download End: %s",accountName);
	PostOrderStatus(cStatus);
}

void CHiStarApp::nextValidId( OrderId orderId){
	m_id = orderId;
	TRACE("nextValidId\n");
}

void CHiStarApp::contractDetails( int reqId, const ContractDetails& contractDetails){
	TRACE("contractDetails\n");
}

void CHiStarApp::bondContractDetails( int reqId, const ContractDetails& contractDetails){
	TRACE("bondContractDetails\n");
}

void CHiStarApp::contractDetailsEnd( int reqId){
	TRACE("contractDetailsEnd\n");
}

void CHiStarApp::execDetails( int reqId, const Contract& contract, const Execution& execution){
	TRACE("execDetails\n");
}

void CHiStarApp::execDetailsEnd( int reqId){
	TRACE("execDetailsEnd\n");
}

void CHiStarApp::error(const int id, const int errorCode, const IBString errorString){
	CString errorStr("Id: ");
	char buf[20];
	_itoa_s(id, buf, 10);
	errorStr += CString(buf);
	errorStr += " | ";
	errorStr += "Error Code: ";
	_itoa_s(errorCode, buf, 10);
	errorStr += CString(buf);
	errorStr += " | ";
	errorStr += "Error Msg: ";
	errorStr += errorString;
	PostErrors(errorStr);
	TRACE("%s\r\n",errorStr);
	for (int ctr=0; ctr < NUM_FA_ERROR_CODES; ctr++) {
		faError |= (errorCode == faErrorCodes[ctr]) ;
	}
}

void CHiStarApp::updateMktDepth(TickerId id, int position, int operation, int side,
	double price, int size){
		if(position == 0){
			if(side == 1){
				if(price != g_a50Bid1){
					g_a50Bid1 = price;
					g_a50Bid1Size = size;
				}
			}
			else{
				if(price != g_a50Ask1){
					g_a50Ask1 = price;
					g_a50Ask1Size = size;
				}
			}
			//TRACE("%f %f\r\n",g_a50Bid1,g_a50Ask1);
			if(AfxGetApp()->m_pMainWnd){
				PostMessage(AfxGetApp()->m_pMainWnd->GetSafeHwnd(),WM_MD_REFRESH,NULL,NULL);
			}
		}
}

void CHiStarApp::updateMktDepthL2(TickerId id, int position, IBString marketMaker, int operation,
	int side, double price, int size){
		TRACE("updateMktDepthL2\n");
}

void CHiStarApp::updateNewsBulletin(int msgId, int msgType, const IBString& newsMessage, const IBString& originExch){
	TRACE("updateNewsBulletin\n");
}

void CHiStarApp::managedAccounts( const IBString& accountsList){
	CString cStatus;
	m_accountIB.m_accountName = accountsList;
	cStatus.Format("Connected : The list of managed accounts are : [%s]", accountsList);
	PostOrderStatus(cStatus);
}

void CHiStarApp::receiveFA(faDataType pFaDataType, const IBString& cxml){
	TRACE("receiveFA\n");
}

void CHiStarApp::historicalData(TickerId reqId, const IBString& date, double open, double high, 
	double low, double close, int volume, int barCount, double WAP, int hasGaps){
		TRACE("historicalData\n");
}

void CHiStarApp::scannerParameters(const IBString &xml){
	TRACE("scannerParameters\n");
}

void CHiStarApp::scannerData(int reqId, int rank, const ContractDetails &contractDetails,
	const IBString &distance, const IBString &benchmark, const IBString &projection,
	const IBString &legsStr){
		TRACE("scannerData\n");
}

void CHiStarApp::scannerDataEnd(int reqId){
	TRACE("scannerDataEnd\n");
}

void CHiStarApp::realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
	long volume, double wap, int count){
		TRACE("realtimeBar\n");
}

void CHiStarApp::currentTime(long time){
	TRACE("currentTime\n");
}

void CHiStarApp::fundamentalData(TickerId reqId, const IBString& data){
	TRACE("fundamentalData\n");
}

void CHiStarApp::deltaNeutralValidation(int reqId, const UnderComp& underComp){
	TRACE("deltaNeutralValidation\n");
}

void CHiStarApp::tickSnapshotEnd( int reqId){
	TRACE("tickSnapshotEnd\n");
}

void CHiStarApp::marketDataType( TickerId reqId, int marketDataType){
	TRACE("marketDataType\n");
}

void CHiStarApp::commissionReport( const CommissionReport &commissionReport){
	TRACE("commissionReport\n");
}

void CHiStarApp::position( const IBString& account, const Contract& contract, int position, double avgCost){
	TRACE("position\n");
}

void CHiStarApp::positionEnd(){
	TRACE("positionEnd\n");
}

void CHiStarApp::accountSummary( int reqId, const IBString& account, const IBString& tag, const IBString& value, const IBString& curency){
	TRACE("accountSummary\n");
}

void CHiStarApp::accountSummaryEnd( int reqId){
	TRACE("accountSummaryEnd\n");
}

void CHiStarApp::verifyMessageAPI( const IBString& apiData){
	TRACE("verifyMessageAPI\n");
}

void CHiStarApp::verifyCompleted( bool isSuccessful, const IBString& errorText){
	TRACE("verifyCompleted\n");
}

void CHiStarApp::displayGroupList( int reqId, const IBString& groups){
	TRACE("displayGroupList\n");
}

void CHiStarApp::displayGroupUpdated( int reqId, const IBString& contractInfo){
	TRACE("displayGroupUpdated\n");
}