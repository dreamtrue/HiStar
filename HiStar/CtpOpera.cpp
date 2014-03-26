#include "stdafx.h"
#include "HiStar.h"
#include "MainDlg.h"
#include "UserMsg.h"
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
	SetIFContract();//设置IF合约
	if (!m_pLoginCtp)
	{
		m_pLoginCtp = NULL;
		if(!(CMainDlg*)m_pMainWnd){
			return;
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
				((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("联线交易平台成功!"), 10);break;
			}
		}
		if(m_cT){
			if (m_cT->IsErrorRspInfo(&m_cT->m_RspMsg)){
				//登陆失败
				((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("交易登陆错误!"), 0);
				m_pLoginCtp = NULL;
				return;
			}
		}
		//CTP行情系统
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
		//合约初始化
		m_cQ->InstNeedSubscribe.push_back(m_accountCtp.m_szInst);
		if(m_MApi){
			m_MApi->Init();
		}
		while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
			if (!bRet){
			}
			else{
				((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("登陆行情成功!"), 15);break;
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
				((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("确认结算单!"), 20);break;
			}
		}
		if(m_cT){
			m_cT->ReqQryInst(NULL);
		}
		while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
			if (!bRet){
			}
			else{
				((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查合约列表!"), 30);break;
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
				((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查持仓信息!"), 40);break;
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
				((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查持仓明细信息!"), 45);break;
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
				((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查资金账户!"), 50);break;
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
				((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查银期信息!"), 60);break;
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
				((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查交易编码!"), 65);break;
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
				((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查指令状态!"), 75);break;
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
				((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("查交易状态!"), 95);break;
			}
		}
		((CMainDlg*)(m_pMainWnd))->addCombInst();//增加合约列表
		((CMainDlg*)m_pMainWnd)->m_basicPage.ProgressUpdate(_T("CTP登陆成功!"),100);
#endif
		m_pLoginCtp = NULL;
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
		m_cQ->ReqUserLogout();//行情无法正常登出，如果在未登出的情况下继续登入，会发生访问冲突；故采用release方式直接删除
		m_cQ = NULL;
		delete m_cQ;
		m_MApi = NULL;//需要将行情API设为NULL，因为已经release
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


