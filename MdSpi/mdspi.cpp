
#pragma once

#include "stdafx.h"
#include "mdspi.h"
#include "HiStar.h"
#include "global.h"
#include <atlconv.h>
#pragma warning(disable : 4996)

extern HANDLE g_hEvent;
BOOL bRecconnect = FALSE;
BOOL bMdSignal = FALSE;
void CtpMdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo,
		int nRequestID, bool bIsLast)
{
	TRACE("OnRspError\n");
	IsErrorRspInfo(pRspInfo);
}

void CtpMdSpi::OnFrontDisconnected(int nReason)
{
	TRACE("ctp行情系统前置断开\n");
	bMdSignal = FALSE;
}
		
void CtpMdSpi::OnHeartBeatWarning(int nTimeLapse)
{
	TRACE("OnHeartBeatWarning\n");
}

void CtpMdSpi::OnFrontConnected()
{
	TRACE("OnFrontConnected\n");
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	if (true)
	{
		bRecconnect = TRUE;
		ReqUserLogin(pApp->m_accountCtp.m_sBROKER_ID);//,pApp->m_sINVESTOR_ID,pApp->m_sPASSWORD);
		DWORD dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
		if (dwRet==WAIT_OBJECT_0)
		{
			ResetEvent(g_hEvent);
		}
		else
		{
			return;
		}
		char szInst[MAX_PATH];
		uni2ansi(CP_ACP,pApp->m_accountCtp.m_szInst,szInst);

		LPSTR* pInst = new LPSTR;
		pInst[0] = szInst;
		SubscribeMarketData(pInst,1);
		dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
		if (dwRet==WAIT_OBJECT_0)
		{
			ResetEvent(g_hEvent);
		}
		bMdSignal = TRUE;
	}
	else
	{
		ReqUserLogin(pApp->m_accountCtp.m_sBROKER_ID);
	}
}

void CtpMdSpi::ReqUserLogin(TThostFtdcBrokerIDType	appId)
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, appId);
	strcpy(m_sBkrID,appId);
	pUserApi->ReqUserLogin(&req, ++m_iRequestID);
}

void CtpMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	TRACE("OnRspUserLogin\n");
	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin)
	{
	}
  if(bIsLast) SetEvent(g_hEvent);
}

void CtpMdSpi::ReqUserLogout()
{
	CThostFtdcUserLogoutField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, m_sBkrID);

	pUserApi->ReqUserLogout(&req, ++m_iRequestID);
}

///登出请求响应
void CtpMdSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	TRACE("OnRspUserLogout\n");
	if( !IsErrorRspInfo(pRspInfo) && pUserLogout)
	{	
	}
	if(bIsLast) SetEvent(g_hEvent);
}

//TThostFtdcInstrumentIDType instId

void CtpMdSpi::SubscribeMarketData(char *pInst[], int nCount)
{
  pUserApi->SubscribeMarketData(pInst, nCount); 
}

void CtpMdSpi::OnRspSubMarketData(
         CThostFtdcSpecificInstrumentField *pSpecificInstrument, 
         CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	TRACE("OnRspSubMarketData\n");
	if (!IsErrorRspInfo(pRspInfo) && pSpecificInstrument)
	{
		bMdSignal = TRUE;	
	}
	else
	{
		bMdSignal = FALSE;
	}
  if(bIsLast)  SetEvent(g_hEvent);
}

void CtpMdSpi::UnSubscribeMarketData(char *pInst[], int nCount)
{
	pUserApi->UnSubscribeMarketData(pInst, nCount);
}

void CtpMdSpi::OnRspUnSubMarketData(
             CThostFtdcSpecificInstrumentField *pSpecificInstrument,
             CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	TRACE("OnRspUnSubMarketData\n");
	if (!IsErrorRspInfo(pRspInfo) && pSpecificInstrument)
	{
		bMdSignal = FALSE;
	}
	else
	{
		bMdSignal = TRUE;
	}
  if(bIsLast)  SetEvent(g_hEvent);
}

void CtpMdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	TRACE("OnRtnDepthMarketData\n");
	if (true)
	{
		/*
		TCHAR szInst[40];
		ansi2uni(CP_ACP,pDepthMarketData->InstrumentID,szInst);

		if (pDlg->m_szInst.Compare(szInst)==0)
		{
			memcpy(pDlg->m_pDepthMd,pDepthMarketData,sizeof(CThostFtdcDepthMarketDataField));		
			pDlg->RefreshMdPane();
		}
		*/
	}
}

bool CtpMdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{	
  bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
  if (ret){
  }
  return ret;
}