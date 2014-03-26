#include "stdafx.h"
#include "HiStar.h"
#include "MainDlg.h"
#include "UserMsg.h"
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
	SetIFContract();//����IF��Լ
	if (!m_pLoginCtp)
	{
		m_pLoginCtp = NULL;
		if(!(CMainDlg*)m_pMainWnd){
			return;
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
		if (!bIsInit){	
			if(m_TApi){
				m_TApi->Init();
			}
			bIsInit = TRUE;
		}
		else{
			if(m_cT){
				m_cT->ReqUserLogin(m_accountCtp.m_sBROKER_ID,m_accountCtp.m_sINVESTOR_ID,m_accountCtp.m_sPASSWORD);
			}
		}
		MSG msg;BOOL bRet;
		while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
			if (!bRet){
			}
			else{
				((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("���߽���ƽ̨�ɹ�!"), 10);break;
			}
		}
		if(m_cT){
			if (m_cT->IsErrorRspInfo(&m_cT->m_RspMsg)){
				//��½ʧ��
				((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("���׵�½����!"), 0);
				m_pLoginCtp = NULL;
				return;
			}
		}
		//CTP����ϵͳ
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
		//��Լ��ʼ��
		m_cQ->InstNeedSubscribe.push_back(m_accountCtp.m_szInst);
		if(m_MApi){
			m_MApi->Init();
		}
		while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
			if (!bRet){
			}
			else{
				((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("��½����ɹ�!"), 15);break;
			}
		}
		///////////////////////////////////////////////////////////
		if(m_cT){
			m_cT->ReqSettlementInfoConfirm();
		}
		while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
			if (!bRet){
			}
			else{
				((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("ȷ�Ͻ��㵥!"), 20);break;
			}
		}
		if(m_cT){
			m_cT->ReqQryInst(NULL);
		}
		while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
			if (!bRet){
			}
			else{
				((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("���Լ�б�!"), 30);break;
			}
		}
		Sleep(1000);
		if(m_cT){
			m_cT->ReqQryInvPos(NULL);
		}
		while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
			if (!bRet){
			}
			else{
				((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("��ֲ���Ϣ!"), 40);break;
			}
		}
		Sleep(1000);
		if(m_cT){
			m_cT->ReqQryInvPosEx(NULL);
		}
		while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
			if (!bRet){
			}
			else{
				((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("��ֲ���ϸ��Ϣ!"), 45);break;
			}
		}
		Sleep(1000);
		if(m_cT){
			m_cT->ReqQryTdAcc();
		}
		while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
			if (!bRet){
			}
			else{
				((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("���ʽ��˻�!"), 50);break;
			}
		}
#ifdef _REAL_CTP_
		Sleep(1000);
		if(m_cT){
			m_cT->ReqQryAccreg();
		}
		while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
			if (!bRet){
			}
			else{
				((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("��������Ϣ!"), 60);break;
			}
		}
		Sleep(1000);
		if(m_cT){
			m_cT->ReqQryTradingCode();
		}
		while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
			if (!bRet){
			}
			else{
				((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("�齻�ױ���!"), 65);break;
			}
		}
		Sleep(1000);
		if(m_cT){
			m_cT->ReqQryOrder(NULL);
		}
		while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
			if (!bRet){
			}
			else{
				((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("��ָ��״̬!"), 75);break;
			}
		}
		Sleep(1000);
		if(m_cT){
			m_cT->ReqQryTrade(NULL);
		}
		while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
			if (!bRet){
			}
			else{
				((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("�齻��״̬!"), 95);break;
			}
		}
		((CMainDlg*)(m_pMainWnd))->addCombInst();//���Ӻ�Լ�б�
		((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("CTP��½�ɹ�!"),100);
#endif
		m_pLoginCtp = NULL;
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
		m_cQ->ReqUserLogout();//�����޷������ǳ��������δ�ǳ�������¼������룬�ᷢ�����ʳ�ͻ���ʲ���release��ʽֱ��ɾ��
		m_cQ = NULL;
		delete m_cQ;
		m_MApi = NULL;//��Ҫ������API��ΪNULL����Ϊ�Ѿ�release
	}
}

void CHiStarApp::OnQryAccCtp(WPARAM wParam,LPARAM lParam){
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	if(g_bLoginCtpT){
		if(pApp->m_cT){
			pApp->m_cT->ReqQryTdAcc();
		}
	}
}
void CHiStarApp::OnUpdateLstCtrl(WPARAM wParam,LPARAM lParam){
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	((CMainDlg*)pApp->m_pMainWnd)->m_statusPage.SynchronizeAllVecs();
}


