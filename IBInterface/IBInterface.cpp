#include "stdafx.h"
#include "afxdialogex.h"
#include "HiStar.h"
#include "global.h"
#include "UserMsg.h"
#include "OrderState.h"
#include "ibaccount.h"
#include "MainDlg.h"
#define NOT_AN_FA_ACCOUNT_ERROR 321
#define NUM_FA_ERROR_CODES 6
extern double AvailIB;
double g_a50Bid1 = 0.0f,g_a50Ask1 = 0.0f,g_a50last;
int g_a50Bid1Size = 0,g_a50Ask1Size = 0;
static int faErrorCodes[NUM_FA_ERROR_CODES] =
{ 503, 504, 505, 522, 1100, NOT_AN_FA_ACCOUNT_ERROR} ;
extern int netPositionA50;
DWORD MainThreadId = 0;
DWORD IndexThreadId = 0;
bool iAccountDownloadEnd = false;
bool iInitMarginOIfAddOneA50 = false;
int idReqA50Margin = -1;
CString getField( TickType tickType) {
	switch( tickType)
	{
		case BID_SIZE:	                    return "bidSize";
		case BID:		                    return "bidPrice";
		case ASK:		                    return "askPrice";
		case ASK_SIZE:	                    return "askSize";
		case LAST:		                    return "lastPrice";
		case LAST_SIZE:	                    return "lastSize";
		case HIGH:		                    return "high";
		case LOW:		                    return "low";
		case VOLUME:	                    return "volume";
		case CLOSE:		                    return "close";
		case BID_OPTION_COMPUTATION:		return "bidOptComp";
		case ASK_OPTION_COMPUTATION:		return "askOptComp";
		case LAST_OPTION_COMPUTATION:		return "lastOptComp";
		case MODEL_OPTION:					return "optionModel";
		case OPEN:                          return "open";
		case LOW_13_WEEK:                   return "13WeekLow";
		case HIGH_13_WEEK:                  return "13WeekHigh";
		case LOW_26_WEEK:                   return "26WeekLow";
		case HIGH_26_WEEK:                  return "26WeekHigh";
		case LOW_52_WEEK:                   return "52WeekLow";
		case HIGH_52_WEEK:                  return "52WeekHigh";
		case AVG_VOLUME:                    return "AvgVolume";
		case OPEN_INTEREST:                 return "OpenInterest";
		case OPTION_HISTORICAL_VOL:         return "OptionHistoricalVolatility";
		case OPTION_IMPLIED_VOL:            return "OptionImpliedVolatility";
		case OPTION_BID_EXCH:               return "OptionBidExchStr";
		case OPTION_ASK_EXCH:               return "OptionAskExchStr";
		case OPTION_CALL_OPEN_INTEREST:     return "OptionCallOpenInterest";
		case OPTION_PUT_OPEN_INTEREST:      return "OptionPutOpenInterest";
		case OPTION_CALL_VOLUME:            return "OptionCallVolume";
		case OPTION_PUT_VOLUME:             return "OptionPutVolume";
		case INDEX_FUTURE_PREMIUM:          return "IndexFuturePremium";
		case BID_EXCH:                      return "bidExch";
		case ASK_EXCH:                      return "askExch";
		case AUCTION_VOLUME:                return "auctionVolume";
		case AUCTION_PRICE:                 return "auctionPrice";
		case AUCTION_IMBALANCE:             return "auctionImbalance";
		case MARK_PRICE:                    return "markPrice";
		case BID_EFP_COMPUTATION:           return "bidEFP";
		case ASK_EFP_COMPUTATION:           return "askEFP";
		case LAST_EFP_COMPUTATION:          return "lastEFP";
		case OPEN_EFP_COMPUTATION:          return "openEFP";
		case HIGH_EFP_COMPUTATION:          return "highEFP";
		case LOW_EFP_COMPUTATION:           return "lowEFP";
		case CLOSE_EFP_COMPUTATION:         return "closeEFP";
		case LAST_TIMESTAMP:                return "lastTimestamp";
		case SHORTABLE:                     return "shortable";
		case FUNDAMENTAL_RATIOS:            return "fundamentals";
		case RT_VOLUME:                     return "RTVolume";
		case HALTED:                        return "halted";
		case BID_YIELD:                     return "bidYield";
		case ASK_YIELD:                     return "askYield";
		case LAST_YIELD:                    return "lastYield";             
		case CUST_OPTION_COMPUTATION:       return "custOptComp";
		case TRADE_COUNT:                   return "trades";
		case TRADE_RATE:                    return "trades/min";
		case VOLUME_RATE:                   return "volume/min";
		case LAST_RTH_TRADE:                return "lastRTHTrade";
		default:                            return "unknown";
	}
}

//IB交易系统
void CHiStarApp::tickPrice( TickerId tickerId, TickType field, double price, int canAutoExecute){
	//TRACE("tickPrice\n");
	if(price > 0.00000001){
		if(strcmp((const char*)getField(field),"bidPrice") == 0){
			g_a50Bid1 = price;
			if((CHiStarApp*)AfxGetApp()->m_pMainWnd){
				while(::PostMessage(((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->GetSafeHwnd(),WM_MD_REFRESH,NULL,NULL) == 0){
					Sleep(100);
				}
			}
		}
		else if(strcmp((const char*)getField(field),"askPrice") == 0){
			g_a50Ask1 = price;
			if((CHiStarApp*)AfxGetApp()->m_pMainWnd){
				while(::PostMessage(((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->GetSafeHwnd(),WM_MD_REFRESH,NULL,NULL) == 0){
					Sleep(100);
				}
			}
		}
		else if(strcmp((const char*)getField(field),"lastPrice") == 0){
			g_a50last = price;
			bool found = false;int index = -1;
			for(unsigned int i = 0;i < m_portfolio.size();i++){
				//暂时这样对比，日后还有期权，因为存在行权价，可能contract比较方法有异
				if( m_A50Contract.symbol == m_portfolio[i].contract.symbol
					&& m_A50Contract.currency == m_portfolio[i].contract.currency
					&& m_A50Contract.expiry.Left(6) == m_portfolio[i].contract.expiry.Left(6)
					&& m_A50Contract.secType == m_portfolio[i].contract.secType){
						index = i;found = true;break;
				}
			}
			if(found){
				double unrealizedPNL = (price - m_portfolio[index].avgCost) * m_portfolio[index].position;
				m_accountvalue.AvailableFunds = m_accountvalue.AvailableFundsO - m_accountvalue.UnrealizedPnLO + unrealizedPNL;
				if((CHiStarApp*)AfxGetApp()->m_pMainWnd){
					while(::PostMessage(((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->GetSafeHwnd(),WM_MD_REFRESH,NULL,NULL) == 0){
						Sleep(100);
					}
				}
			}
		}
	}
}

void CHiStarApp::tickSize( TickerId tickerId, TickType field, int size){
	//TRACE("tickSize\n");
	if(strcmp((const char*)getField(field),"bidSize") == 0){
		g_a50Bid1Size = size;
	}
	else if(strcmp((const char*)getField(field),"askSize") == 0){
		g_a50Ask1Size = size;
	}
}

void CHiStarApp::tickOptionComputation( TickerId tickerId, TickType tickType, double impliedVol, double delta,
	double optPrice, double pvDividend, double gamma, double vega, double theta, double undPrice){
		//TRACE("tickOptionComputation\n");
}
void CHiStarApp::tickGeneric(TickerId tickerId, TickType tickType, double value){
	//TRACE("tickGeneric\n");
}
void CHiStarApp::tickString(TickerId tickerId, TickType tickType, const IBString& value){
	//TRACE("tickString\n");
}
void CHiStarApp::tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const IBString& formattedBasisPoints,
	double totalDividends, int holdDays, const IBString& futureExpiry, double dividendImpact, double dividendsToExpiry){
		//TRACE("tickEFP\n");
}
void CHiStarApp::orderStatus( OrderId orderId, const IBString &status, int filled,
	int remaining, double avgFillPrice, int permId, int parentId,
	double lastFillPrice, int clientId, const IBString& whyHeld){
		TRACE("orderStatus\n");
		if(m_pHedgePostProcessing){
			OrderStatus * pOrderStatus = new OrderStatus;
			pOrderStatus->orderId = orderId;
			pOrderStatus->status = status;
			pOrderStatus->filled = filled;
			pOrderStatus->remaining = remaining;
			pOrderStatus->avgFillPrice = avgFillPrice;
			pOrderStatus->permId = permId;
			pOrderStatus->parentId = parentId;
			pOrderStatus->lastFillPrice = lastFillPrice;
			pOrderStatus->clientId = clientId;
			pOrderStatus->whyHeld = whyHeld;
			while(m_pHedgePostProcessing->PostThreadMessage(WM_RTN_ORDER_IB,NULL,(UINT)pOrderStatus) == 0){
				Sleep(100);
			}
		}
}

void CHiStarApp::openOrder(OrderId orderId, const Contract& contract, const Order& order, const OrderState& orderstate){
	TRACE("openOrder\n");
	if(!iInitMarginOIfAddOneA50 && idReqA50Margin == orderId){
		if(m_A50Contract.symbol == contract.symbol
			&& m_A50Contract.currency == contract.currency
			&& m_A50Contract.expiry.Left(6) == contract.expiry.Left(6)
			&& m_A50Contract.secType == contract.secType){
				m_accountvalue.InitMarginOIfAddOneA50 = atof(orderstate.initMargin);
				iInitMarginOIfAddOneA50 = true;
		}
	}
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
	iAccountDownloadEnd = false;
	iInitMarginOIfAddOneA50 = false;
}

void CHiStarApp::updateAccountValue(const IBString& key, const IBString& val,
	const IBString& currency, const IBString& accountName){
		//TRACE("%s,%s\n",key,val);
		if(!iAccountDownloadEnd){
			if(key == "AvailableFunds" && currency == "USD"){
				//AvailIB = atof(val);
				m_accountvalue.AvailableFundsO = atof(val);
				m_accountvalue.AvailableFunds = atof(val);
			}
			else if(key == "NetLiquidation" && currency == "USD"){
				m_accountvalue.NetLiquidationO = atof(val);
			}
			else if(key == "InitMarginReq" && currency == "USD"){
				m_accountvalue.InitMarginReqO = atof(val);
			}
			else if(key == "UnrealizedPnL" && currency == "USD"){
				m_accountvalue.UnrealizedPnLO = atof(val);
			}
		}
}

void CHiStarApp::updatePortfolio( const Contract& contract, int position,
	double marketPrice, double marketValue, double averageCost,
	double unrealizedPNL, double realizedPNL, const IBString& accountName){
		if(!iAccountDownloadEnd){
			Portfolio porf;
			porf.account = accountName;porf.contract = contract;porf.position = position;porf.avgCost = averageCost;//将成本价修改为当前市场价，为后续计算资金作准备
			porf.marketPrice = marketPrice;porf.marketValue = marketValue;porf.unrealizedPNL = unrealizedPNL;porf.realizedPNL = realizedPNL;
			bool found = false;
			for(unsigned int i = 0;i < m_portfolio.size();i++){
				//暂时这样对比，日后还有期权，因为存在行权价，可能contract比较方法有异
				if(accountName == m_portfolio[i].account 
					&& contract.symbol == m_portfolio[i].contract.symbol
					&& contract.exchange == m_portfolio[i].contract.exchange
					&& contract.currency == m_portfolio[i].contract.currency
					&& contract.expiry.Left(6) == m_portfolio[i].contract.expiry.Left(6)
					&& contract.secType == m_portfolio[i].contract.secType){
						m_portfolio[i] = porf;
						found = true;break;
				}
			}
			if(!found){
				m_portfolio.push_back(porf);
			}
		}
}

void CHiStarApp::updateAccountTime(const IBString& timeStamp){
	//TRACE("updateAccountTime\n");
}

void CHiStarApp::accountDownloadEnd(const IBString& accountName){
	TRACE("accountDownloadEnd\n");
	CString cStatus;
	cStatus.Format("Account Download End: %s",accountName);
	PostOrderStatus(cStatus);
	iAccountDownloadEnd = true;
	if(m_pHedgePostProcessing){
		TRACE("PostAccountDownloadEnd\n");
		while(m_pHedgePostProcessing->PostThreadMessage(WM_REQACCOUNTUPDATES_NOTIFY,NULL,NULL) == 0){
			Sleep(100);
		}
	}
}

void CHiStarApp::nextValidId( OrderId orderId){
	m_id = orderId;
	//查询A50保证金
	m_IBOrder.action = "BUY";
	m_IBOrder.totalQuantity = 1;
	m_IBOrder.lmtPrice = 8000;
	m_IBOrder.whatIf = true;
	m_pIBClient->placeOrder(++m_id,m_A50Contract,m_IBOrder);
	idReqA50Margin = m_id;
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
					if((CHiStarApp*)AfxGetApp()->m_pMainWnd){
						while(::PostMessage(((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->GetSafeHwnd(),WM_MD_REFRESH,NULL,NULL) == 0){
							Sleep(100);
						}
					}
				}
			}
			else{
				if(price != g_a50Ask1){
					g_a50Ask1 = price;
					g_a50Ask1Size = size;
					while(::PostThreadMessage(MainThreadId,WM_MD_REFRESH,NULL,NULL) == 0){
						Sleep(100);
					}
				}
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
	//TRACE("commissionReport\n");
}

void CHiStarApp::position( const IBString& account, const Contract& contract, int position, double avgCost){
	if(m_A50Contract.symbol == contract.symbol && m_A50Contract.expiry.Left(6) == contract.expiry.Left(6)){
		//netPositionA50 = position;
		TRACE("A50持仓%d\r\n",position);
	}
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