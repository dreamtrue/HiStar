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
int MultiA50 = 12;//A50乘数
double MarginA50 = 625.0,MarginIF = 120000.0;
double USDtoRMB = 6.07;//汇率
double datumDiff = 0;
bool isHedgeLoopingPause = true;
double premium = 0,premiumHigh = 0,premiumLow = 0;
double deviation = 0,DeviationSell = 0,DeviationBuy = 0;
extern double g_a50Bid1,g_a50Ask1;
extern double g_ifAsk1,g_ifBid1;
extern double g_A50Index,g_HS300Index;
///////////////////////////////////////////////////////
int NetPositionA50 = 0,PositionIFb = 0,PositionIFs = 0;//净持仓,需要计算;IF分空头净持仓和多头净持仓
double AvailIB = 8000.0,AvailCtp = 250000.0;//可用资金，需要计算
////////////////////////////////////////////////////////
char buffer[1000];
double DeviationSell_save,DeviationBuy_save;
std::vector<HoldDetail> HedgeHold;
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
	DeviationSell = rand() % 120 - 60;
	DeviationBuy = DeviationSell - 6.5;
	sprintf(buffer,_T("当前买偏差%f\r\n"),DeviationBuy);m_HedgeStatusOut = m_HedgeStatusOut + buffer;
	sprintf(buffer,_T("当前卖偏差%f\r\n"),DeviationSell);m_HedgeStatusOut = m_HedgeStatusOut + buffer;
	/////////////////////////////////////////////////
	DeviationSell_save = DeviationSell;
	DeviationBuy_save = DeviationBuy;	

	if(isHedgeLoopingPause){//暂停
		sprintf(buffer,_T("暂停\r\n"));m_HedgeStatusOut = m_HedgeStatusOut + buffer;
		return;
	}
	HWND hEdit = ::GetDlgItem(((CMainDlg*)m_pMainWnd)->m_basicPage.m_hWnd,IDC_RICHEDIT21);
	if(IsWindow(hEdit)){
		::SendMessage(hEdit,WM_SETTEXT,0,(LPARAM)(LPCTSTR)m_HedgeStatusOut);
	}
	/*
	if(_isnan(datumDiff) != 0 || _isnan(premium)!=0 ||_isnan(deviation)!=0){
	return;//判断非零值错误
	}
	if(g_a50Bid1 < 1 || g_a50Ask1 < 1 || g_ifAsk1 < 1 || g_ifBid1 < 1 || g_A50Index < 1 || g_HS300Index < 1){
	return;
	}
	if(fabs(premium) > 300 || fabs(premium) < 0.01){
	return;//排除开盘时有可能报价不全导致的错误溢价计算
	}
	*/
	//统计净持仓
	for(int i = 0;i < HedgeHold.size();i++){
		netPosition = netPosition + HedgeHold[i].HedgeNum;
	}
	sprintf(buffer,_T("净持仓%d\r\n"),netPosition);m_HedgeStatusOut = m_HedgeStatusOut + buffer;
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
	sprintf(buffer,_T("当前买价区间%d,左%f,右%f\r\n"),CurrentSectionBuy,HedgeLadder[CurrentSectionBuy - 1],HedgeLadder[CurrentSectionBuy]);
	m_HedgeStatusOut = m_HedgeStatusOut + buffer;
	sprintf(buffer,_T("当前卖价区间%d,左%f,右%f\r\n"),CurrentSectionSell,HedgeLadder[CurrentSectionSell - 1],HedgeLadder[CurrentSectionSell]);
	m_HedgeStatusOut = m_HedgeStatusOut + buffer;
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
					sprintf(buffer,_T("======================END======================\r\n"));
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
					sprintf(buffer,_T("======================END======================\r\n"));
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
					sprintf(buffer,_T("======================END======================\r\n"));
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
					sprintf(buffer,_T("======================END======================\r\n"));
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
			sprintf(buffer,_T("期望卖开%d期望区间%d,左%f,右%f\r\n"),SupposedSellOpen,i,HedgeLadder[i - 1],HedgeLadder[i]);
			m_HedgeStatusOut = m_HedgeStatusOut + buffer;
			break;
		}
	}
	for(int i = 21;i >= 0;i--){
		if(max(netPosition,0) < PositionAim[i]){
			SupposedBuyOpen = -(max(netPosition,0) - PositionAim[i]);
			SupposedSectionBuyOpen = i;
			isSupposedBuyOpen = true;
			sprintf(buffer,_T("期望买开%d期望区间%d,左%f,右%f\r\n"),SupposedBuyOpen,i,HedgeLadder[i - 1],HedgeLadder[i]);
			m_HedgeStatusOut = m_HedgeStatusOut + buffer;
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
			sprintf(buffer,_T("======================END======================\r\n"));
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
			sprintf(buffer,_T("======================END======================\r\n"));
			m_HedgeStatusOut = m_HedgeStatusOut + buffer;
			return;
		}
	}
	sprintf(buffer,_T("======================END======================\r\n"));
	m_HedgeStatusOut = m_HedgeStatusOut + buffer;
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
	sprintf(buffer,_T("操作:\r\n买A50--%d手\r\n,卖A50--%d手\r\n,买开IF---%d手\r\n,买平IF---%d手\r\n,卖开IF---%d手\r\n,卖平IF---%d手\r\n"),NeedBuyA50,NeedSellA50,NeedBuyOpenIf,NeedBuyCloseIf,NeedSellOpenIf,NeedSellCloseIf);
	m_HedgeStatusOut = m_HedgeStatusOut + buffer;
	////////////////////////////////////////////////////////////////////////////////////
	//IB表示需要新的保证金
	if(abs(PredictPositionA50) > abs(NetPositionA50)){
		NeedNewMarginA50 = MarginA50 * (abs(PredictPositionA50) - abs(NetPositionA50));
		if(NeedNewMarginA50 - AvailIB > 1000.0){
			sprintf(buffer,_T("IB保证金不足,需要%f\r\n"),NeedNewMarginA50);
			m_HedgeStatusOut = m_HedgeStatusOut + buffer;
			sprintf(buffer,_T("======================END======================\r\n"));
			m_HedgeStatusOut = m_HedgeStatusOut + buffer;
			return -1;
		}
	}
	//IF需要新的保证金
	if(PredictPositionIFs + PredictPositionIFb > PositionIFb + PositionIFs){
		NeedNewMarginIF = MarginIF * (PredictPositionIFs + PredictPositionIFb - (PositionIFb + PositionIFs));
		if(NeedNewMarginIF - AvailCtp > 6000.0){
			sprintf(buffer,_T("CTP保证金不足,需要%f\r\n"),NeedNewMarginIF);
			m_HedgeStatusOut = m_HedgeStatusOut + buffer;
			sprintf(buffer,_T("======================END======================\r\n"));
			m_HedgeStatusOut = m_HedgeStatusOut + buffer;
			return -1;
		}
	}
	//测试，统计下持仓
	PositionIFb = PositionIFb - NeedSellCloseIf + NeedBuyOpenIf;
	PositionIFs = PositionIFs - NeedBuyCloseIf + NeedSellOpenIf;
	NetPositionA50 = NetPositionA50 + NeedBuyA50 - NeedSellA50;
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
			((CHiStarApp*)AfxGetApp())->m_cT->ReqOrdLimit(*pInst,THOST_FTDC_D_Buy,kpp,2160,1);
		}
	}
	if(NeedBuyCloseIf > 0){
		kpp[0] = THOST_FTDC_OF_Close;
		if(((CHiStarApp*)AfxGetApp())->m_cT){
			((CHiStarApp*)AfxGetApp())->m_cT->ReqOrdLimit(*pInst,THOST_FTDC_D_Buy,kpp,2160,1);
		}
	}
	if(NeedSellOpenIf > 0){
		kpp[0] = THOST_FTDC_OF_Open;
		if(((CHiStarApp*)AfxGetApp())->m_cT){
			((CHiStarApp*)AfxGetApp())->m_cT->ReqOrdLimit(*pInst,THOST_FTDC_D_Sell,kpp,2160,1);
		}
	}
	if(NeedSellCloseIf > 0){
		kpp[0] = THOST_FTDC_OF_Close;
		if(((CHiStarApp*)AfxGetApp())->m_cT){
			((CHiStarApp*)AfxGetApp())->m_cT->ReqOrdLimit(*pInst,THOST_FTDC_D_Sell,kpp,2160,1);
		}
	}
	//A50下单
	if(abs(NeedBuyA50 - NeedSellA50) != 0){
		if(((CHiStarApp*)AfxGetApp())->m_pIBClient){
			if(NeedBuyA50 - NeedSellA50 > 0){
				((CHiStarApp*)AfxGetApp())->m_IBOrder.action = "BUY";
				((CHiStarApp*)AfxGetApp())->m_IBOrder.totalQuantity = NeedBuyA50 - NeedSellA50;
				((CHiStarApp*)AfxGetApp())->m_IBOrder.lmtPrice = DealA50Price(true,g_a50Ask1);
				if(((CHiStarApp*)AfxGetApp())->m_IBOrder.lmtPrice < 1.0){
					sprintf(buffer,_T("A50买价小于1.0,异常\r\n"));m_HedgeStatusOut = m_HedgeStatusOut + buffer;
				}
				if(((CHiStarApp*)AfxGetApp())->m_pIBClient){
					((CHiStarApp*)AfxGetApp())->m_pIBClient->placeOrder(++((CHiStarApp*)AfxGetApp())->m_id,((CHiStarApp*)AfxGetApp())->m_A50Contract,((CHiStarApp*)AfxGetApp())->m_IBOrder);
				}
			}
			else if(NeedBuyA50 - NeedSellA50 < 0){
				((CHiStarApp*)AfxGetApp())->m_IBOrder.action = "SELL";
				((CHiStarApp*)AfxGetApp())->m_IBOrder.totalQuantity = -(NeedBuyA50 - NeedSellA50);
				((CHiStarApp*)AfxGetApp())->m_IBOrder.lmtPrice = DealA50Price(false,g_a50Bid1);
				if(((CHiStarApp*)AfxGetApp())->m_IBOrder.lmtPrice < 1.0){
					sprintf(buffer,_T("A50买价小于1.0,异常\r\n"));m_HedgeStatusOut = m_HedgeStatusOut + buffer;
				}
				if(((CHiStarApp*)AfxGetApp())->m_pIBClient){
					((CHiStarApp*)AfxGetApp())->m_pIBClient->placeOrder(++((CHiStarApp*)AfxGetApp())->m_id,((CHiStarApp*)AfxGetApp())->m_A50Contract,((CHiStarApp*)AfxGetApp())->m_IBOrder);
				}
			}
		}
	}
	return 0;
}

void CHedgePostProcessing::PostProcessing(UINT wParam,LONG lParam){


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
	premium = (g_a50Bid1 + g_a50Ask1) / 2.0 - (g_ifAsk1 + g_ifBid1) / 2.0 * g_A50Index / g_HS300Index;
	premiumHigh = g_a50Ask1 - g_ifBid1 * g_A50Index / g_HS300Index;
	premiumLow = g_a50Bid1 - g_ifAsk1 * g_A50Index / g_HS300Index;
	deviation = premium - datumDiff;
	DeviationSell = premiumHigh - datumDiff;
	DeviationBuy = premiumLow - datumDiff;
}