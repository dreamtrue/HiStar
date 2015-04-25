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
extern double totalValueSH50ZT;
extern unsigned int A50NUM;
extern unsigned int HS300NUM;
extern unsigned int SH50NUM;
double price[400];
double priceZT[400];
double volume[400];
double g_A50Index = 0;
double g_HS300Index = 0;
double g_SH50Index = 0;
double g_A50IndexZT = 0.0,g_HS300IndexZT = 0.0,g_SH50IndexZT = 0.0;
extern double A50IndexRef,A50totalValueRef,HS300IndexRef,HS300totalValueRef,SH50IndexRef,SH50totalValueRef;
extern std::vector<stock> g_hs300;
extern std::vector<stock> g_a50;
extern std::vector<stock> g_sh50;
MYSQL *connindex; 
VOID CIndex::UpdateIndexData(HWND wnd, UINT msg, UINT_PTR id, DWORD d)
{
	GetQuotation();
	//计算A50和HS300指数
	double totalValueA50 = 0.0;
	double totalValueHS300 = 0.0;
	double totalValueSH50 = 0.0;
	totalValueA50ZT = 0.0;
	totalValueHS300ZT = 0.0;
	totalValueSH50ZT = 0.0;
	for(unsigned int i = 0;i < A50NUM + HS300NUM + SH50NUM;i++){
		if(i < A50NUM){
			totalValueA50 = totalValueA50 + price[i] * volume[i];
			totalValueA50ZT = totalValueA50ZT + priceZT[i] * volume[i];
		}
		else if(i < A50NUM + HS300NUM){
			totalValueHS300 = totalValueHS300 + price[i] * volume[i];
			totalValueHS300ZT = totalValueHS300ZT + priceZT[i] * volume[i];
		}
		else{
			totalValueSH50 = totalValueSH50 + price[i] * volume[i];
			totalValueSH50ZT = totalValueSH50ZT + priceZT[i] * volume[i];
		}
	}
	g_A50Index = A50IndexRef * totalValueA50 / A50totalValueRef;
	g_HS300Index = HS300IndexRef * totalValueHS300 / HS300totalValueRef;
	g_SH50Index = SH50IndexRef * totalValueSH50 / SH50totalValueRef;
	//由于A50指数当前无法从和讯查询，故将昨天指数直接计算出来
	g_A50IndexZT = A50IndexRef * totalValueA50ZT / A50totalValueRef;
	g_HS300IndexZT = HS300IndexRef * totalValueHS300ZT / HS300totalValueRef;
	g_SH50IndexZT = SH50IndexRef * totalValueSH50ZT / SH50totalValueRef;
	return;
}