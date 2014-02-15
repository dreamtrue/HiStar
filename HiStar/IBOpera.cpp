#include "stdafx.h"
#include "HiStar.h"
#include "global.h"
extern HANDLE g_hEvent;
//IB����ϵͳ
extern EClient *g_pIBClient;
void CHiStarApp::OnConnectIB(UINT wParam,LONG lParam){
	UINT clientID = 0;
	CString cStatus;
	cStatus.Format( "Connecting to Tws using clientId %d ...",clientID);
	PostOrderStatus(cStatus);
	g_pIBClient->eConnect("127.0.0.1",4001,clientID);
	//��½�ɹ�
	if( g_pIBClient->serverVersion() > 0)	{
		cStatus.Format( "Connected to Tws server version %d at %s.",
			g_pIBClient->serverVersion(), g_pIBClient->TwsConnectionTime());
		PostOrderStatus(cStatus);
		g_pIBClient->reqAccountUpdates(true,m_accountIB.m_accountName);
		g_pIBClient->reqCurrentTime();
	}
}
void CHiStarApp::OnDisconnectIB(UINT wParam,LONG lParam){
	g_pIBClient->eDisconnect();
}
