
// HiStar.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "HiStar.h"
#include "MainDlg.h"
#include "UserMsg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
HANDLE g_hEvent = 0;
// CHiStarApp

BEGIN_MESSAGE_MAP(CHiStarApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
	ON_THREAD_MESSAGE(WM_CONNECT_IB,OnConnectIB)
	ON_THREAD_MESSAGE(WM_DISCONNECT_IB,OnDisconnectIB)
END_MESSAGE_MAP()


// CHiStarApp ����

CHiStarApp::CHiStarApp()
	: faError(false)
	, m_pIBClient(NULL)
	, m_MApi(NULL)
	, m_TApi(NULL)
	, m_cQ(NULL)
	, m_cT(NULL)
	, m_pLoginCtp(false)
{
	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
	//IB Client
	m_pIBClient = new EClientSocket( this);
    m_accountIB.m_accountName = _T("U1032950");
	//CTP Client
	CreateCtpClient();
}

// Ψһ��һ�� CHiStarApp ����

CHiStarApp theApp;


// CHiStarApp ��ʼ��

BOOL CHiStarApp::InitInstance()
{
	//�ý���ӵ��������ȼ�
	if(!SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS ))
	{
		return false;
	}
	g_hEvent=CreateEvent(NULL, true, false, NULL); 
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()�����򣬽��޷��������ڡ�
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

	//COperaPage dlg;
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
	delete m_pIBClient;
	m_pIBClient = NULL;
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
