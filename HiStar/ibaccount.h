#ifndef IBACCOUNT_H
#define IBACCOUNT_H
#include "contract.h"
#include <vector>
struct Portfolio{
	IBString account;
	Contract contract;
	int position; 
	double avgCost;
	double marketPrice;
	double marketValue;
	double unrealizedPNL;
	double realizedPNL;
};
struct AccountValue{
	double AvailableFunds;
	double NetLiquidation;
	double AvailableFundsO;
	double NetLiquidationO;
	double UnrealizedPnLO;
	double InitMarginReqO;
	double InitMarginOIfAddOneA50;
};
#endif