
// HiStar.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "HiStar.h"
#include "MainDlg.h"
#include "UserMsg.h"
#include "calendar.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
HANDLE g_hEvent;
// CHiStarApp
BEGIN_MESSAGE_MAP(CHiStarApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
	ON_THREAD_MESSAGE(WM_CONNECT_IB,OnConnectIB)
	ON_THREAD_MESSAGE(WM_DISCONNECT_IB,OnDisconnectIB)
	ON_THREAD_MESSAGE(WM_LOGIN_CTP,LoginCtp)
	ON_THREAD_MESSAGE(WM_LOGOUT_CTP,LogoutCtp)
	ON_THREAD_MESSAGE(WM_QRY_ACC_CTP,OnQryAccCtp)
	ON_THREAD_MESSAGE(WM_UPDATE_LSTCTRL,OnUpdateLstCtrl)
	ON_THREAD_MESSAGE(WM_MD_REFRESH,OnHedgeLooping)
END_MESSAGE_MAP()

// CHiStarApp ����

CHiStarApp::CHiStarApp()
	: faError(false)
	, m_pIBClient(NULL)
	, m_MApi(NULL)
	, m_TApi(NULL)
	, m_cQ(NULL)
	, m_cT(NULL)
	, m_pLoginCtp(NULL)
	, m_strPath(_T(""))
	, m_pIndexThread(NULL)
	, m_id(0)
	, m_pHedgePostProcessing(NULL)
	, m_pMSHQ(NULL)
{
	//��λ�ڴ�й©λ��,�ǳ�����
	//_CrtSetBreakAlloc(958);
	//_CrtSetBreakAlloc(638);
	//_CrtSetBreakAlloc(638);
	MainThreadId = GetCurrentThreadId();
	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
	//IB Client
	if(!m_pIBClient){
		m_pIBClient = new EClientSocket( this);
	}
	//CTP Client
	CreateCtpClient();
	//INDEX
	if(!m_pIndexThread){
		m_pIndexThread = (CIndex*)AfxBeginThread(RUNTIME_CLASS(CIndex));
	}
	//MSHQ ��ͨ���Ż�ȡ��ʵʱ����
	if(!m_pMSHQ){
		m_pMSHQ = (CMSHQ*)AfxBeginThread(RUNTIME_CLASS(CMSHQ));
	}
	//���׺����߳�
	if(!(((CHiStarApp*)AfxGetApp())->m_pHedgePostProcessing)){
		((CHiStarApp*)AfxGetApp())->m_pHedgePostProcessing = (CHedgePostProcessing*)AfxBeginThread(RUNTIME_CLASS(CHedgePostProcessing));
	}
	//IB Order��ʼ��
	m_IBOrder.orderType = "LMT";
	m_IBOrder.whatIf = false;
}

// Ψһ��һ�� CHiStarApp ����

CHiStarApp theApp;


// CHiStarApp ��ʼ��

BOOL CHiStarApp::InitInstance()
{
	//�ý���ӵ��������ȼ�
	//if(!SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS ))
	//{
	//	return false;
	//}
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()�����򣬽��޷��������ڡ�
	AfxInitRichEdit2();
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// ���� shell ���������Է��Ի������
	// �κ� shell ����ͼ�ؼ��� shell �б���ͼ�ؼ���
	CShellManager *pShellManager = new CShellManager;

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	//CBasicPage dlg;
	CMainDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}

	// ɾ�����洴���� shell ��������
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

CHiStarApp::~CHiStarApp(void)
{
	if(m_pIBClient){
		delete m_pIBClient;
		m_pIBClient = NULL;
	}
	if(m_TApi){
		m_TApi->RegisterSpi(NULL);
	}
	if(m_TApi){
		m_TApi->Release();
	}
	if(m_TApi){
		m_TApi = NULL;
	}
	if(m_cT){
		delete m_cT;
		m_cT = NULL;
	}
	if(m_MApi){
		m_MApi->RegisterSpi(NULL);
	}
	if(m_MApi){
		m_MApi->Release();
	}
	if(m_MApi){
		m_MApi = NULL;
	}
	if(m_cQ){
		delete m_cQ;
		m_cQ = NULL;
	}
	if(m_pHedgePostProcessing){
		m_pHedgePostProcessing = NULL;
	}
}

void CHiStarApp::PostOrderStatus(CString str)
{
	CString *pStatus = new CString(str);
	PostMessage(AfxGetMainWnd()->m_hWnd,WM_ORDER_STATUS,(UINT)pStatus,NULL);
}

void CHiStarApp::PostErrors(CString str)
{
	CString *pErrors = new CString(str);
	PostMessage(AfxGetMainWnd()->m_hWnd,WM_ERRORS,(UINT)pErrors,NULL);
}

void CHiStarApp::SetIFContract(void)
{
	//�������ú�Լ,�Զ�����
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	WORD ifFinal = ifFinalDay(sys.wYear,sys.wMonth);
	CString insID;
	if(ifFinal - sys.wDay <= 3){
	}
	//�����һ���ǰһ�컻��Լ
	if(sys.wDay < ifFinal){
		insID.Format(_T("%.4d%.2d"),sys.wYear,sys.wMonth);
	}
	else{
		if(sys.wMonth < 12){
			insID.Format(_T("%.4d%.2d"),sys.wYear,sys.wMonth + 1);
		}
		else{//��ĩ
			insID.Format(_T("%.4d%.2d"),sys.wYear + 1,1);
		}
	}
	//���ֽڱ��������ĸ���ֺ�ASCII�Ǽ��ݵģ����Բ���CString��char[]�����Ч��һ����
	//Ϊ�˱������ּ��ݣ�������ֻ�ܲ��ö��ֽڱ��룬��������ġ�
	m_accountCtp.m_szInst = _T("IF") + insID.Right(4);
}
