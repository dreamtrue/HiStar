// E:\新建文件夹\trunk\Hedge\HedgeLoop.cpp : 实现文件
//

#include "stdafx.h"
#include "HiStar.h"
#include "HedgePostProcessing.h"
#include "UserMsg.h"
#include <math.h>
double datumDiff = 0;
bool isHedgeLoopingPause = false;
double premium = 0,premiumHigh = 0,premiumLow = 0;
double deviation = 0,DeviationSell = 0,DeviationBuy = 0;
extern double g_a50Bid1,g_a50Ask1;
extern double g_ifAsk1,g_ifBid1;
extern double g_A50Index,g_HS300Index;
double DeviationSell_save,DeviationBuy_save;;
struct HoldDetail{
	double HedgePrice;
	int HedgeNum;
	int HedgeSection;//开仓时所在的Section
};
std::vector<HoldDetail> HedgeHold;

void CalcDeviation(){
	premium = (g_a50Bid1 + g_a50Ask1) / 2.0 - (g_ifAsk1 + g_ifBid1) / 2.0 * g_A50Index / g_HS300Index;
	premiumHigh = g_a50Ask1 - g_ifBid1 * g_A50Index / g_HS300Index;
	premiumLow = g_a50Bid1 - g_ifAsk1 * g_A50Index / g_HS300Index;
	deviation = premium - datumDiff;
	DeviationSell = premiumHigh - datumDiff;
	DeviationBuy = premiumLow - datumDiff;
}
// CHedgePostProcessing

IMPLEMENT_DYNCREATE(CHedgePostProcessing, CWinThread)

CHedgePostProcessing::CHedgePostProcessing()
{

}

CHedgePostProcessing::~CHedgePostProcessing()
{
}

BOOL CHedgePostProcessing::InitInstance()
{
	return TRUE;
}

int CHedgePostProcessing::ExitInstance()
{
	TRACE("退出Hedge线程\n");
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CHedgePostProcessing, CWinThread)

END_MESSAGE_MAP()


void CHiStarApp::OnHedgeLooping(UINT wParam,LONG lParam){
	int MultiPos = 1;//持仓乘数
	//梯级，一共21个分割点,分割成22(=21+1)个区间
	double HedgeLadder[21] = {   -200, -180, -160, -140, -120, -100, -80, -60, -40, -20, -10,  20,  40,  60,  80,  100,  120,  140,  160,  180,  200};
	int PositionAimUnit[22] = {10,    9,    8,    7,    6,    5,    4,   3,   2,   1,   0,   0,  -1,  -2,  -3,  -4,  -5,    -6,   -7,   -8,   -9,   -10};//默认持仓目标单位（没有乘以乘数）
	int PositionAim[22];
	//默认盈利目标,一共22个区间，包含左右两个无边界区间
	double DefaultProfitAimBuy[22],DefaultProfitAimSell[22],ProfitAimBuy[22],ProfitAimSell[22];
	double MaxProfitAim = 20.0,MinProfitAim = 20.0;//最小盈利目标，最大盈利目标（不分多空）
	int netPosition = 0;//净持仓
	int SupposedBuyOpen = 0,SupposedSellOpen = 0;
	int SupposedSectionBuyOpen = 0,SupposedSectionSellOpen = 0;
	//增加这两个变量主要是考虑两侧边界可能已经饱和，这是isSupposedBuy或isSupposedSell将仍然保持false，表示无需任何操作
	bool isSupposedBuyOpen = false,isSupposedSellOpen = false;
	int CurrentSectionBuy = 0,CurrentSectionSell = 0;//当前所在的区间,Buy和Sell分别表示以买价和卖价计算
	CString PREMIUM;	
	CalcDeviation();
	//测试用，给一个随机的DeviationSell和DeviationBuy
	DeviationSell = rand() % 60 - 30;
	DeviationBuy = DeviationSell - 6.5;
	TRACE("当前买偏差%f\n",DeviationBuy);
	TRACE("当前卖偏差%f\n",DeviationSell);
	/////////////////////////////////////////////////
	DeviationSell_save = DeviationSell;
	DeviationBuy_save = DeviationBuy;	

	if(isHedgeLoopingPause){//暂停
	TRACE(_T("暂停\n"));
	return;
	}
	if(_isnan(datumDiff) != 0 || _isnan(premium)!=0 ||_isnan(deviation)!=0){
	return;//判断非零值错误
	}
	if(g_a50Bid1 < 1 || g_a50Ask1 < 1 || g_ifAsk1 < 1 || g_ifBid1 < 1 || g_A50Index < 1 || g_HS300Index < 1){
	return;
	}
	if(fabs(premium) > 300 || fabs(premium) < 0.01){
	return;//排除开盘时有可能报价不全导致的错误溢价计算
	}

	//统计净持仓
	for(int i = 0;i < HedgeHold.size();i++){
		netPosition = netPosition + HedgeHold[i].HedgeNum;
	}
	TRACE("净持仓%d\n",netPosition);
	for(int i = 1;i < 21;i++){
		if(HedgeLadder[i - 1] > HedgeLadder[i]){
			TRACE("错误的梯级\n");
			return;
		}
		if(PositionAimUnit[i - 1] < PositionAimUnit[i]){
			TRACE("错误的目标持仓\n");
			return;
		}
	}
	if(MaxProfitAim < MinProfitAim){
		TRACE("MaxProfitAim小于MinProfitAim\n");
		return;
	}
	//计算当前区间
	for(int i = 0;i < 22;i++){
		if(i == 0){
			if(DeviationBuy < HedgeLadder[0]){
				CurrentSectionBuy = 0;
			}
			if(DeviationSell < HedgeLadder[0]){
				CurrentSectionSell = 0;
			}
		}
		else if(i == 21){
			if(DeviationBuy >= HedgeLadder[20]){
				CurrentSectionBuy = 21;
			}
			if(DeviationSell >= HedgeLadder[20]){
				CurrentSectionSell = 21;
			}
		}
		else{
			if(DeviationBuy >= HedgeLadder[i] && DeviationBuy < HedgeLadder[i + 1]){
				CurrentSectionBuy = i + 1;
			}
			if(DeviationSell >= HedgeLadder[i] && DeviationSell < HedgeLadder[i + 1]){
				CurrentSectionSell = i + 1;
			}
		}
	}
	TRACE("当前买价区间%d,左%f,右%f\n",CurrentSectionBuy,HedgeLadder[CurrentSectionBuy - 1],HedgeLadder[CurrentSectionBuy]);
	TRACE("当前卖价区间%d,左%f,右%f\n",CurrentSectionSell,HedgeLadder[CurrentSectionSell - 1],HedgeLadder[CurrentSectionSell]);
	for(int i= 0;i < 22;i++){
		PositionAim[i] = PositionAimUnit[i] * MultiPos;
	}
	for(int i = 0;i < 22;i++){
		if(i < 20){
			DefaultProfitAimBuy[i] = HedgeLadder[i + 1] -  HedgeLadder[i];
		}
		else{
			DefaultProfitAimBuy[i] = 20.0;//边界点
		}
		if(i > 1){
			DefaultProfitAimSell[i] = HedgeLadder[i - 1] - HedgeLadder[i - 2];
		}
		else{
			DefaultProfitAimSell[i] = 20.0;//边界点
		}
		ProfitAimBuy[i] = max(DefaultProfitAimBuy[i],MinProfitAim);
		ProfitAimBuy[i] = min(ProfitAimBuy[i],MaxProfitAim);
		ProfitAimSell[i] = max(DefaultProfitAimSell[i],MinProfitAim);
		ProfitAimSell[i] = min(ProfitAimSell[i],MaxProfitAim);
		//TRACE("区间%d买目标盈利%f,最大盈利%f,最小盈利%f\n",i,ProfitAimBuy[i],MaxProfitAim,MinProfitAim);
		//TRACE("区间%d卖目标盈利%f,最大盈利%f,最小盈利%f\n",i,ProfitAimSell[i],MaxProfitAim,MinProfitAim);
	}
	for(int i = 0;i <= 21;i++){
		if(min(netPosition,0) > PositionAim[i]){
			SupposedSellOpen = min(netPosition,0) - PositionAim[i];
			SupposedSectionSellOpen = i;
			isSupposedSellOpen = true;
			TRACE("期望卖开%d期望区间%d,左%f,右%f\n",SupposedSellOpen,i,HedgeLadder[i - 1],HedgeLadder[i]);
			break;
		}
	}
	for(int i = 21;i >= 0;i--){
		if(max(netPosition,0) < PositionAim[i]){
			SupposedBuyOpen = -(max(netPosition,0) - PositionAim[i]);
			SupposedSectionBuyOpen = i;
			isSupposedBuyOpen = true;
			TRACE("期望买开%d期望区间%d,左%f,右%f\n",SupposedBuyOpen,i,HedgeLadder[i - 1],HedgeLadder[i]);
			break;
		}
	}
	//开仓操作
	if(isSupposedBuyOpen){
		if(CurrentSectionSell <= SupposedSectionBuyOpen){
			//需要开仓
			//Open(SupposedBuy,BUY);
			HoldDetail newhold;
			newhold.HedgeNum = SupposedBuyOpen;
			newhold.HedgePrice = DeviationSell;
			newhold.HedgeSection = CurrentSectionSell;
			TRACE("需要开仓%d手,价格%f,所在区间%d,左%f,右%f\n",newhold.HedgeNum,newhold.HedgePrice,newhold.HedgeSection,HedgeLadder[newhold.HedgeSection - 1],HedgeLadder[newhold.HedgeSection]);
			HedgeHold.push_back(newhold);
		}
	}
	if(isSupposedSellOpen){
		if(CurrentSectionBuy >= SupposedSectionSellOpen){
			//需要开仓
			//Open(SupposedSell,SELL);
			HoldDetail newhold;
			newhold.HedgeNum = -SupposedSellOpen;
			newhold.HedgePrice = DeviationBuy;
			newhold.HedgeSection = CurrentSectionBuy;
			TRACE("需要开仓%d手,价格%f,所在区间%d,左%f,右%f\n",newhold.HedgeNum,newhold.HedgePrice,newhold.HedgeSection,HedgeLadder[newhold.HedgeSection - 1],HedgeLadder[newhold.HedgeSection]);
			HedgeHold.push_back(newhold);
		}
	}
	//平仓操作
	for(int i = 0;i < HedgeHold.size();i++){
		if(HedgeHold[i].HedgeNum > 0){//多头持仓
			if(HedgeHold[i].HedgeSection == 21){
				//需要平仓
				//Close(HedgeHold[i].HedgeNum,SELL);
				if(DeviationBuy >= HedgeLadder[20] + ProfitAimBuy[HedgeHold[i].HedgeSection]){
					//需要平仓
					//Close(HedgeHold[i].HedgeNum,SELL);
					TRACE("需要平仓%d手,价格%f\n",-HedgeHold[i].HedgeNum,DeviationBuy);
					HedgeHold.erase(HedgeHold.begin() + i);
					i--;
				}
			}
			else{
				if(DeviationBuy >= HedgeLadder[HedgeHold[i].HedgeSection] + ProfitAimBuy[HedgeHold[i].HedgeSection]){
					//需要平仓
					//Close(HedgeHold[i].HedgeNum,SELL);
					TRACE("需要平仓%d手,价格%f\n",-HedgeHold[i].HedgeNum,DeviationBuy);
					HedgeHold.erase(HedgeHold.begin() + i);
					i--;
				}
			}
		}
		else if(HedgeHold[i].HedgeNum < 0){//空头持仓
			if(HedgeHold[i].HedgeSection == 0){
				if(DeviationSell <= HedgeLadder[0] - ProfitAimSell[HedgeHold[i].HedgeSection]){
					//Close(HedgeHold[i].HedgeNum,BUY);
					TRACE("需要平仓%d手,价格%f\n",-HedgeHold[i].HedgeNum,DeviationSell);
					HedgeHold.erase(HedgeHold.begin() + i);
					i--;
				}
			}
			else{
				if(DeviationSell <= HedgeLadder[HedgeHold[i].HedgeSection - 1] - ProfitAimSell[HedgeHold[i].HedgeSection]){
					//Close(HedgeHold[i].HedgeNum,BUY);
					TRACE("需要平仓%d手,价格%f\n",-HedgeHold[i].HedgeNum,DeviationSell);
					HedgeHold.erase(HedgeHold.begin() + i);
					i--;
				}
			}
		}
	}
	TRACE("-------------------------------------------END--------------------------------------------\n");
}

void CHedgePostProcessing::PostProcessing(UINT wParam,LONG lParam){


}