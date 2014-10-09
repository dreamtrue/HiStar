#include "stdafx.h"
#include "afxsock.h"
#include <afxinet.h> 
#include <vector>
#include "Index.h"
#include <string>
#include "HiStar.h"
#include "me.h"
extern double totalValueA50ZT;
extern double totalValueHS300ZT;
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
	GetQuotation();
	//计算A50和HS300指数
	double totalValueA50 = 0.0;
	double totalValueHS300 = 0.0;
	totalValueA50ZT = 0.0;
	totalValueHS300ZT = 0.0;
	for(unsigned int i = 0;i < A50NUM + HS300NUM;i++){
		if(i < A50NUM){
			totalValueA50 = totalValueA50 + price[i] * volume[i];
			totalValueA50ZT = totalValueA50ZT + priceZT[i] * volume[i];
		}
		if(i >= A50NUM){
			totalValueHS300 = totalValueHS300 + price[i] * volume[i];
			totalValueHS300ZT = totalValueHS300ZT + priceZT[i] * volume[i];
		}
	}
	g_A50Index = A50IndexRef * totalValueA50 / A50totalValueRef;
	g_HS300Index = HS300IndexRef * totalValueHS300 / HS300totalValueRef;
	//由于A50指数当前无法从和讯查询，故将昨天指数直接计算出来
	g_A50IndexZT = A50IndexRef * totalValueA50ZT / A50totalValueRef;
	g_HS300IndexZT = HS300IndexRef * totalValueHS300ZT / HS300totalValueRef;
	return;
}