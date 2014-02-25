
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
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	if (g_bOnceM){
		bRecconnect = TRUE;
		ReqUserLogin(pApp->m_accountCtp.m_sBROKER_ID,pApp->m_accountCtp.m_sINVESTOR_ID,pApp->m_accountCtp.m_sPASSWORD);
		bMdSignal = TRUE;
	}
	else{
		ReqUserLogin(pApp->m_accountCtp.m_sBROKER_ID,pApp->m_accountCtp.m_sINVESTOR_ID,pApp->m_accountCtp.m_sPASSWORD);
		g_bOnceM = true;
		bMdSignal = TRUE;
	}
}
/*
void CtpMdSpi::ReqUserLogin(TThostFtdcBrokerIDType	appId){
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, appId);
	strcpy(m_sBkrID,appId);
	pUserApi->ReqUserLogin(&req, ++m_iRequestID);
}
*/
void CtpMdSpi::ReqUserLogin(TThostFtdcBrokerIDType	vAppId,TThostFtdcUserIDType	vUserId,TThostFtdcPasswordType	vPasswd){
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, vAppId); strcpy(m_sBkrID, vAppId); 
	strcpy(req.UserID, vUserId);  strcpy(m_sINVEST_ID, vUserId); 
	strcpy(req.Password, vPasswd);
	strcpy(req.UserProductInfo,PROD_INFO);
	pUserApi->ReqUserLogin(&req, ++m_iRequestID);
}

void CtpMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
		TRACE("OnRspUserLogin\n");
		CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
		if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin){
			///订阅行情深度
			char szInst[MAX_PATH];
			uni2ansi(CP_ACP,pApp->m_accountCtp.m_szInst,szInst);
			LPSTR* pInst = new LPSTR;
			pInst[0] = szInst;
			SubscribeMarketData(pInst,1);
		}
}

void CtpMdSpi::ReqUserLogout(){
	/*
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	CThostFtdcUserLogoutField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, m_sBkrID);
	strcpy(req.UserID, m_sINVEST_ID);
	pUserApi->ReqUserLogout(&req, ++m_iRequestID);
	*/
	pUserApi->RegisterSpi(NULL);
	pUserApi->Release();
}

///登出请求响应
void CtpMdSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	TRACE("OnRspUserLogoutM\n");
	if( !IsErrorRspInfo(pRspInfo) && pUserLogout){}
}

//TThostFtdcInstrumentIDType instId

void CtpMdSpi::SubscribeMarketData(char *pInst[], int nCount)
{
	pUserApi->SubscribeMarketData(pInst, nCount); 
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

void CtpMdSpi::UnSubscribeMarketData(char *pInst[], int nCount){
	pUserApi->UnSubscribeMarketData(pInst, nCount);
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
	TRACE("%s,%f,%f.\n",pDepthMarketData->InstrumentID,pDepthMarketData->BidPrice1,pDepthMarketData->AskPrice1);
	g_ifAsk1 = pDepthMarketData->AskPrice1;
	g_ifBid1 = pDepthMarketData->BidPrice1;
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	if(pApp->m_pMainWnd){
		memcpy(&(((CMainDlg*)(pApp->m_pMainWnd))->m_basicPage.m_depthMd),pDepthMarketData,sizeof(CThostFtdcDepthMarketDataField));		
		if(AfxGetApp()->m_pMainWnd){
			PostMessage(AfxGetApp()->m_pMainWnd->m_hWnd,WM_MD_REFRESH,NULL,NULL);
		}
		if(pApp->m_pHedgeLoop){
			pApp->m_pHedgeLoop->PostThreadMessage(WM_MD_REFRESH,NULL,NULL);
		}
	}
}

bool CtpMdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo){	
	bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (ret){}
	return ret;
}