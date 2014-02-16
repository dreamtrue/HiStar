#include "stdafx.h"
#include "HiStar.h"
BOOL bIsInit = FALSE;
extern HANDLE g_hEvent;
UINT LoginThread(LPVOID pParam);
void CHiStarApp::CreateCtpClient(void)
{
	//Ctp����ϵͳ
	CString strPath = __targv[0]; 
	strPath = strPath.Left(strPath.ReverseFind('\\'));
	strPath += '\\';
	strPath += _T("log\\");
	if (!PathIsDirectory(strPath))
	{
		CreateDirectory(strPath,NULL);
	}
	//CTP����ϵͳ
	m_TApi = CThostFtdcTraderApi::CreateFtdcTraderApi(strPath);
	m_cT = new CtpTraderSpi(m_TApi);
	m_TApi->RegisterSpi((CThostFtdcTraderSpi*)m_cT);
	m_TApi->SubscribePublicTopic(THOST_TERT_RESTART);
	m_TApi->SubscribePrivateTopic(THOST_TERT_RESTART);
	//CTP����ϵͳ
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
UINT LoginThread(LPVOID pParam)
{
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	//��ʼ������ͽ���API,ע����ǰ�ñ���
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
		TRACE("CTP����ƽ̨��½�ɹ�\n");
		if(ResetEvent(g_hEvent)){
			TRACE(_T("���óɹ�\n"));
		}
		else{
			TRACE(_T("����ʧ��\n"));
		}
	}
	else{
		pApp->m_pLoginCtp = NULL;
		TRACE(_T("CTP����ƽ̨����\n"));
		return 0;
	}
	if (pApp->m_cT->IsErrorRspInfo(&pApp->m_cT->m_RspMsg)){
		//��½ʧ��
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
		TRACE(_T("��½CTP����ɹ�\n"));
		ResetEvent(g_hEvent);
	}	
	///////////////////////////////////////////////////////////
	pApp->m_cT->ReqSettlementInfoConfirm();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		TRACE(_T("ȷ�Ͻ��㵥\n"));
		ResetEvent(g_hEvent);
	}
	else{
		pApp->m_pLoginCtp = NULL;
		TRACE(_T("ȷ�Ͻ��㳬ʱ\n"));
		return 0;
	}

	pApp->m_cT->ReqQryInst(NULL);
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		TRACE(_T("���Լ�б�!\n"));
		ResetEvent(g_hEvent);
	}
	else{
		pApp->m_pLoginCtp = NULL;
		TRACE(_T("���Լ��ʱ!\n"));
		return 0;
	}

	Sleep(1000);
	pApp->m_cT->ReqQryInvPos(NULL);
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		TRACE(_T("��ֲ���Ϣ!\n"));
		ResetEvent(g_hEvent);
	}	
	else{
		pApp->m_pLoginCtp = NULL;
		TRACE(_T("��ֲ���Ϣ��ʱ!\n"));
		return 0;
	}

	Sleep(1000);
	pApp->m_cT->ReqQryTdAcc();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		TRACE(_T("���ʽ��˻�!\n"));
		ResetEvent(g_hEvent);
	}	
	else{
		pApp->m_pLoginCtp = NULL;
		TRACE(_T("���˻���ʱ!\n"));
		return 0;
	}

#ifdef _REAL_CTP_
	Sleep(1000);
	pApp->m_cT->ReqQryAccreg();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		TRACE(_T("��������Ϣ!\n"));
		ResetEvent(g_hEvent);
	}
	else{
		pApp->m_pLoginCtp = NULL;
		TRACE(_T("��������Ϣ��ʱ!\n"));
		return 0;
	}

	Sleep(1000);
	pApp->m_cT->ReqQryTradingCode();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		TRACE(_T("�齻�ױ���!\n"));
		ResetEvent(g_hEvent);
	}	
	else{
		pApp->m_pLoginCtp = NULL;
		TRACE(_T("�齻�ױ��볬ʱ!\n"));
		return 0;
	}	
#endif
	pApp->m_pLoginCtp = NULL;
	return 0;
}
