#include "stdafx.h"
#include "HiStar.h"
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
	//m_cQ->ReqUserLogout();
}
UINT LoginThread(LPVOID pParam)
{
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	//初始化行情和交易API,注册多个前置备用
	int iTdSvr = pApp->m_accountCtp.m_szArTs.GetSize();
	int i =0,iLen;
	char szTd[MAX_PATH],szMd[MAX_PATH];
	for (i=0;i<iTdSvr;i++){
		iLen = pApp->m_accountCtp.m_szArTs[i].GetLength();
		uni2ansi(CP_ACP,pApp->m_accountCtp.m_szArTs[i].GetBuffer(iLen),szTd);
		pApp->m_accountCtp.m_szArTs[i].ReleaseBuffer();
		pApp->m_TApi->RegisterFront(szTd);
	}
	if (!bIsInit){	
		pApp->m_TApi->Init();	
		bIsInit = TRUE;
	}
	else{
		pApp->m_cT->ReqUserLogin(pApp->m_accountCtp.m_sBROKER_ID,pApp->m_accountCtp.m_sINVESTOR_ID,pApp->m_accountCtp.m_sPASSWORD);
	}
	DWORD dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		TRACE("CTP交易平台登陆成功\n");
	}
	else{
		pApp->m_pLoginCtp = NULL;
		TRACE(_T("CTP交易平台离线\n"));
		return 0;
	}
	if (pApp->m_cT->IsErrorRspInfo(&pApp->m_cT->m_RspMsg)){
		//登陆失败
		pApp->m_pLoginCtp = NULL;
		return 0;
	}
	int iMdSvr = pApp->m_accountCtp.m_szArMd.GetSize();
	for (i=0;i<iTdSvr;i++){
		iLen = pApp->m_accountCtp.m_szArMd[i].GetLength();
		uni2ansi(CP_ACP,pApp->m_accountCtp.m_szArMd[i].GetBuffer(iLen),szMd);
		pApp->m_accountCtp.m_szArMd[i].ReleaseBuffer();
		pApp->m_MApi->RegisterFront(szMd);
	}
	pApp->m_MApi->Init();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		TRACE(_T("登陆CTP行情成功\n"));
		ResetEvent(g_hEvent);
	}	
	///////////////////////////////////////////////////////////
	pApp->m_cT->ReqSettlementInfoConfirm();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		TRACE(_T("确认结算单\n"));
		ResetEvent(g_hEvent);
	}
	else{
		pApp->m_pLoginCtp = NULL;
		TRACE(_T("确认结算超时\n"));
		return 0;
	}

	pApp->m_cT->ReqQryInst(NULL);
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		TRACE(_T("查合约列表!\n"));
		ResetEvent(g_hEvent);
	}
	else{
		pApp->m_pLoginCtp = NULL;
		TRACE(_T("查合约超时!\n"));
		return 0;
	}

	Sleep(1000);
	pApp->m_cT->ReqQryInvPos(NULL);
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		TRACE(_T("查持仓信息!\n"));
		ResetEvent(g_hEvent);
	}	
	else{
		pApp->m_pLoginCtp = NULL;
		TRACE(_T("查持仓信息超时!\n"));
		return 0;
	}

	Sleep(1000);
	pApp->m_cT->ReqQryTdAcc();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		TRACE(_T("查资金账户!\n"));
		ResetEvent(g_hEvent);
	}	
	else{
		pApp->m_pLoginCtp = NULL;
		TRACE(_T("查账户超时!\n"));
		return 0;
	}

#ifdef _REAL_CTP_
	Sleep(1000);
	pApp->m_cT->ReqQryAccreg();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		TRACE(_T("查银期信息!\n"));
		ResetEvent(g_hEvent);
	}
	else{
		pApp->m_pLoginCtp = NULL;
		TRACE(_T("查银期信息超时!\n"));
		return 0;
	}

	Sleep(1000);
	pApp->m_cT->ReqQryTradingCode();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		TRACE(_T("查交易编码!\n"));
		ResetEvent(g_hEvent);
	}	
	else{
		pApp->m_pLoginCtp = NULL;
		TRACE(_T("查交易编码超时!\n"));
		return 0;
	}	
#endif
	pApp->m_pLoginCtp = NULL;
	return 0;
}
void CHiStarApp::QryAccCtp(UINT wParam,LONG lParam){
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	if(g_bLoginCtpT)
		pApp->m_cT->ReqQryTdAcc();
}
