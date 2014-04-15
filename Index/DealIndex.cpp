#include "stdafx.h"
#include "afxsock.h"
#include <afxinet.h> 
#include <vector>
#include <string>
#define A50NUM 50
#define HS300NUM 300
#define TOTAL 350
CString m_URL = _T("http://hq.sinajs.cn/list=");
double A50IndexRef;
double A50totalVolumeRef;
double HS300IndexRef;
double HS300totalVolumeRef;
double price[TOTAL];
double priceZT[TOTAL];
double volume[TOTAL];// = {A50PL,HS300PL};
double g_A50Index = 0;
double g_HS300Index = 0;
double g_A50IndexZT = 0,g_HS300IndexZT = 0;
struct stock{
	std::string exch;
	std::string code;
	int volume;
};
extern std::vector<stock> g_hs300;
extern std::vector<stock> g_a50;
int GetIndexData(void)
{
	CInternetSession mySession(NULL,0);
	CHttpFile* myHttpFile = NULL;
	CString myData;
	CString myURL;
	CString myURL_code;
	int i = 0;
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
	myURL = m_URL + myURL_code;
	for(unsigned int j = 0;j < TOTAL;j++){
		if(j < A50NUM){
			volume[j] = g_a50[j].volume;
		}
		else{
			volume[j] = g_hs300[j - A50NUM].volume;
		}
	}
	try{
		myHttpFile = (CHttpFile*)mySession.OpenURL(myURL,1,INTERNET_FLAG_RELOAD|INTERNET_FLAG_TRANSFER_ASCII);
	}
	catch(CInternetException*pException){
		pException->Delete();
		myHttpFile->Close();
		delete myHttpFile;
		myHttpFile = NULL;
		mySession.Close();
		TRACE("读取指数失败!\r\n");
		return -1;//读取失败,返回
	}
	if(myHttpFile != NULL){
		while(myHttpFile->ReadString(myData))
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
			double fValue = atof(chValue); //今天的价格
			double fZT = atof(chValueZT);//昨天的价格
			strGet2.ReleaseBuffer(); 
			if(fValue > 0.1){//防止等于0，等于0就用昨天的收盘价
				price[i] = fValue;
			}
			else{
				price[i] = fZT;
			}
			priceZT[i] = fZT;
			i++;
		}
	}
	//计算A50和HS300指数
	double totalValueA50 = 0,totalValueA50ZT = 0;
	double totalValueHS300 = 0,totalValueHS300ZT = 0;
	for(int i = 0;i < TOTAL;i++){
		if(i < A50NUM){
			totalValueA50 = totalValueA50 + price[i] * volume[i];
			totalValueA50ZT = totalValueA50ZT + priceZT[i] * volume[i];
		}
		if(i >= A50NUM){
			totalValueHS300 = totalValueHS300 + price[i] * volume[i];
			totalValueHS300ZT = totalValueHS300ZT + priceZT[i] * volume[i];
			/*TRACE("%.f\n",price[i] * volume[i]);*/
		}
	}
	//g_A50Index = A50IndexRef * totalValueA50 / A50totalVolumeRef;
	//g_HS300Index = HS300IndexRef * totalValueHS300 / HS300totalVolumeRef;
	g_A50Index = g_A50IndexZT * totalValueA50 / totalValueA50ZT;
	g_HS300Index = g_HS300IndexZT * totalValueHS300 / totalValueHS300ZT;
	myHttpFile->Close();
	delete myHttpFile;
	myHttpFile = NULL;
	mySession.Close();
	return 0;
}