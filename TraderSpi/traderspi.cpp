#include "StdAfx.h"
#include "traderspi.h"
#include "HiStar.h"
#include "UserMsg.h"
#include "Maindlg.h"
#include <stdlib.h>
#include <algorithm>
#include "HedgePostProcessing.h"
#pragma warning(disable :4996)
BOOL g_bRecconnectT = FALSE;
BOOL g_bLoginCtpT = FALSE;
extern int longIf,shortIf;
extern bool isReal;
bool g_bOnceT = FALSE;//交易系统是否曾经登陆过，如果登陆过则是TRUE,否则FALSE
struct sqldb{
	std::string host;
	std::string user;
	std::string passwd;
	std::string db;
};
extern sqldb m_db;
bool CmpByTime(const CThostFtdcInvestorPositionDetailField first,const CThostFtdcInvestorPositionDetailField second) 
{    
	if(strcmp(first.OpenDate,second.OpenDate) < 0){
		return true;
	}
	else if(strcmp(first.OpenDate,second.OpenDate) > 0){
		return false;
	}
	else{
		if(strcmp(first.TradeID,second.TradeID) < 0){
			return true;
		}
		else{
			return false;
		}
	}
}
//网络故障恢复正常后 自动重连
void CtpTraderSpi::OnFrontConnected()
{
	PostThreadMessage(MainThreadId,WM_LOGIN_TD,NULL,NULL);
}

int CtpTraderSpi::ReqUserLogin(TThostFtdcBrokerIDType	vAppId,TThostFtdcUserIDType	vUserId,TThostFtdcPasswordType	vPasswd)
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, vAppId); strcpy(BROKER_ID, vAppId); 
	strcpy(req.UserID, vUserId);  strcpy(INVEST_ID, vUserId); 
	strcpy(req.Password, vPasswd);
	strcpy(req.UserProductInfo,PROD_INFO);
	int iRet = pUserApi->ReqUserLogin(&req, ++m_iRequestID);
	return m_iRequestID;
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
		//建立与数据库的连接
		SYSTEMTIME sys;
		GetLocalTime(&sys);
		char name[100];
		if(isReal){
			sprintf(name,"OrderRtn_%04d%02d%02d",sys.wYear,sys.wMonth,sys.wDay);
			statusTableName = name;
			sprintf(name,"TradeRtn_%04d%02d%02d",sys.wYear,sys.wMonth,sys.wDay);
			tradeTableName = name;
			connctp = mysql_init(NULL); 
			if(connctp == NULL) {
				TRACE("Error %u: %s\n", mysql_errno(connctp), mysql_error(connctp));      
				//exit(1);  
			}  
		}
		if(connctp){
			if(mysql_real_connect(connctp,m_db.host.c_str(),m_db.user.c_str(),m_db.passwd.c_str(),m_db.db.c_str(),0,NULL,0) == NULL)
			{      
				TRACE("Error %u: %s\n", mysql_errno(connctp), mysql_error(connctp));
			}  
		}
		if(connctp){
			if(mysql_query(connctp,"CREATE TABLE IF NOT EXISTS " + statusTableName 
				+ " (datetime DATETIME,millisecond INT,BrokerID VARCHAR(20),InvestorID VARCHAR(20),\
				  InstrumentID VARCHAR(20),OrderRef VARCHAR(20),UserID VARCHAR(20),OrderPriceType VARCHAR(20),Direction VARCHAR(20),CombOffsetFlag VARCHAR(20),\
				  CombHedgeFlag VARCHAR(20),LimitPrice VARCHAR(20),VolumeTotalOriginal VARCHAR(20),TimeCondition VARCHAR(20),\
				  GTDDate VARCHAR(20),VolumeCondition VARCHAR(20),MinVolume VARCHAR(20),ContingentCondition VARCHAR(20),StopPrice VARCHAR(20),ForceCloseReason VARCHAR(20),\
				  IsAutoSuspend VARCHAR(20),BusinessUnit VARCHAR(20),RequestID VARCHAR(20),OrderLocalID VARCHAR(20),ExchangeID VARCHAR(20),ParticipantID VARCHAR(20),ClientID VARCHAR(20),\
				  ExchangeInstID VARCHAR(20),TraderID VARCHAR(20),InstallID VARCHAR(20),OrderSubmitStatus VARCHAR(20),NotifySequence VARCHAR(20),TradingDay VARCHAR(20),\
				  SettlementID VARCHAR(20),OrderSysID VARCHAR(20),OrderSource VARCHAR(20),OrderStatus VARCHAR(20),OrderType VARCHAR(20),VolumeTraded VARCHAR(20),VolumeTotal VARCHAR(20),InsertDate VARCHAR(20),\
				  InsertTime VARCHAR(20),ActiveTime VARCHAR(20),SuspendTime VARCHAR(20),UpdateTime VARCHAR(20),CancelTime VARCHAR(20),ActiveTraderID VARCHAR(20),ClearingPartID VARCHAR(20),\
				  SequenceNo VARCHAR(20),FrontID VARCHAR(20),SessionID VARCHAR(20),UserProductInfo VARCHAR(20),StatusMsg VARCHAR(100),UserForceClose VARCHAR(20),ActiveUserID VARCHAR(20),\
				  BrokerOrderSeq VARCHAR(20),RelativeOrderSysID VARCHAR(20),ZCETotalTradedVolume VARCHAR(20),IsSwapOrder VARCHAR(20))")) 
			{      
				TRACE("Error %u: %s\n", mysql_errno(connctp), mysql_error(connctp));      
			}
			if(mysql_query(connctp,"CREATE TABLE IF NOT EXISTS " + tradeTableName
				+ " (datetime DATETIME,millisecond INT,BrokerID VARCHAR(20),InvestorID VARCHAR(20),InstrumentID VARCHAR(20),OrderRef VARCHAR(20),UserID VARCHAR(20),ExchangeID VARCHAR(20),\
				  TradeID VARCHAR(20),Direction VARCHAR(20),OrderSysID VARCHAR(20),ParticipantID VARCHAR(20),ClientID VARCHAR(20),TradingRole VARCHAR(20),ExchangeInstID VARCHAR(20),\
				  OffsetFlag VARCHAR(20),HedgeFlag VARCHAR(20),Price VARCHAR(20),Volume VARCHAR(20),TradeDate VARCHAR(20),TradeTime VARCHAR(20),TradeType VARCHAR(20),PriceSource VARCHAR(20),\
				  TraderID VARCHAR(20),OrderLocalID VARCHAR(20),ClearingPartID VARCHAR(20),BusinessUnit VARCHAR(20),SequenceNo VARCHAR(20),TradingDay VARCHAR(20),SettlementID VARCHAR(20),\
				  BrokerOrderSeq VARCHAR(20),TradeSource VARCHAR(20))"))
			{    
				TRACE("Error %u: %s\n", mysql_errno(connctp), mysql_error(connctp));      
			}
		}
		if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
}

const char* CtpTraderSpi::GetTradingDay()
{
	return m_sTdday;
}

int CtpTraderSpi::ReqUserLogout()
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
	return m_iRequestID;
}

///登出请求响应
void CtpTraderSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pUserLogout){
		TRACE(_T("已经登出ctpT\n"));
		g_bLoginCtpT = FALSE;
	}
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
}

int CtpTraderSpi::ReqSettlementInfoConfirm()
{
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	pUserApi->ReqSettlementInfoConfirm(&req, ++m_iRequestID);
	return m_iRequestID;
}

void CtpTraderSpi::OnRspSettlementInfoConfirm(
	CThostFtdcSettlementInfoConfirmField  *pSettlementInfoConfirm, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{	
	if( !IsErrorRspInfo(pRspInfo) && pSettlementInfoConfirm){
	}
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
}

int CtpTraderSpi::ReqQryNotice()
{
	CThostFtdcQryNoticeField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	pUserApi->ReqQryNotice(&req, ++m_iRequestID);
	return m_iRequestID;
}

void CtpTraderSpi::OnRspQryNotice(CThostFtdcNoticeField *pNotice, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) &&  pNotice){}
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
}

int CtpTraderSpi::ReqQryTdNotice()
{
	CThostFtdcQryTradingNoticeField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	pUserApi->ReqQryTradingNotice(&req, ++m_iRequestID);
	return m_iRequestID;
}

void CtpTraderSpi::OnRspQryTradingNotice(CThostFtdcTradingNoticeField *pTradingNotice, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) &&  pTradingNotice){}
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
}

int CtpTraderSpi::ReqQrySettlementInfoConfirm()
{
	CThostFtdcQrySettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	pUserApi->ReqQrySettlementInfoConfirm(&req,++m_iRequestID);
	return m_iRequestID;
}

void CtpTraderSpi::OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) &&  pSettlementInfoConfirm){}	
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
}

int CtpTraderSpi::ReqQrySettlementInfo(TThostFtdcDateType TradingDay)
{
	CThostFtdcQrySettlementInfoField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	strcpy(req.TradingDay,TradingDay);

	pUserApi->ReqQrySettlementInfo(&req,++m_iRequestID);
	return m_iRequestID;
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
		PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
		//SendNotifyMessage(HWND_BROADCAST,WM_QRYSMI_MSG,0,0);		
	}
}

int CtpTraderSpi::ReqQryInst(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	if (instId != NULL)
	{ strcpy(req.InstrumentID, instId); }

	pUserApi->ReqQryInstrument(&req, ++m_iRequestID);
	return m_iRequestID;
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
		AcquireSRWLockExclusive(&g_srwLock_Insinf);
		bool founded = false;
		for(unsigned int i = 0;i < m_InsinfVec.size();i++){
			if(!strcmp(m_InsinfVec[i].iinf.InstrumentID,InsInf.iinf.InstrumentID)){
				founded = true;
				break;
			}
		}
		if(!founded){
			m_InsinfVec.push_back(InsInf);
		}
		ReleaseSRWLockExclusive(&g_srwLock_Insinf);
	}
	if(bIsLast){
		TRACE(_T("合约查询完毕\n"));
		PostThreadMessage(MainThreadId,WM_UPDATE_LSTCTRL,NULL,NULL);
		PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
	}
}

int CtpTraderSpi::ReqQryInstrumentMarginRate(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInstrumentMarginRateField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	req.HedgeFlag = THOST_FTDC_HF_Speculation;
	if (instId != NULL)
	{ strcpy(req.InstrumentID, instId); }
	pUserApi->ReqQryInstrumentMarginRate(&req, ++m_iRequestID);
	return m_iRequestID;
}

void CtpTraderSpi::OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pInstrumentMarginRate )
	{
		TRACE("%.02lf,%.02lf,%.02lf,%.02lf\n",pInstrumentMarginRate->LongMarginRatioByMoney,pInstrumentMarginRate->ShortMarginRatioByMoney,pInstrumentMarginRate->LongMarginRatioByVolume,pInstrumentMarginRate->ShortMarginRatioByVolume);
		CThostFtdcInstrumentMarginRateField MarginRate;
		memcpy(&MarginRate,pInstrumentMarginRate,sizeof(CThostFtdcInstrumentMarginRateField));

		AcquireSRWLockExclusive(&g_srwLock_MargRate);
		bool found = false;
		for(unsigned int j = 0;j < m_MargRateVec.size();j++){
			if(strcmp(m_MargRateVec[j].InstrumentID,MarginRate.InstrumentID) == 0
				&& strcmp(m_MargRateVec[j].BrokerID,MarginRate.BrokerID) == 0
				&& strcmp(m_MargRateVec[j].InvestorID,MarginRate.InvestorID) == 0){
					found = true;break;
			}
		}
		if(!found){
			m_MargRateVec.push_back(MarginRate);
		}	
		ReleaseSRWLockExclusive(&g_srwLock_MargRate);

	}
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
}

int CtpTraderSpi::ReqQryTdAcc()
{
	CThostFtdcQryTradingAccountField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	pUserApi->ReqQryTradingAccount(&req, ++m_iRequestID);
	return m_iRequestID;
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
			PostMessage(AfxGetApp()->m_pMainWnd->GetSafeHwnd(),WM_UPDATE_ACC_CTP,NULL,(LPARAM)pAcc);
		}
		PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
	}	
}

//INSTRUMENT_ID设成部分字段,例如IF10,就能查出所有IF10打头的头寸
int CtpTraderSpi::ReqQryInvPos(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInvestorPositionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	if (instId!=NULL)
	{strcpy(req.InstrumentID, instId);}		
	pUserApi->ReqQryInvestorPosition(&req, ++m_iRequestID);
	return m_iRequestID;
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
		for(unsigned int i = 0;i < m_InvPosVec.size();i++){
			//多头和空头是不同的持仓，所以除了判断合约代码还要判断持仓方向。
			if(!strcmp(m_InvPosVec[i].InstrumentID,InvPos.InstrumentID) && m_InvPosVec[i].PosiDirection == InvPos.PosiDirection){
				founded = true;
				break;
			}
		}
		if(!founded){
			m_InvPosVec.push_back(InvPos);
		}
		//初始化IF持仓
		CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
		char szInst[MAX_PATH];
		uni2ansi(CP_ACP,pApp->m_accountCtp.m_szInst,szInst);
		if(!strcmp(szInst,pInvestorPosition->InstrumentID)){
			if(pInvestorPosition->PosiDirection == THOST_FTDC_PD_Long){
				TRACE("IF长仓%d\r\n",pInvestorPosition->Position);
				longIf = pInvestorPosition->Position;
			}
			else if(pInvestorPosition->PosiDirection == THOST_FTDC_PD_Short){
				shortIf = pInvestorPosition->Position;
				TRACE("IF短仓%d\r\n",pInvestorPosition->Position);
			}
		}
	}
	if(bIsLast){ 
		PostThreadMessage(MainThreadId,WM_UPDATE_LSTCTRL,NULL,NULL);
		PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
	}
}

int CtpTraderSpi::ReqQryOrder(TThostFtdcInstrumentIDType instId){
	CThostFtdcQryOrderField req;
	memset(&req,0,sizeof(req));
	strcpy(req.BrokerID,BROKER_ID);
	strcpy(req.InvestorID,INVEST_ID);
	if (instId!=NULL)
	{strcpy(req.InstrumentID, instId);}	
	pUserApi->ReqQryOrder(&req, ++m_iRequestID);
	return m_iRequestID;
}

void  CtpTraderSpi::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	if(!IsErrorRspInfo(pRspInfo)&&pOrder){
		SYSTEMTIME sys;
		GetLocalTime(&sys);
		char data[10000],datetime[100];
		memset(data,0,sizeof(data));memset(datetime,0,sizeof(datetime));
		if(connctp){
			sprintf(datetime,"'%d-%d-%d %d:%d:%d',%d,",sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds);
			sprintf(data,"'%s','%s','%s','%s','%s','%c','%c',\
						 '%s','%s',%.02lf,%d,'%c','%s',\
						 '%c',%d,'%c',%.02lf,'%c',%d,\
						 '%s',%d,'%s','%s','%s','%s','%s',\
						 '%s',%d,'%c',%d,'%s',%d,'%s',\
						 '%c','%c','%c',%d,%d,'%s','%s','%s',\
						 '%s','%s','%s','%s','%s',%d,%d,%d,\
						 '%s','%s',%d,'%s',%d,'%s',\
						 %d,%d",
						 pOrder->BrokerID,pOrder->InvestorID,pOrder->InstrumentID,pOrder->OrderRef,pOrder->UserID,pOrder->OrderPriceType,pOrder->Direction,
						 pOrder->CombOffsetFlag,pOrder->CombHedgeFlag,pOrder->LimitPrice,pOrder->VolumeTotalOriginal,pOrder->TimeCondition,pOrder->GTDDate,
						 pOrder->VolumeCondition,pOrder->MinVolume,pOrder->ContingentCondition,pOrder->StopPrice,pOrder->ForceCloseReason,pOrder->IsAutoSuspend,
						 pOrder->BusinessUnit,pOrder->RequestID,pOrder->OrderLocalID,pOrder->ExchangeID,pOrder->ParticipantID,pOrder->ClientID,pOrder->ExchangeInstID,
						 pOrder->TraderID,pOrder->InstallID,pOrder->OrderSubmitStatus,pOrder->NotifySequence,pOrder->TradingDay,pOrder->SettlementID,pOrder->OrderSysID,
						 pOrder->OrderSource,pOrder->OrderStatus,pOrder->OrderType,pOrder->VolumeTraded,pOrder->VolumeTotal,pOrder->InsertDate,pOrder->InsertTime,pOrder->ActiveTime,
						 pOrder->SuspendTime,pOrder->UpdateTime,pOrder->CancelTime,pOrder->ActiveTraderID,pOrder->ClearingPartID,pOrder->SequenceNo,pOrder->FrontID,pOrder->SessionID,
						 pOrder->UserProductInfo,pOrder->StatusMsg,pOrder->UserForceClose,pOrder->ActiveUserID,pOrder->BrokerOrderSeq,pOrder->RelativeOrderSysID,
						 pOrder->ZCETotalTradedVolume,(int)(pOrder->IsSwapOrder));
			CString insertdata = "INSERT INTO " + statusTableName 
				+ " (datetime,millisecond,BrokerID,InvestorID,InstrumentID,OrderRef,UserID,OrderPriceType,Direction,CombOffsetFlag,CombHedgeFlag,LimitPrice,VolumeTotalOriginal,TimeCondition,\
				  GTDDate,VolumeCondition,MinVolume,ContingentCondition,StopPrice,ForceCloseReason,IsAutoSuspend,BusinessUnit,RequestID,OrderLocalID,ExchangeID,ParticipantID,ClientID,\
				  ExchangeInstID,TraderID,InstallID,OrderSubmitStatus,NotifySequence,TradingDay,SettlementID,OrderSysID,OrderSource,OrderStatus,OrderType,VolumeTraded,VolumeTotal,InsertDate,\
				  InsertTime,ActiveTime,SuspendTime,UpdateTime,CancelTime,ActiveTraderID,ClearingPartID,SequenceNo,FrontID,SessionID,UserProductInfo,StatusMsg,UserForceClose,ActiveUserID,\
				  BrokerOrderSeq,RelativeOrderSysID,ZCETotalTradedVolume,IsSwapOrder) VALUES (" + CString(datetime) + CString(data) +")";
			TRACE(insertdata);
			if(mysql_query(connctp,insertdata.GetBuffer())){
				TRACE("Error %u: %s\n", mysql_errno(connctp), mysql_error(connctp));
			}
		}
		CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
		CThostFtdcOrderField order;
		TRACE("OnRtnOrder所有报单通知%s,%s,%c,%d,已经成交%d\r\n",pOrder->OrderRef, pOrder->OrderSysID,pOrder->OrderStatus,pOrder->BrokerOrderSeq,pOrder->VolumeTraded);
		memcpy(&order,pOrder, sizeof(CThostFtdcOrderField));
		bool founded = false;UINT i = 0;
		for(i = 0;i<m_orderVec.size();i++){
			if(m_orderVec[i].BrokerOrderSeq == order.BrokerOrderSeq && strcmp(m_orderVec[i].BrokerID,order.BrokerID) == 0) { 
				founded = true;
				//修改命令状态
				m_orderVec[i] = order;
				break;
			}
		}		
		if(!founded){
			//将挂单删除，因为这时有消息返回说明已经递送出去了
			int nRet = FindOrdInOnRoadVec(order.OrderRef);
			//未加入挂单列表
			if (nRet==-1){
				//如果没有,则不做任何动作
			}
			else{
				//挂单返回，表示已经递送出去，将该挂单删除(已经变成委托单或其他)
				m_onRoadVec.erase(nRet);
			}
			///////新增加委托单
			m_orderVec.insert(m_orderVec.begin(),order);//从头部插入
		}
	}
	if(bIsLast) {
		PostThreadMessage(MainThreadId,WM_UPDATE_LSTCTRL,NULL,NULL);
		PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
	}
}

int CtpTraderSpi::ReqQryTrade(TThostFtdcInstrumentIDType instId){
	CThostFtdcQryTradeField req;
	memset(&req,0,sizeof(req));
	strcpy(req.BrokerID,BROKER_ID);
	strcpy(req.InvestorID,INVEST_ID);
	if (instId!=NULL)
	{strcpy(req.InstrumentID, instId);}		
	pUserApi->ReqQryTrade(&req, ++m_iRequestID);
	return m_iRequestID;
}

void CtpTraderSpi::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
	if(!IsErrorRspInfo(pRspInfo)&&pTrade){
		TRACE("成交通知,%c;\n",pTrade->TradeSource);//THOST_FTDC_TSRC_NORMAL THOST_FTDC_TSRC_QUERY
		SYSTEMTIME sys;
		GetLocalTime(&sys);
		char data[10000],datetime[100];
		if(connctp){
			sprintf(datetime,"'%d-%d-%d %d:%d:%d',%d,",sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds);
			sprintf(data,"'%s','%s','%s','%s','%s','%s','%s',\
						 '%c','%s','%s','%s',%d,'%s',\
						 '%c','%c',%.02lf,'%d','%s','%s',%d,\
						 %d,'%s','%s','%s','%s',%d,\
						 '%s',%d,%d,'%c'",
						 pTrade->BrokerID,pTrade->InvestorID,pTrade->InstrumentID,pTrade->OrderRef,pTrade->UserID,pTrade->ExchangeID,pTrade->TradeID,
						 pTrade->Direction,pTrade->OrderSysID,pTrade->ParticipantID,pTrade->ClientID,pTrade->TradingRole,pTrade->ExchangeInstID,
						 pTrade->OffsetFlag,pTrade->HedgeFlag,pTrade->Price,pTrade->Volume,pTrade->TradeDate,pTrade->TradeTime,pTrade->TradeType,
						 pTrade->PriceSource,pTrade->TraderID,pTrade->OrderLocalID,pTrade->ClearingPartID,pTrade->BusinessUnit,pTrade->SequenceNo,
						 pTrade->TradingDay,pTrade->SettlementID,pTrade->BrokerOrderSeq,pTrade->TradeSource);
			CString insertdata = "INSERT INTO " + tradeTableName 
				+ " (datetime,millisecond,BrokerID,InvestorID,InstrumentID,OrderRef,UserID,ExchangeID,TradeID,Direction,OrderSysID,ParticipantID,ClientID,TradingRole,\
				  ExchangeInstID,OffsetFlag,HedgeFlag,Price,Volume,TradeDate,TradeTime,TradeType,PriceSource,TraderID,OrderLocalID,ClearingPartID,BusinessUnit,\
				  SequenceNo,TradingDay,SettlementID,BrokerOrderSeq,TradeSource) VALUES (" + CString(datetime) + CString(data) +")";
			if(mysql_query(connctp,insertdata.GetBuffer())){
				TRACE("Error %u: %s\n", mysql_errno(connctp), mysql_error(connctp));
			}
		}	
		CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
		CThostFtdcTradeField trade;
		memcpy(&trade,pTrade,sizeof(CThostFtdcTradeField));
		bool founded = false;     
		unsigned int ii = 0;
		for(ii = 0;ii<m_tradeVec.size();ii++){
			//strcmp返回0时表示相等
			if(strcmp(m_tradeVec[ii].TradeID,trade.TradeID) == 0 
				&& strcmp(m_tradeVec[ii].OrderSysID,trade.OrderSysID) == 0
				&& strcmp(m_tradeVec[ii].ExchangeID,trade.ExchangeID) == 0){
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
		//同步市场数据包括保证金计算、市场深度获取获取等等。
		PostThreadMessage(MainThreadId,WM_SYNCHRONIZE_MARKET,NULL,NULL);
		PostThreadMessage(MainThreadId,WM_UPDATE_LSTCTRL,NULL,NULL);
		PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
	}
}

//INSTRUMENT_ID设成部分字段,例如IF10,就能查出所有IF10打头的头寸
int CtpTraderSpi::ReqQryInvPosEx(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInvestorPositionDetailField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	if (instId!=NULL)
	{strcpy(req.InstrumentID, instId);}		
	pUserApi->ReqQryInvestorPositionDetail(&req, ++m_iRequestID);
	return m_iRequestID;
}

void CtpTraderSpi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	if(!IsErrorRspInfo(pRspInfo) && pInvestorPositionDetail)
	{
		CThostFtdcInvestorPositionDetailField InvPosDetail;
		memcpy(&InvPosDetail,pInvestorPositionDetail, sizeof(CThostFtdcInvestorPositionDetailField));
		bool founded = false;
		unsigned int i = 0;

		AcquireSRWLockExclusive(&g_srwLock_PosDetail);
		for(i = 0;i < m_InvPosDetailVec.size();i++){
			if(strcmp(m_InvPosDetailVec[i].InstrumentID,InvPosDetail.InstrumentID) == 0 
				&& strcmp(m_InvPosDetailVec[i].OpenDate,InvPosDetail.OpenDate) == 0//持仓明细区分日期的
				&& strcmp(m_InvPosDetailVec[i].TradeID,InvPosDetail.TradeID) == 0
				&& strcmp(m_InvPosDetailVec[i].ExchangeID,InvPosDetail.ExchangeID) == 0){
					founded = true;
					break;
			}
		}
		if(!founded){
			if(InvPosDetail.Volume != 0){
				m_InvPosDetailVec.push_back(InvPosDetail);
			}
		}
		else{
			//已经找到,就更新下
			if(InvPosDetail.Volume != 0){
				m_InvPosDetailVec[i] = InvPosDetail;
			}
			else{
				m_InvPosDetailVec.erase(m_InvPosDetailVec.begin() + i);
			}
		}
		ReleaseSRWLockExclusive(&g_srwLock_PosDetail); 

	}
	if(bIsLast){ 

		AcquireSRWLockExclusive(&g_srwLock_PosDetail);
		sort(m_InvPosDetailVec.begin(),m_InvPosDetailVec.end(),CmpByTime);
		ReleaseSRWLockExclusive(&g_srwLock_PosDetail); 

		PostThreadMessage(MainThreadId,WM_UPDATE_LSTCTRL,NULL,NULL);
		PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
	}
}

int CtpTraderSpi::ReqQryInvPosCombEx(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInvestorPositionCombineDetailField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	if (instId!=NULL)
	{strcpy(req.CombInstrumentID, instId);}		
	pUserApi->ReqQryInvestorPositionCombineDetail(&req, ++m_iRequestID);
	return m_iRequestID;
}

void CtpTraderSpi::OnRspQryInvestorPositionCombineDetail(CThostFtdcInvestorPositionCombineDetailField *pInvestorPositionDetail, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) &&  pInvestorPositionDetail)
	{

	}
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
}

int CtpTraderSpi::ReqOrdLimit(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir,
	TThostFtdcCombOffsetFlagType kpp,TThostFtdcPriceType price,TThostFtdcVolumeType vol)
{
	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));	
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID); 
	strcpy(req.InstrumentID, instId); 	
	strcpy(req.OrderRef, m_sOrdRef);
	int orderref = atoi(m_sOrdRef);
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
	m_onRoadVec.push_back(req);
	PostThreadMessage(MainThreadId,WM_UPDATE_LSTCTRL,NULL,NULL);
	return orderref;
}

///市价单
int CtpTraderSpi::ReqOrdAny(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp,TThostFtdcVolumeType vol)
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
	m_onRoadVec.push_back(req);
	PostThreadMessage(MainThreadId,WM_UPDATE_LSTCTRL,NULL,NULL);
	return m_iRequestID;
}

int CtpTraderSpi::ReqOrdCondition(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp,
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
	m_onRoadVec.push_back(req);
	PostThreadMessage(MainThreadId,WM_UPDATE_LSTCTRL,NULL,NULL);
	return m_iRequestID;
}

/*
FAK(Fill And Kill)指令就是将报单的有效期设为THOST_FTDC_TC_IOC,同时,成交量类型设为THOST_FTDC_VC_AV,即任意数量;
FOK(Fill Or Kill)指令是将报单的有效期类型设置为THOST_FTDC_TC_IOC,同时将成交量类型设置为THOST_FTDC_VC_CV,即全部数量.
此外,在FAK指令下,还可指定最小成交量,即在指定价位、满足最小成交数量以上成交,剩余订单被系统撤销,否则被系统全部撤销.此种状况下,
有效期类型设置为THOST_FTDC_TC_IOC,数量条件设为THOST_FTDC_VC_MV,同时设定MinVolume字段.
*/
int CtpTraderSpi::ReqOrdFAOK(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir,TThostFtdcCombOffsetFlagType kpp,
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
	m_onRoadVec.push_back(req);
	PostThreadMessage(MainThreadId,WM_UPDATE_LSTCTRL,NULL,NULL);
	return m_iRequestID;
}

void CtpTraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if(IsErrorRspInfo(pRspInfo) || (pInputOrder == NULL))
	{
		if(pInputOrder){
			for(unsigned int i = 0;i < m_onRoadVec.size();i++){
				if(!strcmp(m_onRoadVec[i].OrderRef,pInputOrder->OrderRef)){
					TRACE(_T("OnRspOrderInsert,%s\n"),pRspInfo->ErrorMsg);//暂时不对挂单状态进行更新
				}
			}
		}
		CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
		if(pApp->m_pHedgePostProcessing){
			CThostFtdcInputOrderField *pInputOrderRtn = new CThostFtdcInputOrderField;
			memcpy(pInputOrderRtn,pInputOrder,sizeof(CThostFtdcInputOrderField));
			pApp->m_pHedgePostProcessing->PostThreadMessage(WM_RTN_INSERT,NULL,(UINT)pInputOrderRtn);
		}
	}
}

int CtpTraderSpi::ReqOrderCancel(TThostFtdcSequenceNoType orderSeq)
{
	bool found=false; UINT i=0;
	for(i=0;i<m_orderVec.size();i++){
		if(m_orderVec[i].BrokerOrderSeq == orderSeq){ found = true; break;}
	}
	if(!found)
	{
		////////报单已被成交或不存在///////////
		return -1;
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
	return m_iRequestID;
}

int CtpTraderSpi::ReqOrderCancel(TThostFtdcInstrumentIDType instId,TThostFtdcOrderRefType OrderRef)
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
	return m_iRequestID;
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
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);	
}

///报单回报
void CtpTraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder){
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	char data[10000],datetime[100];
	if(connctp){
		sprintf(datetime,"'%d-%d-%d %d:%d:%d',%d,",sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds);
		sprintf(data,"'%s','%s','%s','%s','%s','%c','%c','%s','%s',%.02lf,%d,'%c','%s','%c',%d,'%c',%.02lf,'%c',%d,'%s',%d,'%s','%s','%s','%s','%s','%s',%d,'%c',%d,'%s',%d,'%s','%c','%c','%c',%d,%d,'%s',\
					 '%s','%s','%s','%s','%s','%s','%s',%d,%d,%d,'%s','%s',%d,'%s',%d,'%s',%d,%d",
					 pOrder->BrokerID,pOrder->InvestorID,pOrder->InstrumentID,pOrder->OrderRef,pOrder->UserID,pOrder->OrderPriceType,pOrder->Direction,
					 pOrder->CombOffsetFlag,pOrder->CombHedgeFlag,pOrder->LimitPrice,pOrder->VolumeTotalOriginal,pOrder->TimeCondition,pOrder->GTDDate,
					 pOrder->VolumeCondition,pOrder->MinVolume,pOrder->ContingentCondition,pOrder->StopPrice,pOrder->ForceCloseReason,pOrder->IsAutoSuspend,
					 pOrder->BusinessUnit,pOrder->RequestID,pOrder->OrderLocalID,pOrder->ExchangeID,pOrder->ParticipantID,pOrder->ClientID,pOrder->ExchangeInstID,
					 pOrder->TraderID,pOrder->InstallID,pOrder->OrderSubmitStatus,pOrder->NotifySequence,pOrder->TradingDay,pOrder->SettlementID,pOrder->OrderSysID,
					 pOrder->OrderSource,pOrder->OrderStatus,pOrder->OrderType,pOrder->VolumeTraded,pOrder->VolumeTotal,pOrder->InsertDate,pOrder->InsertTime,pOrder->ActiveTime,
					 pOrder->SuspendTime,pOrder->UpdateTime,pOrder->CancelTime,pOrder->ActiveTraderID,pOrder->ClearingPartID,pOrder->SequenceNo,pOrder->FrontID,pOrder->SessionID,
					 pOrder->UserProductInfo,pOrder->StatusMsg,pOrder->UserForceClose,pOrder->ActiveUserID,pOrder->BrokerOrderSeq,pOrder->RelativeOrderSysID,
					 pOrder->ZCETotalTradedVolume,pOrder->IsSwapOrder);
		CString insertdata = "INSERT INTO " + statusTableName 
			+ " (datetime,millisecond,BrokerID,InvestorID,InstrumentID,OrderRef,UserID,OrderPriceType,Direction,CombOffsetFlag,CombHedgeFlag,LimitPrice,VolumeTotalOriginal,TimeCondition,\
			  GTDDate,VolumeCondition,MinVolume,ContingentCondition,StopPrice,ForceCloseReason,IsAutoSuspend,BusinessUnit,RequestID,OrderLocalID,ExchangeID,ParticipantID,ClientID,\
			  ExchangeInstID,TraderID,InstallID,OrderSubmitStatus,NotifySequence,TradingDay,SettlementID,OrderSysID,OrderSource,OrderStatus,OrderType,VolumeTraded,VolumeTotal,InsertDate,\
			  InsertTime,ActiveTime,SuspendTime,UpdateTime,CancelTime,ActiveTraderID,ClearingPartID,SequenceNo,FrontID,SessionID,UserProductInfo,StatusMsg,UserForceClose,ActiveUserID,\
			  BrokerOrderSeq,RelativeOrderSysID,ZCETotalTradedVolume,IsSwapOrder) VALUES (" + CString(datetime) + CString(data) +")";
		if(mysql_query(connctp,insertdata.GetBuffer())){
			TRACE("Error %u: %s\n", mysql_errno(connctp), mysql_error(connctp));
		}
	}
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	CThostFtdcOrderField order;
	TRACE("OnRtnOrder所有报单通知,前置%d,会话%d,%s,%s,%c,%d,已经成交%d\r\n",pOrder->FrontID,pOrder->SessionID,pOrder->OrderRef, pOrder->OrderSysID,pOrder->OrderStatus,pOrder->BrokerOrderSeq,pOrder->VolumeTraded);
	memcpy(&order,pOrder, sizeof(CThostFtdcOrderField));
	bool founded = false;UINT i = 0;
	for(i = 0;i<m_orderVec.size();i++){
		if(m_orderVec[i].BrokerOrderSeq == order.BrokerOrderSeq && strcmp(m_orderVec[i].BrokerID,order.BrokerID) == 0) { 
			founded = true;
			//修改命令状态
			m_orderVec[i] = order;
			PostThreadMessage(MainThreadId,WM_UPDATE_LSTCTRL,NULL,NULL);
			break;
		}
	}		
	if(!founded){
		//将挂单删除，因为这时有消息返回说明已经递送出去了
		int nRet = FindOrdInOnRoadVec(order.OrderRef);
		//未加入挂单列表
		if (nRet==-1){
			//如果没有,则不做任何动作
		}
		else{
			//挂单返回，表示已经递送出去，将该挂单删除(已经变成委托单或其他)
			m_onRoadVec.erase(nRet);
			PostThreadMessage(MainThreadId,WM_UPDATE_LSTCTRL,NULL,NULL);
		}
		///////新增加委托单
		m_orderVec.push_back(order);
		PostThreadMessage(MainThreadId,WM_UPDATE_LSTCTRL,NULL,NULL);
	}
	//后处理
	if(pApp->m_pHedgePostProcessing){
		CThostFtdcOrderField *pOrderPost = new CThostFtdcOrderField;
		memcpy(pOrderPost,pOrder,sizeof(CThostFtdcOrderField));
		pApp->m_pHedgePostProcessing->PostThreadMessage(WM_RTN_ORDER,NULL,(UINT)pOrderPost);
	}
}

///成交通知
void CtpTraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade){
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	char data[10000],datetime[100];
	if(connctp){
		sprintf(datetime,"'%d-%d-%d %d:%d:%d',%d,",sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds);
		sprintf(data,"'%s','%s','%s','%s','%s','%s','%s',\
					 '%c','%s','%s','%s',%d,'%s',\
					 '%c','%c',%.02lf,'%d','%s','%s',%d,\
					 %d,'%s','%s','%s','%s',%d,\
					 '%s',%d,%d,'%c'",
					 pTrade->BrokerID,pTrade->InvestorID,pTrade->InstrumentID,pTrade->OrderRef,pTrade->UserID,pTrade->ExchangeID,pTrade->TradeID,
					 pTrade->Direction,pTrade->OrderSysID,pTrade->ParticipantID,pTrade->ClientID,pTrade->TradingRole,pTrade->ExchangeInstID,
					 pTrade->OffsetFlag,pTrade->HedgeFlag,pTrade->Price,pTrade->Volume,pTrade->TradeDate,pTrade->TradeTime,pTrade->TradeType,
					 pTrade->PriceSource,pTrade->TraderID,pTrade->OrderLocalID,pTrade->ClearingPartID,pTrade->BusinessUnit,pTrade->SequenceNo,
					 pTrade->TradingDay,pTrade->SettlementID,pTrade->BrokerOrderSeq,pTrade->TradeSource);
		CString insertdata = "INSERT INTO " + tradeTableName 
			+ " (datetime,millisecond,BrokerID,InvestorID,InstrumentID,OrderRef,UserID,ExchangeID,TradeID,Direction,OrderSysID,ParticipantID,ClientID,TradingRole,\
			  ExchangeInstID,OffsetFlag,HedgeFlag,Price,Volume,TradeDate,TradeTime,TradeType,PriceSource,TraderID,OrderLocalID,ClearingPartID,BusinessUnit,\
			  SequenceNo,TradingDay,SettlementID,BrokerOrderSeq,TradeSource) VALUES (" + CString(datetime) + CString(data) +")";
		if(mysql_query(connctp,insertdata.GetBuffer())){
			TRACE("Error %u: %s\n", mysql_errno(connctp), mysql_error(connctp));
		}
	}	
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	CThostFtdcTradeField trade;
	memcpy(&trade,pTrade,sizeof(CThostFtdcTradeField));
	bool founded = false;     
	unsigned int ii = 0;
	for(ii = 0;ii<m_tradeVec.size();ii++){
		//strcmp返回0时表示相等
		if(strcmp(m_tradeVec[ii].TradeID,trade.TradeID) == 0 
			&& strcmp(m_tradeVec[ii].OrderSysID,trade.OrderSysID) == 0
			&& strcmp(m_tradeVec[ii].ExchangeID,trade.ExchangeID) == 0){
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
		m_tradeVec.push_back(trade);
		PostThreadMessage(MainThreadId,WM_UPDATE_LSTCTRL,NULL,NULL);
	}
	//更改持仓明细单
	if(trade.OffsetFlag == THOST_FTDC_OF_Open){
		bool foundedInPosDetail = false;

		AcquireSRWLockExclusive(&g_srwLock_PosDetail); 
		for(unsigned int ii = 0;ii < m_InvPosDetailVec.size();ii++){
			if(m_InvPosDetailVec[ii].TradeID == trade.TradeID
				&& strcmp(m_InvPosDetailVec[ii].InstrumentID,trade.InstrumentID) == 0
				&& strcmp(m_InvPosDetailVec[ii].OpenDate,trade.TradeDate) == 0
				&& strcmp(m_InvPosDetailVec[ii].ExchangeID,trade.ExchangeID) == 0){
					foundedInPosDetail = true;
					break;
			}
		}
		if(!foundedInPosDetail){
			CThostFtdcInvestorPositionDetailField posDetail;memset(&posDetail,0,sizeof(CThostFtdcInvestorPositionDetailField));
			posDetail.Direction = trade.Direction;strcpy(posDetail.InstrumentID,trade.InstrumentID);posDetail.OpenPrice = trade.Price;
			strcpy(posDetail.BrokerID,trade.BrokerID);strcpy(posDetail.InvestorID,trade.InvestorID);posDetail.TradeType = trade.TradeType;
			strcpy(posDetail.TradeID,trade.TradeID);strcpy(posDetail.ExchangeID,trade.ExchangeID);
			strcpy(posDetail.OpenDate,trade.TradeDate);strcpy(posDetail.TradingDay,trade.TradingDay);
			posDetail.Volume = trade.Volume;
			m_InvPosDetailVec.push_back(posDetail);
			PostThreadMessage(MainThreadId,WM_UPDATE_LSTCTRL,NULL,NULL);
			PostThreadMessage(MainThreadId,WM_SYNCHRONIZE_MARKET,NULL,NULL);
		}
		ReleaseSRWLockExclusive(&g_srwLock_PosDetail);

	}
	else{//平仓

		AcquireSRWLockExclusive(&g_srwLock_PosDetail); 
		sort(m_InvPosDetailVec.begin(),m_InvPosDetailVec.end(),CmpByTime);//排序
		int closeNum = trade.Volume;TThostFtdcDirectionType closeDirection;
		if(trade.Direction == THOST_FTDC_D_Buy){closeDirection = THOST_FTDC_D_Sell;}
		else{closeDirection = THOST_FTDC_D_Buy;}
		for(unsigned int i = 0;i < m_InvPosDetailVec.size();i++){		
			if(strcmp(m_InvPosDetailVec[i].InstrumentID,trade.InstrumentID) == 0 && m_InvPosDetailVec[i].Direction == closeDirection){
				if(m_InvPosDetailVec[i].Volume > closeNum){
					m_InvPosDetailVec[i].CloseVolume = m_InvPosDetailVec[i].CloseVolume + closeNum;
					m_InvPosDetailVec[i].Volume = m_InvPosDetailVec[i].Volume - closeNum;

					AcquireSRWLockExclusive(&g_srwLock_TradingAccount);
					if(m_InvPosDetailVec[i].Direction == THOST_FTDC_D_Buy){
						TradingAccount.CloseProfit = TradingAccount.CloseProfit + (trade.Price - m_InvPosDetailVec[i].OpenPrice) * closeNum * 300.0;
					}
					else{
						TradingAccount.CloseProfit = TradingAccount.CloseProfit - (trade.Price - m_InvPosDetailVec[i].OpenPrice) * closeNum * 300.0;
					}
					ReleaseSRWLockExclusive(&g_srwLock_TradingAccount);

					PostThreadMessage(MainThreadId,WM_UPDATE_LSTCTRL,NULL,NULL);
					PostThreadMessage(MainThreadId,WM_SYNCHRONIZE_MARKET,NULL,NULL);
					break;
				}
				else if(m_InvPosDetailVec[i].Volume == closeNum){

					AcquireSRWLockExclusive(&g_srwLock_TradingAccount);
					if(m_InvPosDetailVec[i].Direction == THOST_FTDC_D_Buy){
						TradingAccount.CloseProfit = TradingAccount.CloseProfit + (trade.Price - m_InvPosDetailVec[i].OpenPrice) * closeNum * 300.0;
					}
					else{
						TradingAccount.CloseProfit = TradingAccount.CloseProfit - (trade.Price - m_InvPosDetailVec[i].OpenPrice) * closeNum * 300.0;
					}
					ReleaseSRWLockExclusive(&g_srwLock_TradingAccount);

					std::vector<CThostFtdcInvestorPositionDetailField>::iterator it = m_InvPosDetailVec.begin() + i;
					m_InvPosDetailVec.erase(it);
					i--;
					PostThreadMessage(MainThreadId,WM_UPDATE_LSTCTRL,NULL,NULL);
					PostThreadMessage(MainThreadId,WM_SYNCHRONIZE_MARKET,NULL,NULL);
					break;
				}
				else{
					closeNum = closeNum - m_InvPosDetailVec[i].Volume;

					AcquireSRWLockExclusive(&g_srwLock_TradingAccount);
					if(m_InvPosDetailVec[i].Direction == THOST_FTDC_D_Buy){
						TradingAccount.CloseProfit = TradingAccount.CloseProfit + (trade.Price - m_InvPosDetailVec[i].OpenPrice) * m_InvPosDetailVec[i].Volume * 300.0;
					}
					else{
						TradingAccount.CloseProfit = TradingAccount.CloseProfit - (trade.Price - m_InvPosDetailVec[i].OpenPrice) * m_InvPosDetailVec[i].Volume * 300.0;
					}
					ReleaseSRWLockExclusive(&g_srwLock_TradingAccount);

					std::vector<CThostFtdcInvestorPositionDetailField>::iterator it = m_InvPosDetailVec.begin() + i;
					m_InvPosDetailVec.erase(it);
					i--;
				}
			}
		}
		ReleaseSRWLockExclusive(&g_srwLock_PosDetail);

	}
	//后处理
	if(pApp->m_pHedgePostProcessing){
		CThostFtdcTradeField *pTradePost = new CThostFtdcTradeField;
		memcpy(pTradePost,pTrade,sizeof(CThostFtdcTradeField));
		pApp->m_pHedgePostProcessing->PostThreadMessage(WM_RTN_TRADE,NULL,(UINT)pTradePost);
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
int CtpTraderSpi::ReqQryTradingCode()
{
	CThostFtdcQryTradingCodeField req;
	memset(&req, 0, sizeof(req));
	req.ClientIDType = THOST_FTDC_CIDT_Speculation;
	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	
	strcpy(req.InvestorID, INVEST_ID); //投资者代码
	pUserApi->ReqQryTradingCode(&req,++m_iRequestID);
	return m_iRequestID;
}

void CtpTraderSpi::OnRspQryTradingCode(CThostFtdcTradingCodeField *pTradingCode, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pTradingCode ){
		CThostFtdcTradingCodeField TdCode;
		memcpy(&TdCode, pTradingCode, sizeof(CThostFtdcTradingCodeField));
		m_TdCodeVec.push_back(TdCode);
	}
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
}

///请求查询合约保证金率
int CtpTraderSpi::ReqQryInstMgr(TThostFtdcInstrumentIDType instId){
	CThostFtdcQryInstrumentMarginRateField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	
	strcpy(req.InvestorID, INVEST_ID); //投资者代码
	if (instId!=NULL)
	{strcpy(req.InstrumentID, instId);}	
	req.HedgeFlag = '1';
	pUserApi->ReqQryInstrumentMarginRate(&req,++m_iRequestID);
	return m_iRequestID;
}

///请求查询合约手续费率
int CtpTraderSpi::ReqQryInstFee(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInstrumentCommissionRateField req;

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	
	strcpy(req.InvestorID, INVEST_ID); //投资者代码
	if (instId!=NULL)
	{strcpy(req.InstrumentID, instId);}	
	pUserApi->ReqQryInstrumentCommissionRate(&req,++m_iRequestID);
	return m_iRequestID;
}

void CtpTraderSpi::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pInstrumentCommissionRate )
	{
		//CThostFtdcInstrumentCommissionRateField* pFeeRate = new CThostFtdcInstrumentCommissionRateField();
		memcpy(&m_FeeRateRev,  pInstrumentCommissionRate, sizeof(CThostFtdcInstrumentCommissionRateField)); 
		//FeeRateList.push_back(pFeeRate);

	}
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
}

//////////////////请求查询用户资料/////////////
int CtpTraderSpi::ReqQryInvestor()
{
	CThostFtdcQryInvestorField req;

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	
	strcpy(req.InvestorID, INVEST_ID); //投资者代码

	pUserApi->ReqQryInvestor(&req, ++m_iRequestID);
	return m_iRequestID;
}

void CtpTraderSpi::OnRspQryInvestor(CThostFtdcInvestorField *pInvestor, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pInvestor )
	{
		CThostFtdcInvestorField Inv;
		memcpy(&Inv,pInvestor,sizeof(CThostFtdcInvestorField));

		//SendNotifyMessage(((CXTraderDlg*)g_pCWnd)->m_hWnd,WM_QRYUSER_MSG,0,(LPARAM)pInv);
	}
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
}

int CtpTraderSpi::ReqUserPwdUpdate(TThostFtdcPasswordType szNewPass,TThostFtdcPasswordType szOldPass)
{
	CThostFtdcUserPasswordUpdateField req;

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	
	strcpy(req.UserID, INVEST_ID); //用户代码
	strcpy(req.NewPassword, szNewPass);    	
	strcpy(req.OldPassword,szOldPass);  

	pUserApi->ReqUserPasswordUpdate(&req,++m_iRequestID);
	return m_iRequestID;
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
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
}

//资金账户密码
int CtpTraderSpi::ReqTdAccPwdUpdate(TThostFtdcPasswordType szNewPass,TThostFtdcPasswordType szOldPass)
{
	CThostFtdcTradingAccountPasswordUpdateField req;

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	
	strcpy(req.AccountID, INVEST_ID); //用户代码
	strcpy(req.NewPassword, szNewPass);    	
	strcpy(req.OldPassword,szOldPass);  

	pUserApi->ReqTradingAccountPasswordUpdate(&req,++m_iRequestID);
	return m_iRequestID;
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
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
}

int CtpTraderSpi::ReqAuthenticate(TThostFtdcProductInfoType UserProdInf,TThostFtdcAuthCodeType	AuthCode)
{
	CThostFtdcReqAuthenticateField req;

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	
	strcpy(req.UserID, INVEST_ID); //用户代码
	strcpy(req.UserProductInfo, UserProdInf);    	
	strcpy(req.AuthCode,AuthCode);  

	pUserApi->ReqAuthenticate(&req,++m_iRequestID);
	return m_iRequestID;
}

void CtpTraderSpi::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pRspAuthenticateField )
	{
		//memcpy(&g_InvInf,pInvestor,sizeof(CThostFtdcInvestorField));
		//AfxMessageBox(tName);
	}
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
}

int CtpTraderSpi::ReqQryAccreg()
{
	CThostFtdcQryAccountregisterField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	
	strcpy(req.AccountID, INVEST_ID); //用户代码

	pUserApi->ReqQryAccountregister(&req,++m_iRequestID);
	return m_iRequestID;
}

void CtpTraderSpi::OnRspQryAccountregister(CThostFtdcAccountregisterField *pAccountregister, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pAccountregister)
	{
		CThostFtdcAccountregisterField AccReg;
		memcpy(&AccReg,  pAccountregister, sizeof(CThostFtdcAccountregisterField));
		m_AccRegVec.push_back(AccReg);
	}
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
}


int CtpTraderSpi::ReqQryTransBk(TThostFtdcBankIDType BankID,TThostFtdcBankBrchIDType BankBrchID)
{
	CThostFtdcQryTransferBankField req;
	memset(&req, 0, sizeof(req));
	if(BankID != NULL)
		strcpy(req.BankID,BankID);
	if(BankBrchID != NULL)
		strcpy(req.BankBrchID,BankBrchID);

	pUserApi->ReqQryTransferBank(&req,++m_iRequestID);
	return m_iRequestID;
}

void CtpTraderSpi::OnRspQryTransferBank(CThostFtdcTransferBankField *pTransferBank, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pTransferBank)
	{
		//memcpy(&g_InvInf,pInvestor,sizeof(CThostFtdcInvestorField));
		//AfxMessageBox(tName);
	}
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
}

int CtpTraderSpi::ReqQryContractBk(TThostFtdcBankIDType BankID,TThostFtdcBankBrchIDType BankBrchID)
{
	CThostFtdcQryContractBankField req;

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	

	if(BankID != NULL)
		strcpy(req.BankID,BankID);
	if(BankBrchID != NULL)
		strcpy(req.BankBrchID,BankBrchID);

	pUserApi->ReqQryContractBank(&req,++m_iRequestID);
	return m_iRequestID;
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
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
}

//////////////////////////////////////////期货发起银行资金转期货请求///////////////////////////////////////
int CtpTraderSpi::ReqBk2FByF(TThostFtdcBankIDType BkID,TThostFtdcPasswordType BkPwd,
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
	return m_iRequestID;
}

///期货发起银行资金转期货应答
void CtpTraderSpi::OnRspFromBankToFutureByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//AfxMessageBox(_T("OnRspFromBankToFutureByFuture"));
	if( IsErrorRspInfo(pRspInfo) || (pReqTransfer==NULL))
	{

		ShowErrTips(pRspInfo->ErrorMsg);
	}
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);	 
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

	//PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,NULL);
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
int CtpTraderSpi::ReqF2BkByF(TThostFtdcBankIDType BkID,TThostFtdcPasswordType BkPwd,
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
	return m_iRequestID;
}


///期货发起期货资金转银行应答
void CtpTraderSpi::OnRspFromFutureToBankByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( IsErrorRspInfo(pRspInfo) || (pReqTransfer==NULL))
	{

		ShowErrTips(pRspInfo->ErrorMsg);
	}

	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);	
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

	//PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,NULL);
}

///期货发起期货资金转银行错误回报
void CtpTraderSpi::OnErrRtnFutureToBankByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo)
{

}

///////////////////////////////////////////////////期货发起查询银行余额请求///////////////////////////////////////////////
int CtpTraderSpi::ReqQryBkAccMoneyByF(TThostFtdcBankIDType BkID,TThostFtdcPasswordType BkPwd,
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
	return m_iRequestID;
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
	//if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,NULL);	

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

	PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,NULL);	
}

///期货发起查询银行余额错误回报
void CtpTraderSpi::OnErrRtnQueryBankBalanceByFuture(CThostFtdcReqQueryAccountField *pReqQueryAccount, CThostFtdcRspInfoField *pRspInfo)
{

}

///////////////////////////////////////查询转账流水////////////////////////////////////////////
/// "204005"
int CtpTraderSpi::ReqQryTfSerial(TThostFtdcBankIDType BkID)
{
	CThostFtdcQryTransferSerialField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	 
	strcpy(req.AccountID, INVEST_ID); //用户代码

	strcpy(req.BankID,BkID);

	pUserApi->ReqQryTransferSerial(&req,++m_iRequestID);
	return m_iRequestID;
}
///请求查询转帐流水响应
void CtpTraderSpi::OnRspQryTransferSerial(CThostFtdcTransferSerialField *pTransferSerial, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( IsErrorRspInfo(pRspInfo) || (pTransferSerial==NULL))
	{
		ShowErrTips(pRspInfo->ErrorMsg);	
	}
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);	
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



int CtpTraderSpi::ReqQryCFMMCTdAccKey()
{
	CThostFtdcQryCFMMCTradingAccountKeyField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //经纪公司代码	 
	strcpy(req.InvestorID, INVEST_ID); //用户代码

	pUserApi->ReqQryCFMMCTradingAccountKey(&req,++m_iRequestID);
	return m_iRequestID;
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
	//if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,NULL);	
}

int CtpTraderSpi::ReqQryBkrTdParams()
{
	CThostFtdcQryBrokerTradingParamsField  req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID,BROKER_ID);
	strcpy(req.InvestorID,INVEST_ID);

	pUserApi->ReqQryBrokerTradingParams(&req, ++m_iRequestID);
	return m_iRequestID;
}

void CtpTraderSpi::OnRspQryBrokerTradingParams(CThostFtdcBrokerTradingParamsField *pBrokerTradingParams, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pBrokerTradingParams)
	{

	}
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
}

int CtpTraderSpi::ReqQryBkrTdAlgos(TThostFtdcExchangeIDType ExhID,TThostFtdcInstrumentIDType instID)
{
	CThostFtdcQryBrokerTradingAlgosField  req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID,BROKER_ID);
	if(ExhID != NULL)
		strcpy(req.ExchangeID, ExhID);
	if(instID != NULL)
		strcpy(req.InstrumentID, instID);

	pUserApi->ReqQryBrokerTradingAlgos(&req, ++m_iRequestID);
	return m_iRequestID;
}

void CtpTraderSpi::OnRspQryBrokerTradingAlgos(CThostFtdcBrokerTradingAlgosField *pBrokerTradingAlgos, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pBrokerTradingAlgos)
	{

	}
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
}

///预埋单录入请求
int CtpTraderSpi::ReqParkedOrderInsert(CThostFtdcParkedOrderField *ParkedOrder)
{
	pUserApi->ReqParkedOrderInsert(ParkedOrder,++m_iRequestID);
	return m_iRequestID;
}
///预埋单录入请求响应
void CtpTraderSpi::OnRspParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pParkedOrder)
	{

	}
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
}

///预埋撤单录入请求
int CtpTraderSpi::ReqParkedOrderAction(CThostFtdcParkedOrderActionField *ParkedOrderAction)
{
	pUserApi->ReqParkedOrderAction(ParkedOrderAction,++m_iRequestID);
	return m_iRequestID;
}
///预埋撤单录入请求响应
void CtpTraderSpi::OnRspParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pParkedOrderAction)
	{

	}
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
}

///请求查询预埋单
int CtpTraderSpi::ReqQryParkedOrder(TThostFtdcInstrumentIDType InstrumentID,TThostFtdcExchangeIDType ExchangeID)
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
	return m_iRequestID;
}

///请求查询预埋单响应
void CtpTraderSpi::OnRspQryParkedOrder(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pParkedOrder)
	{

	}
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
}

///请求撤销预埋
int CtpTraderSpi::ReqQryParkedOrderAction(TThostFtdcInstrumentIDType InstrumentID,TThostFtdcExchangeIDType ExchangeID)
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
	return m_iRequestID;
}

///删除预埋撤单响应
void CtpTraderSpi::OnRspRemoveParkedOrderAction(CThostFtdcRemoveParkedOrderActionField *pRemoveParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if( !IsErrorRspInfo(pRspInfo) && pRemoveParkedOrderAction)
	{

	}
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
}

///请求删除预埋单
int CtpTraderSpi::ReqRemoveParkedOrder(TThostFtdcParkedOrderIDType ParkedOrder_ID)
{
	CThostFtdcRemoveParkedOrderField  req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID,BROKER_ID);
	strcpy(req.InvestorID,INVEST_ID);
	strcpy(req.ParkedOrderID,ParkedOrder_ID);
	pUserApi->ReqRemoveParkedOrder(&req, ++m_iRequestID);
	return m_iRequestID;
}

///删除预埋单响应
void CtpTraderSpi::OnRspRemoveParkedOrder(CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pRemoveParkedOrder)
	{

	}
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
}

///请求删除预埋撤单
int CtpTraderSpi::ReqRemoveParkedOrderAction(TThostFtdcParkedOrderActionIDType ParkedOrderAction_ID)
{
	CThostFtdcRemoveParkedOrderActionField  req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID,BROKER_ID);
	strcpy(req.InvestorID,INVEST_ID);
	strcpy(req.ParkedOrderActionID,ParkedOrderAction_ID);
	pUserApi->ReqRemoveParkedOrderAction(&req, ++m_iRequestID);
	return m_iRequestID;
}

///请求删除预埋撤单响应
void CtpTraderSpi::OnRspQryParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if( !IsErrorRspInfo(pRspInfo) && pParkedOrderAction)
	{

	}
	if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT,NULL,nRequestID);
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

int CtpTraderSpi::ShowErrTips(TThostFtdcErrorMsgType ErrorMsg)
{
	ShowErroTips(ErrorMsg,MY_TIPS);
	return m_iRequestID;
}

void CtpTraderSpi::ClearAllVectors(){
	m_onRoadVec.clear();
	m_orderVec.clear();
	m_tradeVec.clear();

	AcquireSRWLockExclusive(&g_srwLock_Insinf);
	m_InsinfVec.clear();
	ReleaseSRWLockExclusive(&g_srwLock_Insinf);

	AcquireSRWLockExclusive(&g_srwLock_MargRate);
	m_MargRateVec.clear();
	ReleaseSRWLockExclusive(&g_srwLock_MargRate);

	m_StmiVec.clear();
	m_AccRegVec.clear();
	m_TdCodeVec.clear();
	m_InvPosVec.clear();
	m_BfTransVec.clear();

	AcquireSRWLockExclusive(&g_srwLock_PosDetail);
	m_InvPosDetailVec.clear();
	ReleaseSRWLockExclusive(&g_srwLock_PosDetail); 
}

int CtpTraderSpi::FindOrdInOnRoadVec(TThostFtdcOrderRefType OrderRef)
{	
	UINT i=0;
	for(i=0; i<m_onRoadVec.size(); i++)
	{
		if(!strcmp(m_onRoadVec[i].OrderRef,OrderRef)) 
		{ return i;}
	}
	return (-1);
}