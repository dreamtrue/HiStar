#include "stdafx.h"
#include "HiStar.h"
#include "MainDlg.h"
BOOL bIsInit = FALSE;
extern HANDLE g_hEvent;
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
}
int CHiStarApp::FindInstMul(TThostFtdcInstrumentIDType InstID){
	bool founded=false;
	int iMul = 1;
	for (UINT i=0; i < m_cT->m_InsinfVec.size();i++)
	{
		if (strcmp(InstID,m_cT->m_InsinfVec[i].iinf.InstrumentID) == 0)
		{
			iMul = m_cT->m_InsinfVec[i].iinf.VolumeMultiple;
			founded = true;
			break;
		}
	}
	if (founded){return iMul;}
	return (-1);
}
void CHiStarApp::LoginCtp(UINT wParam,LONG lParam)
{
	SetIFContract();//设置IF合约
	if (!m_pLoginCtp)
	{
		AfxBeginThread((AFX_THREADPROC)LoginThread,this);
		m_pLoginCtp = NULL;
	}
}
void CHiStarApp::LogoutCtp(UINT wParam,LONG lParam)
{
	//交易模块登出,行情模块不需要,否则会报错(暂时不知道原因?)
	if(m_cT){
		m_cT->ReqUserLogout();
	}
	if(m_cQ){
		m_cQ->ReqUserLogout();//行情无法正常登出，如果在未登出的情况下继续登入，会发生访问冲突；故采用release方式直接删除
		m_cQ = NULL;
		delete m_cQ;
		m_MApi = NULL;//需要将行情API设为NULL，因为已经release
	}
}
UINT LoginThread(LPVOID pParam)
{
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	if(!(CMainDlg*)pApp->m_pMainWnd){
		return 0;
	}
	//不做清除，因为trade信息在重新登陆时并不会重新推送
	//if(pApp->m_cT){
	//	pApp->m_cT->ClearAllVectors();//清除掉所有的vectors
	//}
	g_hEvent = CreateEvent(NULL, true, false, NULL); 
	//初始化交易API,注册多个前置备用
	int iTdSvr = pApp->m_accountCtp.m_szArTs.GetSize();
	int i =0,iLen;
	char szTd[MAX_PATH],szMd[MAX_PATH];
	for (i=0;i<iTdSvr;i++){
		iLen = pApp->m_accountCtp.m_szArTs[i].GetLength();
		uni2ansi(CP_ACP,pApp->m_accountCtp.m_szArTs[i].GetBuffer(iLen),szTd);
		pApp->m_accountCtp.m_szArTs[i].ReleaseBuffer();
		if(pApp->m_TApi){
			pApp->m_TApi->RegisterFront(szTd);
		}
	}
	if (!bIsInit){	
		if(pApp->m_TApi){
			pApp->m_TApi->Init();
		}
		bIsInit = TRUE;
	}
	else{
		if(pApp->m_cT){
			pApp->m_cT->ReqUserLogin(pApp->m_accountCtp.m_sBROKER_ID,pApp->m_accountCtp.m_sINVESTOR_ID,pApp->m_accountCtp.m_sPASSWORD);
		}
	}
	DWORD dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("联线交易平台成功!"), 10);
		ResetEvent(g_hEvent);
	}
	else{
		pApp->m_pLoginCtp = NULL;
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("交易平台已离线!"), 0);
		return 0;
	}
	if(pApp->m_cT){
		if (pApp->m_cT->IsErrorRspInfo(&pApp->m_cT->m_RspMsg)){
			//登陆失败
			((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("交易登陆错误!"), 0);
			pApp->m_pLoginCtp = NULL;
			return 0;
		}
	}
	//CTP行情系统
	pApp->m_MApi = CThostFtdcMdApi::CreateFtdcMdApi(pApp->m_strPath);
	if(pApp->m_MApi){
		if(pApp->m_cQ){
			delete pApp->m_cQ;
			pApp->m_cQ = new CtpMdSpi(pApp->m_MApi);
		}
		else{
			pApp->m_cQ = new CtpMdSpi(pApp->m_MApi);
		}
	}
	if(pApp->m_MApi){
		pApp->m_MApi->RegisterSpi(pApp->m_cQ);
	}
	int iMdSvr = pApp->m_accountCtp.m_szArMd.GetSize();
	for (i=0;i<iTdSvr;i++){
		iLen = pApp->m_accountCtp.m_szArMd[i].GetLength();
		uni2ansi(CP_ACP,pApp->m_accountCtp.m_szArMd[i].GetBuffer(iLen),szMd);
		pApp->m_accountCtp.m_szArMd[i].ReleaseBuffer();
		if(pApp->m_MApi){
			pApp->m_MApi->RegisterFront(szMd);
		}
	}
	if(pApp->m_MApi){
		pApp->m_MApi->Init();
	}
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("登陆行情成功!"), 15);
		ResetEvent(g_hEvent);
	}	
	///////////////////////////////////////////////////////////
	if(pApp->m_cT){
		pApp->m_cT->ReqSettlementInfoConfirm();
	}
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("确认结算单!"), 20);
		ResetEvent(g_hEvent);
	}
	else{
		pApp->m_pLoginCtp = NULL;
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("确认结算超时!"), 0);
		return 0;
	}
	if(pApp->m_cT){
		pApp->m_cT->ReqQryInst(NULL);
	}
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		TRACE(_T("合约完毕返回\n"));
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查合约列表!"), 30);
		ResetEvent(g_hEvent);
	}
	else{
		pApp->m_pLoginCtp = NULL;
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查合约列表超时!"), 0);
		return 0;
	}
	Sleep(1000);
	/*
	if(pApp->m_cT){
		pApp->m_cT->ReqQryInvPos(NULL);
	}
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查持仓信息!"), 40);
		ResetEvent(g_hEvent);
	}	
	else{
		pApp->m_pLoginCtp = NULL;
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查持仓信息超时!"), 0);
		return 0;
	}
	*/
	if(pApp->m_cT){
		pApp->m_cT->ReqQryInvPosEx(NULL);
	}
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查持仓明细信息!"), 40);
		ResetEvent(g_hEvent);
	}	
	else{
		pApp->m_pLoginCtp = NULL;
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查持仓明细信息超时!"), 0);
		return 0;
	}
	Sleep(1000);
	if(pApp->m_cT){
		pApp->m_cT->ReqQryTdAcc();
	}
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查资金账户!"), 45);
		ResetEvent(g_hEvent);
	}	
	else{
		pApp->m_pLoginCtp = NULL;
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查账户超时!"), 0);
		return 0;
	}
#ifdef _REAL_CTP_
	Sleep(1000);
	if(pApp->m_cT){
		pApp->m_cT->ReqQryAccreg();
	}
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查银期信息!"), 50);
		ResetEvent(g_hEvent);
	}
	else{
		pApp->m_pLoginCtp = NULL;
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查银期信息超时!"), 0);
		return 0;
	}
	Sleep(1000);
	if(pApp->m_cT){
		pApp->m_cT->ReqQryTradingCode();
	}
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查交易编码!"), 55);
		ResetEvent(g_hEvent);
	}	
	else{
		pApp->m_pLoginCtp = NULL;
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查交易编码超时!"), 0);
		return 0;
	}
	Sleep(1000);
	if(pApp->m_cT){
		pApp->m_cT->ReqQryOrder(NULL);
	}
	dwRet = WaitForSingleObject(g_hEvent,INFINITE);
	if (dwRet==WAIT_OBJECT_0){
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查指令状态!"), 75);
		ResetEvent(g_hEvent);
	}	
	else{
		pApp->m_pLoginCtp = NULL;
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查指令状态超时!"), 0);
		return 0;
	}
	Sleep(1000);
	if(pApp->m_cT){
		pApp->m_cT->ReqQryTrade(NULL);
	}
	dwRet = WaitForSingleObject(g_hEvent,INFINITE);
	if (dwRet==WAIT_OBJECT_0){
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查交易状态!"), 95);
		ResetEvent(g_hEvent);
	}	
	else{
		pApp->m_pLoginCtp = NULL;
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查交易状态超时!"), 0);
		return 0;
	}
	((CMainDlg*)(pApp->m_pMainWnd))->addCombInst();//增加合约列表
	((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("CTP登陆成功!"),100);
#endif
	pApp->m_pLoginCtp = NULL;
	return 0;
}
void CHiStarApp::OnQryAccCtp(UINT wParam,LONG lParam){
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	if(g_bLoginCtpT){
		if(pApp->m_cT){
			pApp->m_cT->ReqQryTdAcc();
		}
	}
}
void CHiStarApp::OnUpdateLstCtrl(UINT wParam,LONG lParam){
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	((CMainDlg*)pApp->m_pMainWnd)->m_statusPage.SynchronizeAllVecs();
}


