// Index.cpp : 实现文件
//

#include "stdafx.h"
#include "Index.h"
#include "UserMsg.h"
#include "afxsock.h"
#include <afxinet.h>
#include <vector>
#define A50NUM 50
#define HS300NUM 300
extern double g_A50IndexZT,g_HS300IndexZT;
double totalValueA50ZT = 0;
double totalValueHS300ZT = 0;
double A50IndexRef;double A50totalValueRef;double HS300IndexRef;double HS300totalValueRef;
struct stock{
	std::string exch;
	std::string code;
	int volume;
};
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

CIndex::CIndex()
{
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

	A50totalValueRef = totalValueA50ZT;A50IndexRef = g_A50IndexZT;
	HS300totalValueRef = totalValueHS300ZT;HS300IndexRef = g_HS300IndexZT;

	_timerID = SetTimer(NULL,0,(UINT)3000,UpdateIndexData); 
	return TRUE;
}

int CIndex::ExitInstance()
{
	TRACE("退出指数获取线程\r\n");
	KillTimer(NULL, _timerID);
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CIndex, CWinThread)
END_MESSAGE_MAP()