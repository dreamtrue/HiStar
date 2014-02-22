#include "stdafx.h"
#include "HiStar.h"
#include "MainDlg.h"
BOOL bIsInit = FALSE;
extern HANDLE g_hEvent;
extern BOOL g_bLoginCtpT;
UINT LoginThread(LPVOID pParam);
void CHiStarApp::CreateCtpClient(void)
{
	//Ctp����ϵͳ
	m_strPath = __targv[0]; 
	m_strPath = m_strPath.Left(m_strPath.ReverseFind('\\'));
	m_strPath += '\\';
	m_strPath += _T("log\\");
	if (!PathIsDirectory(m_strPath))
	{
		CreateDirectory(m_strPath,NULL);
	}
	//CTP����ϵͳ
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
		m_TApi->SubscribePublicTopic(THOST_TERT_RESTART);
	}
	if(m_TApi){
		m_TApi->SubscribePrivateTopic(THOST_TERT_RESTART);
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
	if (!m_pLoginCtp)
	{
		AfxBeginThread((AFX_THREADPROC)LoginThread,this);
		m_pLoginCtp = NULL;
	}
}
void CHiStarApp::LogoutCtp(UINT wParam,LONG lParam)
{
	//����ģ��ǳ�,����ģ�鲻��Ҫ,����ᱨ��(��ʱ��֪��ԭ��?)
	m_cT->ReqUserLogout();
	m_cQ->ReqUserLogout();//�����޷������ǳ��������δ�ǳ�������¼������룬�ᷢ�����ʳ�ͻ���ʲ���release��ʽֱ��ɾ��
	m_MApi = NULL;//��Ҫ������API��ΪNULL����Ϊ�Ѿ�release
	m_cQ = NULL;
	delete m_cQ;
}
UINT LoginThread(LPVOID pParam)
{
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	if(!(CMainDlg*)pApp->m_pMainWnd){
		return 0;
	}
	g_hEvent = CreateEvent(NULL, true, false, NULL); 
	//��ʼ������API,ע����ǰ�ñ���
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
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("���߽���ƽ̨�ɹ�!"), 10);
		ResetEvent(g_hEvent);
	}
	else{
		pApp->m_pLoginCtp = NULL;
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("����ƽ̨������!"), 0);
		return 0;
	}
	if(pApp->m_cT){
		if (pApp->m_cT->IsErrorRspInfo(&pApp->m_cT->m_RspMsg)){
			//��½ʧ��
			((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("���׵�½����!"), 0);
			pApp->m_pLoginCtp = NULL;
			return 0;
		}
	}
	//CTP����ϵͳ
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
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("��½����ɹ�!"), 20);
		ResetEvent(g_hEvent);
	}	
	///////////////////////////////////////////////////////////
	if(pApp->m_cT){
		pApp->m_cT->ReqSettlementInfoConfirm();
	}
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("ȷ�Ͻ��㵥!"), 40);
		ResetEvent(g_hEvent);
	}
	else{
		pApp->m_pLoginCtp = NULL;
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("ȷ�Ͻ��㳬ʱ!"), 0);
		return 0;
	}
	if(pApp->m_cT){
		pApp->m_cT->ReqQryInst(NULL);
	}

	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		TRACE(_T("��Լ��Ϸ���\n"));
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("���Լ�б�!"), 60);
		ResetEvent(g_hEvent);
	}
	else{
		pApp->m_pLoginCtp = NULL;
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("���Լ�б�ʱ!"), 0);
		return 0;
	}

	Sleep(1000);
	if(pApp->m_cT){
		pApp->m_cT->ReqQryInvPos(NULL);
	}
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("��ֲ���Ϣ!"), 70);
		ResetEvent(g_hEvent);
	}	
	else{
		pApp->m_pLoginCtp = NULL;
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("��ֲ���Ϣ��ʱ!"), 0);
		return 0;
	}

	Sleep(1000);
	if(pApp->m_cT){
		pApp->m_cT->ReqQryTdAcc();
	}
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("���ʽ��˻�!"), 80);
		ResetEvent(g_hEvent);
	}	
	else{
		pApp->m_pLoginCtp = NULL;
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("���˻���ʱ!"), 0);
		return 0;
	}

#ifdef _REAL_CTP_
	Sleep(1000);
	if(pApp->m_cT){
		pApp->m_cT->ReqQryAccreg();
	}
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("��������Ϣ!"), 90);
		ResetEvent(g_hEvent);
	}
	else{
		pApp->m_pLoginCtp = NULL;
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("��������Ϣ��ʱ!"), 0);
		return 0;
	}

	Sleep(1000);
	if(pApp->m_cT){
		pApp->m_cT->ReqQryTradingCode();
	}
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0){
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("�齻�ױ���!"), 99);
		ResetEvent(g_hEvent);
	}	
	else{
		pApp->m_pLoginCtp = NULL;
		((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("�齻�ױ��볬ʱ!"), 0);
		return 0;
	}
	((CMainDlg*)(pApp->m_pMainWnd))->addCombInst();//���Ӻ�Լ�б�
	((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.InitAllHdrs();
	((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.FiltInsList();
	((CMainDlg*)(pApp->m_pMainWnd))->m_statusPage.SynchronizeAllVecs();
	((CMainDlg*)pApp->m_pMainWnd)->m_basicPage.ProgressUpdate(_T("CTP��½�ɹ�!"),100);
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
