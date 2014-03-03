// E:\新建文件夹\trunk\Hedge\HedgeLoop.cpp : 实现文件
//

#include "stdafx.h"
#include "HiStar.h"
#include "HedgePostProcessing.h"
#include "UserMsg.h"
#include "MainDlg.h"
#include <math.h>
#define OPEN true
#define CLOSE false
double datumDiff = -10;
double MultiInsA50 = 1.0;double MultiInsIf = 300.0;//合约每个点的价值
extern double g_A50IndexMSHQ;
extern double g_HS300IndexMSHQ;
int MultiA50 = 1;//A50乘数
double MarginA50 = 625.0,MarginIF = 0.15;
double USDtoRMB = 6.07;//汇率
bool isHedgeLoopingPause = true;
double premium = 0,premiumHigh = 0,premiumLow = 0;
double deviation = 0,DeviationSell = 0,DeviationBuy = 0;
extern double g_a50Bid1,g_a50Ask1;
extern double g_ifAsk1,g_ifBid1;
extern double g_A50Index,g_HS300Index;
/////////////////////////////////Hedge变量///////////////////////////////////////////////
int MultiPos = 1;//持仓乘数
//梯级，一共21个分割点,分割成22(=21+1)个区间
double HedgeLadder[21] = {   -200, -180, -160, -140, -120, -100, -80, -60, -40, -20, -10,  20,  40,  60,  80,  100,  120,  140,  160,  180,  200};
int PositionAimUnit[22] = {10,    9,    8,    7,    6,    5,    4,   3,   2,   1,   0,   0,  -1,  -2,  -3,  -4,  -5,    -6,   -7,   -8,   -9,   -10};//默认持仓目标单位（没有乘以乘数）
int PositionAim[22];
double MaxProfitAim = 20.0,MinProfitAim = 20.0;//最小盈利目标，最大盈利目标（不分多空）
///////////////////////////////////////////////////////
int NetPositionA50 = 0,PositionIFb = 0,PositionIFs = 0;//净持仓,需要计算;IF分空头净持仓和多头净持仓
double AvailIB = 0.0,AvailCtp = 0.0;//可用资金，需要计算
////////////////////////////////////////////////////////
char buffer[1000];
std::vector<HoldDetail> HedgeHold;
struct A50Task{
	int volumeRecord;char direction;//'l'表示长仓,'s'表示短舱
	double priceRecord;int id;int traded;double avgPrice;
	bool bReceivedAllStatus;
};
struct IfTask{
	int volumeRecord;TThostFtdcDirectionType direction;TThostFtdcCombOffsetFlagType offset;
	double priceRecord;int ref;int sysid;int traded;double avgPrice;
	bool bReceivedInsertRtn;bool bReceivedAllOrder;int receivedTradedVolume;double receivedValue;
};
struct HedgeTask{
	std::vector<A50Task> a50alltask;
	std::vector<IfTask>  ifalltask;
};
HedgeTask hedgetask;
//对冲交易初始状态
#define NEW_HEDGE 'n'
#define WAITING_FOR_OLD 'w'
char TaskStatus = 'n';
double DealA50Price(bool isBuy, double A50Price);
void CalcDeviation();
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
	TRACE(_T("退出Hedge线程\n"));
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CHedgePostProcessing, CWinThread)
	ON_THREAD_MESSAGE(WM_PREPARE_POST_PROCESSING,PostProcessing)
END_MESSAGE_MAP()


void CHiStarApp::OnHedgeLooping(UINT wParam,LONG lParam){
	if(TaskStatus == NEW_HEDGE){
		for(int i= 0;i < 22;i++){
			PositionAim[i] = PositionAimUnit[i] * MultiPos;
		}
		//默认盈利目标,一共22个区间，包含左右两个无边界区间
		double DefaultProfitAimBuy[22],DefaultProfitAimSell[22],ProfitAimBuy[22],ProfitAimSell[22];
		int netPosition = 0;//净持仓
		int SupposedBuyOpen = 0,SupposedSellOpen = 0;
		int SupposedSectionBuyOpen = 0,SupposedSectionSellOpen = 0;
		//增加这两个变量主要是考虑两侧边界可能已经饱和，这时isSupposedBuy或isSupposedSell将仍然保持false，表示无需任何操作
		bool isSupposedBuyOpen = false,isSupposedSellOpen = false;
		int CurrentSectionBuy = 0,CurrentSectionSell = 0;//当前所在的区间,Buy和Sell分别表示以买价和卖价计算
		CalcDeviation();
		//测试用，给一个随机的DeviationSell和DeviationBuy
		/*
		DeviationSell = rand() % 120 - 60;
		DeviationBuy = DeviationSell - 6.5;
		sprintf(buffer,_T("当前买偏差%f\r\n"),DeviationBuy);m_HedgeStatusOut = m_HedgeStatusOut + buffer;
		sprintf(buffer,_T("当前卖偏差%f\r\n"),DeviationSell);m_HedgeStatusOut = m_HedgeStatusOut + buffer;
		*/
		g_ifBid1 = 2163.6;g_ifAsk1 = 2163.8;
		////////////////////////结束测试//////////////////////////////
		if(isHedgeLoopingPause){//暂停
			//sprintf(buffer,_T("暂停\r\n"));m_HedgeStatusOut = m_HedgeStatusOut + buffer;
			return;
		}
		HWND hEdit = ::GetDlgItem(((CMainDlg*)m_pMainWnd)->m_basicPage.m_hWnd,IDC_RICHEDIT21);
		if(IsWindow(hEdit)){
			::SendMessage(hEdit,WM_SETTEXT,0,(LPARAM)(LPCTSTR)m_HedgeStatusOut);
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
		//sprintf(buffer,_T("净持仓%d\r\n"),netPosition);m_HedgeStatusOut = m_HedgeStatusOut + buffer;
		for(int i = 1;i < 21;i++){
			if(HedgeLadder[i - 1] > HedgeLadder[i]){
				sprintf(buffer,_T("错误的梯级\r\n"));m_HedgeStatusOut = m_HedgeStatusOut + buffer;
				return;
			}
			if(PositionAimUnit[i - 1] < PositionAimUnit[i]){
				sprintf(buffer,_T("错误的目标持仓\r\n"));m_HedgeStatusOut = m_HedgeStatusOut + buffer;
				return;
			}
		}
		if(MaxProfitAim < MinProfitAim){
			sprintf(buffer,_T("MaxProfitAim小于MinProfitAim\r\n"));m_HedgeStatusOut = m_HedgeStatusOut + buffer;
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
		//sprintf(buffer,_T("当前买价区间%d,左%f,右%f\r\n"),CurrentSectionBuy,HedgeLadder[CurrentSectionBuy - 1],HedgeLadder[CurrentSectionBuy]);
		//m_HedgeStatusOut = m_HedgeStatusOut + buffer;
		//sprintf(buffer,_T("当前卖价区间%d,左%f,右%f\r\n"),CurrentSectionSell,HedgeLadder[CurrentSectionSell - 1],HedgeLadder[CurrentSectionSell]);
		//m_HedgeStatusOut = m_HedgeStatusOut + buffer;
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
			//sprintf(buffer,"区间%d买目标盈利%f,最大盈利%f,最小盈利%f\r\n",i,ProfitAimBuy[i],MaxProfitAim,MinProfitAim);
			//sprintf(buffer,"区间%d卖目标盈利%f,最大盈利%f,最小盈利%f\r\n",i,ProfitAimSell[i],MaxProfitAim,MinProfitAim);
		}
		//每次循环只进行一次开仓或者平仓操作,完成后即return,进入下一个循环;
		//这么做因为持仓和资金只有在每个循环的开始才计算，中途不计算,
		//所以完成一个操作后返回重新计算持仓和资金才能进行下一个操作。
		//平仓操作
		for(int i = 0;i < HedgeHold.size();i++){
			if(HedgeHold[i].HedgeNum > 0){//多头持仓
				if(HedgeHold[i].HedgeSection == 21){
					//需要平仓
					if(DeviationBuy >= HedgeLadder[20] + ProfitAimBuy[HedgeHold[i].HedgeSection]){
						//需要平仓
						sprintf(buffer,_T("需要平仓%d手,价格%f,盈利%f\r\n"),-HedgeHold[i].HedgeNum,DeviationBuy,DeviationBuy - HedgeHold[i].HedgePrice);
						m_HedgeStatusOut = m_HedgeStatusOut + buffer;
						ReqHedgeOrder(&HedgeHold[i],CLOSE);
						HedgeHold.erase(HedgeHold.begin() + i);
						i--;
						sprintf(buffer,_T("======================END01======================\r\n"));
						return;
					}
				}
				else{
					if(DeviationBuy >= HedgeLadder[HedgeHold[i].HedgeSection] + ProfitAimBuy[HedgeHold[i].HedgeSection]){
						//需要平仓
						sprintf(buffer,_T("需要平仓%d手,价格%f,盈利%f\r\n"),-HedgeHold[i].HedgeNum,DeviationBuy,DeviationBuy - HedgeHold[i].HedgePrice);
						m_HedgeStatusOut = m_HedgeStatusOut + buffer;
						ReqHedgeOrder(&HedgeHold[i],CLOSE);
						HedgeHold.erase(HedgeHold.begin() + i);
						i--;
						sprintf(buffer,_T("======================END02======================\r\n"));
						m_HedgeStatusOut = m_HedgeStatusOut + buffer;
						return;
					}
				}
			}
			else if(HedgeHold[i].HedgeNum < 0){//空头持仓
				if(HedgeHold[i].HedgeSection == 0){
					if(DeviationSell <= HedgeLadder[0] - ProfitAimSell[HedgeHold[i].HedgeSection]){
						sprintf(buffer,_T("需要平仓%d手,价格%f,盈利%f\r\n"),-HedgeHold[i].HedgeNum,DeviationSell,HedgeHold[i].HedgePrice - DeviationSell);
						m_HedgeStatusOut = m_HedgeStatusOut + buffer;
						ReqHedgeOrder(&HedgeHold[i],CLOSE);
						HedgeHold.erase(HedgeHold.begin() + i);
						i--;
						sprintf(buffer,_T("======================END03======================\r\n"));
						m_HedgeStatusOut = m_HedgeStatusOut + buffer;
						return;
					}
				}
				else{
					if(DeviationSell <= HedgeLadder[HedgeHold[i].HedgeSection - 1] - ProfitAimSell[HedgeHold[i].HedgeSection]){
						sprintf(buffer,_T("需要平仓%d手,价格%f,盈利%f\r\n"),-HedgeHold[i].HedgeNum,DeviationSell,HedgeHold[i].HedgePrice - DeviationSell);
						m_HedgeStatusOut = m_HedgeStatusOut + buffer;
						ReqHedgeOrder(&HedgeHold[i],CLOSE);
						HedgeHold.erase(HedgeHold.begin() + i);
						i--;
						sprintf(buffer,_T("======================END04======================\r\n"));
						m_HedgeStatusOut = m_HedgeStatusOut + buffer;
						return;
					}
				}
			}
		}
		for(int i = 0;i <= 21;i++){
			if(min(netPosition,0) > PositionAim[i]){
				SupposedSellOpen = min(netPosition,0) - PositionAim[i];
				SupposedSectionSellOpen = i;
				isSupposedSellOpen = true;
				//sprintf(buffer,_T("期望卖开%d期望区间%d,左%f,右%f\r\n"),SupposedSellOpen,i,HedgeLadder[i - 1],HedgeLadder[i]);
				//m_HedgeStatusOut = m_HedgeStatusOut + buffer;
				break;
			}
		}
		for(int i = 21;i >= 0;i--){
			if(max(netPosition,0) < PositionAim[i]){
				SupposedBuyOpen = -(max(netPosition,0) - PositionAim[i]);
				SupposedSectionBuyOpen = i;
				isSupposedBuyOpen = true;
				//sprintf(buffer,_T("期望买开%d期望区间%d,左%f,右%f\r\n"),SupposedBuyOpen,i,HedgeLadder[i - 1],HedgeLadder[i]);
				//m_HedgeStatusOut = m_HedgeStatusOut + buffer;
				break;
			}
		}
		//开仓操作
		if(isSupposedBuyOpen){
			if(CurrentSectionSell <= SupposedSectionBuyOpen){
				//需要开仓
				HoldDetail newhold;
				newhold.HedgeNum = SupposedBuyOpen;
				newhold.HedgePrice = DeviationSell;
				newhold.HedgeSection = CurrentSectionSell;
				sprintf(buffer,_T("需要开仓%d手,价格%f,所在区间%d,左%f,右%f\r\n"),newhold.HedgeNum,newhold.HedgePrice,newhold.HedgeSection,HedgeLadder[newhold.HedgeSection - 1],HedgeLadder[newhold.HedgeSection]);
				m_HedgeStatusOut = m_HedgeStatusOut + buffer;
				HedgeHold.push_back(newhold);
				ReqHedgeOrder(&newhold,OPEN);
				sprintf(buffer,_T("======================END05======================\r\n"));
				m_HedgeStatusOut = m_HedgeStatusOut + buffer;
				return;
			}
		}
		if(isSupposedSellOpen){
			if(CurrentSectionBuy >= SupposedSectionSellOpen){
				//需要开仓
				HoldDetail newhold;
				newhold.HedgeNum = -SupposedSellOpen;
				newhold.HedgePrice = DeviationBuy;
				newhold.HedgeSection = CurrentSectionBuy;
				sprintf(buffer,_T("需要开仓%d手,价格%f,所在区间%d,左%f,右%f\r\n"),newhold.HedgeNum,newhold.HedgePrice,newhold.HedgeSection,HedgeLadder[newhold.HedgeSection - 1],HedgeLadder[newhold.HedgeSection]);
				m_HedgeStatusOut = m_HedgeStatusOut + buffer;
				HedgeHold.push_back(newhold);
				ReqHedgeOrder(&newhold,OPEN);
				sprintf(buffer,_T("======================END06======================\r\n"));
				m_HedgeStatusOut = m_HedgeStatusOut + buffer;
				return;
			}
		}
	}
	else if(TaskStatus == WAITING_FOR_OLD){



	}
}

int CHiStarApp::ReqHedgeOrder(HoldDetail *pHD,bool OffsetFlag){
	int NeedBuyA50 = 0,NeedSellA50 = 0;
	int NeedBuyOpenIf = 0,NeedBuyCloseIf = 0,NeedSellOpenIf = 0,NeedSellCloseIf = 0;
	int PredictPositionA50 = 0;
	int PredictBuyA50 = 0,PredictSellA50 = 0;
	double NeedNewMarginA50 = 0,NeedNewMarginIF = 0;
	int PredictPositionIFb = 0,PredictPositionIFs = 0;
	//IF预计要采取的操作包括买、卖、开、平
	int PredictBuyOpenIF = 0,PredictBuyCloseIF = 0,PredictSellOpenIF = 0,PredictSellCloseIF = 0;	
	if(OffsetFlag == OPEN){
		//开仓
		PredictPositionA50 = NetPositionA50 + pHD->HedgeNum * MultiA50;
		//注意IF的实际方向与HedgeNum相反;IF遵循先平仓再开仓的原则。
		if(pHD->HedgeNum > 0){
			NeedBuyA50 = pHD->HedgeNum * MultiA50;
			if(PositionIFb - pHD->HedgeNum >= 0){
				//全部卖平
				PredictPositionIFb = PositionIFb - pHD->HedgeNum;
				NeedSellCloseIf = pHD->HedgeNum;
			}
			else{
				//先卖平,再卖开
				PredictPositionIFb = 0;
				NeedSellCloseIf = PositionIFb;
				PredictPositionIFs = -(PositionIFb - pHD->HedgeNum);
				NeedSellOpenIf = -(PositionIFb - pHD->HedgeNum);
			}
		}
		else{
			NeedSellA50 = -pHD->HedgeNum * MultiA50;
			if(PositionIFs + pHD->HedgeNum >= 0){
				//全部买平
				PredictPositionIFs = PositionIFs + pHD->HedgeNum;
				NeedBuyCloseIf = -pHD->HedgeNum;
			}
			else{
				//先买平,再买开
				PredictPositionIFs = 0;
				NeedBuyCloseIf = PositionIFs;
				PredictPositionIFb = -(PositionIFs + pHD->HedgeNum);
				NeedBuyOpenIf = -(PositionIFs + pHD->HedgeNum);
			}
		}
	}
	else{
		//平仓
		PredictPositionA50 = NetPositionA50 - pHD->HedgeNum * MultiA50;
		//平仓时,IF的实际方向与HedgeNum一致;IF遵循先平仓再开仓的原则。
		if(pHD->HedgeNum > 0){
			NeedSellA50 = pHD->HedgeNum * MultiA50;
			if(PositionIFs - pHD->HedgeNum >= 0){
				//全部买平
				PredictPositionIFs = PositionIFs - pHD->HedgeNum;
				NeedBuyCloseIf = pHD->HedgeNum;
			}
			else{
				//先买平,再买开
				PredictPositionIFs = 0;
				NeedBuyCloseIf = PositionIFs;
				PredictPositionIFb = -(PositionIFs - pHD->HedgeNum);
				NeedBuyOpenIf = -(PositionIFs - pHD->HedgeNum);
			}
		}
		else{
			NeedBuyA50 = -pHD->HedgeNum * MultiA50;
			if(PositionIFb + pHD->HedgeNum >= 0){
				//全部卖平
				PredictPositionIFb = PositionIFb + pHD->HedgeNum;
				NeedSellCloseIf = -pHD->HedgeNum;
			}
			else{
				//先卖平,再卖开
				PredictPositionIFb = 0;
				NeedSellCloseIf = PositionIFb;
				PredictPositionIFs = -(PositionIFb + pHD->HedgeNum);
				NeedSellOpenIf = -(PositionIFb + pHD->HedgeNum);
			}
		}
	}
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	sprintf(buffer,_T("%2d:%2d:%2d:%3d\r\n"),sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds);m_HedgeStatusOut = m_HedgeStatusOut + buffer;
	sprintf(buffer,_T("操作:\r\n买A50--%d手\r\n,卖A50--%d手\r\n,买开IF---%d手\r\n,买平IF---%d手\r\n,卖开IF---%d手\r\n,卖平IF---%d手\r\n"),NeedBuyA50,NeedSellA50,NeedBuyOpenIf,NeedBuyCloseIf,NeedSellOpenIf,NeedSellCloseIf);
	m_HedgeStatusOut = m_HedgeStatusOut + buffer;
	sprintf(buffer,"A50指数%f,HS300指数%f, %f, %f, %f, %f, %f\r\n",g_A50IndexMSHQ,g_HS300IndexMSHQ,g_ifBid1,g_ifAsk1,g_a50Bid1,g_a50Ask1);m_HedgeStatusOut = m_HedgeStatusOut + buffer;
	sprintf(buffer,"premiumHigh%f,premiumLow%f\r\n",g_a50Ask1 - g_ifBid1 * g_A50IndexMSHQ / g_HS300IndexMSHQ,g_a50Bid1 - g_ifAsk1 * g_A50IndexMSHQ / g_HS300IndexMSHQ);m_HedgeStatusOut = m_HedgeStatusOut + buffer;	
	////////////////////////////////////////////////////////////////////////////////////
	//IB表示需要新的保证金
	if(abs(PredictPositionA50) > abs(NetPositionA50)){
		NeedNewMarginA50 = MultiInsA50 * MarginA50 * (abs(PredictPositionA50) - abs(NetPositionA50));
		if(NeedNewMarginA50 - AvailIB > 1000.0){
			sprintf(buffer,_T("IB保证金不足,需要%f\r\n"),NeedNewMarginA50);
			m_HedgeStatusOut = m_HedgeStatusOut + buffer;
			sprintf(buffer,_T("======================END07======================\r\n"));
			m_HedgeStatusOut = m_HedgeStatusOut + buffer;
			return -1;
		}
	}
	//IF需要新的保证金
	if(PredictPositionIFs + PredictPositionIFb > PositionIFb + PositionIFs){
		NeedNewMarginIF = (g_ifAsk1 + g_ifBid1) / 2.0 *  MultiInsIf * MarginIF * (PredictPositionIFs + PredictPositionIFb - (PositionIFb + PositionIFs));
		if(NeedNewMarginIF - AvailCtp > 6000.0){
			sprintf(buffer,_T("CTP保证金不足,需要%f\r\n"),NeedNewMarginIF);
			m_HedgeStatusOut = m_HedgeStatusOut + buffer;
			sprintf(buffer,_T("======================END08======================\r\n"));
			m_HedgeStatusOut = m_HedgeStatusOut + buffer;
			return -1;
		}
	}
	//测试，统计下持仓
	//临时以此做统计,认为所有的任务都会瞬间成交。
	//PositionIFb = PositionIFb - NeedSellCloseIf + NeedBuyOpenIf;
	//PositionIFs = PositionIFs - NeedBuyCloseIf + NeedSellOpenIf;
	//NetPositionA50 = NetPositionA50 + NeedBuyA50 - NeedSellA50;

	TaskStatus = WAITING_FOR_OLD;//标记等待状态，在等待状态下对冲循环不会有新的任务
	if(m_pHedgePostProcessing){
		m_pHedgePostProcessing->PostThreadMessage(WM_PREPARE_POST_PROCESSING,NULL,NULL);
	}
	////////////////////////////////////////////////////////////
	IfTask iftask;A50Task a50task;
	////////////////////////////////////////////////////////////
	//IF下单
	TThostFtdcCombOffsetFlagType kpp;
	char szInst[MAX_PATH];
	uni2ansi(CP_ACP,((CHiStarApp*)AfxGetApp())->m_accountCtp.m_szInst,szInst);
	LPSTR* pInst = new LPSTR;
	pInst[0] = szInst;
	if(NeedBuyOpenIf > 0){
		kpp[0] = THOST_FTDC_OF_Open;
		if(((CHiStarApp*)AfxGetApp())->m_cT){
			iftask.ref = ((CHiStarApp*)AfxGetApp())->m_cT->ReqOrdLimit(*pInst,THOST_FTDC_D_Buy,kpp,g_ifAsk1 + 20.0 - 60.0,NeedBuyOpenIf);
			iftask.direction = THOST_FTDC_D_Buy;iftask.offset[0] = THOST_FTDC_OF_Open;iftask.priceRecord = g_ifAsk1;iftask.volumeRecord = NeedBuyOpenIf;
			hedgetask.ifalltask.push_back(iftask);
		}
	}
	if(NeedBuyCloseIf > 0){
		kpp[0] = THOST_FTDC_OF_Close;
		if(((CHiStarApp*)AfxGetApp())->m_cT){
			iftask.ref = ((CHiStarApp*)AfxGetApp())->m_cT->ReqOrdLimit(*pInst,THOST_FTDC_D_Buy,kpp,g_ifAsk1 + 20.0 - 60.0,NeedBuyCloseIf);
			iftask.direction = THOST_FTDC_D_Buy;iftask.offset[0] = THOST_FTDC_OF_Close;iftask.priceRecord = g_ifAsk1;iftask.volumeRecord = NeedBuyCloseIf;
			hedgetask.ifalltask.push_back(iftask);
		}
	}
	if(NeedSellOpenIf > 0){
		kpp[0] = THOST_FTDC_OF_Open;
		if(((CHiStarApp*)AfxGetApp())->m_cT){
			iftask.ref = ((CHiStarApp*)AfxGetApp())->m_cT->ReqOrdLimit(*pInst,THOST_FTDC_D_Sell,kpp,g_ifBid1 - 20.0 + 60.0,NeedSellOpenIf);
			iftask.direction = THOST_FTDC_D_Sell;iftask.offset[0] = THOST_FTDC_OF_Open;iftask.priceRecord = g_ifBid1;iftask.volumeRecord = NeedSellOpenIf;
			hedgetask.ifalltask.push_back(iftask);
		}
	}
	if(NeedSellCloseIf > 0){
		kpp[0] = THOST_FTDC_OF_Close;
		if(((CHiStarApp*)AfxGetApp())->m_cT){
			iftask.ref = ((CHiStarApp*)AfxGetApp())->m_cT->ReqOrdLimit(*pInst,THOST_FTDC_D_Sell,kpp,g_ifBid1 - 20.0 + 60.0,NeedSellCloseIf);
			iftask.direction = THOST_FTDC_D_Sell;iftask.offset[0] = THOST_FTDC_OF_Close;iftask.priceRecord = g_ifBid1;iftask.volumeRecord = NeedSellCloseIf;
			hedgetask.ifalltask.push_back(iftask);
		}
	}
	//A50下单
	if(abs(NeedBuyA50 - NeedSellA50) != 0){
		if(((CHiStarApp*)AfxGetApp())->m_pIBClient){
			if(NeedBuyA50 - NeedSellA50 > 0){
				((CHiStarApp*)AfxGetApp())->m_IBOrder.action = "BUY";
				((CHiStarApp*)AfxGetApp())->m_IBOrder.totalQuantity = NeedBuyA50 - NeedSellA50;
				((CHiStarApp*)AfxGetApp())->m_IBOrder.lmtPrice = DealA50Price(true,g_a50Ask1 + 100.0 - 200);
				if(((CHiStarApp*)AfxGetApp())->m_IBOrder.lmtPrice < 1.0){
					sprintf(buffer,_T("A50买价小于1.0,异常\r\n"));m_HedgeStatusOut = m_HedgeStatusOut + buffer;
				}
				if(((CHiStarApp*)AfxGetApp())->m_pIBClient){
					((CHiStarApp*)AfxGetApp())->m_pIBClient->placeOrder(++((CHiStarApp*)AfxGetApp())->m_id,((CHiStarApp*)AfxGetApp())->m_A50Contract,((CHiStarApp*)AfxGetApp())->m_IBOrder);
					a50task.id = ((CHiStarApp*)AfxGetApp())->m_id;a50task.volumeRecord = NeedBuyA50 - NeedSellA50;a50task.direction = 'l';a50task.priceRecord = g_a50Ask1;
					hedgetask.a50alltask.push_back(a50task);
				}
			}
			else if(NeedBuyA50 - NeedSellA50 < 0){
				((CHiStarApp*)AfxGetApp())->m_IBOrder.action = "SELL";
				((CHiStarApp*)AfxGetApp())->m_IBOrder.totalQuantity = -(NeedBuyA50 - NeedSellA50);
				((CHiStarApp*)AfxGetApp())->m_IBOrder.lmtPrice = DealA50Price(false,g_a50Bid1 - 100.0 + 200);
				if(((CHiStarApp*)AfxGetApp())->m_IBOrder.lmtPrice < 1.0){
					sprintf(buffer,_T("A50买价小于1.0,异常\r\n"));m_HedgeStatusOut = m_HedgeStatusOut + buffer;
				}
				if(((CHiStarApp*)AfxGetApp())->m_pIBClient){
					((CHiStarApp*)AfxGetApp())->m_pIBClient->placeOrder(++((CHiStarApp*)AfxGetApp())->m_id,((CHiStarApp*)AfxGetApp())->m_A50Contract,((CHiStarApp*)AfxGetApp())->m_IBOrder);
					a50task.id = ((CHiStarApp*)AfxGetApp())->m_id;a50task.volumeRecord = -(NeedBuyA50 - NeedSellA50);a50task.direction = 's';a50task.priceRecord = g_a50Bid1;
					hedgetask.a50alltask.push_back(a50task);
				}
			}
		}
	}
	//开始后处理
	if(m_pHedgePostProcessing){
		m_pHedgePostProcessing->PostThreadMessage(WM_BEGIN_POST_PROCESSING,NULL,NULL);
	}
	return 0;
}

//后处理
void CHedgePostProcessing::PostProcessing(UINT wParam,LONG lParam){
	MSG msg;BOOL bRet;
	while((bRet = GetMessage(&msg,NULL,WM_BEGIN_POST_PROCESSING,WM_BEGIN_POST_PROCESSING)) != 0){
		if (!bRet){// handle the error and possibly exit
		}
		else{
			break;//已经开始，往下正式进行处理。
		}
	}
	//检索两种消息，分别是WM_RTN_INSERT和WM_RTN_ORDER
	while((bRet = GetMessage(&msg,NULL,WM_RTN_INSERT,WM_RTN_ORDER)) != 0){
		if (!bRet){
		}
		else{
			switch(msg.message)
			{
			case WM_RTN_INSERT:
				{
					TRACE("收到WM_RTN_INSERT\r\n");
					CThostFtdcInputOrderField orderInsert = *(CThostFtdcInputOrderField*)lParam;
					delete (CThostFtdcInputOrderField*)lParam;
					for(int i = 0;i < hedgetask.ifalltask.size();i++){
						if(hedgetask.ifalltask[i].ref == atoi(orderInsert.OrderRef)){
							hedgetask.ifalltask[i].bReceivedInsertRtn = true;
						}
					}
					break;
				}
			case WM_RTN_ORDER:
				{
					TRACE("收到WM_RTN_ORDER\r\n");
					CThostFtdcOrderField orderRtn = *(CThostFtdcOrderField*)lParam;
					delete (CThostFtdcOrderField*)lParam;
					for(int i = 0;i < hedgetask.ifalltask.size();i++){
						if(hedgetask.ifalltask[i].ref == atoi(orderRtn.OrderRef)){
							if(orderRtn.OrderStatus == THOST_FTDC_OST_AllTraded || orderRtn.OrderStatus == THOST_FTDC_OST_Canceled 
								|| orderRtn.OrderStatus == THOST_FTDC_OST_NoTradeNotQueueing || orderRtn.OrderStatus == THOST_FTDC_OST_PartTradedNotQueueing){
									hedgetask.ifalltask[i].traded = orderRtn.VolumeTraded;
									hedgetask.ifalltask[i].sysid = atoi(orderRtn.OrderSysID);
									hedgetask.ifalltask[i].bReceivedAllOrder = true;
							}			
						}
					}
					break;
				}
			}
			bool bBreakGetMsg = true;
			for(int i = 0;i < hedgetask.ifalltask.size();i++){
				if(!(hedgetask.ifalltask[i].bReceivedAllOrder || hedgetask.ifalltask[i].bReceivedInsertRtn)){
					bBreakGetMsg = false;break;
				}
			}
			if(bBreakGetMsg) break;
		}
	}
	//检查是否有成交量,可能都被取消而没有成交量
	bool bVolumeTraded = false;
	for(int i = 0;i < hedgetask.ifalltask.size();i++){
		if(hedgetask.ifalltask[i].traded > 0){
			bVolumeTraded = true;break;
		}
	}
	if(bVolumeTraded){
		while((bRet = GetMessage(&msg,NULL,WM_RTN_TRADE,WM_RTN_TRADE)) != 0){
			if (!bRet){
			}
			else{
				TRACE("收到WM_RTN_TRADE\r\n");
				CThostFtdcTradeField tradeRtn = *(CThostFtdcTradeField*)lParam;
				delete (CThostFtdcTradeField*)lParam;
				for(int i = 0;i < hedgetask.ifalltask.size();i++){
					if(hedgetask.ifalltask[i].sysid == atoi(tradeRtn.OrderSysID)){
						hedgetask.ifalltask[i].receivedTradedVolume = hedgetask.ifalltask[i].receivedTradedVolume + tradeRtn.Volume;
						hedgetask.ifalltask[i].receivedValue = hedgetask.ifalltask[i].receivedValue + tradeRtn.Price * tradeRtn.Volume;	
					}
				}
				bool bBreakGetMsg = true;
				for(int i = 0;i < hedgetask.ifalltask.size();i++){
					if(hedgetask.ifalltask[i].receivedTradedVolume != hedgetask.ifalltask[i].traded){
						bBreakGetMsg = false;break;
					}
				}
				if(bBreakGetMsg) break;
			}
		}
		///calc avg price
		for(int i = 0;i < hedgetask.ifalltask.size();i++){
			hedgetask.ifalltask[i].avgPrice = hedgetask.ifalltask[i].receivedValue / hedgetask.ifalltask[i].receivedTradedVolume;

		}
	}

	while((bRet = GetMessage(&msg,NULL,WM_RTN_ORDER_IB,WM_RTN_ORDER_IB)) != 0){
		if (!bRet){
		}
		else{
			TRACE("WM_RTN_ORDER_IB\r\n");
			OrderStatus status = *(OrderStatus*)lParam; 
			delete (OrderStatus*)lParam;
			for(int i = 0;i < hedgetask.a50alltask.size();i++){
				if(hedgetask.a50alltask[i].id == status.orderId){
					if(status.status == CString("Cancelled") || status.status == CString("ApiCancelled") || hedgetask.a50alltask[i].volumeRecord == status.filled){
						hedgetask.a50alltask[i].traded = status.filled;
						hedgetask.a50alltask[i].avgPrice = status.avgFillPrice;
						hedgetask.a50alltask[i].bReceivedAllStatus = true;
					}
				}
			}
			bool bBreakGetMsg = true;
			for(int i = 0;i < hedgetask.a50alltask.size();i++){
				if(!hedgetask.a50alltask[i].bReceivedAllStatus){
					bBreakGetMsg = false;
				}
			}
			if(bBreakGetMsg) break;
		}
	}
}

double DealA50Price(bool isBuy, double A50Price)
{
	double rtn;
	if(isBuy){
		if(((int)A50Price) % 5 < 4){
			rtn = (double)(((int)A50Price) / 5 * 5);
		}
		else{
			rtn = (double)(((int)A50Price) / 5 * 5 + 5);
		}
	}
	else{
		if(((int)A50Price) % 5 >= 1){
			rtn = (double)(((int)A50Price) / 5 * 5 + 5);
		}
		else{
			rtn = (double)(((int)A50Price) / 5 * 5);
		}
	}
	return rtn;
}

void CalcDeviation(){
	double A50Index = 0.0,HS300Index = 0.0;
	if(fabs(g_A50IndexMSHQ - g_A50Index) / g_A50Index > 0.01){
		//TRACE("over 0.1,MS%f,SN%f\r\n",g_A50IndexMSHQ,g_A50Index);
		A50Index = g_A50Index;
	}
	else{
		A50Index = g_A50IndexMSHQ;
		TRACE("MS,SN,%f,%f\r\n",g_A50IndexMSHQ,g_A50Index);
	}
	if(fabs(g_HS300IndexMSHQ - g_HS300Index) / g_HS300Index > 0.01){
		//TRACE("over 0.1,MS%f,SN%f\r\n",g_HS300IndexMSHQ,g_HS300Index);
		HS300Index = g_HS300Index;
	}
	else{
		HS300Index = g_HS300IndexMSHQ;
		//TRACE("MS%f,SN%f\r\n",g_HS300IndexMSHQ,g_HS300Index);
	}
	premium = (g_a50Bid1 + g_a50Ask1) / 2.0 - (g_ifAsk1 + g_ifBid1) / 2.0 * A50Index / HS300Index;
	premiumHigh = g_a50Ask1 - g_ifBid1 * A50Index / HS300Index;
	premiumLow = g_a50Bid1 - g_ifAsk1 * A50Index / HS300Index;
	deviation = premium - datumDiff;
	DeviationSell = premiumHigh - datumDiff;
	DeviationBuy = premiumLow - datumDiff;
}