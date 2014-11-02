
// HiStar.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "HiStar.h"
#include "MainDlg.h"
#include "UserMsg.h"
#include "calendar.h"
#include "afxsock.h"
#include <afxinet.h>
#include <fstream>
#include <sstream>
#include "me.h"
#include "Csv.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
extern double A50Index,HS300Index;//������ʵ������ָ��
extern DWORD IndexThreadId;
std::fstream fileInput;
bool isReal = true,isDownload = false;
extern CVector<HoldDetail> HedgeHold;
extern unsigned int HS300NUM,A50NUM;
sqldb::sqldb(){
	host = "";user = "";passwd = "";db = "";
}
std::vector<stock> g_hs300;
std::vector<stock> g_a50;
sqldb m_db;
//��д��
SRWLOCK g_srwLock_PosDetail;  
SRWLOCK g_srwLock_TradingAccount;
SRWLOCK g_srwLock_MargRate;
SRWLOCK g_srwLock_Insinf;
SRWLOCK g_srwLock_WaitForFee;
SRWLOCK g_srwLock_FeeRate;
/*
std::string stringtrim(std::string s)
{
	int i=0;
	while(s[i] == ' '|| s[i] == '\t' || s[i] == '"' || s[i] == '-')//��ͷ��Ϊ�ո����Tab��������
	{
		i++;
	}
	s=s.substr(i);
	i=s.size()-1;
	while(s[i] == ' '|| s[i] == '\t' || s[i] == '"' || s[i] == '-')////��β��Ϊ�ո����Tab��������
	{
		i--;
	}
	s=s.substr(0,i+1);
	return s;
}
*/
// CHiStarApp
BEGIN_MESSAGE_MAP(CHiStarApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
	/*���������Ϣ��Ӧ���ĵ�������,��֤��Ϣ����ʧ
	ON_THREAD_MESSAGE(WM_CONNECT_IB,OnConnectIB)
	ON_THREAD_MESSAGE(WM_DISCONNECT_IB,OnDisconnectIB)
	ON_THREAD_MESSAGE(WM_LOGIN_CTP,LoginCtp)
	ON_THREAD_MESSAGE(WM_LOGOUT_CTP,LogoutCtp)
	ON_THREAD_MESSAGE(WM_UPDATE_HEDGEHOLD,UpdateHedgeHold)
	ON_THREAD_MESSAGE(WM_UPDATE_LSTCTRL,OnUpdateLstCtrl)
	ON_THREAD_MESSAGE(WM_REQ_MSHQ,OnReqMshq)
	ON_THREAD_MESSAGE(WM_MD_REFRESH,OnHedgeLooping)
	ON_THREAD_MESSAGE(WM_CONNECT_SQL,OnConnectSql)
	ON_THREAD_MESSAGE(WM_INI,OnIni)
	ON_THREAD_MESSAGE(WM_LOGIN_TD,LoginCtpTD)
	ON_THREAD_MESSAGE(WM_LOGIN_MD,LoginCtpMD)
	ON_THREAD_MESSAGE(WM_SYNCHRONIZE_MARKET,OnSynchronizeMarket)
	*/
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
	InitializeSRWLock(&g_srwLock_PosDetail);//��д����ʼ��
	InitializeSRWLock(&g_srwLock_TradingAccount);
	InitializeSRWLock(&g_srwLock_MargRate);
	InitializeSRWLock(&g_srwLock_Insinf);
	InitializeSRWLock(&g_srwLock_WaitForFee);
	InitializeSRWLock(&g_srwLock_FeeRate);
}

void CHiStarApp::OnInitialize(WPARAM wParam,LPARAM lParam){
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	TThostFtdcDateType buffer;
	memset(buffer,0,sizeof(TThostFtdcDateType));memset(m_accountCtp.m_todayDate,0,sizeof(TThostFtdcDateType));
	sprintf_s(buffer,"%04d%02d%02d",sys.wYear,sys.wMonth,sys.wDay);/*������ֵ*/
	strcpy_s(m_accountCtp.m_todayDate,buffer);
	//����ָ���ļ�
	if(isDownload){
		CString cStatus;
		if(Download("https://www.blackrock.com/hk/zh/products/251797/ishares-ftse-a50-china-index-etf/1404292000448.ajax?fileType=csv&fileName=2823_holdings&dataType=fund",".\\2823_Holdings.csv")){
			cStatus.Format("Download 2823 Holdings Success.");
			PostOrderStatus(cStatus);
		}
		if(Download("https://www.blackrock.com/hk/zh/products/251754/ishares-csi-300-a-share-index-etf/1404292000448.ajax?fileType=csv&fileName=2846_holdings&dataType=fund",".\\2846_Holdings.csv")){
			cStatus.Format("Download 2846 Holdings Success.");
			PostOrderStatus(cStatus);
		}
	}
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
		IndexThreadId = m_pIndexThread->m_nThreadID;
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
}

void CHiStarApp::PostOrderStatus(CString str)
{
	CString *pStatus = new CString(str);
	if(m_pMainWnd){
		while(PostMessage(m_pMainWnd->GetSafeHwnd(),WM_ORDER_STATUS,(UINT)pStatus,NULL) == 0){
			Sleep(100);
		}
	}
}

void CHiStarApp::PostErrors(CString str)
{
	CString *pErrors = new CString(str);
	if(m_pMainWnd){
		while(PostMessage(m_pMainWnd->GetSafeHwnd(),WM_ERRORS,(UINT)pErrors,NULL) == 0){
			Sleep(100);
		}
	}
}

void CHiStarApp::SetIFContract(void)
{
	//�������ú�Լ,�Զ�����
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	WORD ifFinal = ifFinalDay(sys.wYear,sys.wMonth);
	CString insID;
	//�����һ���ǰһ�컻��Լ
	if(sys.wDay < ifFinal - 1){
		insID.Format(_T("%.4d%.2d"),sys.wYear,sys.wMonth);
		m_LifeIf = ifFinalDay(sys.wYear,sys.wMonth + 1) - sys.wDay + 1;
	}
	else{
		if(sys.wMonth < 12){
			insID.Format(_T("%.4d%.2d"),sys.wYear,sys.wMonth + 1);
			m_LifeIf = MonthDays(sys.wYear,sys.wMonth) - sys.wDay
				+ ifFinalDay(sys.wYear,sys.wMonth + 1) + 1;
		}
		else{//��ĩ
			insID.Format(_T("%.4d%.2d"),sys.wYear + 1,1);
			m_LifeIf = MonthDays(sys.wYear,sys.wMonth) - sys.wDay
				+ ifFinalDay(sys.wYear + 1,1) + 1;
		}
	}
	//���ֽڱ��������ĸ���ֺ�ASCII�Ǽ��ݵģ����Բ���CString��char[]�����Ч��һ����
	//Ϊ�˱������ּ��ݣ�������ֻ�ܲ��ö��ֽڱ��룬��������ġ�
	m_accountCtp.m_szInst = _T("IF") + insID.Right(4);
	char IFNAME[100];memset(IFNAME,0,sizeof(IFNAME));
	sprintf_s(IFNAME,"IF%s,%d",insID,m_LifeIf);
	if(sys.wDay < ifFinal - 1){
		((CMainDlg*)m_pMainWnd)->m_basicPage.m_csIfShow.SetWindowText(IFNAME,RED);
	}
	else{
		((CMainDlg*)m_pMainWnd)->m_basicPage.m_csIfShow.SetWindowText(IFNAME,YELLOW);
	}
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
				strcpy_s(m_accountCtp.m_sINVESTOR_ID,str01.c_str());
				strcpy_s(m_accountCtp.m_sPASSWORD,str02.c_str());
				strcpy_s(m_accountCtp.m_sBROKER_ID,str03.c_str());
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
	std::string line;
	int numStockLine;
	fileInput.open("2823_Holdings.csv");
	Csv csv2823(fileInput,",");
	numStockLine = 0;
	while (csv2823.getline(line) != 0)
	{
		TRACE("%d\r\n",csv2823.getnfield());
		//��9�е����ǹ�Ʊ�ֲֲ��ֵĸ�ʽ,���е�һ��Ϊ������
		if(csv2823.getnfield() == 11){
			numStockLine++;
			if(numStockLine != 1){
				//��һ���Ǳ���,��������
				if(atoi(csv2823.getfield(0).c_str()) >= 60000){st.exch = "sh";}else{st.exch = "sz";}
				TRACE(csv2823.getfield(7).c_str());
				std::string field07 = csv2823.getfield(7);
				std::string field08 = csv2823.getfield(8);
				for(unsigned int k = 0;k < field07.size();k++){
					if(field07[k] == ',' || field07[k] == '-' || field07[k] == '"'){
						field07.erase(k,1);
						k--;
						TRACE(field07.c_str());
					}
				}
				for(unsigned int k = 0;k < field08.size();k++){
					if(field08[k] == ',' || field08[k] == '-' || field08[k] == '"'){
						field08.erase(k,1);
						k--;
						TRACE(field08.c_str());
					}
				}
				char codename[10];
				sprintf_s(codename,"%06d",atoi(csv2823.getfield(0).c_str()));
				st.code = codename;
				st.volume = atoi(field07.c_str()) + atoi(field08.c_str());
				g_a50.push_back(st);
			}
		}
	}
	A50NUM = numStockLine - 1;
	fileInput.close();
	fileInput.open("2846_Holdings.csv");
	Csv csv2846(fileInput,",");
	numStockLine = 0;
	while (csv2846.getline(line) != 0)
	{
		TRACE("%d\r\n",csv2846.getnfield());
		//��9�е����ǹ�Ʊ�ֲֲ��ֵĸ�ʽ,���е�һ��Ϊ������
		if(csv2846.getnfield() == 10){
			numStockLine++;
			if(numStockLine != 1){
				//��һ���Ǳ���,��������
				if(atoi(csv2846.getfield(0).c_str()) >= 60000){st.exch = "sh";}else{st.exch = "sz";}
				TRACE(csv2846.getfield(7).c_str());
				std::string field = csv2846.getfield(7);
				for(unsigned int k = 0;k < field.size();k++){
					if(field[k] == ',' || field[k] == '-' || field[k] == '"'){
						field.erase(k,1);
						k--;
						TRACE(field.c_str());
					}
				}
				char codename[10];
				sprintf_s(codename,"%06d",atoi(csv2846.getfield(0).c_str()));
				st.code = codename;
				st.volume = atoi(field.c_str());
				g_hs300.push_back(st);
			}
		}
	}
	HS300NUM = numStockLine - 1;//ȥ��������
	fileInput.close();
	return 0;
}

void CHiStarApp::OnConnectSql(WPARAM wParam,LPARAM lParam)
{
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	char name[100];
	sprintf_s(name,"Market_%04d%02d%02d",sys.wYear,sys.wMonth,sys.wDay);
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
		if(mysql_query(conn,"CREATE TABLE IF NOT EXISTS " + m_positionTableName + " (ID INTEGER,amount INTEGER,section INTEGER,price DOUBLE,numIf INTEGER,priceIf DOUBLE,indexHS300 DOUBLE,numA50 INTEGER,priceA50 DOUBLE,indexA50 DOUBLE,primary key (ID))")) 
		{      
			TRACE("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));      
		}
		if(mysql_query(conn,"select * from position")){
			TRACE("Error %u: %s\n", mysql_errno(conn), mysql_error(conn)); 
		}
		MYSQL_RES * res_set = NULL;MYSQL_ROW row;unsigned int num_fields;
		res_set = mysql_store_result(conn);
		if(res_set != NULL){
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
				hd.numIf = atol(row[4]);
				hd.priceIf = atof(row[5]);
				hd.indexHS300 = atof(row[6]);
				hd.numA50 = atol(row[7]);
				hd.priceA50 = atof(row[8]);
				hd.indexA50 = atof(row[9]);
				if(hd.HedgeNum * hd.numA50 < 0 || hd.HedgeNum * hd.numIf > 0){
					int res = ::MessageBox(NULL,_T("���ݿ�ֲ�����ì�ܣ�"),_T(""),MB_OK|MB_ICONERROR);
					if(res == IDOK){
						return;
					}
				}
				HedgeHold.push_back(hd);
			}
		}
	}
	//���³ֲ�
	UpdateHedgeHold(NULL,NULL);
}

void CHiStarApp::OnSynchronizeMarket(WPARAM wParam,LPARAM lParam){
	MSG msg;BOOL bRet;
	if(m_cQ){

		AcquireSRWLockShared(&g_srwLock_PosDetail);
		m_cQ->SynchronizeMarket(m_cQ->InstSubscribed,m_cQ->InstMustSubscribe,m_cT->m_InvPosDetailVec);
		ReleaseSRWLockShared(&g_srwLock_PosDetail); 

	}
	if(m_cT){
		//��֤��
		double totalMargin = 0;
		AcquireSRWLockShared(&g_srwLock_PosDetail);
		for(unsigned int i = 0;i < m_cT->m_InvPosDetailVec.size();i++){
			bool found = false;int requestId = -1;

			AcquireSRWLockShared(&g_srwLock_MargRate);
			for(unsigned int j = 0;j < m_cT->m_MargRateVec.size();j++){
				if(strcmp(m_cT->m_InvPosDetailVec[i].InstrumentID,m_cT->m_MargRateVec[j].InstrumentID) == 0){
					found = true;break;
				}
			}
			ReleaseSRWLockShared(&g_srwLock_MargRate);

			if(!found){
				requestId = m_cT->ReqQryInstrumentMarginRate(m_cT->m_InvPosDetailVec[i].InstrumentID);
				while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
					if (!bRet){
					}
					else if(requestId == msg.lParam){
						break;
					}
				}
			}
			AcquireSRWLockShared(&g_srwLock_Insinf);
			bool found01 = false;int index01 = -1;
			for (UINT k=0; k < m_cT->m_InsinfVec.size();k++){
				if(strcmp(m_cT->m_InvPosDetailVec[i].InstrumentID,m_cT->m_InsinfVec[k].iinf.InstrumentID) == 0){
					found01 = true;index01 = k;
					break;
				}
			}
			AcquireSRWLockShared(&g_srwLock_MargRate);
			bool found02 = false;int index02 = -1;
			for(unsigned int j = 0;j < m_cT->m_MargRateVec.size();j++){
				if(strcmp(m_cT->m_InvPosDetailVec[i].InstrumentID,m_cT->m_MargRateVec[j].InstrumentID) == 0){
					found02 = true;index02 = j;
					break;
				}
			}
			if(found01 && found02){
				if(strcmp(m_cT->m_InvPosDetailVec[i].OpenDate,m_accountCtp.m_todayDate) >= 0){
					if(m_cT->m_InvPosDetailVec[i].Direction == THOST_FTDC_D_Buy || m_cT->m_InvPosDetailVec[i].LastSettlementPrice < 0.01){
						totalMargin = totalMargin + m_cT->m_InvPosDetailVec[i].OpenPrice * m_cT->m_InvPosDetailVec[i].Volume * m_cT->m_MargRateVec[index02].LongMarginRatioByMoney * m_cT->m_InsinfVec[index01].iinf.VolumeMultiple;
					}
					else{
						totalMargin = totalMargin + m_cT->m_InvPosDetailVec[i].OpenPrice * m_cT->m_InvPosDetailVec[i].Volume * m_cT->m_MargRateVec[index02].ShortMarginRatioByMoney * m_cT->m_InsinfVec[index01].iinf.VolumeMultiple;
					}
				}
				else{
					if(m_cT->m_InvPosDetailVec[i].Direction == THOST_FTDC_D_Buy || m_cT->m_InvPosDetailVec[i].LastSettlementPrice < 0.01){
						totalMargin = totalMargin + m_cT->m_InvPosDetailVec[i].LastSettlementPrice * m_cT->m_InvPosDetailVec[i].Volume * m_cT->m_MargRateVec[index02].LongMarginRatioByMoney * m_cT->m_InsinfVec[index01].iinf.VolumeMultiple;
					}
					else{
						totalMargin = totalMargin + m_cT->m_InvPosDetailVec[i].LastSettlementPrice * m_cT->m_InvPosDetailVec[i].Volume * m_cT->m_MargRateVec[index02].ShortMarginRatioByMoney * m_cT->m_InsinfVec[index01].iinf.VolumeMultiple;
					}
				}
			}
			ReleaseSRWLockShared(&g_srwLock_MargRate);
			ReleaseSRWLockShared(&g_srwLock_Insinf);
		}
		ReleaseSRWLockShared(&g_srwLock_PosDetail);
	
		AcquireSRWLockExclusive(&g_srwLock_TradingAccount);
		m_cT->TradingAccount.CurrMargin = totalMargin;
		ReleaseSRWLockExclusive(&g_srwLock_TradingAccount);

		//����
		double totalFee = 0;
		AcquireSRWLockExclusive(&g_srwLock_WaitForFee);
		for(unsigned int i = 0;i < m_cT->WaitingForSettlementFee.size();i++){
			bool found = false;int requestId = -1;

			AcquireSRWLockShared(&g_srwLock_FeeRate);
			for(unsigned int j = 0;j < m_cT->FeeRateList.size();j++){
				if(strcmp(m_cT->WaitingForSettlementFee[i].InstrumentID,m_cT->FeeRateList[j].InstrumentID) == 0){
					found = true;break;
				}
			}
			ReleaseSRWLockShared(&g_srwLock_FeeRate);

			if(!found){
				requestId = m_cT->ReqQryInstFee(m_cT->WaitingForSettlementFee[i].InstrumentID);
				while((bRet = GetMessage(&msg,NULL,WM_NOTIFY_EVENT,WM_NOTIFY_EVENT)) != 0){
					if (!bRet){
					}
					else if(requestId == msg.lParam){
						break;
					}
				}
			}
			AcquireSRWLockShared(&g_srwLock_Insinf);
			bool found01 = false;int index01 = -1;
			for (UINT k=0; k < m_cT->m_InsinfVec.size();k++){
				if(strcmp(m_cT->WaitingForSettlementFee[i].InstrumentID,m_cT->m_InsinfVec[k].iinf.InstrumentID) == 0){
					found01 = true;index01 = k;
					break;
				}
			}
			AcquireSRWLockShared(&g_srwLock_FeeRate);
			bool found02 = false;int index02 = -1;
			for(unsigned int j = 0;j < m_cT->FeeRateList.size();j++){
				if(strcmp(m_cT->WaitingForSettlementFee[i].InstrumentID,m_cT->FeeRateList[j].InstrumentID) == 0){
					found02 = true;index02 = j;
					break;
				}
			}
			if(found01 && found02){
				if(m_cT->WaitingForSettlementFee[i].OffsetFlag == THOST_FTDC_OF_Open){
					if(abs(m_cT->FeeRateList[index02].OpenRatioByMoney) > 0.00000001){
						totalFee = totalFee + m_cT->WaitingForSettlementFee[i].Price * m_cT->WaitingForSettlementFee[i].Volume * m_cT->m_InsinfVec[index01].iinf.VolumeMultiple * m_cT->FeeRateList[index02].OpenRatioByMoney;
					}
					else{
						totalFee = totalFee + m_cT->WaitingForSettlementFee[i].Volume * m_cT->FeeRateList[index02].OpenRatioByVolume;
					}
				}
				else if(m_cT->WaitingForSettlementFee[i].OffsetFlag == THOST_FTDC_OF_CloseYesterday){
					if(abs(m_cT->FeeRateList[index02].CloseRatioByMoney) > 0.00000001){
						totalFee = totalFee + m_cT->WaitingForSettlementFee[i].Price * m_cT->WaitingForSettlementFee[i].Volume * m_cT->m_InsinfVec[index01].iinf.VolumeMultiple * m_cT->FeeRateList[index02].CloseRatioByMoney;
					}
					else{
						totalFee = totalFee + m_cT->WaitingForSettlementFee[i].Volume * m_cT->FeeRateList[index02].CloseRatioByVolume;
					}
				}
				else{
					if(abs(m_cT->FeeRateList[index02].CloseTodayRatioByMoney) > 0.0000001){
						totalFee = totalFee + m_cT->WaitingForSettlementFee[i].Price * m_cT->WaitingForSettlementFee[i].Volume * m_cT->m_InsinfVec[index01].iinf.VolumeMultiple * m_cT->FeeRateList[index02].CloseTodayRatioByMoney;
					}
					else{
						totalFee = totalFee + m_cT->WaitingForSettlementFee[i].Volume * m_cT->FeeRateList[index02].CloseTodayRatioByVolume;
					}
				}
			}
			ReleaseSRWLockShared(&g_srwLock_FeeRate);
			ReleaseSRWLockShared(&g_srwLock_Insinf);
		}
		m_cT->WaitingForSettlementFee.clear();//ͳ�������
		ReleaseSRWLockExclusive(&g_srwLock_WaitForFee);

		AcquireSRWLockExclusive(&g_srwLock_TradingAccount);
		m_cT->TradingAccount.Commission = m_cT->TradingAccount.Commission + totalFee;
		ReleaseSRWLockExclusive(&g_srwLock_TradingAccount);
	}
	if(m_pHedgePostProcessing){
		while(m_pHedgePostProcessing->PostThreadMessage(WM_SYNCHRONIZE_NOTIFY,NULL,lParam) == 0){
			Sleep(100);
		}
	}
}

BOOL CHiStarApp::OnIdle(LONG lCount)
{
	//��Ϊ��ģ̬�Ի�������ط����ò���
	TRACE("i,%ld\n",lCount);
	return __super::OnIdle(lCount);
}


int CHiStarApp::ExitInstance()
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
		while(m_pHedgePostProcessing->PostThreadMessage(WM_QUIT,NULL,NULL) == 0){
			Sleep(100);
		}
		WaitForSingleObject(m_pHedgePostProcessing->m_hThread,INFINITE); 
		m_pHedgePostProcessing = NULL;
	}
	if(m_pIndexThread){
		while(::PostThreadMessage(m_pIndexThread->m_nThreadID,WM_QUIT,0,0) == 0){
			Sleep(100);
		}
		WaitForSingleObject(m_pIndexThread->m_hThread,INFINITE);
		m_pIndexThread = NULL;
	}
	//MSHQ ��ͨ���Ż�ȡ��ʵʱ����
	if(m_pMSHQ){
		while(::PostThreadMessage(m_pMSHQ->m_nThreadID,WM_QUIT,0,0) == 0){
			Sleep(100);
		}
		WaitForSingleObject(m_pMSHQ->m_hThread, INFINITE); 
		m_pMSHQ = NULL;
	}
	return __super::ExitInstance();
}

bool CHiStarApp::Download(const CString& strFileURLInServer,const CString& strFileLocalFullPath)
{
	ASSERT(strFileURLInServer != "");
	ASSERT(strFileLocalFullPath != "");
	CHttpConnection *pHttpConnection = NULL;
	CHttpFile *pHttpFile = NULL;
	CString strServer,strObject;
	INTERNET_PORT wPort;
	DWORD dwType;
	const int nTimeOut = 2000;
    char *pszBuffer = NULL;
	CInternetSession session("download",0);
	session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT,nTimeOut);
	session.SetOption(INTERNET_OPTION_CONNECT_RETRIES,1);
	try
	{
		AfxParseURL(strFileURLInServer,dwType,strServer,strObject,wPort);
		pHttpConnection = session.GetHttpConnection(strServer,wPort);
		pHttpFile = pHttpConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET,strObject);
		if (pHttpFile->SendRequest() == FALSE)
			return false;
		DWORD dwStateCode;
		pHttpFile->QueryInfoStatusCode(dwStateCode);
		if (dwStateCode == HTTP_STATUS_OK)
		{
			HANDLE hFile = CreateFile(strFileLocalFullPath,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL); //���������ļ� 
			if (hFile == INVALID_HANDLE_VALUE)
			{
				pHttpFile->Close();
				pHttpConnection->Close();
				session.Close();
				return false ;
			} 
			char szInfoBuffer[ 1000 ];
			DWORD dwFileSize = 0;
			DWORD dwInfoBufferSize = sizeof(szInfoBuffer);
			BOOL bResult = FALSE;
			bResult = pHttpFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH,
				(void*)szInfoBuffer,&dwInfoBufferSize,NULL);
			dwFileSize = atoi(szInfoBuffer);
			const int BUFFER_LENGTH = 1024*10;
			pszBuffer = new char[BUFFER_LENGTH];
			DWORD dwWrite,dwTotalWrite;
			dwWrite = dwTotalWrite = 0;
			UINT nRead = pHttpFile->Read(pszBuffer,BUFFER_LENGTH);

			while(nRead > 0)
			{
				WriteFile(hFile,pszBuffer,nRead,&dwWrite,NULL);
				dwTotalWrite += dwWrite;
				nRead = pHttpFile->Read(pszBuffer, BUFFER_LENGTH);
			} 

			delete[]pszBuffer;
			pszBuffer = NULL;
			CloseHandle(hFile);
		} 
		else 
		{
			delete[]pszBuffer;
			pszBuffer = NULL;
			if (pHttpFile != NULL)
			{
				pHttpFile->Close();
				delete pHttpFile;
				pHttpFile = NULL;
			} 
			if (pHttpConnection != NULL)
			{
				pHttpConnection->Close();
				delete pHttpConnection;
				pHttpConnection = NULL;
			} 
			session.Close();
			return false ;
		} 
	} 
	catch(CInternetException*)
	{
		delete[]pszBuffer;
		pszBuffer = NULL;
		if (pHttpFile != NULL)
		{
			pHttpFile->Close();
			delete pHttpFile;
			pHttpFile = NULL;
		} 
		if (pHttpConnection != NULL)
		{
			pHttpConnection->Close();
			delete pHttpConnection;
			pHttpConnection = NULL;
		} 
		session.Close();
		return false ;
	} 
	if (pHttpFile != NULL)
		pHttpFile->Close();
	if (pHttpConnection != NULL)
		pHttpConnection->Close();
	session.Close();
	return true ;
}
