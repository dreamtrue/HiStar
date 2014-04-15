
#pragma once

#include "stdafx.h"
#include "mdspi.h"
#include "HiStar.h"
#include "global.h"
#include <atlconv.h>
#include "MainDlg.h"
#include "UserMsg.h"
#pragma warning(disable : 4996)
double g_ifAsk1 = 0,g_ifBid1 = 0;
extern DWORD MainThreadId;
bool g_bOnceM = false;
BOOL bRecconnect = FALSE;
BOOL bMdSignal = FALSE;
void CtpMdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo,
	int nRequestID, bool bIsLast){
		TRACE("OnRspErrorM\n");
		IsErrorRspInfo(pRspInfo);
}

void CtpMdSpi::OnFrontDisconnected(int nReason){
	TRACE("ctp行情系统前置断开\n");
	bMdSignal = FALSE;
}

void CtpMdSpi::OnHeartBeatWarning(int nTimeLapse){
	TRACE("OnHeartBeatWarning\n");
}

void CtpMdSpi::OnFrontConnected(){
	TRACE("OnFrontConnected\n");
	PostThreadMessage(MainThreadId,WM_LOGIN_MD,NULL,NULL);
}

int CtpMdSpi::ReqUserLogin(TThostFtdcBrokerIDType	vAppId,TThostFtdcUserIDType	vUserId,TThostFtdcPasswordType	vPasswd){
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, vAppId); strcpy(m_sBkrID, vAppId); 
	strcpy(req.UserID, vUserId);  strcpy(m_sINVEST_ID, vUserId); 
	strcpy(req.Password, vPasswd);
	strcpy(req.UserProductInfo,PROD_INFO);
	pUserApi->ReqUserLogin(&req, ++m_iRequestID);
	return m_iRequestID;
}

void CtpMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
		TRACE("OnRspUserLogin\n");
		CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
		if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin){
		}
		if(bIsLast) PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT_MD,NULL,nRequestID);
}

int CtpMdSpi::ReqUserLogout(){
	return 0;
}

///登出请求响应
void CtpMdSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	TRACE("OnRspUserLogoutM\n");
	if(!IsErrorRspInfo(pRspInfo) && pUserLogout){}
}

//TThostFtdcInstrumentIDType instId

int CtpMdSpi::SubscribeMarketData(char *pInst[], int nCount)
{
	for(int i = 0;i < nCount;i++){
		CString str = pInst[i];
		TRACE("%s,%s,%d\n",pInst[i],str,sizeof(pInst[i]));
		bool founded = false;
		for(unsigned int j = 0;j < InstSubscribed.size();j++){
			if(InstSubscribed[j] == str){
				std::vector<CString>::iterator it = InstSubscribed.begin() + j;
				founded = true;
				break;
			}
		}
		if(!founded){
			InstSubscribed.push_back(str);
		}
	}
	pUserApi->SubscribeMarketData(pInst, nCount);
	return 0;
}

//同步市场
void CtpMdSpi::SynchronizeMarket(std::vector<CString> &InstSubscribed,std::vector<CString> &InstMustSubscribe,std::vector<CThostFtdcInvestorPositionDetailField> &InvPosDetailVec){
	std::vector<CString> InvPos;
	for(unsigned int j = 0;j < InvPosDetailVec.size();j++){
		CString str;str = InvPosDetailVec[j].InstrumentID;
		InvPos.push_back(str);
	}
	std::vector<CString> InstNeedSubscribe;
	InstNeedSubscribe.insert(InstNeedSubscribe.begin(),InstMustSubscribe.begin(),InstMustSubscribe.end());
	InstNeedSubscribe.insert(InstNeedSubscribe.begin(),InvPos.begin(),InvPos.end());
	//订阅
	for(unsigned int i = 0;i < InstNeedSubscribe.size();i++){
		bool founded = false;
		for(unsigned int j = 0;j < InstSubscribed.size();j++){
			if(InstNeedSubscribe[i] == InstSubscribed[j]){
				founded = true;break;
			}
		}
		if(!founded){
			char szInst[MAX_PATH];
			uni2ansi(CP_ACP,InstNeedSubscribe[i].GetBuffer(),szInst);
			LPSTR* pInst = new LPSTR;
			pInst[0] = szInst;
			SubscribeMarketData(pInst,1);
			delete []pInst;
		}
	}
	//取消多余的订阅
	for(unsigned int ii = 0;ii < InstSubscribed.size();ii++){
		bool founded = false;
		for(unsigned int jj = 0;jj < InstNeedSubscribe.size();jj++){
			if(InstNeedSubscribe[jj] == InstSubscribed[ii]){
				founded = true;break;
			}
		}
		if(!founded){
			char szInst[MAX_PATH];
			uni2ansi(CP_ACP,InstSubscribed[ii].GetBuffer(),szInst);
			LPSTR* pInst = new LPSTR;
			pInst[0] = szInst;
			UnSubscribeMarketData(pInst,1);
			delete []pInst;
		}
	}
}

void CtpMdSpi::OnRspSubMarketData(
	CThostFtdcSpecificInstrumentField *pSpecificInstrument, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
		TRACE("OnRspSubMarketData\n");
		if (!IsErrorRspInfo(pRspInfo) && pSpecificInstrument){
			bMdSignal = TRUE;	
		}
		else{
			bMdSignal = FALSE;
		}
}

int CtpMdSpi::UnSubscribeMarketData(char *pInst[], int nCount){
	for(int i = 0;i < nCount;i++){
		CString str = pInst[i];
		for(unsigned int j = 0;j < InstSubscribed.size();j++){
			if(InstSubscribed[j] == str){
				std::vector<CString>::iterator it = InstSubscribed.begin() + j;
				InstSubscribed.erase(it);
				j--;
			}
		}
		InstSubscribed.push_back(str);
	}
	pUserApi->UnSubscribeMarketData(pInst, nCount);
	return 0;
}

void CtpMdSpi::OnRspUnSubMarketData(
	CThostFtdcSpecificInstrumentField *pSpecificInstrument,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
		TRACE("OnRspUnSubMarketData\n");
		if (!IsErrorRspInfo(pRspInfo) && pSpecificInstrument){
			bMdSignal = FALSE;
		}
		else{
			bMdSignal = TRUE;
		}
}

void CtpMdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData){
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	bool founded = false;
	for(unsigned int j = 0;Marketdata.size();j++){
		if(strcmp(Marketdata[j].InstrumentID,pDepthMarketData->InstrumentID) == 0){
			Marketdata[j] = *pDepthMarketData;
			founded = true;
			break;
		}
	}
	if(!founded){
		Marketdata.push_back(*pDepthMarketData);
	}
	if(strcmp(pDepthMarketData->InstrumentID,pApp->m_accountCtp.m_szInst) == 0){
		if(pDepthMarketData->AskPrice1 != g_ifAsk1 || pDepthMarketData->BidPrice1 != g_ifBid1){
			g_ifAsk1 = pDepthMarketData->AskPrice1;
			g_ifBid1 = pDepthMarketData->BidPrice1;
			if(pApp->m_pMainWnd){
				memcpy(&(((CMainDlg*)(pApp->m_pMainWnd))->m_basicPage.m_depthMd),pDepthMarketData,sizeof(CThostFtdcDepthMarketDataField));
			}
			PostThreadMessage(MainThreadId,WM_MD_REFRESH,NULL,NULL);
		}
	}
	calcPositionProfit();
}

double CtpMdSpi::calcPositionProfit(){
	double positionProfit = 0;
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	std::vector<CThostFtdcInvestorPositionDetailField> InvPosDetailVec = pApp->m_cT->m_InvPosDetailVec.GetBuffer();
	for(unsigned int i = 0;i < InvPosDetailVec.size();i++){
		bool founed = false;int index = 0;
		for(unsigned int j = 0;j < Marketdata.size();j++){
			if(!strcmp(InvPosDetailVec[i].InstrumentID,Marketdata[j].InstrumentID)){
				founed = true;index = j;
			}
		}
		if(founed){
			if(Marketdata[index].AskPrice1 > 0.000001 &&  Marketdata[index].BidPrice1 > 0.000001){
				if(!strcmp(InvPosDetailVec[i].OpenDate,pApp->m_accountCtp.m_todayDate)){
					if(InvPosDetailVec[i].Direction == THOST_FTDC_D_Buy){
						positionProfit = positionProfit + ((Marketdata[index].AskPrice1 + Marketdata[index].BidPrice1) / 2.0 - InvPosDetailVec[i].OpenPrice) * InvPosDetailVec[i].Volume;
					}
					else{
						positionProfit = positionProfit - ((Marketdata[index].AskPrice1 + Marketdata[index].BidPrice1) / 2.0 - InvPosDetailVec[i].OpenPrice) * InvPosDetailVec[i].Volume;
					}
				}
				else{
					if(InvPosDetailVec[i].Direction == THOST_FTDC_D_Buy){
						positionProfit = positionProfit + ((Marketdata[index].AskPrice1 + Marketdata[index].BidPrice1) / 2.0 - InvPosDetailVec[i].LastSettlementPrice) * InvPosDetailVec[i].Volume;
					}
					else{
						positionProfit = positionProfit - ((Marketdata[index].AskPrice1 + Marketdata[index].BidPrice1) / 2.0 - InvPosDetailVec[i].LastSettlementPrice) * InvPosDetailVec[i].Volume;
					}
				}
			}
		}
	}
	TRACE("动态 %.02lf\n",positionProfit);
	return positionProfit;
}

bool CtpMdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo){	
	bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (ret){}
	return ret;
}