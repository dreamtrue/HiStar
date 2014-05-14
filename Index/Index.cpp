// Index.cpp : 实现文件
//

#include "stdafx.h"
#include "Index.h"
#include "UserMsg.h"
#include "afxsock.h"
#include "mysql.h"
#include "HiStar.h"
#include <afxinet.h>
#include <vector>
#define A50NUM 50
#define HS300NUM 300
SYSTEMTIME systime01,time_09_00_00,time_09_40_00;
extern double g_A50IndexZT,g_HS300IndexZT;
char data01[1000];
int seconds(SYSTEMTIME &time);
double totalValueA50ZT = 0;
double totalValueHS300ZT = 0;
double A50IndexRef;double A50totalValueRef;double HS300IndexRef;double HS300totalValueRef;
struct stock{
	std::string exch;
	std::string code;
	int volume;
};
struct sqldb{
	std::string host;
	std::string user;
	std::string passwd;
	std::string db;
	sqldb();
};
extern sqldb m_db;
extern std::vector<stock> g_hs300;
extern std::vector<stock> g_a50;
#define TOTAL 350
extern double volume[TOTAL];
extern double priceZT[TOTAL];
// CIndex

IMPLEMENT_DYNCREATE(CIndex, CWinThread)

CInternetSession CIndex::mySession("ssision",0);
CHttpFile* CIndex::myHttpFile = NULL;
CString CIndex::myData("");
CString CIndex::myURL("");
CString CIndex::myURL_code("");
CString CIndex::m_URL("http://hq.sinajs.cn/list=");
CString CIndex::m_hexunA50("http://webglobal.hermes.hexun.com/global_index/quotelist?code=FTSE.FTXIN9&column=LastClose");
CString CIndex::m_hexunHS300("http://flashquote.stock.hexun.com/Stock_Combo.ASPX?mc=1_000300&dt=Q,MI&t=0.9522008465137333");
extern MYSQL *connindex;
CIndex::CIndex()
{
	connindex = NULL;
	myHttpFile = NULL;
	for(int k = 0;k < A50NUM;k++){
		myURL_code = myURL_code + ",";
		myURL_code = myURL_code + g_a50[k].exch.c_str();
		myURL_code = myURL_code + g_a50[k].code.c_str();
	}
	for(int k = 0;k < HS300NUM;k++){
		myURL_code = myURL_code + ",";
		myURL_code = myURL_code + g_hs300[k].exch.c_str();
		myURL_code = myURL_code + g_hs300[k].code.c_str();
	}
	for(unsigned int j = 0;j < TOTAL;j++){
		if(j < A50NUM){
			volume[j] = g_a50[j].volume;
		}
		else{
			volume[j] = g_hs300[j - A50NUM].volume;
		}
	}
	myURL = m_URL + myURL_code;
}

CIndex::~CIndex()
{
	mySession.Close();
}

BOOL CIndex::InitInstance()
{
	connindex = mysql_init(NULL); 
	if(connindex == NULL) {
		TRACE("Error %u: %s\n", mysql_errno(connindex), mysql_error(connindex));      
		//exit(1);  
	}  
	if(connindex){
		if(mysql_real_connect(connindex,m_db.host.c_str(),m_db.user.c_str(),m_db.passwd.c_str(),m_db.db.c_str(),0,NULL,0) == NULL)
		{      
			TRACE("Error %u: %s\n", mysql_errno(connindex), mysql_error(connindex));
		}
	}
	//创建指数储存表格
	if(connindex){
		if(mysql_query(connindex,"CREATE TABLE IF NOT EXISTS HISTARINDEX (name VARCHAR(40),A50REF DOUBLE,A50VALUE DOUBLE,HS300REF DOUBLE,HS300VALUE DOUBLE,primary key (name))")) 
		{      
			TRACE("Error %u: %s\n", mysql_errno(connindex), mysql_error(connindex));      
		}
		if(mysql_query(connindex,"select * from HISTARINDEX")){
			TRACE("Error %u: %s\n", mysql_errno(connindex), mysql_error(connindex)); 
		}
		MYSQL_RES * res_set;MYSQL_ROW row;unsigned int num_fields;
		res_set = mysql_store_result(connindex);
		num_fields = mysql_num_fields(res_set);
		while ((row = mysql_fetch_row(res_set))){
			A50IndexRef = atof(row[1]);
			A50totalValueRef = atof(row[2]);
			HS300IndexRef = atof(row[3]);
			HS300totalValueRef = atof(row[4]);
		}	
	}
	try{
		myHttpFile = (CHttpFile*)mySession.OpenURL(m_hexunA50,1,INTERNET_FLAG_RELOAD|INTERNET_FLAG_TRANSFER_ASCII);
	}
	catch(CInternetException*pException){
		pException->Delete();
		myHttpFile->Close();
		delete myHttpFile;
		myHttpFile = NULL;
		TRACE("读取指数失败!\r\n");
		return FALSE;//读取失败,返回
	}
	if(myHttpFile != NULL){
		while(myHttpFile->ReadString(myData)){
			CString strGet1(_T("")); 
			CString strGet2(_T(""));
			AfxExtractSubString(strGet1,myData,3, _T('['));
			AfxExtractSubString(strGet2,strGet1,0, _T(']'));
			g_A50IndexZT = atof(strGet2.GetBuffer()) / 100.0;
		}
	}
	myHttpFile->Close();
	delete myHttpFile;
	myHttpFile = NULL;
	try{
		myHttpFile = (CHttpFile*)mySession.OpenURL(m_hexunHS300,1,INTERNET_FLAG_RELOAD|INTERNET_FLAG_TRANSFER_ASCII);
	}
	catch(CInternetException*pException){
		pException->Delete();
		myHttpFile->Close();
		delete myHttpFile;
		myHttpFile = NULL;
		TRACE("读取行情失败!\r\n");
		return FALSE;//读取失败,返回
	}
	if(myHttpFile != NULL){
		while(myHttpFile->ReadString(myData)){
			CString strGet1(_T("")); 
			CString strGet2(_T(""));
			AfxExtractSubString(strGet1,myData,4, _T(','));
			AfxExtractSubString(strGet2,strGet1,1, _T('\''));
			g_HS300IndexZT = atof(strGet2.GetBuffer());
		}
	}
	myHttpFile->Close();
	delete myHttpFile;
	myHttpFile = NULL;

	try{
		myHttpFile = (CHttpFile*)mySession.OpenURL(myURL,1,INTERNET_FLAG_RELOAD|INTERNET_FLAG_TRANSFER_ASCII);
	}
	catch(CInternetException*pException){
		pException->Delete();
		delete myHttpFile;
		myHttpFile = NULL;
		TRACE("读取昨日行情失败!\r\n");
		return FALSE;//读取失败,返回
	}
	if(myHttpFile != NULL){
		int i = 0;
		while(myHttpFile->ReadString(myData))
		{
			CString strGet1(_T("")); 
			CString strGet3(_T(""));
			double temp = 0;
			AfxExtractSubString(strGet1,myData,1, _T('\"'));
			AfxExtractSubString(strGet3,strGet1,2, _T(','));//昨天的价格
			LPTSTR  chValueZT = strGet3.GetBuffer( strGet3.GetLength() );
			double fZT = atof(chValueZT);//昨天的价格
			strGet3.ReleaseBuffer(); 
			priceZT[i] = fZT;
			i++;
		}
	}
	totalValueA50ZT = 0;totalValueHS300ZT = 0;
	for(int i = 0;i < TOTAL;i++){
		if(i < A50NUM){
			totalValueA50ZT = totalValueA50ZT + priceZT[i] * volume[i];
		}
		if(i >= A50NUM){
			totalValueHS300ZT = totalValueHS300ZT + priceZT[i] * volume[i];
		}
	}
	myHttpFile->Close();
	delete myHttpFile;
	myHttpFile = NULL;

	static bool iFirst = true;
	if(iFirst){
		time_09_00_00.wHour = 9;time_09_00_00.wMinute = 0;time_09_00_00.wSecond = 0;
		time_09_40_00.wHour = 9;time_09_40_00.wMinute = 40;time_09_40_00.wSecond = 0;
		iFirst = false;
	}
	GetLocalTime(&systime01);
	if(seconds(systime01) >= seconds(time_09_00_00) && seconds(systime01) < seconds(time_09_40_00)){
	}
	else{
		A50totalValueRef = totalValueA50ZT;A50IndexRef = g_A50IndexZT;
		HS300totalValueRef = totalValueHS300ZT;HS300IndexRef = g_HS300IndexZT;
		//存入指数
		sprintf(data01,"INSERT INTO HISTARINDEX (name,A50REF,A50VALUE,HS300REF,HS300VALUE) VALUES('indexref',%.02lf,%.02lf,%.02lf,%.02lf) ON DUPLICATE KEY UPDATE A50REF = %.02lf,A50VALUE = %.02lf,HS300REF = %.02lf,HS300VALUE = %.02lf",g_A50IndexZT,totalValueA50ZT,g_HS300IndexZT,totalValueHS300ZT,g_A50IndexZT,totalValueA50ZT,g_HS300IndexZT,totalValueHS300ZT);
		if(connindex){
			if(mysql_query(connindex,data01)){
				TRACE("Error %u: %s\n", mysql_errno(connindex), mysql_error(connindex));
			}
		}
	}
	_timerID = SetTimer(NULL,0,(UINT)3000,UpdateIndexData); 
	return TRUE;
}

int CIndex::ExitInstance()
{
	TRACE("退出指数获取线程\r\n");
	KillTimer(NULL, _timerID);
	return CWinThread::ExitInstance();
}

void CIndex::OnUpdateIndexRef(WPARAM wParam,LPARAM lParam){
	InitInstance();
}

BEGIN_MESSAGE_MAP(CIndex, CWinThread)
	ON_THREAD_MESSAGE(WM_UPDATE_INDEX_REF,OnUpdateIndexRef)
END_MESSAGE_MAP()