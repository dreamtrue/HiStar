#include "stdafx.h"
#include "HiStar.h"
#include "global.h"
#include "calendar.h"
#include "MainDlg.h"
extern bool iAccountDownloadEnd;
//IB交易系统
void CHiStarApp::OnConnectIB(WPARAM wParam,LPARAM lParam){
	UINT clientID = 0;
	CString cStatus;
	cStatus.Format( "Connecting to Tws using clientId %d ...",clientID);
	PostOrderStatus(cStatus);
	if(m_pIBClient){
		m_pIBClient->eConnect("127.0.0.1",4001,clientID);
		//登陆成功
		if( m_pIBClient->serverVersion() > 0)	{
			cStatus.Format( "Connected to Tws server version %d at %s.",
				m_pIBClient->serverVersion(), m_pIBClient->TwsConnectionTime());
			PostOrderStatus(cStatus);
			OnReqAccountUpdates(NULL,0);
			OnReqAccountUpdates(NULL,1);
			//m_pIBClient->reqCurrentTime();
			//m_pIBClient->reqPositions();
			//m_pIBClient->reqMktDepth(++m_id,m_A50Contract,20,m_mktDepthOptions);//取消市场深度,用reqMktData代替。
			m_pIBClient->reqMktData(++m_id,m_A50Contract,"100,101,104,105,106,107,165,221,225,233,236,258,293,294,295,318",false,m_mktDataOptions);
		}
	}
}

void CHiStarApp::OnReqAccountUpdates(WPARAM wParam,LPARAM lParam){
	if(lParam == 0){
		m_pIBClient->reqAccountUpdates(false,m_accountIB.m_accountName);
	}
	else{
		TRACE("OnReqAccountUpdates != 0\n");
		//持仓先清空
		iAccountDownloadEnd = false;
		m_portfolio.clear();
		m_pIBClient->reqAccountUpdates(true,m_accountIB.m_accountName);
	}
}

void CHiStarApp::OnDisconnectIB(WPARAM wParam,LPARAM lParam){
	if(m_pIBClient){
		m_pIBClient->eDisconnect();
	}
}

void CHiStarApp::SetA50Contract(){
	long conId;
	CString symbol;
	CString expiry;
	CString right;
	CString multiplier;
	CString exchange;
	CString primaryExchange;
	CString currency;
	bool includeExpired;
	CString secType;
	double strike;
	CString localSymbol;
	CString secIdType;
	CString secId;
	//填充合约参数
	secIdType = _T("");
	secId = _T("");
	conId = 0;
	symbol = _T("XINA50");
	//计算需要采用的合约的月份
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	WORD A50Final = A50FinalDay(sys.wYear,sys.wMonth);
	CString a50days;
	a50days.Format("%d",A50Final - sys.wDay);	
	//到最后一天的前一天换合约
	if(sys.wDay < A50Final - 3){
		expiry.Format(_T("%.4d%.2d"),sys.wYear,sys.wMonth);
		m_LifeA50 = A50FinalDay(sys.wYear,sys.wMonth + 1) - sys.wDay + 1;
	}
	else{
		if(sys.wMonth < 12){
			expiry.Format(_T("%.4d%.2d"),sys.wYear,sys.wMonth + 1);
			m_LifeA50 = MonthDays(sys.wYear,sys.wMonth) - sys.wDay
				+ A50FinalDay(sys.wYear,sys.wMonth + 1) + 1;
		}
		else{//年末
			expiry.Format(_T("%.4d%.2d"),sys.wYear + 1,1);
			m_LifeA50 = MonthDays(sys.wYear,sys.wMonth) - sys.wDay
				+ A50FinalDay(sys.wYear + 1,1) + 1;
		}
	}
	char A50NAME[100];memset(A50NAME,0,sizeof(A50NAME));
	sprintf(A50NAME,"%s %s,%d",symbol,expiry,m_LifeA50);
	if(sys.wDay < A50Final - 3){
		((CMainDlg*)m_pMainWnd)->m_basicPage.m_csA50Show.SetWindowText(A50NAME,RED);
	}
	else{
		((CMainDlg*)m_pMainWnd)->m_basicPage.m_csA50Show.SetWindowText(A50NAME,YELLOW);
	}
	right = _T("");
	multiplier = _T("");
	exchange = _T("SGX");
	primaryExchange = _T("");
	currency = _T("USD");
	includeExpired = 0;
	strike = 0.0f;
	secType = _T("FUT");
	localSymbol = _T("");
#define PUT_PROP(Prop) \
	m_A50Contract.##Prop = Prop;
	PUT_PROP(conId);
	PUT_PROP(symbol);
	PUT_PROP(secType);
	PUT_PROP(expiry);
	PUT_PROP(strike);
	PUT_PROP(right);
	PUT_PROP(multiplier);
	PUT_PROP(exchange);
	PUT_PROP(primaryExchange);
	PUT_PROP(currency);
	PUT_PROP(localSymbol);
	PUT_PROP(includeExpired);
	PUT_PROP(secIdType);
	PUT_PROP(secId);
#undef PUT_PROP
}
