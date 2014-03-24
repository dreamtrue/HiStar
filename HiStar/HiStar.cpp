
// HiStar.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "HiStar.h"
#include "MainDlg.h"
#include "UserMsg.h"
#include "calendar.h"
#include <fstream>
#include <sstream>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
std::fstream fileInput;
extern std::vector<HoldDetail> HedgeHold;
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
	, conn(NULL)
{
	//��λ�ڴ�й©λ��,�ǳ�����
	//_CrtSetBreakAlloc(538);
	//_CrtSetBreakAlloc(152);
	//_CrtSetBreakAlloc(151);
	//_CrtSetBreakAlloc(149);
	iniFileInput();
	iniSql();
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
	if(conn){
		mysql_close(conn);
		conn = NULL;
	}
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

int CHiStarApp::iniFileInput(void)
{
	fileInput.open("histar.ini");
	std::vector<std::string> inputData;
	std::string str,str01,str02,str03,str04;
	while(getline(fileInput,str)){
		TRACE("%s\r\n",str.c_str());
		std::stringstream stream(str);
		stream >> str01;
		if(str01.c_str()[0] == '*'){continue;}
		else if(str01 == "@account"){
			while(getline(fileInput,str)){
				stream.clear();stream.str("");stream << str;
				stream >> str01 >> str02 >> str03;
				if(str01.c_str()[0] == '*'){continue;}
				if(str01 == "@end")break;
				memset(&m_accountCtp.m_sBROKER_ID, 0, sizeof(m_accountCtp.m_sBROKER_ID));
				memset(&m_accountCtp.m_sINVESTOR_ID, 0, sizeof(m_accountCtp.m_sINVESTOR_ID));
				memset(&m_accountCtp.m_sPASSWORD, 0, sizeof(m_accountCtp.m_sPASSWORD));
				strcpy(m_accountCtp.m_sINVESTOR_ID,str01.c_str());
				strcpy(m_accountCtp.m_sPASSWORD,str02.c_str());
				strcpy(m_accountCtp.m_sBROKER_ID,str03.c_str());
			}
		}
		else if(str01 == "@hedgehold"){
			while(getline(fileInput,str)){
				stream.clear();stream.str("");stream << str;
				stream >> str01 >> str02 >> str03 >> str04;
				if(str01.c_str()[0] == '*'){continue;}
				if(str01 == "@end")break;
				HoldDetail hd;
				hd.id = atol(str01.c_str());
				hd.HedgeNum = atoi(str02.c_str());
				hd.HedgeSection = atoi(str03.c_str());
				hd.originalCost = atof(str04.c_str());
				HedgeHold.push_back(hd);
			}
		}
		else if(str01 == "@md_address"){
			while(getline(fileInput,str)){
				stream.clear();stream.str("");stream << str;
				stream >> str01;
				if(str01.c_str()[0] == '*'){continue;}
				if(str == "@end")break;
				m_accountCtp.m_szArMd.Add(str01.c_str());
			}
		}
		else if(str01 == "@td_address"){
			while(getline(fileInput,str)){
				stream.clear();stream.str("");stream << str;
				stream >> str01;
				if(str01.c_str()[0] == '*'){continue;}
				if(str == "@end")break;
				m_accountCtp.m_szArTs.Add(str01.c_str());
			}
		}
	}
	return 0;
}

void CHiStarApp::iniSql(void)
{
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	char name[100];
	sprintf(name,"%04d%02d%02dmarket",sys.wYear,sys.wMonth,sys.wDay);
	m_marketTableName = name;
	sprintf(name,"%04d%02d%02dstatusIb",sys.wYear,sys.wMonth,sys.wDay);
    m_statusTableIbName = name;
	conn = mysql_init(NULL); 
	if(conn == NULL) {
		TRACE("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));      
		//exit(1);  
	}  
	if(conn){
		if(mysql_real_connect(conn,"rdsnqzb3iqzqyeb.mysql.rds.aliyuncs.com","dbwgnn1gn0u90u6n","203891", "dbwgnn1gn0u90u6n",0,NULL,0) == NULL) 
		{      
			TRACE("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));     
			//exit(1);  
		}  
	}
	if(conn){
		if(mysql_query(conn,"CREATE TABLE IF NOT EXISTS " + m_marketTableName + "(datetime DATETIME,millisecond INT,a50index VARCHAR(20),a50bid VARCHAR(20),a50ask VARCHAR(20),hs300index VARCHAR(20),hs300bid VARCHAR(20),hs300ask VARCHAR(20))")) 
		{      
			TRACE("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));      
		}
	}
	if(conn){
		if(mysql_query(conn,"CREATE TABLE IF NOT EXISTS " + m_statusTableIbName + "(datetime DATETIME,millisecond INT,a50index VARCHAR(20),a50bid VARCHAR(20),a50ask VARCHAR(20),hs300index VARCHAR(20),hs300bid VARCHAR(20),hs300ask VARCHAR(20))")) 
		{      
			TRACE("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));      
		}
	}
}
