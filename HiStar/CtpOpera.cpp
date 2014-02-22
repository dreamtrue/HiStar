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
	CString strPath = __targv[0]; 
	strPath = strPath.Left(strPath.ReverseFind('\\'));
	strPath += '\\';
	strPath += _T("log\\");
	if (!PathIsDirectory(strPath))
	{
		CreateDirectory(strPath,NULL);
	}
	//CTP交易系统
	m_TApi = CThostFtdcTraderApi::CreateFtdcTraderApi(strPath);
	m_cT = new CtpTraderSpi(m_TApi);
	m_TApi->RegisterSpi((CThostFtdcTraderSpi*)m_cT);
	m_TApi->SubscribePublicTopic(THOST_TERT_RESTART);
	m_TApi->SubscribePrivateTopic(THOST_TERT_RESTART);
	//CTP行情系统
	m_MApi = CThostFtdcMdApi::CreateFtdcMdApi(strPath);
	m_cQ = new CtpMdSpi(m_MApi);
	m_MApi->RegisterSpi(m_cQ);
}
int CHiStarApp::FindInstMul(TThostFtdcInstrumentIDType InstID){
	bool founded=false;
	int iMul = 1;
	for (UINT i=0; i < m_cT->m_InsinfVec.size();i++)
	{
		if (strcmp(InstID,m_cT->m_InsinfVec[i]->iinf.InstrumentID) == 0)
		{
			iMul = m_cT->m_InsinfVec[i]->iinf.VolumeMultiple;
			founded = true;
			break;
		}
	}
	if (founded){return iMul;}
	return (-1);
}
void CHiStarApp::LoginCtp(UINT wParam,LONG lParam)
{
	if (!m_pLoginCtp)
	{
		AfxBeginThread((AFX_THREADPROC)LoginThread,this);
		m_pLoginCtp = NULL;
	}
}
void CHiStarApp::LogoutCtp(UINT wParam,LONG lParam)
{
	//交易模块登出,行情模块不需要,否则会报错(暂时不知道原因?)
	m_cT->ReqUserLogout();
	m_cQ->ReqUserLogout();
}
UINT LoginThread(LPVOID pParam)
{
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	if(!(CMainDlg*)pApp->m_pMainWnd){
		return 0;
	}
	g_hEvent = CreateEvent(NULL, true, false, NULL); 
	//初始化行情和交易API,注册多个前置备用
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
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("登陆行情成功!"), 20);
		ResetEvent(g_hEvent);
	}	
	///////////////////////////////////////////////////////////
	if(pApp->m_cT){
		pApp->m_cT->ReqSettlementInfoConfirm();
	}
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("确认结算单!"), 40);
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
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查合约列表!"), 60);
		ResetEvent(g_hEvent);
	}
	else{
		pApp->m_pLoginCtp = NULL;
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查合约列表超时!"), 0);
		return 0;
	}

	Sleep(1000);
	if(pApp->m_cT){
		pApp->m_cT->ReqQryInvPos(NULL);
	}
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查持仓信息!"), 70);
		ResetEvent(g_hEvent);
	}	
	else{
		pApp->m_pLoginCtp = NULL;
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查持仓信息超时!"), 0);
		return 0;
	}

	Sleep(1000);
	if(pApp->m_cT){
		pApp->m_cT->ReqQryTdAcc();
	}
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查资金账户!"), 80);
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
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查银期信息!"), 90);
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
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查交易编码!"), 99);
		ResetEvent(g_hEvent);
	}	
	else{
		pApp->m_pLoginCtp = NULL;
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查交易编码超时!"), 0);
		return 0;
	}
	((CMainDlg*)(pApp->m_pMainWnd))->addCombInst();//增加合约列表
	((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.InitAllVecs();
	((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.InitAllHdrs();
	((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.FiltInsList();
	((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_LstOnRoad.SetItemCountEx(((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_onRoadVec.size());
	((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_LstOnRoad.Invalidate();
	((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_LstOrdInf.SetItemCountEx(((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_orderVec.size());
	((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_LstOrdInf.Invalidate();
	((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_LstInvPosInf.SetItemCountEx(((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_InvPosVec.size());
	((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_LstInvPosInf.Invalidate();
	((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_LstTdInf.SetItemCountEx(((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_tradeVec.size());
	((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_LstTdInf.Invalidate();
	((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_LstAllInsts.SetItemCountEx(((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_InsinfVec.size());
	((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.m_LstAllInsts.Invalidate();
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
