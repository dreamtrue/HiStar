
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
bool isReal = true;
extern CVector<HoldDetail> HedgeHold;
struct sqldb{
	std::string host;
	std::string user;
	std::string passwd;
	std::string db;
	sqldb();
};
sqldb::sqldb(){
	host = "";user = "";passwd = "";db = "";
}
struct stock{
	std::string exch;
	std::string code;
	int volume;
};
std::vector<stock> g_hs300;
std::vector<stock> g_a50;
sqldb m_db;
// CHiStarApp
BEGIN_MESSAGE_MAP(CHiStarApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
	ON_THREAD_MESSAGE(WM_CONNECT_IB,OnConnectIB)
	ON_THREAD_MESSAGE(WM_DISCONNECT_IB,OnDisconnectIB)
	ON_THREAD_MESSAGE(WM_LOGIN_CTP,LoginCtp)
	ON_THREAD_MESSAGE(WM_LOGOUT_CTP,LogoutCtp)
	ON_THREAD_MESSAGE(WM_QRY_ACC_CTP,OnQryAccCtp)
	ON_THREAD_MESSAGE(WM_UPDATE_HEDGEHOLD,UpdateHedgeHold)
	ON_THREAD_MESSAGE(WM_UPDATE_LSTCTRL,OnUpdateLstCtrl)
	ON_THREAD_MESSAGE(WM_REQ_MSHQ,OnReqMshq)
	ON_THREAD_MESSAGE(WM_MD_REFRESH,OnHedgeLooping)
	ON_THREAD_MESSAGE(WM_CONNECT_SQL,OnConnectSql)
	ON_THREAD_MESSAGE(WM_INI,OnIni)
	ON_THREAD_MESSAGE(WM_SYNCHRONIZEMARKET,OnSynchronizeMarket)
	ON_THREAD_MESSAGE(WM_LOGIN_TD,LoginCtpTD)
	ON_THREAD_MESSAGE(WM_LOGIN_MD,LoginCtpMD)
	ON_THREAD_MESSAGE(WM_SYNCHRONIZE_MARKET,OnSynchronizeMarket)
END_MESSAGE_MAP()

// CHiStarApp ����

CHiStarApp::CHiStarApp()
	: faError(false)
	, m_pIBClient(NULL)
	, m_MApi(NULL)
	, m_TApi(NULL)
	, m_cQ(NULL)
	, m_cT(NULL)
	, m_strPath(_T(""))
	, m_pIndexThread(NULL)
	, m_id(0)
	, m_pHedgePostProcessing(NULL)
	, m_pMSHQ(NULL)
	, conn(NULL)
{
	//��λ�ڴ�й©λ��,�ǳ�����
	//_CrtSetBreakAlloc(144);
	MainThreadId = GetCurrentThreadId();
	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}

void CHiStarApp::OnIni(WPARAM wParam,LPARAM lParam){
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	TThostFtdcDateType buffer;
	memset(buffer,0,sizeof(TThostFtdcDateType));memset(m_accountCtp.m_todayDate,0,sizeof(TThostFtdcDateType));
	sprintf(buffer,"%04d%02d%02d",sys.wYear,sys.wMonth,sys.wDay);/*������ֵ*/
	strcpy(m_accountCtp.m_todayDate,buffer);
	//TThostFtdcDateType
	FileInput();
	SetIFContract();//����IF��Լ
	SetA50Contract();//����A50��Լ
	//IB Client
	if(!m_pIBClient){
		m_pIBClient = new EClientSocket( this);
	}
	//CTP Client
	CreateCtpClient();
	//INDEX
	if(!m_pIndexThread){
		m_pIndexThread = (CIndex*)AfxBeginThread(RUNTIME_CLASS(CIndex));
		m_pIndexThread->m_bAutoDelete = true;
	}
	//���׺����߳�
	if(!m_pHedgePostProcessing){
		m_pHedgePostProcessing = (CHedgePostProcessing*)AfxBeginThread(RUNTIME_CLASS(CHedgePostProcessing));
		m_pHedgePostProcessing->m_bAutoDelete = true;
	}
	//IB Order��ʼ��
	m_IBOrder.orderType = "LMT";
	m_IBOrder.whatIf = false;
	//���³ֲ�
	//UpdateHedgeHold(NULL,NULL);
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
		::PostThreadMessage(m_pHedgePostProcessing->m_nThreadID, WM_QUIT,0,0);
		WaitForSingleObject(m_pHedgePostProcessing->m_hThread, INFINITE); 
		m_pHedgePostProcessing = NULL;
	}
	if(m_pIndexThread){
		::PostThreadMessage(m_pIndexThread->m_nThreadID, WM_QUIT,0,0);
		WaitForSingleObject(m_pIndexThread->m_hThread, INFINITE);
		m_pIndexThread = NULL;
	}
	//MSHQ ��ͨ���Ż�ȡ��ʵʱ����
	if(m_pMSHQ){
		::PostThreadMessage(m_pMSHQ->m_nThreadID, WM_QUIT,0,0);
		WaitForSingleObject(m_pMSHQ->m_hThread, INFINITE); 
		m_pMSHQ = NULL;
	}
}

void CHiStarApp::PostOrderStatus(CString str)
{
	CString *pStatus = new CString(str);
	if(m_pMainWnd){
		PostMessage(m_pMainWnd->GetSafeHwnd(),WM_ORDER_STATUS,(UINT)pStatus,NULL);
	}
}

void CHiStarApp::PostErrors(CString str)
{
	CString *pErrors = new CString(str);
	if(m_pMainWnd){
		PostMessage(m_pMainWnd->GetSafeHwnd(),WM_ERRORS,(UINT)pErrors,NULL);
	}
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

int CHiStarApp::FileInput(void)
{
	std::stringstream stream;
	if(isReal){
		fileInput.open("histar.ini");
	}
	else{
		fileInput.open("histar_demo.ini");
	}
	while(getline(fileInput,str)){
		TRACE("%s\r\n",str.c_str());
		stream.str("");
		stream.clear();
		stream << str;
		stream >> str01;
		if(str01.c_str()[0] == '*'){continue;}
		else if(str01 == "@account"){
			while(getline(fileInput,str)){
				stream.str("");
				stream.clear();
				stream << str;
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
		else if(str01 == "@mysql"){
			while(getline(fileInput,str)){
				stream.str("");stream.clear();stream << str;
				stream >> str01 >> str02 >> str03 >> str04;
				if(str01.c_str()[0] == '*'){continue;}
				if(str == "@end")break;
				m_db.host = str01;
				m_db.user = str02;
				m_db.passwd = str03;
				m_db.db = str04;
			}
		}
		else if(str01 == "@md_address"){
			while(getline(fileInput,str)){
				stream.str("");stream.clear();stream << str;
				stream >> str01;
				if(str01.c_str()[0] == '*'){continue;}
				if(str == "@end")break;
				m_accountCtp.m_szArMd.Add(str01.c_str());
			}
		}
		else if(str01 == "@td_address"){
			while(getline(fileInput,str)){
				stream.str("");stream.clear();stream << str;
				stream >> str01;
				if(str01.c_str()[0] == '*'){continue;}
				if(str == "@end")break;
				m_accountCtp.m_szArTs.Add(str01.c_str());
			}
		}
	}
	fileInput.close();
	stock st;
	fileInput.open("2823_Holdings.csv");
	int i = 0;
	while(getline(fileInput,str)){
		i++;
		stream.str("");stream.clear();stream << str;
		if(i >= 12 && i <= 61){
			std::string sub_str;
			std::vector<std::string> subvec;
			while(getline(stream,sub_str,',')){
				subvec.push_back(sub_str);
			}
			if(subvec[2] >= "60000"){st.exch = "sh";}else{st.exch = "sz";}
			st.code = subvec[2].substr(0,6);st.volume = atoi(subvec[6].c_str()) + atoi(subvec[7].c_str());
			g_a50.push_back(st);
		}
	}
	fileInput.close();
	fileInput.open("2846_Holdings.csv");
	i = 0;
	while(getline(fileInput,str)){
		i++;
		stream.str("");stream.clear();stream << str;
		if(i >= 13 && i <= 312){
			std::string sub_str;
			std::vector<std::string> subvec;
			while(getline(stream,sub_str,',')){
				subvec.push_back(sub_str);
			}
			if(subvec[2] >= "60000"){st.exch = "sh";}else{st.exch = "sz";}
			st.code = subvec[2].substr(0,6);st.volume = atoi(subvec[6].c_str());
			g_hs300.push_back(st);
		}
	}
	stream.str("");stream.clear();
	fileInput.close();
	return 0;
}

void CHiStarApp::OnConnectSql(WPARAM wParam,LPARAM lParam)
{
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	char name[100];
	sprintf(name,"Market_%04d%02d%02d",sys.wYear,sys.wMonth,sys.wDay);
	m_marketTableName = name;
	conn = mysql_init(NULL); 
	if(conn == NULL) {
		TRACE("Error %u: %s\n", mysql_errno(conn), mysql_error(conn)); 
	}  
	if(conn){
		if(mysql_real_connect(conn,m_db.host.c_str(),m_db.user.c_str(),m_db.passwd.c_str(),m_db.db.c_str(),0,NULL,0) == NULL)
		{      
			TRACE("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
		}  
	}
	if(conn){
		if(mysql_query(conn,"CREATE TABLE IF NOT EXISTS " + m_marketTableName + " (datetime DATETIME,millisecond INT,a50index DOUBLE,a50bid DOUBLE,a50ask DOUBLE,hs300index DOUBLE,hs300bid DOUBLE,hs300ask DOUBLE,preniumHigh DOUBLE,preniumLow DOUBLE)")) 
		{      
			TRACE("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));      
		}
	}
	//����Hedge�ֲ���ϸ���
	m_positionTableName = "position";
	if(conn){
		if(mysql_query(conn,"CREATE TABLE IF NOT EXISTS " + m_positionTableName + " (ID INTEGER,amount INTEGER,section INTEGER,price DOUBLE,primary key (ID))")) 
		{      
			TRACE("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));      
		}
		if(mysql_query(conn,"select * from position")){
			TRACE("Error %u: %s\n", mysql_errno(conn), mysql_error(conn)); 
		}
		MYSQL_RES * res_set;MYSQL_ROW row;unsigned int num_fields;
		res_set = mysql_store_result(conn);
		num_fields = mysql_num_fields(res_set);
		HedgeHold.clear();//�����
		while ((row = mysql_fetch_row(res_set))){
			unsigned long *lengths;
			lengths = mysql_fetch_lengths(res_set);
			HoldDetail hd;
			hd.id = atol(row[0]);
			hd.HedgeNum = atoi(row[1]);
			hd.HedgeSection = atoi(row[2]);
			hd.originalCost = atof(row[3]);
			HedgeHold.push_back(hd);
		}
	}
	//���³ֲ�
	UpdateHedgeHold(NULL,NULL);
}

void CHiStarApp::OnSynchronizeMarket(WPARAM wParam,LPARAM lParam){
	if(m_cQ){
		m_cQ->SynchronizeMarket(m_cQ->InstSubscribed,m_cQ->InstMustSubscribe,m_cT->m_InvPosDetailVec.GetBuffer());
	}
}

BOOL CHiStarApp::OnIdle(LONG lCount)
{
	//��Ϊ��ģ̬�Ի�������ط����ò���
	TRACE("i,%ld\n",lCount);
	return __super::OnIdle(lCount);
}
