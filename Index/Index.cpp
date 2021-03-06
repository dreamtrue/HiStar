// Index.cpp : 实现文件
//

#include "stdafx.h"
#include "Index.h"
#include "UserMsg.h"
#include "afxsock.h"
#include "HiStar.h"
#include <afxinet.h>
#include <vector>
#include "me.h"
unsigned int A50NUM = 50;
unsigned int HS300NUM = 300;
unsigned int SH50NUM = 50;
SYSTEMTIME systime01,time_09_00_00,time_09_50_00;
extern double g_A50IndexZT,g_HS300IndexZT,g_SH50IndexZT;
char data01[1000];
int seconds(SYSTEMTIME &time);
double totalValueA50ZT = 0;
double totalValueHS300ZT = 0;
double totalValueSH50ZT = 0;
double A50IndexRef,A50totalValueRef,HS300IndexRef,HS300totalValueRef,SH50IndexRef,SH50totalValueRef;
extern sqldb m_db;
extern std::vector<stock> g_hs300;
extern std::vector<stock> g_a50;
extern std::vector<stock> g_sh50;
extern double volume[400];
extern double price[400];
extern double priceZT[400];
// CIndex

IMPLEMENT_DYNCREATE(CIndex, CWinThread)

	CInternetSession CIndex::mySession("ssision",0);
CHttpFile* CIndex::myHttpFile = NULL;
CHttpFile* CIndex::myHttpFile01 = NULL;
//CHttpFile* CIndex::myHttpFile02 = NULL;
CString CIndex::myData("");
CString CIndex::myURL01("");
//CString CIndex::myURL02("");
CString CIndex::myURL_code01("");
//CString CIndex::myURL_code02("");
CString CIndex::m_URL("http://hq.sinajs.cn/list=");
CString CIndex::m_hexunA50("http://webglobal.hermes.hexun.com/global_index/quotelist?code=FTSE.FTXIN9&column=LastClose");
CString CIndex::m_hexunHS300("http://flashquote.stock.hexun.com/Stock_Combo.ASPX?mc=1_000300&dt=Q,MI&t=0.9522008465137333");
extern MYSQL *connindex;
CIndex::CIndex()
{
	connindex = NULL;
	myHttpFile = NULL;
	myHttpFile01 = NULL;
	//myHttpFile02 = NULL;
	for(unsigned int k = 0;k < A50NUM;k++){
		myURL_code01 = myURL_code01 + ",";
		myURL_code01 = myURL_code01 + g_a50[k].exch.c_str();
		myURL_code01 = myURL_code01 + g_a50[k].code.c_str();
	}
	for(unsigned int k = 0;k < HS300NUM;k++){
		myURL_code01 = myURL_code01 + ",";
		myURL_code01 = myURL_code01 + g_hs300[k].exch.c_str();
		myURL_code01 = myURL_code01 + g_hs300[k].code.c_str();
	}
	for(unsigned int k = 0;k < SH50NUM;k++){
		myURL_code01 = myURL_code01 + ",";
		myURL_code01 = myURL_code01 + g_sh50[k].exch.c_str();
		myURL_code01 = myURL_code01 + g_sh50[k].code.c_str();
	}
	/*
	unsigned int tempnum;
	tempnum = HS300NUM + SH50NUM;//分两批获取HS300股价行情的首批数目
	for(unsigned int k = 0;k < HS300NUM + SH50NUM;k++){
	if(tempnum <= HS300NUM + SH50NUM){
	if(k < tempnum){
	myURL_code01 = myURL_code01 + ",";
	myURL_code01 = myURL_code01 + g_hs300[k].exch.c_str();
	myURL_code01 = myURL_code01 + g_hs300[k].code.c_str();
	}
	else{
	myURL_code02 = myURL_code02 + ",";
	myURL_code02 = myURL_code02 + g_hs300[k].exch.c_str();
	myURL_code02 = myURL_code02 + g_hs300[k].code.c_str();
	}
	}
	else{
	myURL_code01 = myURL_code01 + ",";
	myURL_code01 = myURL_code01 + g_hs300[k].exch.c_str();
	myURL_code01 = myURL_code01 + g_hs300[k].code.c_str();
	myURL_code02 = "";
	}
	}
	*/
	for(unsigned int j = 0;j < A50NUM + HS300NUM + SH50NUM;j++){
		if(j < A50NUM){
			volume[j] = g_a50[j].volume;
		}
		else if(j < A50NUM + HS300NUM){
			volume[j] = g_hs300[j - A50NUM].volume;
		}
		else{
			volume[j] = g_sh50[j - A50NUM - HS300NUM].volume;
		}
	}
	myURL01 = m_URL + myURL_code01;
	/*
	if(tempnum <= HS300NUM + SH50NUM){
	myURL02 = m_URL + myURL_code02;
	}
	else{
	myURL02 = "";
	}
	*/
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
		if(mysql_query(connindex,"CREATE TABLE IF NOT EXISTS HISTARINDEX (name VARCHAR(40),A50REF DOUBLE,A50VALUE DOUBLE,HS300REF DOUBLE,HS300VALUE DOUBLE,SH50REF DOUBLE,SH50VALUE DOUBLE,primary key (name))")) 
		{      
			TRACE("Error %u: %s\n", mysql_errno(connindex), mysql_error(connindex));      
		}
		if(mysql_query(connindex,"select * from HISTARINDEX")){
			TRACE("Error %u: %s\n", mysql_errno(connindex), mysql_error(connindex)); 
		}
		MYSQL_RES * res_set = NULL;MYSQL_ROW row;unsigned int num_fields = 0;
		res_set = mysql_store_result(connindex);
		if(res_set != NULL){
			num_fields = mysql_num_fields(res_set);
			while ((row = mysql_fetch_row(res_set))){
				A50IndexRef = atof(row[1]);
				A50totalValueRef = atof(row[2]);
				HS300IndexRef = atof(row[3]);
				HS300totalValueRef = atof(row[4]);
				SH50IndexRef = atof(row[5]);
				SH50totalValueRef = atof(row[6]);
			}	
		}
	}
	GetIndexYd();//获得昨天的指数
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
	GetQuotation();//获取行情
	totalValueA50ZT = 0.0;totalValueHS300ZT = 0.0;totalValueSH50ZT = 0.0;
	for(unsigned int i = 0;i < A50NUM + HS300NUM + SH50NUM;i++){
		if(i < A50NUM){
			totalValueA50ZT = totalValueA50ZT + priceZT[i] * volume[i];
		}
		else if(i < A50NUM + HS300NUM){
			totalValueHS300ZT = totalValueHS300ZT + priceZT[i] * volume[i];
		}
		else{
			totalValueSH50ZT = totalValueSH50ZT + priceZT[i] * volume[i];
		}
	}
	GetIndexYd();//获得昨天的指数
	static bool iFirst = true;
	if(iFirst){
		time_09_00_00.wHour = 9;time_09_00_00.wMinute = 0;time_09_00_00.wSecond = 0;
		time_09_50_00.wHour = 9;time_09_50_00.wMinute = 50;time_09_50_00.wSecond = 0;
		iFirst = false;
	}
	GetLocalTime(&systime01);
	if(seconds(systime01) < seconds(time_09_50_00)){
	}
	else{
		A50totalValueRef = totalValueA50ZT;A50IndexRef = g_A50IndexZT;
		HS300totalValueRef = totalValueHS300ZT;HS300IndexRef = g_HS300IndexZT;
		SH50totalValueRef = totalValueSH50ZT;SH50IndexRef = g_SH50IndexZT;
		//存入指数
		sprintf_s(data01,"INSERT INTO HISTARINDEX (name,A50REF,A50VALUE,HS300REF,HS300VALUE,SH50REF,SH50VALUE) VALUES('indexref',%.02lf,%.02lf,%.02lf,%.02lf,%.02lf,%.02lf) ON DUPLICATE KEY UPDATE A50REF = %.02lf,A50VALUE = %.02lf,HS300REF = %.02lf,HS300VALUE = %.02lf,SH50REF = %.02lf,SH50VALUE = %.02lf",g_A50IndexZT,totalValueA50ZT,g_HS300IndexZT,totalValueHS300ZT,g_A50IndexZT,totalValueA50ZT,g_HS300IndexZT,totalValueHS300ZT,g_SH50IndexZT,totalValueSH50ZT);
		if(connindex){
			if(mysql_query(connindex,data01)){
				TRACE("Error %u: %s\n", mysql_errno(connindex), mysql_error(connindex));
			}
		}
	}
}

BEGIN_MESSAGE_MAP(CIndex, CWinThread)
	ON_THREAD_MESSAGE(WM_UPDATE_INDEX_REF,OnUpdateIndexRef)
END_MESSAGE_MAP()

void CIndex::GetQuotation(void)
{
	try{
		myHttpFile01 = (CHttpFile*)mySession.OpenURL(myURL01,1,INTERNET_FLAG_RELOAD|INTERNET_FLAG_TRANSFER_ASCII);
		/*
		if(myURL02 != ""){
		myHttpFile02 = (CHttpFile*)mySession.OpenURL(myURL02,1,INTERNET_FLAG_RELOAD|INTERNET_FLAG_TRANSFER_ASCII);
		}
		*/
	}
	catch(CInternetException*pException){
		pException->Delete();
		myHttpFile01->Close();
		delete myHttpFile01;
		myHttpFile01 = NULL;
		/*
		if(myURL02 != ""){
		myHttpFile02->Close();
		delete myHttpFile02;	
		myHttpFile02 = NULL;
		}*/
		TRACE("读取指数失败!\r\n");
		return;//读取失败,返回
	}
	int i = 0;
	DWORD dwStatusCode;
	myHttpFile01->QueryInfoStatusCode(dwStatusCode);
	if(myHttpFile01 != NULL && dwStatusCode == HTTP_STATUS_OK){
		while(myHttpFile01->ReadString(myData))
		{
			CString strGet1(_T("")); 
			CString strGet2(_T(""));
			CString strGet3(_T(""));
			double temp = 0;
			AfxExtractSubString(strGet1,myData,1, _T('\"'));
			AfxExtractSubString(strGet2,strGet1,3, _T(','));//现在的价格
			AfxExtractSubString(strGet3,strGet1,2, _T(','));//昨天的价格
			LPTSTR  chValue = strGet2.GetBuffer( strGet2.GetLength() );
			LPTSTR  chValueZT = strGet3.GetBuffer( strGet3.GetLength() );
			double fZT = atof(chValueZT);//昨天的价格
			strGet3.ReleaseBuffer(); 
			priceZT[i] = fZT;
			double fValue = atof(chValue);//今天的价格
			strGet2.ReleaseBuffer(); 
			if(fValue > 0.1){//防止等于0，等于0就用昨天的收盘价
				price[i] = fValue;
			}
			else{
				price[i] = priceZT[i];
			}
			i++;
		}
	}
	/*
	if(myURL02 != ""){
	myHttpFile02->QueryInfoStatusCode(dwStatusCode);
	if(myHttpFile02 != NULL && dwStatusCode == HTTP_STATUS_OK){
	while(myHttpFile02->ReadString(myData))
	{
	CString strGet1(_T("")); 
	CString strGet2(_T(""));
	CString strGet3(_T(""));
	double temp = 0;
	AfxExtractSubString(strGet1,myData,1, _T('\"'));
	AfxExtractSubString(strGet2,strGet1,3, _T(','));//现在的价格
	AfxExtractSubString(strGet3,strGet1,2, _T(','));//昨天的价格
	LPTSTR  chValue = strGet2.GetBuffer( strGet2.GetLength() );
	LPTSTR  chValueZT = strGet3.GetBuffer( strGet3.GetLength() );
	double fZT = atof(chValueZT);//昨天的价格
	strGet3.ReleaseBuffer(); 
	priceZT[i] = fZT;
	double fValue = atof(chValue);//今天的价格
	strGet2.ReleaseBuffer(); 
	if(fValue > 0.1){//防止等于0，等于0就用昨天的收盘价
	price[i] = fValue;
	}
	else{
	price[i] = priceZT[i];
	}
	i++;
	}
	}

	}
	*/
	myHttpFile01->Close();
	delete myHttpFile01;
	myHttpFile01 = NULL;
	/*
	if(myURL02 != ""){
	myHttpFile02->Close();
	delete myHttpFile02;	
	myHttpFile02 = NULL;
	}
	*/
}

void CIndex::GetIndexYd(void)
{
	DWORD dwStatusCode;
	try{
		myHttpFile = (CHttpFile*)mySession.OpenURL(m_hexunA50,1,INTERNET_FLAG_RELOAD|INTERNET_FLAG_TRANSFER_ASCII);
	}
	catch(CInternetException*pException){
		pException->Delete();
		myHttpFile->Close();
		delete myHttpFile;
		myHttpFile = NULL;
		TRACE("读取指数失败!\r\n");
		return;//读取失败,返回
	}
	myHttpFile->QueryInfoStatusCode(dwStatusCode);
	if(myHttpFile != NULL && dwStatusCode == HTTP_STATUS_OK){
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
		return;//读取失败,返回
	}
	myHttpFile->QueryInfoStatusCode(dwStatusCode);
	if(myHttpFile != NULL && dwStatusCode == HTTP_STATUS_OK){
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
}
