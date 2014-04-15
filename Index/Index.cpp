// Index.cpp : 实现文件
//

#include "stdafx.h"
#include "Index.h"
#include "UserMsg.h"
#include "afxsock.h"
#include <afxinet.h> 
extern int GetIndexData(void);
extern double g_A50IndexZT,g_HS300IndexZT;
VOID CALLBACK timerFun(HWND wnd, UINT msg, UINT_PTR id, DWORD d);
// CIndex

IMPLEMENT_DYNCREATE(CIndex, CWinThread)

CIndex::CIndex()
{
}

CIndex::~CIndex()
{
}

BOOL CIndex::InitInstance()
{
	CInternetSession mySession(NULL,0);
	CHttpFile* myHttpFile = NULL;
	CString myData;
	CString myURL;
	CString myURL_code;
	CString m_hexunA50 = _T("http://webglobal.hermes.hexun.com/global_index/quotelist?code=FTSE.FTXIN9&column=LastClose");
	CString m_hexunHS300 = _T("http://flashquote.stock.hexun.com/Stock_Combo.ASPX?mc=1_000300&dt=Q,MI&t=0.9522008465137333");
	try{
		myHttpFile = (CHttpFile*)mySession.OpenURL(m_hexunA50,1,INTERNET_FLAG_RELOAD|INTERNET_FLAG_TRANSFER_ASCII);
	}
	catch(CInternetException*pException){
		pException->Delete();
		myHttpFile->Close();
		delete myHttpFile;
		myHttpFile = NULL;
		mySession.Close();
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
		mySession.Close();
		TRACE("读取指数失败!\r\n");
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
	_timerID = SetTimer(NULL,0,(UINT)3000,timerFun); 
	mySession.Close();
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
VOID CALLBACK timerFun(HWND wnd, UINT msg, UINT_PTR id, DWORD d){
	GetIndexData();
}
