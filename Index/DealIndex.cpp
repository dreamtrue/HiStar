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
		myHttpFile = (CHttpFile*)mySession.OpenURL(myURL);
	}
	catch(CInternetException*pException){
		pException->Delete();
		return -1;//��ȡʧ��,����
	}
	if(myHttpFile != NULL){
		try{
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
		catch(CInternetException*pException){
			pException->Delete();
			return -1;//����
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
	mySession.Close();
	return 0;
}