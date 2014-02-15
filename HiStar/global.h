#ifndef GLOBAL_H
#define GLOBAL_H
#include "ThostFtdcMdApi.h"
#include "ThostFtdcTraderApi.h"
#pragma comment(lib,"..\\api_libs\\thostmduserapi.lib")
#pragma comment(lib,"..\\api_libs\\thosttraderapi.lib")
#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"ws2_32.lib")

#include <vector>

#define WAIT_MS	5000

using namespace std;
struct SAccountIB{
	CString  m_accountName;
};
struct SAccountCtp{

};

///合约保证金率
typedef CThostFtdcInstrumentMarginRateField MGRATE,*PMGRATE;
///合约手续费率
typedef CThostFtdcInstrumentCommissionRateField FEERATE,*PFEERATE;

typedef CThostFtdcInstrumentField INSTINFO,*PINSTINFO;

typedef struct CThostFtdcInstrumentFieldEx
{
	INSTINFO iinf;
	TThostFtdcRatioType	OpenRatioByMoney;
	TThostFtdcRatioType	OpenRatioByVolume;
	TThostFtdcRatioType	CloseRatioByMoney;
	TThostFtdcRatioType	CloseRatioByVolume;
	TThostFtdcRatioType	CloseTodayRatioByMoney;
	TThostFtdcRatioType	CloseTodayRatioByVolume;
}INSINFEX,*PINSINFEX;

typedef vector<CThostFtdcOrderField*> ::iterator VOrd;
typedef vector<CThostFtdcInvestorPositionField*> ::iterator VInvP;
typedef vector<CThostFtdcInstrumentFieldEx*>::iterator VIT_if;
typedef vector<CThostFtdcInstrumentMarginRateField*>::iterator VIT_mr;
typedef vector<CThostFtdcInstrumentCommissionRateField*>::iterator VIT_cf;






#endif