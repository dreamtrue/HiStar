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
double volume[TOTAL];// = {A50PL,HS300PL};
double g_A50Index = 0;
double g_HS300Index = 0;
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
	//myURL = m_URL + _T(A50ID) + _T(HS300ID);
	try{
		myHttpFile = (CHttpFile*)mySession.OpenURL(myURL,1,INTERNET_FLAG_RELOAD|INTERNET_FLAG_TRANSFER_ASCII);
	}
	catch(CInternetException*pException){
		pException->Delete();
		myHttpFile->Close();
		delete myHttpFile;
		myHttpFile = NULL;
		mySession.Close();
		TRACE("��ȡָ��ʧ��!\r\n");
		return -1;//��ȡʧ��,����
	}
	if(myHttpFile != NULL){
		while(myHttpFile->ReadString(myData))
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
			double fValue = atof(chValue); //����ļ۸�
			double fZT = atof(chValueZT);//����ļ۸�
			strGet2.ReleaseBuffer(); 
			if(fValue > 0.1){//��ֹ����0������0������������̼�
				price[i] = fValue;
			}
			else{
				price[i] = fZT;
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