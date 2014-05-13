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
		TRACE("��ȡָ��ʧ��!\r\n");
		return;//��ȡʧ��,����
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
			AfxExtractSubString(strGet2,strGet1,3, _T(','));//���ڵļ۸�
			LPTSTR  chValue = strGet2.GetBuffer( strGet2.GetLength() );
			LPTSTR  chValueZT = strGet3.GetBuffer( strGet3.GetLength() );
			double fValue = atof(chValue); //����ļ۸�
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
	//����ָ��
	char data01[1000];
	sprintf(data01,"UPDATE HISTARINDEX SET A50REF = %.02lf,A50VALUE = %.02lf,HS300REF = %.02lf,HS300VALUE = %.02lf WHERE NAME = 'index'",g_A50Index,totalValueA50,g_HS300Index,totalValueHS300);
	if(connindex){
		if(mysql_query(connindex,data01)){
			TRACE("Error %u: %s\n", mysql_errno(connindex), mysql_error(connindex));
		}
	}
	myHttpFile->Close();
	delete myHttpFile;
	myHttpFile = NULL;
	return;
}