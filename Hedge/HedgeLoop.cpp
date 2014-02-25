// E:\新建文件夹\trunk\Hedge\HedgeLoop.cpp : 实现文件
//

#include "stdafx.h"
#include "HiStar.h"
#include "HedgeLoop.h"
#include "UserMsg.h"
int netHold = 0;//净持有
double datumDiff = 0;
int ladder = 0;
int needHold = 0;
double profitTarget = 0;//盈利目标
double premium = 0,premiumHigh = 0,premiumLow = 0;
double deviation = 0,deviationHigh = 0,deviationLow = 0;
std::vector<HoldAndDirec>::iterator itHedge;
extern double g_a50Bid1,g_a50Ask1;
extern double g_ifAsk1,g_ifBid1;
extern double g_A50Index,g_HS300Index;
double deviationHigh_save,deviationLow_save;
std::vector<HoldAndDirec> holdHedge;//在只允许一个A50交易线程的情况下,暂不考虑同步
int CloseOrOpen(int num,bool isbuy,bool isClose);
int BS(int needHold_temp,int netHold_temp);
void CalDeviation(){
	premium = (g_a50Bid1 + g_a50Ask1) / 2.0 - (g_ifAsk1 + g_ifBid1) / 2.0 * g_A50Index / g_HS300Index;
	premiumHigh = g_a50Ask1 - g_ifBid1 * g_A50Index / g_HS300Index;
	premiumLow = g_a50Bid1 - g_ifAsk1 * g_A50Index / g_HS300Index;
	deviation = premium - datumDiff;
	deviationHigh = premiumHigh - datumDiff;
	deviationLow = premiumLow - datumDiff;
}
// CHedgeLoop

IMPLEMENT_DYNCREATE(CHedgeLoop, CWinThread)

CHedgeLoop::CHedgeLoop()
: m_isPause(false)
{

}

CHedgeLoop::~CHedgeLoop()
{
}

BOOL CHedgeLoop::InitInstance()
{
	//初始化,step/multiply/aimOfLadder/datumDiff/tempIni均需要从界面获取初始化值
	step = 20.0;
	multiply = 12;
	aimOfLadder = -1;
	datumDiff = 0;
	tempIni.price = 0;//持仓相对价格
	tempIni.numDirec = 0;//带有方向的持仓
	holdHedge.push_back(tempIni);
	netHold = tempIni.numDirec;
	return TRUE;
}

int CHedgeLoop::ExitInstance()
{
	TRACE("退出Hedge线程\n");
	holdHedge.clear();
	netHold = 0;
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CHedgeLoop, CWinThread)
	ON_THREAD_MESSAGE(WM_MD_REFRESH,BeginHedge)
END_MESSAGE_MAP()


// CHedgeLoop 消息处理程序
void CHedgeLoop::BeginHedge(UINT wParam,LONG lParam){

	CString PREMIUM;	
	CalDeviation();
	deviationHigh_save = deviationHigh;
	deviationLow_save = deviationLow;	

	if(m_isPause){//暂停
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
	if(deviation >= 0){
		ladder = (int)(deviationLow / step);
	}
	else{
		ladder = -(int)(-deviationHigh / step);
	}

	if(ladder > 1){
		ladder = 1;
	}
	else if(ladder < -1){
		ladder = -1;
	}
	needHold = aimOfLadder * ladder;//需要的合约与偏差的关系,可以根据资金量进行修改
	profitTarget = step;

	double profitUnit = 0;
	int needBuyClose = 0,needSellClose = 0;//始终大于0
	double totalTradingValue = 0;//准备交易的总价值
	for(unsigned int j = 0;j < holdHedge.size();j++){
		itHedge = holdHedge.begin() + j;
		//if(itHedge->numDirec < 0){//其实只要nethold为负，就保证了所有的numDirec为负，不存在既有numDirec为正，又有为负的情况
		if(netHold < 0){
			profitUnit = itHedge->price - deviationHigh;
			if(profitUnit >= profitTarget){
				needBuyClose = needBuyClose - itHedge->numDirec;
				totalTradingValue = totalTradingValue + itHedge->price * (-itHedge->numDirec);
				//更新记录
				netHold = netHold - itHedge->numDirec;
				holdHedge.erase(itHedge);
				j--;//出现删除操作时,需要将指针向前倒退
			}
		}
		//else if(itHedge->numDirec > 0)
		else if(netHold > 0){
			profitUnit = -(itHedge->price - deviationLow);
			if(profitUnit >= profitTarget){
				needSellClose = needSellClose + itHedge->numDirec;
				totalTradingValue = totalTradingValue + itHedge->price * itHedge->numDirec;
				//更新记录
				netHold = netHold - itHedge->numDirec;
				holdHedge.erase(itHedge);
				j--;
			}
		}
		else{
			//等于0时什么也不做
		}
	}
	//平仓操作,,其实买平和卖平一定只有一个得到执行的，否则便出现了问题
	CloseOrOpen(needBuyClose,true,true);
	CloseOrOpen(needSellClose,false,true);
	//开仓操作
	BS(needHold,netHold);
}

int CloseOrOpen(int num,bool isbuy,bool isflat){
	return 0;
}

int BS(int needHold_temp,int netHold_temp){
	return 0;
}