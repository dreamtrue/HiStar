
// HiStar.cpp : 定义应用程序的类行为。
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
extern DWORD IndexThreadId;
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
//读写锁
SRWLOCK g_srwLock_PosDetail;  
SRWLOCK g_srwLock_TradingAccount;
SRWLOCK g_srwLock_MargRate;
SRWLOCK g_srwLock_Insinf;
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

// CHiStarApp 构造

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
	//定位内存泄漏位置,非常好用
	//_CrtSetBreakAlloc(144);
	MainThreadId = GetCurrentThreadId();
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
	InitializeSRWLock(&g_srwLock_PosDetail);//读写锁初始化
	InitializeSRWLock(&g_srwLock_TradingAccount);
	InitializeSRWLock(&g_srwLock_MargRate);
	InitializeSRWLock(&g_srwLock_Insinf);
}

void CHiStarApp::OnIni(WPARAM wParam,LPARAM lParam){
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	TThostFtdcDateType buffer;
	memset(buffer,0,sizeof(TThostFtdcDateType));memset(m_accountCtp.m_todayDate,0,sizeof(TThostFtdcDateType));
	sprintf(buffer,"%04d%02d%02d",sys.wYear,sys.wMonth,sys.wDay);/*赋予数值*/
	strcpy(m_accountCtp.m_todayDate,buffer);
	//TThostFtdcDateType
	FileInput();
	SetIFContract();//设置IF合约
	SetA50Contract();//设置A50合约
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
	//交易后处理线程
	if(!m_pHedgePostProcessing){
		m_pHedgePostProcessing = (CHedgePostProcessing*)AfxBeginThread(RUNTIME_CLASS(CHedgePostProcessing));
		m_pHedgePostProcessing->m_bAutoDelete = true;
	}
	//IB Order初始化
	m_IBOrder.orderType = "LMT";
	m_IBOrder.whatIf = false;
	//更新持仓
	//UpdateHedgeHold(NULL,NULL);
}

// 唯一的一个 CHiStarApp 对象

CHiStarApp theApp;


// CHiStarApp 初始化

BOOL CHiStarApp::InitInstance()
{
	//让进程拥有最高优先级
	//if(!SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS ))
	//{
	//	return false;
	//}
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。否则，将无法创建窗口。
	AfxInitRichEdit2();
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager *pShellManager = new CShellManager;

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	CMainDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}

	// 删除上面创建的 shell 管理器。
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
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
	//MSHQ 从通达信获取的实时行情
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
	//重新设置合约,自动换月
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	WORD ifFinal = ifFinalDay(sys.wYear,sys.wMonth);
	CString insID;
	if(ifFinal - sys.wDay <= 3){
	}
	//到最后一天的前一天换合约
	if(sys.wDay < ifFinal){
		insID.Format(_T("%.4d%.2d"),sys.wYear,sys.wMonth);
	}
	else{
		if(sys.wMonth < 12){
			insID.Format(_T("%.4d%.2d"),sys.wYear,sys.wMonth + 1);
		}
		else{//年末
			insID.Format(_T("%.4d%.2d"),sys.wYear + 1,1);
		}
	}
	//多字节编码里的字母数字和ASCII是兼容的，所以才用CString和char[]在这儿效果一样。
	//为了保持这种兼容，本程序只能采用多字节编码，否则会出错的。
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
	//创建Hedge持仓明细表格
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
		HedgeHold.clear();//先清空
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
	//更新持仓
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
				Sleep(1000);
			}
		}
		ReleaseSRWLockShared(&g_srwLock_PosDetail); 

	}
	if(m_cT){
		double totalMargin = 0;
		AcquireSRWLockShared(&g_srwLock_PosDetail);
		for(unsigned int i = 0;i < m_cT->m_InvPosDetailVec.size();i++){
			AcquireSRWLockShared(&g_srwLock_Insinf);
			bool found01 = false;int index01 = -1;
			for (UINT k=0; k < m_cT->m_InsinfVec.size();k++){
				if(strcmp(m_cT->m_InvPosDetailVec[i].InstrumentID,m_cT->m_InsinfVec[k].iinf.InstrumentID) == 0){
					found01 = true;index01 = k;
					break;
				}
			}
			AcquireSRWLockShared(&g_srwLock_MargRate);
			bool found = false;int index = -1;
			for(unsigned int j = 0;j < m_cT->m_MargRateVec.size();j++){
				if(strcmp(m_cT->m_InvPosDetailVec[i].InstrumentID,m_cT->m_MargRateVec[j].InstrumentID) == 0){
					found = true;index = j;
					break;
				}
			}
			if(found01 && found){
				if(m_cT->m_InvPosDetailVec[i].Direction == THOST_FTDC_D_Buy){
					totalMargin = totalMargin + m_cT->m_InvPosDetailVec[i].OpenPrice * m_cT->m_InvPosDetailVec[i].Volume * m_cT->m_MargRateVec[index].LongMarginRatioByMoney * m_cT->m_InsinfVec[index01].iinf.VolumeMultiple;
				}
				else{
					totalMargin = totalMargin + m_cT->m_InvPosDetailVec[i].OpenPrice * m_cT->m_InvPosDetailVec[i].Volume * m_cT->m_MargRateVec[index].ShortMarginRatioByMoney * m_cT->m_InsinfVec[index01].iinf.VolumeMultiple;
				}
			}
			ReleaseSRWLockShared(&g_srwLock_MargRate);
			ReleaseSRWLockShared(&g_srwLock_Insinf);
		}
		ReleaseSRWLockShared(&g_srwLock_PosDetail);

		AcquireSRWLockExclusive(&g_srwLock_TradingAccount);
		m_cT->TradingAccount.CurrMargin = totalMargin;
		ReleaseSRWLockExclusive(&g_srwLock_TradingAccount);
	}

	if(m_pHedgePostProcessing){
		m_pHedgePostProcessing->PostThreadMessage(WM_SYNCHRONIZE_NOTIFY,NULL,lParam);
	}
}

BOOL CHiStarApp::OnIdle(LONG lCount)
{
	//因为是模态对话框，这个地方调用不到
	TRACE("i,%ld\n",lCount);
	return __super::OnIdle(lCount);
}
