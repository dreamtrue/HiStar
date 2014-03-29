#include "stdafx.h"
#include "afxsock.h"
#include <afxinet.h> 
#include "DealIndex.h"
#define A50NUM 50
#define HS300NUM 300
#define TOTAL 350
CString m_URL = _T("http://hq.sinajs.cn/list=");
double price[TOTAL];
double volume[TOTAL] = {A50PL,HS300PL};
double g_A50Index = 0;
double g_HS300Index = 0;
int GetIndexData(void)
{
	CInternetSession mySession(NULL,0);
	CHttpFile* myHttpFile = NULL;
	CString myData;
	CString myURL;
	int i = 0;
	myURL = m_URL + _T(A50ID) + _T(HS300ID);
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
			/*TRACE("%.f\n",price[i] * volume[i]);*/
		}
	}
	g_A50Index = A50IndexRef * totalValueA50 / A50totalVolumeRef;
	g_HS300Index = HS300IndexRef * totalValueHS300 / HS300totalVolumeRef;
	myHttpFile->Close();
	delete myHttpFile;
	myHttpFile = NULL;
	mySession.Close();
	return 0;
}