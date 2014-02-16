#include "stdafx.h"
#include "HiStar.h"
#include "global.h"
extern CEvent g_hEvent;
//IB交易系统
void CHiStarApp::OnConnectIB(UINT wParam,LONG lParam){
	UINT clientID = 0;
	CString cStatus;
	cStatus.Format( "Connecting to Tws using clientId %d ...",clientID);
	PostOrderStatus(cStatus);
	m_pIBClient->eConnect("127.0.0.1",4001,clientID);
	//登陆成功
	if( m_pIBClient->serverVersion() > 0)	{
		cStatus.Format( "Connected to Tws server version %d at %s.",
			m_pIBClient->serverVersion(), m_pIBClient->TwsConnectionTime());
		PostOrderStatus(cStatus);
		m_pIBClient->reqAccountUpdates(true,m_accountIB.m_accountName);
		m_pIBClient->reqCurrentTime();
	}
}
void CHiStarApp::OnDisconnectIB(UINT wParam,LONG lParam){
	m_pIBClient->eDisconnect();
}
