#include "StdAfx.h"
#include "traderspi.h"
#include "HiStar.h"
#include "UserMsg.h"
#include "Maindlg.h"
#pragma warning(disable :4996)
extern HANDLE g_hEvent;
BOOL g_bRecconnectT = FALSE;
BOOL g_bLoginCtpT = FALSE;
bool g_bOnceT = FALSE;//交易系统是否曾经登陆过，如果登陆过则是TRUE,否则FALSE
//网络故障恢复正常后 自动重连
void CtpTraderSpi::OnFrontConnected()
{
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	if (g_bOnceT)
	{
		g_bRecconnectT = TRUE;
		ReqUserLogin(pApp->m_accountCtp.m_sBROKER_ID,pApp->m_accountCtp.m_sINVESTOR_ID,pApp->m_accountCtp.m_sPASSWORD);
		SYSTEMTIME curTime;
		::GetLocalTime(&curTime);
		CString	szT;
		szT.Format(_T("%02d:%02d:%02d CTP重登录"), curTime.wHour, curTime.wMinute, curTime.wSecond);	
	}	
	else
	{//第一次登陆
		ReqUserLogin(pApp->m_accountCtp.m_sBROKER_ID,pApp->m_accountCtp.m_sINVESTOR_ID,pApp->m_accountCtp.m_sPASSWORD);
		g_bOnceT = true;
	}
}

void CtpTraderSpi::ReqUserLogin(TThostFtdcBrokerIDType	vAppId,TThostFtdcUserIDType	vUserId,TThostFtdcPasswordType	vPasswd)
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, vAppId); strcpy(BROKER_ID, vAppId); 
	strcpy(req.UserID, vUserId);  strcpy(INVEST_ID, vUserId); 
	strcpy(req.Password, vPasswd);
	strcpy(req.UserProductInfo,PROD_INFO);
	int iRet = pUserApi->ReqUserLogin(&req, ++m_iRequestID);
}

#define TIME_NULL "--:--:--"

void CtpTraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
		if(pRspInfo){memcpy(&m_RspMsg,pRspInfo,sizeof(CThostFtdcRspInfoField));}
		if(!IsErrorRspInfo(pRspInfo) && pRspUserLogin){ 
			g_bLoginCtpT = true;
			// 保存会话参数	
			m_ifrontId = pRspUserLogin->FrontID;
			m_isessionId = pRspUserLogin->SessionID;

			strcpy(m_sTdday,pRspUserLogin->TradingDay);

			int nextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
			sprintf(m_sOrdRef, "%d", ++nextOrderRef);

			SYSTEMTIME curTime;
			::GetLocalTime(&curTime);
			CTime tc(curTime);
			int i=0;
			int iHour[4],iMin[4],iSec[4];
			if (!strcmp(pRspUserLogin->DCETime,TIME_NULL) || !strcmp(pRspUserLogin->SHFETime,TIME_NULL)){
				for (i=0;i<4;i++){
					iHour[i]=curTime.wHour;
					iMin[i]=curTime.wMinute;
					iSec[i]=curTime.wSecond;
				}
			}
			else{
				sscanf(pRspUserLogin->SHFETime, "%d:%d:%d", &iHour[0], &iMin[0], &iSec[0]);
				sscanf(pRspUserLogin->DCETime, "%d:%d:%d", &iHour[1], &iMin[1], &iSec[1]);
				sscanf(pRspUserLogin->CZCETime, "%d:%d:%d", &iHour[2], &iMin[2], &iSec[2]);
				sscanf(pRspUserLogin->FFEXTime, "%d:%d:%d", &iHour[3], &iMin[3], &iSec[3]);
			}
			CTime t[4];
			for (i=0;i<4;i++){
				t[i] = CTime(curTime.wYear,curTime.wMonth,curTime.wDay,iHour[i],iMin[i],iSec[i]);
				m_tsEXnLocal[i] = t[i]-tc;
			}
			sprintf(m_sTmBegin,"%02d:%02d:%02d.%03d",curTime.wHour,curTime.wMinute,curTime.wSecond,curTime.wMilliseconds); 
		}
		if(bIsLast) SetEvent(g_hEvent);
}

const char* CtpTraderSpi::GetTradingDay()
{
	return m_sTdday;
}

void CtpTraderSpi::ReqUserLogout()
{
	CThostFtdcUserLogoutField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.UserID, INVEST_ID);
	int iResult = pUserApi->ReqUserLogout(&req, ++m_iRequestID);
	if(iResult == 0){
		TRACE("ctp行情系统登出指令发送成功\r\n");
	}
	else{
		TRACE("ctp行情系统登出指令发送失败\r\n");
	}
}

///登出请求响应
void CtpTraderSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pUserLogout){
		TRACE(_T("已经登出ctpT\n"));
		g_bLoginCtpT = FALSE;
	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqSettlementInfoConfirm()
{
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	pUserApi->ReqSettlementInfoConfirm(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspSettlementInfoConfirm(
	CThostFtdcSettlementInfoConfirmField  *pSettlementInfoConfirm, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{	
	if( !IsErrorRspInfo(pRspInfo) && pSettlementInfoConfirm){
	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQryNotice()
{
	CThostFtdcQryNoticeField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	pUserApi->ReqQryNotice(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspQryNotice(CThostFtdcNoticeField *pNotice, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) &&  pNotice){}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQryTdNotice()
{
	CThostFtdcQryTradingNoticeField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	pUserApi->ReqQryTradingNotice(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspQryTradingNotice(CThostFtdcTradingNoticeField *pTradingNotice, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) &&  pTradingNotice){}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQrySettlementInfoConfirm()
{
	CThostFtdcQrySettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	pUserApi->ReqQrySettlementInfoConfirm(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) &&  pSettlementInfoConfirm){}	
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQrySettlementInfo(TThostFtdcDateType TradingDay)
{
	CThostFtdcQrySettlementInfoField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	strcpy(req.TradingDay,TradingDay);

	pUserApi->ReqQrySettlementInfo(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) &&  pSettlementInfo)
	{
		CThostFtdcSettlementInfoField Si;
		memcpy(&Si,pSettlementInfo,sizeof(CThostFtdcSettlementInfoField));
		m_StmiVec.push_back(Si);
	}
	if(bIsLast) 
	{ 
		SetEvent(g_hEvent);
		//SendNotifyMessage(HWND_BROADCAST,WM_QRYSMI_MSG,0,0);		
	}
}

void CtpTraderSpi::ReqQryInst(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	if (instId != NULL)
	{ strcpy(req.InstrumentID, instId); }

	pUserApi->ReqQryInstrument(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{ 
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	if (!IsErrorRspInfo(pRspInfo) &&  pInstrument)
	{
		INSINFEX InsInf;
		ZeroMemory(&InsInf,sizeof(INSINFEX));
		memcpy(&InsInf,pInstrument,sizeof(INSTINFO));
		bool founded = false;
		for(int i = 0;i < m_InsinfVec.size();i++){
			if(!strcmp(m_InsinfVec[i].iinf.InstrumentID,InsInf.iinf.InstrumentID)){
				founded = true;
				break;
			}
		}
		if(!founded){
			m_InsinfVec.push_back(InsInf);
		}
	}
	if(bIsLast){
		TRACE(_T("合约查询完毕\n"));
		/*
		((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_InsinfVec = pApp->m_cT->m_InsinfVec;
		((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_LstAllInsts.SetItemCountEx(((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_InsinfVec.size());
		((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_LstAllInsts.Invalidate();
		*/
		PostThreadMessage(MainThreadId,WM_UPDATE_LSTCTRL,NULL,NULL);
		SetEvent(g_hEvent);
	}
}

void CtpTraderSpi::ReqQryTdAcc()
{
	CThostFtdcQryTradingAccountField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	pUserApi->ReqQryTradingAccount(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspQryTradingAccount(
	CThostFtdcTradingAccountField *pTradingAccount, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{ 
	CThostFtdcTradingAccountField *pAcc;
	if (!IsErrorRspInfo(pRspInfo) &&  pTradingAccount)
	{
		pAcc = new CThostFtdcTradingAccountField();
		memcpy(pAcc,pTradingAccount,sizeof(CThostFtdcTradingAccountField));
		m_TdAcc = *pAcc;
	}
	if(bIsLast){
		if(AfxGetApp()->m_pMainWnd){
			PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd,WM_UPDATE_ACC_CTP,NULL,(LPARAM)pAcc);
		}
		SetEvent(g_hEvent);
	}	
}

//INSTRUMENT_ID设成部分字段,例如IF10,就能查出所有IF10打头的头寸
void CtpTraderSpi::ReqQryInvPos(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInvestorPositionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	if (instId!=NULL)
	{strcpy(req.InstrumentID, instId);}		
	pUserApi->ReqQryInvestorPosition(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspQryInvestorPosition(
	CThostFtdcInvestorPositionField *pInvestorPosition, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{ 
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	if(pInvestorPosition && pInvestorPosition->YdPosition != 0){
		TRACE("%s昨持仓%d\n",pInvestorPosition->InstrumentID,pInvestorPosition->YdPosition);
	}
	if(!pInvestorPosition){
		TRACE("持仓为0\n");
	}
	if(!IsErrorRspInfo(pRspInfo)&&pInvestorPosition)
	{
		TRACE("%s持仓%d昨仓%d方向%c\n",pInvestorPosition->InstrumentID,pInvestorPosition->Position,pInvestorPosition->YdPosition,pInvestorPosition->PosiDirection);
		CThostFtdcInvestorPositionField InvPos;
		memcpy(&InvPos,pInvestorPosition, sizeof(CThostFtdcInvestorPositionField));
		bool founded = false;
		for(int i = 0;i < m_InvPosVec.size();i++){
			//多头和空头是不同的持仓，所以除了判断合约代码还要判断持仓方向。
			if(!strcmp(m_InvPosVec[i].InstrumentID,InvPos.InstrumentID) && m_InvPosVec[i].PosiDirection == InvPos.PosiDirection){
				founded = true;
				break;
			}
		}
		if(!founded){
			m_InvPosVec.push_back(InvPos);
		}
	}
	if(bIsLast){ 
		PostThreadMessage(MainThreadId,WM_UPDATE_LSTCTRL,NULL,NULL);
		SetEvent(g_hEvent);
	}
}
void CtpTraderSpi::ReqQryOrder(TThostFtdcInstrumentIDType instId){
	CThostFtdcQryOrderField req;
	memset(&req,0,sizeof(req));
	strcpy(req.BrokerID,BROKER_ID);
	strcpy(req.InvestorID,INVEST_ID);
	if (instId!=NULL)
	{strcpy(req.InstrumentID, instId);}	
	pUserApi->ReqQryOrder(&req, ++m_iRequestID);
}

void  CtpTraderSpi::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	if(!IsErrorRspInfo(pRspInfo)&&pOrder){
		CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
		CThostFtdcOrderField order;
		TRACE("OnRtnOrder所有报单通知%s,%s,%c,%d,已经成交%d\r\n",pOrder->OrderRef, pOrder->OrderSysID,pOrder->OrderStatus,pOrder->BrokerOrderSeq,pOrder->VolumeTraded);
		memcpy(&order,pOrder, sizeof(CThostFtdcOrderField));
		bool founded = false;UINT i = 0;
		for(i = 0;i<m_orderVec.size();i++){
			if(m_orderVec[i].BrokerOrderSeq == order.BrokerOrderSeq) { 
				founded = true;
				//修改命令状态
				m_orderVec[i] = order;
				break;
			}
		}		
		if(!founded){
			//将挂单删除，因为这时有消息返回说明已经递送出去了
			int nRet = ((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.FindOrdInOnRoadVec(order.BrokerOrderSeq);
			//未加入挂单列表
			if (nRet==-1){
				//如果没有,则不做任何动作
			}
			else{
				//挂单返回，表示已经递送出去，将该挂单删除(已经变成委托单或其他)
			}
			///////新增加委托单
			m_orderVec.insert(m_orderVec.begin(),order);//从头部插入
		}
	}
	if(bIsLast) {
		PostThreadMessage(MainThreadId,WM_UPDATE_LSTCTRL,NULL,NULL);
		SetEvent(g_hEvent);
	}
}

void CtpTraderSpi::ReqQryTrade(TThostFtdcInstrumentIDType instId){
	CThostFtdcQryTradeField req;
	memset(&req,0,sizeof(req));
	strcpy(req.BrokerID,BROKER_ID);
	strcpy(req.InvestorID,INVEST_ID);
	if (instId!=NULL)
	{strcpy(req.InstrumentID, instId);}		
	pUserApi->ReqQryTrade(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if(!IsErrorRspInfo(pRspInfo)&&pTrade){
		TRACE("成交通知\n");
		CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
		CThostFtdcTradeField trade;
		memcpy(&trade,pTrade,sizeof(CThostFtdcTradeField));
		bool founded = false;     
		unsigned int ii = 0;
		for(ii = 0;ii<m_tradeVec.size();ii++){
			//strcmp返回0时表示相等
			if(strcmp(m_tradeVec[ii].TradeID,trade.TradeID) == 0){
				founded = true;   
				break;
			}
		}
		//////修改成交单状态
		if(founded){
		}
		///////新增加已成交单 
		else 
		{
			m_tradeVec.insert(m_tradeVec.begin(),trade);
		}
	}
	if(bIsLast) {
		PostThreadMessage(MainThreadId,WM_UPDATE_LSTCTRL,NULL,NULL);
		SetEvent(g_hEvent);
	}
}

//INSTRUMENT_ID设成部分字段,例如IF10,就能查出所有IF10打头的头寸
void CtpTraderSpi::ReqQryInvPosEx(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInvestorPositionDetailField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	if (instId!=NULL)
	{strcpy(req.InstrumentID, instId);}		
	pUserApi->ReqQryInvestorPositionDetail(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) &&  pInvestorPositionDetail)
	{

	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQryInvPosCombEx(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInvestorPositionCombineDetailField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	if (instId!=NULL)
	{strcpy(req.CombInstrumentID, instId);}		
	pUserApi->ReqQryInvestorPositionCombineDetail(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspQryInvestorPositionCombineDetail(CThostFtdcInvestorPositionCombineDetailField *pInvestorPositionDetail, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) &&  pInvestorPositionDetail)
	{

	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqOrdLimit(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir,
	TThostFtdcCombOffsetFlagType kpp,TThostFtdcPriceType price,   TThostFtdcVolumeType vol)
{
	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));	
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID); 
	strcpy(req.InstrumentID, instId); 	
	strcpy(req.OrderRef, m_sOrdRef);
	int nextOrderRef = atoi(m_sOrdRef);
	sprintf(m_sOrdRef, "%d", ++nextOrderRef);

	req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;//价格类型=限价	
	req.Direction = MapDirection(dir,true);  //买卖方向	
	req.CombOffsetFlag[0] = MapOffset(kpp[0],true); //组合开平标志:开仓
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;	  //组合投机套保标志
	req.LimitPrice = price;	//价格
	req.VolumeTotalOriginal = vol;	///数量	
	req.TimeCondition = THOST_FTDC_TC_GFD;  //有效期类型:当日有效
	req.VolumeCondition = THOST_FTDC_VC_AV; //成交量类型:任何数量
	req.MinVolume = 1;	//最小成交量:1	
	req.ContingentCondition = THOST_FTDC_CC_Immediately;  //触发条件:立即

	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;	//强平原因:非强平	
	req.IsAutoSuspend = 0;  //自动挂起标志:否	
	req.UserForceClose = 0;   //用户强评标志:否

	pUserApi->ReqOrderInsert(&req, ++m_iRequestID);

}

///市价单
void CtpTraderSpi::ReqOrdAny(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp,TThostFtdcVolumeType vol)
{
	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));	
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID); 
	strcpy(req.InstrumentID, instId); 	
	strcpy(req.OrderRef, m_sOrdRef);
	int nextOrderRef = atoi(m_sOrdRef);
	sprintf(m_sOrdRef, "%d", ++nextOrderRef);

	req.OrderPriceType = THOST_FTDC_OPT_AnyPrice;//市价
	req.Direction = MapDirection(dir,true);  //买卖方向	
	req.CombOffsetFlag[0] = MapOffset(kpp[0],true); //组合开平标志:开仓
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;	  //组合投机套保标志
	//req.LimitPrice = price;	//价格
	req.VolumeTotalOriginal = vol;	///数量	
	req.TimeCondition = THOST_FTDC_TC_IOC;;  //立即有效
	req.VolumeCondition = THOST_FTDC_VC_AV; //成交量类型:任何数量
	req.MinVolume = 1;	//最小成交量:1	
	req.ContingentCondition = THOST_FTDC_CC_Immediately;  //触发条件:立即

	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;	//强平原因:非强平	
	req.IsAutoSuspend = 0;  //自动挂起标志:否	
	req.UserForceClose = 0;   //用户强评标志:否

	pUserApi->ReqOrderInsert(&req, ++m_iRequestID);	
}

void CtpTraderSpi::ReqOrdCondition(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp,
	TThostFtdcPriceType price,TThostFtdcVolumeType vol,TThostFtdcPriceType stopPrice,TThostFtdcContingentConditionType conType)
{
	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));	
	strcpy(req.BrokerID, BROKER_ID);  //经纪公司代码	
	strcpy(req.InvestorID, INVEST_ID); //投资者代码	
	strcpy(req.InstrumentID, instId); //合约代码	
	strcpy(req.OrderRef, m_sOrdRef);  //报单引用
	int nextOrderRef = atoi(m_sOrdRef);
	sprintf(m_sOrdRef, "%d", ++nextOrderRef);

	req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;	
	req.Direction = MapDirection(dir,true);  //买卖方向	
	req.CombOffsetFlag[0] = MapOffset(kpp[0],true); //组合开平标志:开仓
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	req.LimitPrice = price;	//价格
	req.VolumeTotalOriginal = vol;	///数量	
	req.TimeCondition = THOST_FTDC_TC_GFD;  //当日有效
	req.VolumeCondition = THOST_FTDC_VC_AV; //成交量类型:任何数量
	req.MinVolume = 1;	//最小成交量:1	
	req.ContingentCondition = conType;  //触发条件

	req.StopPrice = stopPrice;  //止损价
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;	//强平原因:非强平	
	req.IsAutoSuspend = 0;  //自动挂起标志:否	
	req.UserForceClose = 0;   //用户强评标志:否

	pUserApi->ReqOrderInsert(&req, ++m_iRequestID);
}

/*
FAK(Fill And Kill)指令就是将报单的有效期设为THOST_FTDC_TC_IOC,同时,成交量类型设为THOST_FTDC_VC_AV,即任意数量;
FOK(Fill Or Kill)指令是将报单的有效期类型设置为THOST_FTDC_TC_IOC,同时将成交量类型设置为THOST_FTDC_VC_CV,即全部数量.
此外,在FAK指令下,还可指定最小成交量,即在指定价位、满足最小成交数量以上成交,剩余订单被系统撤销,否则被系统全部撤销.此种状况下,
有效期类型设置为THOST_FTDC_TC_IOC,数量条件设为THOST_FTDC_VC_MV,同时设定MinVolume字段.
*/
void CtpTraderSpi::ReqOrdFAOK(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir,TThostFtdcCombOffsetFlagType kpp,
	TThostFtdcPriceType price,/*TThostFtdcVolumeType vol,*/TThostFtdcVolumeConditionType volconType,TThostFtdcVolumeType minVol)
{
	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));	
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID); 
	strcpy(req.InstrumentID, instId); 	
	strcpy(req.OrderRef, m_sOrdRef);
	int nextOrderRef = atoi(m_sOrdRef);
	sprintf(m_sOrdRef, "%d", ++nextOrderRef);

	req.OrderPriceType = THOST_FTDC_OPT_LimitPrice; //限价
	req.Direction = MapDirection(dir,true);  //买卖方向	
	req.CombOffsetFlag[0] = MapOffset(kpp[0],true); //组合开平标志
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;	//组合投机套保标志
	req.LimitPrice = price;	//价格
	//req.VolumeTotalOriginal = vol;	///数量	
	req.TimeCondition = THOST_FTDC_TC_IOC;  //立即
	req.VolumeCondition = volconType; //THOST_FTDC_VC_AV,THOST_FTDC_VC_MV;THOST_FTDC_VC_CV
	req.MinVolume = minVol;	//FAK在THOST_FTDC_VC_MV时候可指定MinVol,其它情况置0
	req.ContingentCondition = THOST_FTDC_CC_Immediately;  //触发条件:立即

	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;	//强平原因:非强平	
	req.IsAutoSuspend = 0;  //自动挂起标志:否	
	req.UserForceClose = 0;   //用户强评标志:否

	pUserApi->ReqOrderInsert(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( IsErrorRspInfo(pRspInfo) || (pInputOrder==NULL) )
	{
		TRACE(_T("OnRspOrderInsert,%s\n"),pRspInfo->ErrorMsg);
		TCHAR szErr[MAX_PATH];
		//ansi2uni(CP_ACP,pRspInfo->ErrorMsg,szErr);

		/////////////////////////////////////////////////////////////////////
		CString szItems[ORDER_ITMES],szStat;

		szItems[0].Empty();
		//ansi2uni(CP_ACP,pInputOrder->InstrumentID,szItems[1].GetBuffer(MAX_PATH));
		szItems[2]=JgBsType(pInputOrder->Direction);
		szItems[3]=JgOcType(pInputOrder->CombOffsetFlag[0]);

		szItems[4]=_GERR;

		szItems[5].Format(_T("%f"),pInputOrder->LimitPrice);
		szItems[5].TrimRight('0');
		int iLen = szItems[5].GetLength();
		if (szItems[5].Mid(iLen-1,1)==_T(".")) {szItems[5].TrimRight(_T("."));}

		szItems[6].Format(_T("%d"),pInputOrder->VolumeTotalOriginal);

		////////////////////////////////////////////////////////
		szStat.Format(_T("失败:%s,%s,%s"),szItems[1],szItems[2],szItems[3]);

		szItems[7].Format(_T("%d"),pInputOrder->VolumeTotalOriginal);
		szItems[8] = _T("0");


		szItems[9] = szItems[5];
		getCurTime(szItems[10]);
		szItems[11] =  _T("―");
		szItems[12] =  _T("0");
		szItems[13] =  _T("0");
		szItems[14]= szErr;
	}
	if(bIsLast) SetEvent(g_hEvent);	
}

void CtpTraderSpi::ReqOrderCancel(TThostFtdcSequenceNoType orderSeq)
{
	bool found=false; UINT i=0;
	for(i=0;i<m_orderVec.size();i++){
		if(m_orderVec[i].BrokerOrderSeq == orderSeq){ found = true; break;}
	}
	if(!found)
	{
		////////报单已被成交或不存在///////////
		return;
	} 

	CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	
	strcpy(req.InvestorID, INVEST_ID); //投资者代码
	strcpy(req.ExchangeID, m_orderVec[i].ExchangeID);
	strcpy(req.OrderSysID, m_orderVec[i].OrderSysID);
	req.ActionFlag = THOST_FTDC_AF_Delete;  //操作标志 

	pUserApi->ReqOrderAction(&req, ++m_iRequestID);
	//cerr<< " 请求 | 发送撤单..." <<((ret == 0)?"成功":"失败") << endl;
}

void CtpTraderSpi::ReqOrderCancel(TThostFtdcInstrumentIDType instId,TThostFtdcOrderRefType OrderRef)
{
	// FrontID + SessionID + OrdRef + InstrumentID
	CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	
	strcpy(req.InvestorID, INVEST_ID); //投资者代码
	strcpy(req.InstrumentID, instId); //合约代码
	strcpy(req.OrderRef, m_sOrdRef); //报单引用	
	req.FrontID = m_ifrontId;           //前置编号	
	req.SessionID = m_isessionId;       //会话编号

	req.ActionFlag = THOST_FTDC_AF_Delete;  //操作标志 

	pUserApi->ReqOrderAction(&req, ++m_iRequestID);
	//cerr<< " 请求 | 发送撤单..." <<((ret == 0)?"成功":"失败") << endl;
}

void CtpTraderSpi::OnRspOrderAction(
	CThostFtdcInputOrderActionField *pInputOrderAction, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{	
	if (IsErrorRspInfo(pRspInfo) || (pInputOrderAction==NULL))
	{
		if (true)
		{
			//////////////////////////////////////////
			//pDlg->SetStatusTxt(_T("撤单失败!"),2);

		}
	}
	if(bIsLast) SetEvent(g_hEvent);	
}

///报单回报
void CtpTraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder){
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	CThostFtdcOrderField order;
	TRACE("OnRtnOrder所有报单通知%s,%s,%c,%d,已经成交%d\r\n",pOrder->OrderRef, pOrder->OrderSysID,pOrder->OrderStatus,pOrder->BrokerOrderSeq,pOrder->VolumeTraded);
	memcpy(&order,pOrder, sizeof(CThostFtdcOrderField));
	bool founded = false;UINT i = 0;
	for(i = 0;i<m_orderVec.size();i++){
		if(m_orderVec[i].BrokerOrderSeq == order.BrokerOrderSeq) { 
			founded = true;
			//修改命令状态
			m_orderVec[i] = order;
			PostThreadMessage(MainThreadId,WM_UPDATE_LSTCTRL,NULL,NULL);
			break;
		}
	}		
	if(!founded){
		//将挂单删除，因为这时有消息返回说明已经递送出去了
		int nRet = ((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.FindOrdInOnRoadVec(order.BrokerOrderSeq);
		//未加入挂单列表
		if (nRet==-1){
			//如果没有,则不做任何动作
		}
		else{
			//挂单返回，表示已经递送出去，将该挂单删除(已经变成委托单或其他)
		}
		///////新增加委托单
		m_orderVec.push_back(order);
		PostThreadMessage(MainThreadId,WM_UPDATE_LSTCTRL,NULL,NULL);
	}
}

///成交通知
void CtpTraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade){
	TRACE("成交通知\n");
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	CThostFtdcTradeField trade;
	memcpy(&trade,pTrade,sizeof(CThostFtdcTradeField));
	bool founded = false;     
	unsigned int ii = 0;
	for(ii = 0;ii<m_tradeVec.size();ii++){
		//strcmp返回0时表示相等
		if(strcmp(m_tradeVec[ii].TradeID,trade.TradeID) == 0){
			founded = true;   
			break;
		}
	}
	//////修改成交单状态
	if(founded){
		/*该处删去,因为成交编号是表示单笔成交,并不是表示单笔委托,分几笔成交的单子将会有几个成交编号
		//不过是重新覆盖成交信息
		int VolumeTotal;double PriceAvg;
		VolumeTotal = m_tradeVec[ii].Volume + trade.Volume;
		PriceAvg = (m_tradeVec[ii].Price * m_tradeVec[ii].Volume + trade.Price * trade.Volume) / VolumeTotal;
		m_tradeVec[ii].Volume = VolumeTotal;
		m_tradeVec[ii].Price = PriceAvg;
		*/
	}
	///////新增加已成交单 
	else 
	{
		m_tradeVec.push_back(trade);
		PostThreadMessage(MainThreadId,WM_UPDATE_LSTCTRL,NULL,NULL);
		/*
		//有关持仓的计算暂时取消??????
		/////////////////////////刷新持仓////////////////////////////////
		bool bExist = false;//是否存在该持仓
		unsigned int j = 0;
		for(j = 0;j<m_InvPosVec.size();j++)
		{
		//到持仓列表里搜索
		if (strcmp(trade->InstrumentID,m_InvPosVec[j]->InstrumentID) == 0){ 
		bExist = true; 
		break;
		}	
		}
		if (bExist){//在持仓列表里,只需要更新持仓状态
		if (trade->Direction == THOST_FTDC_D_Buy){
		m_InvPosVec[j]->Position = m_InvPosVec[j]->Position + trade->Volume;
		((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_InvPosVec[j]->Position = ((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_InvPosVec[j]->Position + trade->Volume;
		//持仓均价暂时无法计算？
		((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_LstInvPosInf.Invalidate();
		switch(trade->OffsetFlag){
		case THOST_FTDC_OF_Open:
		break;
		case THOST_FTDC_OF_Close:

		break;
		case THOST_FTDC_OF_CloseToday:

		break;
		case THOST_FTDC_OF_CloseYesterday:

		break;
		case THOST_FTDC_OF_ForceOff:

		break;
		case THOST_FTDC_OF_LocalForceClose:

		break;			
		}
		}
		if (trade->Direction == THOST_FTDC_D_Sell)
		{
		m_InvPosVec[j]->Position = m_InvPosVec[j]->Position - trade->Volume;
		((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_InvPosVec[j]->Position = ((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_InvPosVec[j]->Position - trade->Volume;
		//持仓均价暂时无法计算？
		((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_LstInvPosInf.Invalidate();
		switch(trade->OffsetFlag){
		case THOST_FTDC_OF_Open:
		break;
		case THOST_FTDC_OF_Close:
		break;
		case THOST_FTDC_OF_CloseToday:
		break;
		case THOST_FTDC_OF_CloseYesterday:
		break;
		case THOST_FTDC_OF_ForceOff:
		break;
		case THOST_FTDC_OF_LocalForceClose:
		break;			
		}
		}
		}
		else{
		CThostFtdcInvestorPositionField* newInvPos = new CThostFtdcInvestorPositionField();
		ZeroMemory(newInvPos,sizeof(CThostFtdcInvestorPositionField));
		int iMul = pApp->FindInstMul(trade->InstrumentID);//合约乘数
		strcpy(newInvPos->InstrumentID,trade->InstrumentID);
		strcpy(newInvPos->BrokerID,trade->BrokerID);
		strcpy(newInvPos->InvestorID,trade->InvestorID);
		newInvPos->PosiDirection = trade->Direction + 2;
		newInvPos->HedgeFlag = trade->HedgeFlag;
		newInvPos->PositionDate = (strcmp(m_sTdday,trade->TradeDate)==0)?THOST_FTDC_PSD_Today:THOST_FTDC_PSD_History;
		newInvPos->Position = trade->Volume;
		newInvPos->OpenVolume = trade->Volume;
		newInvPos->OpenAmount = trade->Volume * trade->Price * iMul;
		newInvPos->PositionCost = trade->Volume * trade->Price * iMul;
		m_InvPosVec.push_back(newInvPos);
		((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_InvPosVec.push_back(newInvPos);
		((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_LstInvPosInf.SetItemCountEx(((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_InvPosVec.size());
		((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_LstInvPosInf.Invalidate();
		}
		/////////////////////////////////////////////////////////////////
		*/
	}
}

void CtpTraderSpi::OnFrontDisconnected(int nReason){
	if (true){
		g_bLoginCtpT = FALSE;
		SYSTEMTIME curTime;
		::GetLocalTime(&curTime);
		CString	szT;
		szT.Format(_T("%02d:%02d:%02d CTP中断等待重连"), curTime.wHour, curTime.wMinute, curTime.wSecond);
		//ShowErroTips(IDS_DISCONTIPS,IDS_STRTIPS);
	}
}
void CtpTraderSpi::OnHeartBeatWarning(int nTimeLapse){
	TRACE(_T("OnHeartBeatWarningT\n"));
}
///请求查询交易编码
void CtpTraderSpi::ReqQryTradingCode()
{
	CThostFtdcQryTradingCodeField req;
	memset(&req, 0, sizeof(req));
	req.ClientIDType = THOST_FTDC_CIDT_Speculation;
	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	
	strcpy(req.InvestorID, INVEST_ID); //投资者代码
	pUserApi->ReqQryTradingCode(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQryTradingCode(CThostFtdcTradingCodeField *pTradingCode, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pTradingCode ){
		CThostFtdcTradingCodeField TdCode;
		memcpy(&TdCode, pTradingCode, sizeof(CThostFtdcTradingCodeField));
		m_TdCodeVec.push_back(TdCode);
	}
	if(bIsLast) SetEvent(g_hEvent);
}

///请求查询合约保证金率
void CtpTraderSpi::ReqQryInstMgr(TThostFtdcInstrumentIDType instId){
	CThostFtdcQryInstrumentMarginRateField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	
	strcpy(req.InvestorID, INVEST_ID); //投资者代码
	if (instId!=NULL)
	{strcpy(req.InstrumentID, instId);}	
	req.HedgeFlag = '1';
	pUserApi->ReqQryInstrumentMarginRate(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pInstrumentMarginRate )
	{

		CThostFtdcInstrumentMarginRateField MaginRate;
		memcpy(&MaginRate,  pInstrumentMarginRate, sizeof(CThostFtdcInstrumentMarginRateField));
		m_MargRateVec.push_back(MaginRate);
	}
	if(bIsLast) SetEvent(g_hEvent);

}

///请求查询合约手续费率
void CtpTraderSpi::ReqQryInstFee(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInstrumentCommissionRateField req;

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	
	strcpy(req.InvestorID, INVEST_ID); //投资者代码
	if (instId!=NULL)
	{strcpy(req.InstrumentID, instId);}	
	pUserApi->ReqQryInstrumentCommissionRate(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pInstrumentCommissionRate )
	{
		//CThostFtdcInstrumentCommissionRateField* pFeeRate = new CThostFtdcInstrumentCommissionRateField();
		memcpy(&m_FeeRateRev,  pInstrumentCommissionRate, sizeof(CThostFtdcInstrumentCommissionRateField)); 
		//FeeRateList.push_back(pFeeRate);

	}
	if(bIsLast) SetEvent(g_hEvent);

}

//////////////////请求查询用户资料/////////////
void CtpTraderSpi::ReqQryInvestor()
{
	CThostFtdcQryInvestorField req;

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	
	strcpy(req.InvestorID, INVEST_ID); //投资者代码

	pUserApi->ReqQryInvestor(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspQryInvestor(CThostFtdcInvestorField *pInvestor, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pInvestor )
	{
		CThostFtdcInvestorField Inv;
		memcpy(&Inv,pInvestor,sizeof(CThostFtdcInvestorField));

		//SendNotifyMessage(((CXTraderDlg*)g_pCWnd)->m_hWnd,WM_QRYUSER_MSG,0,(LPARAM)pInv);
	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqUserPwdUpdate(TThostFtdcPasswordType szNewPass,TThostFtdcPasswordType szOldPass)
{
	CThostFtdcUserPasswordUpdateField req;

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	
	strcpy(req.UserID, INVEST_ID); //用户代码
	strcpy(req.NewPassword, szNewPass);    	
	strcpy(req.OldPassword,szOldPass);  

	pUserApi->ReqUserPasswordUpdate(&req,++m_iRequestID);
}

///用户口令更新请求响应
void CtpTraderSpi::OnRspUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pUserPasswordUpdate )
	{
		//ShowErroTips(IDS_MODPASSOK,IDS_STRTIPS);
	}
	else
	{
		ShowErrTips(pRspInfo->ErrorMsg);
	}
	if(bIsLast) SetEvent(g_hEvent);
}

//资金账户密码
void CtpTraderSpi::ReqTdAccPwdUpdate(TThostFtdcPasswordType szNewPass,TThostFtdcPasswordType szOldPass)
{
	CThostFtdcTradingAccountPasswordUpdateField req;

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	
	strcpy(req.AccountID, INVEST_ID); //用户代码
	strcpy(req.NewPassword, szNewPass);    	
	strcpy(req.OldPassword,szOldPass);  

	pUserApi->ReqTradingAccountPasswordUpdate(&req,++m_iRequestID);
}

///资金账户口令更新请求响应
void CtpTraderSpi::OnRspTradingAccountPasswordUpdate(CThostFtdcTradingAccountPasswordUpdateField *pTradingAccountPasswordUpdate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pTradingAccountPasswordUpdate )
	{
		//ShowErroTips(IDS_MODPASSOK,IDS_STRTIPS);
	}
	else
	{
		ShowErrTips(pRspInfo->ErrorMsg);
	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqAuthenticate(TThostFtdcProductInfoType UserProdInf,TThostFtdcAuthCodeType	AuthCode)
{
	CThostFtdcReqAuthenticateField req;

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	
	strcpy(req.UserID, INVEST_ID); //用户代码
	strcpy(req.UserProductInfo, UserProdInf);    	
	strcpy(req.AuthCode,AuthCode);  

	pUserApi->ReqAuthenticate(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pRspAuthenticateField )
	{
		//memcpy(&g_InvInf,pInvestor,sizeof(CThostFtdcInvestorField));
		//AfxMessageBox(tName);
	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQryAccreg()
{
	CThostFtdcQryAccountregisterField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	
	strcpy(req.AccountID, INVEST_ID); //用户代码

	pUserApi->ReqQryAccountregister(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQryAccountregister(CThostFtdcAccountregisterField *pAccountregister, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pAccountregister)
	{
		CThostFtdcAccountregisterField AccReg;
		memcpy(&AccReg,  pAccountregister, sizeof(CThostFtdcAccountregisterField));
		m_AccRegVec.push_back(AccReg);
	}
	if(bIsLast) SetEvent(g_hEvent);
}


void CtpTraderSpi::ReqQryTransBk(TThostFtdcBankIDType BankID,TThostFtdcBankBrchIDType BankBrchID)
{
	CThostFtdcQryTransferBankField req;
	memset(&req, 0, sizeof(req));
	if(BankID != NULL)
		strcpy(req.BankID,BankID);
	if(BankBrchID != NULL)
		strcpy(req.BankBrchID,BankBrchID);

	pUserApi->ReqQryTransferBank(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQryTransferBank(CThostFtdcTransferBankField *pTransferBank, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pTransferBank)
	{
		//memcpy(&g_InvInf,pInvestor,sizeof(CThostFtdcInvestorField));
		//AfxMessageBox(tName);
	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQryContractBk(TThostFtdcBankIDType BankID,TThostFtdcBankBrchIDType BankBrchID)
{
	CThostFtdcQryContractBankField req;

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	

	if(BankID != NULL)
		strcpy(req.BankID,BankID);
	if(BankBrchID != NULL)
		strcpy(req.BankBrchID,BankBrchID);

	pUserApi->ReqQryContractBank(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQryContractBank(CThostFtdcContractBankField *pContractBank, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pContractBank)
	{
		//TCHAR szBankID[20],szBankBrchID[20],szBankName[MAX_PATH];
		//ansi2uni(CP_ACP,pContractBank->BankID,szBankID);
		//ansi2uni(CP_ACP,pContractBank->BankBrchID,szBankBrchID);
		//ansi2uni(CP_ACP,pContractBank->BankName,szBankName);

		//CString str;
		//str.Format(_T("BankID: %s; BankBrchID: %s; BankName: %s"),szBankID,szBankBrchID,szBankName);
		//memcpy(&g_InvInf,pInvestor,sizeof(CThostFtdcInvestorField));
		//AfxMessageBox(str);
	}
	if(bIsLast) SetEvent(g_hEvent);
}

//////////////////////////////////////////期货发起银行资金转期货请求///////////////////////////////////////
void CtpTraderSpi::ReqBk2FByF(TThostFtdcBankIDType BkID,TThostFtdcPasswordType BkPwd,
	TThostFtdcPasswordType Pwd,TThostFtdcTradeAmountType TdAmt)
{
	CThostFtdcReqTransferField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	 
	strcpy(req.AccountID, INVEST_ID); //用户代码
	strcpy(req.TradeCode,"202001");
	strcpy(req.BankBranchID,"0000");
	strcpy(req.CurrencyID,"RMB");
	strcpy(req.BankID,BkID);
	strcpy(req.BankPassWord,BkPwd);
	strcpy(req.Password,Pwd);
	req.TradeAmount=TdAmt;
	req.SecuPwdFlag = THOST_FTDC_BPWDF_BlankCheck;

	pUserApi->ReqFromBankToFutureByFuture(&req,++m_iRequestID);
}

///期货发起银行资金转期货应答
void CtpTraderSpi::OnRspFromBankToFutureByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//AfxMessageBox(_T("OnRspFromBankToFutureByFuture"));
	if( IsErrorRspInfo(pRspInfo) || (pReqTransfer==NULL))
	{

		ShowErrTips(pRspInfo->ErrorMsg);
	}
	if(bIsLast) SetEvent(g_hEvent);	 
}
///期货发起银行资金转期货通知
void CtpTraderSpi::OnRtnFromBankToFutureByFuture(CThostFtdcRspTransferField *pRspTransfer)
{
	CThostFtdcRspTransferField bfTrans;
	memcpy(&bfTrans,  pRspTransfer, sizeof(CThostFtdcRspTransferField));
	bool founded=false;    UINT i=0;
	for(i=0; i<m_BfTransVec.size(); i++)
	{
		if(m_BfTransVec[i].FutureSerial==bfTrans.FutureSerial) 
		{ founded=true;    break;}
	}
	//////覆盖
	if(founded) 
	{
		m_BfTransVec[i] = bfTrans; 
	} 
	///////新增银期反馈
	else 
	{
		m_BfTransVec.push_back(bfTrans);
		if(!g_bRecconnectT)
		{
			if(pRspTransfer->ErrorID!=0)
			{
				ShowErrTips(pRspTransfer->ErrorMsg);
			}
			else
			{
				//ShowErroTips(IDS_BFTRANS_OK,IDS_STRTIPS);
			}
		}

	}

	//SetEvent(g_hEvent);
}

///期货发起银行资金转期货错误回报
void CtpTraderSpi::OnErrRtnBankToFutureByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo)
{
	//AfxMessageBox(_T("OnErrRtnBankToFutureByFuture"));
	/*
	CThostFtdcRspTransferField* bfTrans = new CThostFtdcRspTransferField();
	ZeroMemory(bfTrans,sizeof(CThostFtdcRspTransferField));

	bfTrans->FutureSerial = pReqTransfer->FutureSerial;
	strcpy(bfTrans->TradeCode,pReqTransfer->TradeCode);
	bfTrans->TradeAmount = pReqTransfer->TradeAmount;
	strcpy(bfTrans->TradeTime,pReqTransfer->TradeTime);
	strcpy(bfTrans->ErrorMsg,pRspInfo->ErrorMsg);

	BfTransVec.push_back(bfTrans);
	*/
}


///////////////////////////////////////////期货发起期货资金转银行请求///////////////////////////////////////////
void CtpTraderSpi::ReqF2BkByF(TThostFtdcBankIDType BkID,TThostFtdcPasswordType BkPwd,
	TThostFtdcPasswordType Pwd,TThostFtdcTradeAmountType TdAmt)
{
	CThostFtdcReqTransferField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	 
	strcpy(req.AccountID, INVEST_ID); //用户代码
	strcpy(req.TradeCode,"202002");
	strcpy(req.BankBranchID,"0000");
	strcpy(req.CurrencyID,"RMB");
	strcpy(req.BankID,BkID);
	strcpy(req.BankPassWord,BkPwd);
	strcpy(req.Password,Pwd);
	req.TradeAmount=TdAmt;
	req.SecuPwdFlag = THOST_FTDC_BPWDF_BlankCheck;

	pUserApi->ReqFromFutureToBankByFuture(&req,++m_iRequestID);
}


///期货发起期货资金转银行应答
void CtpTraderSpi::OnRspFromFutureToBankByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( IsErrorRspInfo(pRspInfo) || (pReqTransfer==NULL))
	{

		ShowErrTips(pRspInfo->ErrorMsg);
	}

	if(bIsLast) SetEvent(g_hEvent);	
}


///期货发起期货资金转银行通知
void CtpTraderSpi::OnRtnFromFutureToBankByFuture(CThostFtdcRspTransferField *pRspTransfer)
{
	//AfxMessageBox(_T("OnRtnFromFutureToBankByFuture"));

	CThostFtdcRspTransferField bfTrans;
	memcpy(&bfTrans,  pRspTransfer, sizeof(CThostFtdcRspTransferField));
	bool founded=false;    UINT i=0;
	for(i=0; i<m_BfTransVec.size(); i++)
	{
		if(m_BfTransVec[i].FutureSerial==bfTrans.FutureSerial)
		{ founded=true;    break;}
	}
	//////覆盖
	if(founded) 
	{
		m_BfTransVec[i] = bfTrans; 

	} 
	///////新增银期反馈
	else 
	{
		m_BfTransVec.push_back(bfTrans);
		if(!g_bRecconnectT)
		{
			if(pRspTransfer->ErrorID!=0)
			{
				//TCHAR szMsg[MAX_PATH];
				//ansi2uni(CP_ACP,pRspTransfer->ErrorMsg,szMsg);

				//ShowErroTips(szMsg,MY_TIPS);
			}
			else
			{
				//ShowErroTips(IDS_BFTRANS_OK,IDS_STRTIPS);
			}
		}
	}

	//SetEvent(g_hEvent);
}

///期货发起期货资金转银行错误回报
void CtpTraderSpi::OnErrRtnFutureToBankByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo)
{

}

///////////////////////////////////////////////////期货发起查询银行余额请求///////////////////////////////////////////////
void CtpTraderSpi::ReqQryBkAccMoneyByF(TThostFtdcBankIDType BkID,TThostFtdcPasswordType BkPwd,
	TThostFtdcPasswordType Pwd)
{
	CThostFtdcReqQueryAccountField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	 
	strcpy(req.AccountID, INVEST_ID); //用户代码
	strcpy(req.TradeCode,"204002");
	strcpy(req.BankBranchID,"0000");
	strcpy(req.CurrencyID,"RMB");
	strcpy(req.BankID,BkID);
	strcpy(req.BankPassWord,BkPwd);
	strcpy(req.Password,Pwd);

	req.SecuPwdFlag = THOST_FTDC_BPWDF_BlankCheck;
	pUserApi->ReqQueryBankAccountMoneyByFuture(&req,++m_iRequestID);
}

///期货发起查询银行余额应答
void CtpTraderSpi::OnRspQueryBankAccountMoneyByFuture(CThostFtdcReqQueryAccountField *pReqQueryAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

	if( IsErrorRspInfo(pRspInfo) || (pReqQueryAccount==NULL))
	{
		CThostFtdcRspTransferField bfTrans;
		ZeroMemory(&bfTrans,sizeof(CThostFtdcRspTransferField));

		bfTrans.FutureSerial = pReqQueryAccount->FutureSerial;
		strcpy(bfTrans.TradeCode,pReqQueryAccount->TradeCode);
		//bfTrans->TradeAmount = pReqQueryAccount->TradeAmount;
		strcpy(bfTrans.TradeTime,pReqQueryAccount->TradeTime);
		strcpy(bfTrans.ErrorMsg,pRspInfo->ErrorMsg);

		m_BfTransVec.push_back(bfTrans);

		ShowErrTips(pRspInfo->ErrorMsg);
	}
	//if(bIsLast) SetEvent(g_hEvent);	

}

///期货发起查询银行余额通知
void CtpTraderSpi::OnRtnQueryBankBalanceByFuture(CThostFtdcNotifyQueryAccountField *pNotifyQueryAccount)
{
	if(pNotifyQueryAccount->ErrorID ==0)
	{
		if (!g_bRecconnectT)
		{

			CThostFtdcNotifyQueryAccountField Notify;
			memcpy(&Notify,pNotifyQueryAccount,sizeof(CThostFtdcNotifyQueryAccountField));

			//::PostMessage(g_pCWnd->m_hWnd,WM_QRYBKYE_MSG,0,(LPARAM)pNotify);
			/*
			HWND hwnd = g_pCWnd->m_hWnd;

			////////////////////////////////////
			{
			COPYDATASTRUCT cpd;
			cpd.dwData = 0x10000;		//标识
			cpd.cbData = sizeof(CThostFtdcNotifyQueryAccountField);
			cpd.lpData = (PVOID)pNotify;
			::SendMessage( hwnd, WM_COPYDATA, NULL, (LPARAM)&cpd );
			}
			*/

		}

	}

	SetEvent(g_hEvent);	
}

///期货发起查询银行余额错误回报
void CtpTraderSpi::OnErrRtnQueryBankBalanceByFuture(CThostFtdcReqQueryAccountField *pReqQueryAccount, CThostFtdcRspInfoField *pRspInfo)
{

}

///////////////////////////////////////查询转账流水////////////////////////////////////////////
/// "204005"
void CtpTraderSpi::ReqQryTfSerial(TThostFtdcBankIDType BkID)
{
	CThostFtdcQryTransferSerialField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	 
	strcpy(req.AccountID, INVEST_ID); //用户代码

	strcpy(req.BankID,BkID);

	pUserApi->ReqQryTransferSerial(&req,++m_iRequestID);
}
///请求查询转帐流水响应
void CtpTraderSpi::OnRspQryTransferSerial(CThostFtdcTransferSerialField *pTransferSerial, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( IsErrorRspInfo(pRspInfo) || (pTransferSerial==NULL))
	{
		ShowErrTips(pRspInfo->ErrorMsg);	
	}
	if(bIsLast) SetEvent(g_hEvent);	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
///系统运行时期货端手工发起冲正银行转期货请求，银行处理完毕后报盘发回的通知
void CtpTraderSpi::OnRtnRepealFromBankToFutureByFutureManual(CThostFtdcRspRepealField *pRspRepeal)
{
	//AfxMessageBox(_T("0"));
}


///系统运行时期货端手工发起冲正期货转银行请求，银行处理完毕后报盘发回的通知
void CtpTraderSpi::OnRtnRepealFromFutureToBankByFutureManual(CThostFtdcRspRepealField *pRspRepeal)
{
	//AfxMessageBox(_T("1"));
}


///系统运行时期货端手工发起冲正银行转期货错误回报
void CtpTraderSpi::OnErrRtnRepealBankToFutureByFutureManual(CThostFtdcReqRepealField *pReqRepeal, CThostFtdcRspInfoField *pRspInfo)
{
	//AfxMessageBox(_T("2"));
}

///系统运行时期货端手工发起冲正期货转银行错误回报
void CtpTraderSpi::OnErrRtnRepealFutureToBankByFutureManual(CThostFtdcReqRepealField *pReqRepeal, CThostFtdcRspInfoField *pRspInfo)
{
	//AfxMessageBox(_T("3"));
}


///期货发起冲正银行转期货请求，银行处理完毕后报盘发回的通知
void CtpTraderSpi::OnRtnRepealFromBankToFutureByFuture(CThostFtdcRspRepealField *pRspRepeal)
{
	//AfxMessageBox(_T("4"));
}


///期货发起冲正期货转银行请求，银行处理完毕后报盘发回的通知
void CtpTraderSpi::OnRtnRepealFromFutureToBankByFuture(CThostFtdcRspRepealField *pRspRepeal)
{
	//AfxMessageBox(_T("5"));
}



void CtpTraderSpi::ReqQryCFMMCTdAccKey()
{
	CThostFtdcQryCFMMCTradingAccountKeyField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	 
	strcpy(req.InvestorID, INVEST_ID); //用户代码

	pUserApi->ReqQryCFMMCTradingAccountKey(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQryCFMMCTradingAccountKey(CThostFtdcCFMMCTradingAccountKeyField *pCFMMCTradingAccountKey, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pCFMMCTradingAccountKey)
	{
		//memcpy(&g_Cfmmc,pCFMMCTradingAccountKey,sizeof(CThostFtdcCFMMCTradingAccountKeyField));
		char strMsg[1000];
		sprintf(strMsg,CFMMC_TMPL,pCFMMCTradingAccountKey->ParticipantID,pCFMMCTradingAccountKey->AccountID,
			pCFMMCTradingAccountKey->KeyID,pCFMMCTradingAccountKey->CurrentKey);
		ShellExecuteA(NULL,"open",strMsg,NULL, NULL, SW_SHOW);

	}
	//if(bIsLast) SetEvent(g_hEvent);	
}

void CtpTraderSpi::ReqQryBkrTdParams()
{
	CThostFtdcQryBrokerTradingParamsField  req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID,BROKER_ID);
	strcpy(req.InvestorID,INVEST_ID);

	pUserApi->ReqQryBrokerTradingParams(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspQryBrokerTradingParams(CThostFtdcBrokerTradingParamsField *pBrokerTradingParams, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pBrokerTradingParams)
	{

	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQryBkrTdAlgos(TThostFtdcExchangeIDType ExhID,TThostFtdcInstrumentIDType instID)
{
	CThostFtdcQryBrokerTradingAlgosField  req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID,BROKER_ID);
	if(ExhID != NULL)
		strcpy(req.ExchangeID, ExhID);
	if(instID != NULL)
		strcpy(req.InstrumentID, instID);

	pUserApi->ReqQryBrokerTradingAlgos(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspQryBrokerTradingAlgos(CThostFtdcBrokerTradingAlgosField *pBrokerTradingAlgos, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pBrokerTradingAlgos)
	{

	}
	if(bIsLast) SetEvent(g_hEvent);
}

///预埋单录入请求
void CtpTraderSpi::ReqParkedOrderInsert(CThostFtdcParkedOrderField *ParkedOrder)
{
	pUserApi->ReqParkedOrderInsert(ParkedOrder,++m_iRequestID);
}
///预埋单录入请求响应
void CtpTraderSpi::OnRspParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pParkedOrder)
	{

	}
	if(bIsLast) SetEvent(g_hEvent);
}

///预埋撤单录入请求
void CtpTraderSpi::ReqParkedOrderAction(CThostFtdcParkedOrderActionField *ParkedOrderAction)
{
	pUserApi->ReqParkedOrderAction(ParkedOrderAction,++m_iRequestID);
}
///预埋撤单录入请求响应
void CtpTraderSpi::OnRspParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pParkedOrderAction)
	{

	}
	if(bIsLast) SetEvent(g_hEvent);
}

///请求查询预埋单
void CtpTraderSpi::ReqQryParkedOrder(TThostFtdcInstrumentIDType InstrumentID,TThostFtdcExchangeIDType ExchangeID)
{
	CThostFtdcQryParkedOrderField  req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID,BROKER_ID);
	strcpy(req.InvestorID,INVEST_ID);
	if(InstrumentID != NULL)
		strcpy(req.InstrumentID,InstrumentID);
	if(ExchangeID != NULL)
		strcpy(req.ExchangeID,ExchangeID);
	pUserApi->ReqQryParkedOrder(&req, ++m_iRequestID);
}

///请求查询预埋单响应
void CtpTraderSpi::OnRspQryParkedOrder(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pParkedOrder)
	{

	}
	if(bIsLast) SetEvent(g_hEvent);
}

///请求撤销预埋
void CtpTraderSpi::ReqQryParkedOrderAction(TThostFtdcInstrumentIDType InstrumentID,TThostFtdcExchangeIDType ExchangeID)
{
	CThostFtdcQryParkedOrderActionField  req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID,BROKER_ID);
	strcpy(req.InvestorID,INVEST_ID);
	if(InstrumentID != NULL)
		strcpy(req.InstrumentID,InstrumentID);
	if(ExchangeID != NULL)
		strcpy(req.ExchangeID,ExchangeID);
	pUserApi->ReqQryParkedOrderAction(&req, ++m_iRequestID);
}

///删除预埋撤单响应
void CtpTraderSpi::OnRspRemoveParkedOrderAction(CThostFtdcRemoveParkedOrderActionField *pRemoveParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if( !IsErrorRspInfo(pRspInfo) && pRemoveParkedOrderAction)
	{

	}
	if(bIsLast) SetEvent(g_hEvent);
}

///请求删除预埋单
void CtpTraderSpi::ReqRemoveParkedOrder(TThostFtdcParkedOrderIDType ParkedOrder_ID)
{
	CThostFtdcRemoveParkedOrderField  req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID,BROKER_ID);
	strcpy(req.InvestorID,INVEST_ID);
	strcpy(req.ParkedOrderID,ParkedOrder_ID);
	pUserApi->ReqRemoveParkedOrder(&req, ++m_iRequestID);
}

///删除预埋单响应
void CtpTraderSpi::OnRspRemoveParkedOrder(CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pRemoveParkedOrder)
	{

	}
	if(bIsLast) SetEvent(g_hEvent);
}

///请求删除预埋撤单
void CtpTraderSpi::ReqRemoveParkedOrderAction(TThostFtdcParkedOrderActionIDType ParkedOrderAction_ID)
{
	CThostFtdcRemoveParkedOrderActionField  req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID,BROKER_ID);
	strcpy(req.InvestorID,INVEST_ID);
	strcpy(req.ParkedOrderActionID,ParkedOrderAction_ID);
	pUserApi->ReqRemoveParkedOrderAction(&req, ++m_iRequestID);
}

///请求删除预埋撤单响应
void CtpTraderSpi::OnRspQryParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if( !IsErrorRspInfo(pRspInfo) && pParkedOrderAction)
	{

	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus)
{
	if(pInstrumentStatus && true)
	{
		//CXTraderDlg* pDlg = (CXTraderDlg*)g_pCWnd;
		CString szStat,szMsg,szExh;
		//TCHAR szTm[30];
		//ansi2uni(CP_ACP,pInstrumentStatus->EnterTime,szTm);
		JgTdStatus(szStat,pInstrumentStatus->InstrumentStatus);
		szExh = JgExchage(pInstrumentStatus->ExchangeID);

		//szMsg.Format(_T("%s %s:%s"),szTm,(LPCTSTR)szExh,(LPCTSTR)szStat);
		//pDlg->SetStatusTxt(szMsg,2);	
	}
}

void CtpTraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	TRACE(_T("OnRspErrorT\n"));
	IsErrorRspInfo(pRspInfo);
}

bool CtpTraderSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	// 如果ErrorID != 0, 说明收到了错误的响应
	bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	return ret;
}

void CtpTraderSpi::ShowErrTips(TThostFtdcErrorMsgType ErrorMsg)
{
	ShowErroTips(ErrorMsg,MY_TIPS);
}

void CtpTraderSpi::ClearAllVectors(){
	m_orderVec.clear();
	m_tradeVec.clear();
	m_InsinfVec.clear();
	m_MargRateVec.clear();
	m_StmiVec.clear();
	m_AccRegVec.clear();
	m_TdCodeVec.clear();
	m_InvPosVec.clear();
	m_BfTransVec.clear();
}
