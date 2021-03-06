
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
	if((CHiStarApp*)AfxGetApp()->m_pMainWnd){
		while(::PostMessage(((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->GetSafeHwnd(),WM_LOGIN_MD,NULL,NULL) == 0){
			Sleep(100);
		}
	}
}

int CtpMdSpi::ReqUserLogin(TThostFtdcBrokerIDType	vAppId,TThostFtdcUserIDType	vUserId,TThostFtdcPasswordType	vPasswd){
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy_s(req.BrokerID, vAppId); strcpy_s(m_sBkrID, vAppId); 
	strcpy_s(req.UserID, vUserId);  strcpy_s(m_sINVEST_ID, vUserId); 
	strcpy_s(req.Password, vPasswd);
	strcpy_s(req.UserProductInfo,PROD_INFO);
	while(pUserApi->ReqUserLogin(&req, ++m_iRequestID) != 0){}
	return m_iRequestID;
}

void CtpMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
		TRACE("OnRspUserLogin\n");
		CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
		if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin){
		}
		if(bIsLast){
			while(PostThreadMessage(MainThreadId,WM_NOTIFY_EVENT_MD,NULL,nRequestID) == 0){
				Sleep(100);
			}
		}
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
	while(pUserApi->SubscribeMarketData(pInst, nCount) != 0){}
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
	while(pUserApi->UnSubscribeMarketData(pInst, nCount) != 0){}
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
	for(unsigned int j = 0;j < Marketdata.size();j++){
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
			if((CHiStarApp*)AfxGetApp()->m_pMainWnd){
				while(::PostMessage(((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->GetSafeHwnd(),WM_MD_REFRESH,NULL,NULL) == 0){
					Sleep(100);
				}
			}
		}
	}
	CalcPositionProfit();
}

double CtpMdSpi::CalcPositionProfit(){
	double positionProfit = 0;
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();

	if(pApp && pApp->m_cT){
		AcquireSRWLockShared(&g_srwLock_PosDetail);
		for(unsigned int i = 0;i < pApp->m_cT->m_InvPosDetailVec.size();i++){
			//TRACE("%.02lf,%.02lf,%.02lf\n",pApp->m_cT->m_InvPosDetailVec[i].Margin,pApp->m_cT->m_InvPosDetailVec[i].MarginRateByMoney,pApp->m_cT->m_InvPosDetailVec[i].MarginRateByVolume);
			bool found = false;int index = 0;
			for(unsigned int j = 0;j < Marketdata.size();j++){
				if(!strcmp(pApp->m_cT->m_InvPosDetailVec[i].InstrumentID,Marketdata[j].InstrumentID)){
					found = true;index = j;
				}
			}
			AcquireSRWLockShared(&g_srwLock_Insinf);
			bool found01 = false;int index01 = -1;
			for (UINT k=0; k < pApp->m_cT->m_InsinfVec.size();k++){
				if(strcmp(pApp->m_cT->m_InvPosDetailVec[i].InstrumentID,pApp->m_cT->m_InsinfVec[k].iinf.InstrumentID) == 0){
					found01 = true;index01 = k;
					break;
				}
			}
			if(found && found01){
				if(Marketdata[index].AskPrice1 > 0.000001 && Marketdata[index].BidPrice1 > 0.000001){
					if(strcmp(pApp->m_cT->m_InvPosDetailVec[i].OpenDate,pApp->m_accountCtp.m_todayDate) >= 0){
						if(pApp->m_cT->m_InvPosDetailVec[i].Direction == THOST_FTDC_D_Buy){
							positionProfit = positionProfit + (Marketdata[index].LastPrice - pApp->m_cT->m_InvPosDetailVec[i].OpenPrice) * pApp->m_cT->m_InvPosDetailVec[i].Volume * pApp->m_cT->m_InsinfVec[index01].iinf.VolumeMultiple;
						}
						else{
							positionProfit = positionProfit - (Marketdata[index].LastPrice - pApp->m_cT->m_InvPosDetailVec[i].OpenPrice) * pApp->m_cT->m_InvPosDetailVec[i].Volume * pApp->m_cT->m_InsinfVec[index01].iinf.VolumeMultiple;
						}
					}
					else{
						if(pApp->m_cT->m_InvPosDetailVec[i].Direction == THOST_FTDC_D_Buy){
							positionProfit = positionProfit + (Marketdata[index].LastPrice - pApp->m_cT->m_InvPosDetailVec[i].LastSettlementPrice) * pApp->m_cT->m_InvPosDetailVec[i].Volume * pApp->m_cT->m_InsinfVec[index01].iinf.VolumeMultiple;
						}
						else{
							positionProfit = positionProfit - (Marketdata[index].LastPrice - pApp->m_cT->m_InvPosDetailVec[i].LastSettlementPrice) * pApp->m_cT->m_InvPosDetailVec[i].Volume * pApp->m_cT->m_InsinfVec[index01].iinf.VolumeMultiple;
						}
					}
				}
			}
			ReleaseSRWLockShared(&g_srwLock_Insinf);

		}
		ReleaseSRWLockShared(&g_srwLock_PosDetail);

		AcquireSRWLockExclusive(&g_srwLock_TradingAccount);
		pApp->m_cT->TradingAccount.PositionProfit = positionProfit;
		ReleaseSRWLockExclusive(&g_srwLock_TradingAccount);

	}
	//TRACE("动态 %.02lf\n",positionProfit);
	return positionProfit;
}

bool CtpMdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo){	
	bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (ret){}
	return ret;
}