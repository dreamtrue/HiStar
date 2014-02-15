
#pragma once

#include "stdafx.h"
#include "mdspi.h"
#include "HiStar.h"
#pragma warning(disable : 4996)

extern HANDLE g_hEvent;
BOOL bRecconnect = FALSE;
BOOL bMdSignal = FALSE;
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
	if (g_pCWnd)
	{
		bRecconnect = TRUE;
		ReqUserLogin(pApp->m_sBROKER_ID);//,pApp->m_sINVESTOR_ID,pApp->m_sPASSWORD);
		DWORD dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
		if (dwRet==WAIT_OBJECT_0)
		{
			pDlg->SetTipTxt(_T("行情在线"),IDS_MD_TIPS);
			pDlg->SetPaneTxtColor(0,RED);
			ResetEvent(g_hEvent);
		}
		else
		{
			return;
		}

		//int iLen = pDlg->m_szInst.GetLength();
		char szInst[MAX_PATH];
		uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)pDlg->m_szInst,szInst);

		LPSTR* pInst = new LPSTR;
		pInst[0] = szInst;
		SubscribeMarketData(pInst,1);
		dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
		if (dwRet==WAIT_OBJECT_0)
		{
			ResetEvent(g_hEvent);
		}
			//Sleep(100);
			bMdSignal = TRUE;
	}
	
	else
	{
		ReqUserLogin(pApp->m_sBROKER_ID);
	}


  //if (g_bOnce)SetEvent(g_hEvent);
}

void CtpMdSpi::ReqUserLogin(TThostFtdcBrokerIDType	appId)
	     //TThostFtdcUserIDType	userId,	TThostFtdcPasswordType	passwd)
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, appId);
	strcpy(m_sBkrID,appId);
	//strcpy(req.UserID, userId);
	//strcpy(req.Password, passwd);

	pUserApi->ReqUserLogin(&req, ++m_iRequestID);
  //cerr<<" 请求 | 发送登录..."<<((ret == 0) ? "成功" :"失败") << endl;	
  //SetEvent(g_hEvent);
}

void CtpMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	TRACE("OnRspUserLogin\n");
	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin)
	{
		//m_pDlg->ProgressUpdate(_T("登陆行情成功!"), 60);
    //cerr<<" 响应 | 登录成功...当前交易日:"<<pRspUserLogin->TradingDay<<endl;
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
{/*
  vector<char*> list;

  char* token = strsep(&instIdList, ",");
  while( token != NULL ){
    list.push_back(token); 
    token = strsep(&instIdList, ",");
  }
  
  UINT len = list.size();
  char** pInstId = new char*[len];
  for(UINT i=0; i<len;i++)  {pInstId[i]=list[i];}
  */

  pUserApi->SubscribeMarketData(pInst, nCount); 
  //SetEvent(g_hEvent);
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
	//cerr<<" 响应 |  行情订阅...成功"<<endl;
  if(bIsLast)  SetEvent(g_hEvent);
}

void CtpMdSpi::UnSubscribeMarketData(char *pInst[], int nCount)
{
	/*
  vector<char*> list;

  char* token = strsep(&instIdList, ",");
  while( token != NULL ){
    list.push_back(token); 
    token = strsep(&instIdList, ",");
  }
	UINT len = list.size();
	char** pInstId = new char*[len];
	for(UINT i=0; i<len;i++)  {pInstId[i]=list[i]; }
	//pInstId[len] = NULL;
	*/
	pUserApi->UnSubscribeMarketData(pInst, nCount);
	//SetEvent(g_hEvent);
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
	//__int64 dwMs1,dwMs2;
	//dwMs1 = GetCycleCount();
	if (g_pCWnd!=NULL)
	{
		TCHAR szInst[40];
		ansi2uni(CP_ACP,pDepthMarketData->InstrumentID,szInst);

		CXTraderDlg* pDlg = (CXTraderDlg*)g_pCWnd;

		if (pDlg->m_szInst.Compare(szInst)==0)
		{
			memcpy(pDlg->m_pDepthMd,pDepthMarketData,sizeof(CThostFtdcDepthMarketDataField));		
			pDlg->RefreshMdPane();
		}
		//CThostFtdcDepthMarketDataField *pDepthMd = new CThostFtdcDepthMarketDataField;

		//PostMessage(pDlg->m_hWnd,WM_UPDATEMD_MSG,0,(LPARAM)pDepthMd);
		//SendNotifyMessage(pDlg->m_hWnd,WM_UPDATEMD_MSG,0,(LPARAM)pDepthMarketData);
		
	}
	
}

bool CtpMdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{	
  bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
  if (ret){
    cerr<<" 响应 | "<<pRspInfo->ErrorMsg<<endl;
  }
  return ret;
}