#include "stdafx.h"
#include "afxsock.h"
#include <afxinet.h> 
#include <vector>
#include "Index.h"
#include <string>
#include "HiStar.h"
#include "me.h"
extern unsigned int A50NUM;
extern unsigned int HS300NUM;
double price[350];
double priceZT[350];
double volume[350];
double g_A50Index = 0;
double g_HS300Index = 0;
double g_A50IndexZT = 0,g_HS300IndexZT = 0;
extern double A50IndexRef,A50totalValueRef,HS300IndexRef,HS300totalValueRef;
extern std::vector<stock> g_hs300;
extern std::vector<stock> g_a50;
MYSQL *connindex;
VOID CIndex::UpdateIndexData(HWND wnd, UINT msg, UINT_PTR id, DWORD d)
{
	try{
		myHttpFile01 = (CHttpFile*)mySession.OpenURL(myURL01,1,INTERNET_FLAG_RELOAD|INTERNET_FLAG_TRANSFER_ASCII);
		myHttpFile02 = (CHttpFile*)mySession.OpenURL(myURL02,1,INTERNET_FLAG_RELOAD|INTERNET_FLAG_TRANSFER_ASCII);
	}
	catch(CInternetException*pException){
		pException->Delete();
		myHttpFile01->Close();
		myHttpFile02->Close();
		delete myHttpFile01;
		delete myHttpFile02;
		myHttpFile01 = NULL;
		myHttpFile02 = NULL;
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
			LPTSTR  chValue = strGet2.GetBuffer( strGet2.GetLength() );
			LPTSTR  chValueZT = strGet3.GetBuffer( strGet3.GetLength() );
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
			LPTSTR  chValue = strGet2.GetBuffer( strGet2.GetLength() );
			LPTSTR  chValueZT = strGet3.GetBuffer( strGet3.GetLength() );
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
	//����A50��HS300ָ��
	double totalValueA50 = 0;
	double totalValueHS300 = 0;
	for(unsigned int i = 0;i < A50NUM + HS300NUM;i++){
		if(i < A50NUM){
			totalValueA50 = totalValueA50 + price[i] * volume[i];
		}
		if(i >= A50NUM){
			totalValueHS300 = totalValueHS300 + price[i] * volume[i];
		}
	}
	g_A50Index = A50IndexRef * totalValueA50 / A50totalValueRef;
	g_HS300Index = HS300IndexRef * totalValueHS300 / HS300totalValueRef;
	myHttpFile01->Close();
	myHttpFile02->Close();
	delete myHttpFile01;
	delete myHttpFile02;
	myHttpFile01 = NULL;
	myHttpFile02 = NULL;
	return;
}