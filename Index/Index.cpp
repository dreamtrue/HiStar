// Index.cpp : ʵ���ļ�
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
unsigned int HS300NUM=300;
SYSTEMTIME systime01,time_09_00_00,time_09_50_00;
extern double g_A50IndexZT,g_HS300IndexZT;
char data01[1000];
int seconds(SYSTEMTIME &time);
double totalValueA50ZT = 0;
double totalValueHS300ZT = 0;
double A50IndexRef;double A50totalValueRef;double HS300IndexRef;double HS300totalValueRef;
extern sqldb m_db;
extern std::vector<stock> g_hs300;
extern std::vector<stock> g_a50;
extern double volume[350];
extern double price[350];
extern double priceZT[350];
// CIndex

IMPLEMENT_DYNCREATE(CIndex, CWinThread)

CInternetSession CIndex::mySession("ssision",0);
CHttpFile* CIndex::myHttpFile = NULL;
CHttpFile* CIndex::myHttpFile01 = NULL;
CHttpFile* CIndex::myHttpFile02 = NULL;
CString CIndex::myData("");
CString CIndex::myURL01("");
CString CIndex::myURL02("");
CString CIndex::myURL_code01("");
CString CIndex::myURL_code02("");
CString CIndex::m_URL("http://hq.sinajs.cn/list=");
CString CIndex::m_hexunA50("http://webglobal.hermes.hexun.com/global_index/quotelist?code=FTSE.FTXIN9&column=LastClose");
CString CIndex::m_hexunHS300("http://flashquote.stock.hexun.com/Stock_Combo.ASPX?mc=1_000300&dt=Q,MI&t=0.9522008465137333");
extern MYSQL *connindex;
CIndex::CIndex()
{
	connindex = NULL;
	myHttpFile = NULL;
	myHttpFile01 = NULL;
	myHttpFile02 = NULL;
	for(unsigned int k = 0;k < A50NUM;k++){
		myURL_code01 = myURL_code01 + ",";
		myURL_code01 = myURL_code01 + g_a50[k].exch.c_str();
		myURL_code01 = myURL_code01 + g_a50[k].code.c_str();
	}
	unsigned int tempnum;
	tempnum = HS300NUM;//��������ȡHS300�ɼ������������Ŀ
	for(unsigned int k = 0;k < HS300NUM;k++){
		if(tempnum <= HS300NUM){
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
	for(unsigned int j = 0;j < A50NUM + HS300NUM;j++){
		if(j < A50NUM){
			volume[j] = g_a50[j].volume;
		}
		else{
			volume[j] = g_hs300[j - A50NUM].volume;
		}
	}
	myURL01 = m_URL + myURL_code01;
	if(tempnum <= HS300NUM){
		myURL02 = m_URL + myURL_code02;
	}
	else{
		myURL02 = "";
	}
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
	//����ָ��������
	if(connindex){
		if(mysql_query(connindex,"CREATE TABLE IF NOT EXISTS HISTARINDEX (name VARCHAR(40),A50REF DOUBLE,A50VALUE DOUBLE,HS300REF DOUBLE,HS300VALUE DOUBLE,primary key (name))")) 
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
			}	
		}
	}
	GetIndexYd();//��������ָ��
	_timerID = SetTimer(NULL,0,(UINT)3000,UpdateIndexData); 
	return TRUE;
}

int CIndex::ExitInstance()
{
	TRACE("�˳�ָ����ȡ�߳�\r\n");
	KillTimer(NULL, _timerID);
	return CWinThread::ExitInstance();
}

void CIndex::OnUpdateIndexRef(WPARAM wParam,LPARAM lParam){
	GetQuotation();//��ȡ����
	totalValueA50ZT = 0;totalValueHS300ZT = 0;
	for(unsigned int i = 0;i <A50NUM + HS300NUM;i++){
		if(i < A50NUM){
			totalValueA50ZT = totalValueA50ZT + priceZT[i] * volume[i];
		}
		if(i >= A50NUM){
			totalValueHS300ZT = totalValueHS300ZT + priceZT[i] * volume[i];
		}
	}
	GetIndexYd();//��������ָ��
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
		//����ָ��
		sprintf_s(data01,"INSERT INTO HISTARINDEX (name,A50REF,A50VALUE,HS300REF,HS300VALUE) VALUES('indexref',%.02lf,%.02lf,%.02lf,%.02lf) ON DUPLICATE KEY UPDATE A50REF = %.02lf,A50VALUE = %.02lf,HS300REF = %.02lf,HS300VALUE = %.02lf",g_A50IndexZT,totalValueA50ZT,g_HS300IndexZT,totalValueHS300ZT,g_A50IndexZT,totalValueA50ZT,g_HS300IndexZT,totalValueHS300ZT);
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
		if(myURL02 != ""){
			myHttpFile02 = (CHttpFile*)mySession.OpenURL(myURL02,1,INTERNET_FLAG_RELOAD|INTERNET_FLAG_TRANSFER_ASCII);
		}
	}
	catch(CInternetException*pException){
		pException->Delete();
		myHttpFile01->Close();
		delete myHttpFile01;
		myHttpFile01 = NULL;
		if(myURL02 != ""){
			myHttpFile02->Close();
			delete myHttpFile02;	
			myHttpFile02 = NULL;
		}
		TRACE("��ȡָ��ʧ��!\r\n");
		return;//��ȡʧ��,����
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
			AfxExtractSubString(strGet2,strGet1,3, _T(','));//���ڵļ۸�
			AfxExtractSubString(strGet3,strGet1,2, _T(','));//����ļ۸�
			LPTSTR  chValue = strGet2.GetBuffer( strGet2.GetLength() );
			LPTSTR  chValueZT = strGet3.GetBuffer( strGet3.GetLength() );
			double fZT = atof(chValueZT);//����ļ۸�
			strGet3.ReleaseBuffer(); 
			priceZT[i] = fZT;
			double fValue = atof(chValue);//����ļ۸�
			strGet2.ReleaseBuffer(); 
			if(fValue > 0.1){//��ֹ����0������0������������̼�
				price[i] = fValue;
			}
			else{
				price[i] = priceZT[i];
			}
			i++;
		}
	}
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
				AfxExtractSubString(strGet2,strGet1,3, _T(','));//���ڵļ۸�
				AfxExtractSubString(strGet3,strGet1,2, _T(','));//����ļ۸�
				LPTSTR  chValue = strGet2.GetBuffer( strGet2.GetLength() );
				LPTSTR  chValueZT = strGet3.GetBuffer( strGet3.GetLength() );
				double fZT = atof(chValueZT);//����ļ۸�
				strGet3.ReleaseBuffer(); 
				priceZT[i] = fZT;
				double fValue = atof(chValue);//����ļ۸�
				strGet2.ReleaseBuffer(); 
				if(fValue > 0.1){//��ֹ����0������0������������̼�
					price[i] = fValue;
				}
				else{
					price[i] = priceZT[i];
				}
				i++;
			}
		}
	}
	myHttpFile01->Close();
	delete myHttpFile01;
	myHttpFile01 = NULL;
	if(myURL02 != ""){
		myHttpFile02->Close();
		delete myHttpFile02;	
		myHttpFile02 = NULL;
	}
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
		TRACE("��ȡָ��ʧ��!\r\n");
		return;//��ȡʧ��,����
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
		TRACE("��ȡ����ʧ��!\r\n");
		return;//��ȡʧ��,����
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
