#include "stdafx.h"
#include "afxsock.h"
#include <afxinet.h> 
#include <vector>
#include "Index.h"
#include <string>
#include "mysql.h"
#include "HiStar.h"
#define A50NUM 50
#define HS300NUM 300
#define TOTAL 350
double price[TOTAL];
double priceZT[TOTAL];
double volume[TOTAL];
double g_A50Index = 0;
double g_HS300Index = 0;
double g_A50IndexZT = 0,g_HS300IndexZT = 0;
extern double A50IndexRef,A50totalValueRef,HS300IndexRef,HS300totalValueRef;
struct stock{
	std::string exch;
	std::string code;
	int volume;
};
extern std::vector<stock> g_hs300;
extern std::vector<stock> g_a50;
MYSQL *connindex;
VOID CIndex::UpdateIndexData(HWND wnd, UINT msg, UINT_PTR id, DWORD d)
{
	try{
		myHttpFile = (CHttpFile*)mySession.OpenURL(myURL,1,INTERNET_FLAG_RELOAD|INTERNET_FLAG_TRANSFER_ASCII);
	}
	catch(CInternetException*pException){
		pException->Delete();
		myHttpFile->Close();
		delete myHttpFile;
		myHttpFile = NULL;
		TRACE("读取指数失败!\r\n");
		return;//读取失败,返回
	}
	if(myHttpFile != NULL){
		int i = 0;
		while(myHttpFile->ReadString(myData))
		{
			CString strGet1(_T("")); 
			CString strGet2(_T(""));
			CString strGet3(_T(""));
			double temp = 0;
			AfxExtractSubString(strGet1,myData,1, _T('\"'));
			AfxExtractSubString(strGet2,strGet1,3, _T(','));//现在的价格
			LPTSTR  chValue = strGet2.GetBuffer( strGet2.GetLength() );
			LPTSTR  chValueZT = strGet3.GetBuffer( strGet3.GetLength() );
			double fValue = atof(chValue); //今天的价格
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
	//计算A50和HS300指数
	double totalValueA50 = 0;
	double totalValueHS300 = 0;
	for(int i = 0;i < TOTAL;i++){
		if(i < A50NUM){
			totalValueA50 = totalValueA50 + price[i] * volume[i];
		}
		if(i >= A50NUM){
			totalValueHS300 = totalValueHS300 + price[i] * volume[i];
		}
	}
	g_A50Index = A50IndexRef * totalValueA50 / A50totalValueRef;
	g_HS300Index = HS300IndexRef * totalValueHS300 / HS300totalValueRef;
	myHttpFile->Close();
	delete myHttpFile;
	myHttpFile = NULL;
	return;
}