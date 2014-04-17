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
	//��Լ��ʼ��
	if(m_cQ){
		m_cQ->InstMustSubscribe.push_back(m_accountCtp.m_szInst);
	}
	//��ʼ������API,ע����ǰ�ñ���
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
	//CTP����ϵͳ
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
		//�ȵ�¼����ϵͳ����ʼ���ֲֺ��ٵ�¼����ϵͳ��ͬ�����飻��Ҫ��������
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
	//����ģ��ǳ�,����ģ�鲻��Ҫ,����ᱨ��(��ʱ��֪��ԭ��?)
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
		else if(true/*requestID == msg.lParam*/){break;}//����ϵͳ��¼ʱ��ID�ͷ��ص�ID��֪Ϊʲô��һ�£�
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
			((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("���߽���ƽ̨�ɹ�!"), 10);break;
		}
	}
	if(m_cT){
		if (m_cT->IsErrorRspInfo(&m_cT->m_RspMsg)){
			//��½ʧ��
			((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("���׵�½����!"), 0);
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
			((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("ȷ�Ͻ��㵥!"), 20);break;
		}
	}
	if(m_cT){
		requestID = m_cT->ReqQryInst(NULL);
	}
	while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
		if (!bRet){
		}
		else if(requestID == msg.lParam){
			((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("���Լ�б�!"), 30);break;
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
			((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("��ֲ���Ϣ!"), 40);break;
		}
	}
	Sleep(1000);
	if(m_cT){

		AcquireSRWLockExclusive(&g_srwLock_PosDetail);
		m_cT->m_InvPosDetailVec.clear();//�����
		ReleaseSRWLockExclusive(&g_srwLock_PosDetail); 

		requestID = m_cT->ReqQryInvPosEx(NULL);
	}
	while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
		if (!bRet){
		}
		else if(requestID == msg.lParam){
			((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("��ֲ���ϸ��Ϣ!"), 45);
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
			((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("���ʽ��˻�!"), 50);break;
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
			((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("��������Ϣ!"), 60);break;
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
			((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("�齻�ױ���!"), 65);break;
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
			((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("��ָ��״̬!"), 75);break;
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
			((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("�齻��״̬!"), 95);break;
		}
	}
	((CMainDlg*)(m_pMainWnd))->addCombInst();//���Ӻ�Լ�б�
	((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("CTP��½�ɹ�!"),100);
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
	//MSHQ ��ͨ���Ż�ȡ��ʵʱ����
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


