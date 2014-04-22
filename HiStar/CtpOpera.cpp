#include "stdafx.h"
#include "HiStar.h"
#include "MainDlg.h"
#include "UserMsg.h"
#include "TraderSpi.h"
#include "CVector.h"
extern CVector<HoldDetail> HedgeHold;
BOOL bIsInit = FALSE;
extern BOOL g_bLoginCtpT;
UINT LoginThread(LPVOID pParam);
void CHiStarApp::CreateCtpClient(void)
{
	//Ctp行情系统
	m_strPath = __targv[0]; 
	m_strPath = m_strPath.Left(m_strPath.ReverseFind('\\'));
	m_strPath += '\\';
	m_strPath += _T("log\\");
	if (!PathIsDirectory(m_strPath))
	{
		CreateDirectory(m_strPath,NULL);
	}
	//CTP交易系统
	m_TApi = CThostFtdcTraderApi::CreateFtdcTraderApi(m_strPath);
	if(m_cT){
		delete m_cT;
		m_cT = new CtpTraderSpi(m_TApi);
	}
	else{
		m_cT = new CtpTraderSpi(m_TApi);
	}
	if(m_TApi){
		m_TApi->RegisterSpi((CThostFtdcTraderSpi*)m_cT);
	}
	if(m_TApi){
		m_TApi->SubscribePublicTopic(THOST_TERT_QUICK);
	}
	if(m_TApi){
		m_TApi->SubscribePrivateTopic(THOST_TERT_QUICK);
	}
	m_MApi = CThostFtdcMdApi::CreateFtdcMdApi(m_strPath);
	if(m_MApi){
		if(m_cQ){
			delete m_cQ;
			m_cQ = new CtpMdSpi(m_MApi);
		}
		else{
			m_cQ = new CtpMdSpi(m_MApi);
		}
	}
	if(m_MApi){
		m_MApi->RegisterSpi(m_cQ);
	}
	//合约初始化
	if(m_cQ){
		m_cQ->InstMustSubscribe.push_back(m_accountCtp.m_szInst);
	}
	//初始化交易API,注册多个前置备用
	int iTdSvr = m_accountCtp.m_szArTs.GetSize();
	int i =0,iLen;
	char szTd[MAX_PATH],szMd[MAX_PATH];
	for (i=0;i<iTdSvr;i++){
		iLen = m_accountCtp.m_szArTs[i].GetLength();
		TRACE("%s\r\n",m_accountCtp.m_szArTs[i]);
		uni2ansi(CP_ACP,m_accountCtp.m_szArTs[i].GetBuffer(iLen),szTd);
		m_accountCtp.m_szArTs[i].ReleaseBuffer();
		if(m_TApi){
			m_TApi->RegisterFront(szTd);
		}
	}
	//CTP行情系统
	int iMdSvr = m_accountCtp.m_szArMd.GetSize();
	for (i=0;i<iTdSvr;i++){
		iLen = m_accountCtp.m_szArMd[i].GetLength();
		uni2ansi(CP_ACP,m_accountCtp.m_szArMd[i].GetBuffer(iLen),szMd);
		TRACE("%s\r\n",m_accountCtp.m_szArMd[i]);
		m_accountCtp.m_szArMd[i].ReleaseBuffer();
		if(m_MApi){
			m_MApi->RegisterFront(szMd);
		}
	}
}
int CHiStarApp::FindInstMul(TThostFtdcInstrumentIDType InstID){
	bool founded=false;
	int iMul = 1;
	AcquireSRWLockShared(&g_srwLock_Insinf);
	for (UINT i=0; i < m_cT->m_InsinfVec.size();i++)
	{
		if (strcmp(InstID,m_cT->m_InsinfVec[i].iinf.InstrumentID) == 0)
		{
			iMul = m_cT->m_InsinfVec[i].iinf.VolumeMultiple;
			founded = true;
			break;
		}
	}
	ReleaseSRWLockShared(&g_srwLock_Insinf);
	if (founded){return iMul;}
	return (-1);
}

void CHiStarApp::LoginCtp(WPARAM wParam,LPARAM lParam)
{
	MSG msg;BOOL bRet;
	if(!(CMainDlg*)m_pMainWnd){
		return;
	}
	if (!bIsInit){	
		if(m_TApi){
			m_TApi->Init();
		}
		else{
			return;
		}
		if(m_MApi){
			m_MApi->Init();
		}
		else{
			return;
		}
		bIsInit = TRUE;
		//先登录交易系统并初始化持仓后，再登录行情系统并同步行情；不要反过来。
		while((bRet = GetMessage(&msg,NULL,WM_LOGIN_TD,WM_LOGIN_TD)) != 0){
			if (!bRet){}
			else{
				LoginCtpTD(NULL,NULL);
				break;
			}
		}
		while((bRet = GetMessage(&msg,NULL,WM_LOGIN_MD,WM_LOGIN_MD)) != 0){
			if (!bRet){}
			else{
				LoginCtpMD(NULL,NULL);
				break;
			}
		}
	}
	else{
		LoginCtpTD(NULL,NULL);
		LoginCtpMD(NULL,NULL);
	}
	return;
}

void CHiStarApp::LogoutCtp(WPARAM wParam,LPARAM lParam)
{
	//交易模块登出,行情模块不需要,否则会报错(暂时不知道原因?)
	if(m_cT){
		m_cT->ReqUserLogout();
	}
	if(m_cQ){
		m_cQ->ReqUserLogout();
	}
}

void CHiStarApp::LoginCtpMD(WPARAM wParam,LPARAM lParam){
	MSG msg;BOOL bRet;
	int requestID;
	if(m_cQ){
		requestID = m_cQ->ReqUserLogin(m_accountCtp.m_sBROKER_ID,m_accountCtp.m_sINVESTOR_ID,m_accountCtp.m_sPASSWORD);
	}
	while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT_MD,WM_NOTIFY_EVENT_MD)) != 0){
		if (!bRet){}
		else if(true/*requestID == msg.lParam*/){break;}//行情系统登录时的ID和返回的ID不知为什么不一致？
	}
	OnSynchronizeMarket(NULL,NULL);
}

void CHiStarApp::LoginCtpTD(WPARAM wParam,LPARAM lParam){
	MSG msg;BOOL bRet;int requestID;
	if(m_cT){
		requestID = m_cT->ReqUserLogin(m_accountCtp.m_sBROKER_ID,m_accountCtp.m_sINVESTOR_ID,m_accountCtp.m_sPASSWORD);
	}
	while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
		if (!bRet){
		}
		else if(requestID == msg.lParam){
			((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("联线交易平台成功!"), 10);break;
		}
	}
	if(m_cT){
		if (m_cT->IsErrorRspInfo(&m_cT->m_RspMsg)){
			//登陆失败
			((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("交易登陆错误!"), 0);
			return;
		}
	}
	if(m_cT){
		requestID = m_cT->ReqSettlementInfoConfirm();
	}
	while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
		if (!bRet){
		}
		else if(requestID == msg.lParam){
			((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("确认结算单!"), 20);break;
		}
	}
	if(m_cT){
		requestID = m_cT->ReqQryInst(NULL);
	}
	while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
		if (!bRet){
		}
		else if(requestID == msg.lParam){
			((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查合约列表!"), 30);break;
		}
	}
	Sleep(1000);
	if(m_cT){
		requestID = m_cT->ReqQryInvPos(NULL);
	}
	while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
		if (!bRet){
		}
		else if(requestID == msg.lParam){
			((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查持仓信息!"), 40);break;
		}
	}
	Sleep(1000);
	if(m_cT){

		AcquireSRWLockExclusive(&g_srwLock_PosDetail);
		m_cT->m_InvPosDetailVec.clear();//先清空
		ReleaseSRWLockExclusive(&g_srwLock_PosDetail); 

		requestID = m_cT->ReqQryInvPosEx(NULL);
	}
	while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
		if (!bRet){
		}
		else if(requestID == msg.lParam){
			((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查持仓明细信息!"), 45);
			break;
		}
	}
	Sleep(1000);
	if(m_cT){
		requestID = m_cT->ReqQryTdAcc();
	}
	while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
		if (!bRet){
		}
		else if(requestID == msg.lParam){
			((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查资金账户!"), 50);break;
		}
	}
#ifdef _REAL_CTP_
	Sleep(1000);
	if(m_cT){
		requestID = m_cT->ReqQryAccreg();
	}
	while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
		if (!bRet){
		}
		else if(requestID == msg.lParam){
			((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查银期信息!"), 60);break;
		}
	}
	Sleep(1000);
	if(m_cT){
		requestID = m_cT->ReqQryTradingCode();
	}
	while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
		if (!bRet){
		}
		else if(requestID == msg.lParam){
			((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查交易编码!"), 65);break;
		}
	}
	Sleep(1000);
	if(m_cT){
		requestID = m_cT->ReqQryOrder(NULL);
	}
	while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
		if (!bRet){
		}
		else if(requestID == msg.lParam){
			((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查指令状态!"), 75);break;
		}
	}
	Sleep(1000);
	if(m_cT){
		requestID = m_cT->ReqQryTrade(NULL);
	}
	while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
		if (!bRet){
		}
		else if(requestID == msg.lParam){
			((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查交易状态!"), 95);break;
		}
	}
	((CMainDlg*)(m_pMainWnd))->addCombInst();//增加合约列表
	((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("CTP登陆成功!"),100);
#endif
}

void CHiStarApp::OnQryAccCtp(WPARAM wParam,LPARAM lParam){
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	if(g_bLoginCtpT){
		if(pApp->m_cT){
			pApp->m_cT->ReqQryTdAcc();
		}
	}
}

void CHiStarApp::UpdateHedgeHold(WPARAM wParam,LPARAM lParam){
	if(m_pMainWnd){
		((CMainDlg*)m_pMainWnd)->m_basicPage.SynchronizeHoldDataToView();
	}
}

void CHiStarApp::OnUpdateLstCtrl(WPARAM wParam,LPARAM lParam){
	if(m_pMainWnd){
		((CMainDlg*)m_pMainWnd)->m_statusPage.SynchronizeAllVecs();
	}
}

void CHiStarApp::OnReqMshq(WPARAM wParam,LPARAM lParam){
	//MSHQ 从通达信获取的实时行情
	if(!m_pMSHQ){
		m_pMSHQ = (CMSHQ*)AfxBeginThread(RUNTIME_CLASS(CMSHQ));
		m_pMSHQ->m_bAutoDelete = true;
	}
	else{
		::PostThreadMessage(m_pMSHQ->m_nThreadID, WM_QUIT,0,0);
		WaitForSingleObject(m_pMSHQ->m_hThread, INFINITE); 
		m_pMSHQ = NULL;
	}
}


