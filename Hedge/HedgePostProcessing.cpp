#include "stdafx.h"
#include "HiStar.h"
#include "HedgePostProcessing.h"
#include "UserMsg.h"
#include "MainDlg.h"
#include <math.h>
#define SHOW if(IsWindow(hEdit)){::SendMessage(hEdit,WM_SETTEXT,0,(LPARAM)(LPCTSTR)hedgeStatusPrint);}
#define OPEN true
#define CLOSE false
double datumDiff = 0.0;
double pointValueA50 = 1.0;double pointValueIf = 300.0;//合约每个点的价值
int MultiA50 = 12;//A50乘数
double MarginA50 = 625.0,MarginIf = 0.15;
double USDtoRMB = 6.07;//汇率
bool isHedgeLoopingPause = true;
double premium = 0,premiumHigh = 0,premiumLow = 0;
double deviation = 0,DeviationSell = 0,DeviationBuy = 0;
extern double g_a50Bid1,g_a50Ask1;
extern double g_ifAsk1,g_ifBid1;
extern double g_A50Index,g_HS300Index;
extern double g_A50IndexMSHQ,g_HS300IndexMSHQ;
double A50Index = 0.0,HS300Index = 0.0;//本程序实际所用指数
CString hedgeStatusPrint;
void SelectIndex();
/////////////////////////////////Hedge变量///////////////////////////////////////////////
int MultiPos = 1;//持仓乘数
//梯级，一共21个分割点,分割成22(=21+1)个区间
double HedgeLadder[21] = {   -200, -180, -160, -140, -120, -100, -80, -60, -40, -20, -10,  20,  40,  60,  80,  100,  120,  140,  160,  180,  200};
int PositionAimUnit[22] = {10,    9,    8,    7,    6,    5,    4,   3,   2,   1,   1,   0,  -1,  -2,  -3,  -4,  -5,    -6,   -7,   -8,   -9,   -10};//默认持仓目标单位（没有乘以乘数）
int PositionAim[22];
double MaxProfitAim = 20.0,MinProfitAim = 20.0;//最小盈利目标，最大盈利目标（不分多空）
///////////////////////////////////////////////////////
int netPositionA50 = 0,longIf = 0,shortIf = 0;//净持仓,需要计算;IF分空头净持仓和多头净持仓
double AvailIB = 0.0,AvailCtp = 0.0;//可用资金，需要计算
////////////////////////////////////////////////////////
char buffer[1000];
std::vector<HoldDetail> HedgeHoldTemp;//临时使用的，先用这个进行预计算，最后同步到HedgeHold
std::vector<HoldDetail> HedgeHold;
int idHold = 0;//持仓id
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
#define WAITING_FOR_FINISHED 'w'
char hedgeTaskStatus = 'n';
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
    HWND hEdit = ::GetDlgItem(((CMainDlg*)m_pMainWnd)->m_basicPage.m_hWnd,IDC_RICHEDIT_STATUS);
	SelectIndex();
    if(hedgeTaskStatus == NEW_HEDGE){
		HedgeHoldTemp = HedgeHold;
		for(unsigned int i = 0;i < HedgeHoldTemp.size();i++){
			HedgeHoldTemp[i].adjustedCost = HedgeHoldTemp[i].originalCost - datumDiff;
		}
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
		//测试用，给一个随机的DeviationSell和DeviationBuy
		/*
		DeviationSell = rand() % 120 - 60;
		DeviationBuy = DeviationSell - 6.5;
		g_ifBid1 = 2163.6;g_ifAsk1 = 2163.8;
		*/
		//sprintf(buffer,_T("当前买偏差%f\r\n"),DeviationBuy);hedgeStatusPrint = hedgeStatusPrint + buffer;
		//sprintf(buffer,_T("当前卖偏差%f\r\n"),DeviationSell);hedgeStatusPrint = hedgeStatusPrint + buffer;
        ////////////////////////结束测试//////////////////////////////
        CalcDeviation();
        if(isHedgeLoopingPause){//暂停
            //sprintf(buffer,_T("暂停\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;
            return;
        }
        if(_isnan(datumDiff) != 0 || _isnan(premium)!=0 ||_isnan(deviation)!=0){
            return;//判断非零值错误
        }
        if(g_a50Bid1 < 1 || g_a50Ask1 < 1 || g_ifAsk1 < 1 || g_ifBid1 < 1 || A50Index < 1 || HS300Index < 1){
            return;
        }
        if(fabs(premium) > 300 || fabs(premium) < 0.01){
            return;//排除开盘时有可能报价不全导致的错误溢价计算
        }
        //统计净持仓
        for(unsigned int i = 0;i < HedgeHoldTemp.size();i++){
            netPosition = netPosition + HedgeHoldTemp[i].HedgeNum;
        }
        //sprintf(buffer,_T("净持仓%d\r\n"),netPosition);hedgeStatusPrint = hedgeStatusPrint + buffer;
        for(int i = 1;i < 21;i++){
            if(HedgeLadder[i - 1] > HedgeLadder[i]){
                sprintf(buffer,_T("错误的梯级\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
                return;
            }
            if(PositionAimUnit[i - 1] < PositionAimUnit[i]){
                sprintf(buffer,_T("错误的目标持仓\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
                return;
            }
        }
        if(MaxProfitAim < MinProfitAim){
            sprintf(buffer,_T("MaxProfitAim小于MinProfitAim\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
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
        //hedgeStatusPrint = hedgeStatusPrint + buffer;
        //sprintf(buffer,_T("当前卖价区间%d,左%f,右%f\r\n"),CurrentSectionSell,HedgeLadder[CurrentSectionSell - 1],HedgeLadder[CurrentSectionSell]);
        //hedgeStatusPrint = hedgeStatusPrint + buffer;
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
        for(unsigned int i = 0;i < HedgeHoldTemp.size();i++){
            if(HedgeHoldTemp[i].HedgeNum > 0){//多头持仓
                if(HedgeHoldTemp[i].HedgeSection == 21){
                    //需要平仓
                    if(DeviationBuy >= HedgeLadder[20] + ProfitAimBuy[HedgeHoldTemp[i].HedgeSection]){
                        //需要平仓
                        sprintf(buffer,_T("需要平仓%d手,相对价格%f,盈利%f\r\n"),-HedgeHoldTemp[i].HedgeNum,DeviationBuy,DeviationBuy - HedgeHoldTemp[i].adjustedCost);
                        hedgeStatusPrint = hedgeStatusPrint + buffer;
                        ReqHedgeOrder(&HedgeHoldTemp[i],CLOSE);
                        HedgeHoldTemp.erase(HedgeHoldTemp.begin() + i);
                        i--;
                        sprintf(buffer,_T("======================END01======================\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
                        return;
                    }
                }
                else{
                    if(DeviationBuy >= HedgeLadder[HedgeHoldTemp[i].HedgeSection] + ProfitAimBuy[HedgeHoldTemp[i].HedgeSection]){
                        //需要平仓
                        sprintf(buffer,_T("需要平仓%d手,相对价格%f,盈利%f\r\n"),-HedgeHoldTemp[i].HedgeNum,DeviationBuy,DeviationBuy - HedgeHoldTemp[i].adjustedCost);
                        hedgeStatusPrint = hedgeStatusPrint + buffer;
                        ReqHedgeOrder(&HedgeHoldTemp[i],CLOSE);
                        HedgeHoldTemp.erase(HedgeHoldTemp.begin() + i);
                        i--;
                        sprintf(buffer,_T("======================END02======================\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
                        return;
                    }
                }
            }
            else if(HedgeHoldTemp[i].HedgeNum < 0){//空头持仓
                if(HedgeHoldTemp[i].HedgeSection == 0){
                    if(DeviationSell <= HedgeLadder[0] - ProfitAimSell[HedgeHoldTemp[i].HedgeSection]){
                        sprintf(buffer,_T("需要平仓%d手,相对价格%f,盈利%f\r\n"),-HedgeHoldTemp[i].HedgeNum,DeviationSell,HedgeHoldTemp[i].adjustedCost - DeviationSell);
                        hedgeStatusPrint = hedgeStatusPrint + buffer;
                        ReqHedgeOrder(&HedgeHoldTemp[i],CLOSE);
                        HedgeHoldTemp.erase(HedgeHoldTemp.begin() + i);
                        i--;
                        sprintf(buffer,_T("======================END03======================\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
                        return;
                    }
                }
                else{
                    if(DeviationSell <= HedgeLadder[HedgeHoldTemp[i].HedgeSection - 1] - ProfitAimSell[HedgeHoldTemp[i].HedgeSection]){
                        sprintf(buffer,_T("需要平仓%d手,相对价格%f,盈利%f\r\n"),-HedgeHoldTemp[i].HedgeNum,DeviationSell,HedgeHoldTemp[i].adjustedCost - DeviationSell);
                        hedgeStatusPrint = hedgeStatusPrint + buffer;
                        ReqHedgeOrder(&HedgeHoldTemp[i],CLOSE);
                        HedgeHoldTemp.erase(HedgeHoldTemp.begin() + i);
                        i--;
                        sprintf(buffer,_T("======================END04======================\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
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
                //hedgeStatusPrint = hedgeStatusPrint + buffer;
                break;
            }
        }
        for(int i = 21;i >= 0;i--){
            if(max(netPosition,0) < PositionAim[i]){
                SupposedBuyOpen = -(max(netPosition,0) - PositionAim[i]);
                SupposedSectionBuyOpen = i;
                isSupposedBuyOpen = true;
                //sprintf(buffer,_T("期望买开%d期望区间%d,左%f,右%f\r\n"),SupposedBuyOpen,i,HedgeLadder[i - 1],HedgeLadder[i]);
                //hedgeStatusPrint = hedgeStatusPrint + buffer;
                break;
            }
        }
        //开仓操作
        if(isSupposedBuyOpen){
            if(CurrentSectionSell <= SupposedSectionBuyOpen){
                //需要开仓
                HoldDetail newhold;
                newhold.HedgeNum = SupposedBuyOpen;
                newhold.adjustedCost = DeviationSell;
				newhold.originalCost = DeviationSell + datumDiff;
                newhold.HedgeSection = CurrentSectionSell;
                sprintf(buffer,_T("需要开仓%d手,相对价格%f,所在区间%d,左%f,右%f\r\n"),newhold.HedgeNum,newhold.adjustedCost,newhold.HedgeSection,HedgeLadder[newhold.HedgeSection - 1],HedgeLadder[newhold.HedgeSection]);
                hedgeStatusPrint = hedgeStatusPrint + buffer;
				newhold.id = ++idHold;
                HedgeHoldTemp.push_back(newhold);
                ReqHedgeOrder(&newhold,OPEN);
                sprintf(buffer,_T("======================END05======================\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
                return;
            }
        }
        if(isSupposedSellOpen){
            if(CurrentSectionBuy >= SupposedSectionSellOpen){
                //需要开仓
                HoldDetail newhold;
                newhold.HedgeNum = -SupposedSellOpen;
				newhold.adjustedCost = DeviationBuy;
				newhold.originalCost = DeviationBuy + datumDiff;
                newhold.HedgeSection = CurrentSectionBuy;
                sprintf(buffer,_T("需要开仓%d手,相对价格%f,所在区间%d,左%f,右%f\r\n"),newhold.HedgeNum,newhold.adjustedCost,newhold.HedgeSection,HedgeLadder[newhold.HedgeSection - 1],HedgeLadder[newhold.HedgeSection]);
                hedgeStatusPrint = hedgeStatusPrint + buffer;
				newhold.id = ++idHold;
                HedgeHoldTemp.push_back(newhold);
                ReqHedgeOrder(&newhold,OPEN);
                sprintf(buffer,_T("======================END06======================\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
                return;
            }
        }
    }
    else if(hedgeTaskStatus == WAITING_FOR_FINISHED){

    }
}

int CHiStarApp::ReqHedgeOrder(HoldDetail *pHD,bool OffsetFlag){
    HWND hEdit = ::GetDlgItem(((CMainDlg*)m_pMainWnd)->m_basicPage.m_hWnd,IDC_RICHEDIT_STATUS);
    int NeedBuyA50 = 0,NeedSellA50 = 0;
    int NeedBuyOpenIf = 0,NeedBuyCloseIf = 0,NeedSellOpenIf = 0,NeedSellCloseIf = 0;
    int PredictPositionA50 = 0;
    int PredictBuyA50 = 0,PredictSellA50 = 0;
    double NeedNewMarginA50 = 0,NeedNewMarginIf = 0;
    int PredictlongIf = 0,PredictshortIf = 0;
    //IF预计要采取的操作包括买、卖、开、平
    int PredictBuyOpenIF = 0,PredictBuyCloseIF = 0,PredictSellOpenIF = 0,PredictSellCloseIF = 0;	
    if(OffsetFlag == OPEN){
        //开仓
        PredictPositionA50 = netPositionA50 + pHD->HedgeNum * MultiA50;
        //注意IF的实际方向与HedgeNum相反;IF遵循先平仓再开仓的原则。
        if(pHD->HedgeNum > 0){
            NeedBuyA50 = pHD->HedgeNum * MultiA50;
            if(longIf - pHD->HedgeNum >= 0){
                //全部卖平
                PredictlongIf = longIf - pHD->HedgeNum;
                NeedSellCloseIf = pHD->HedgeNum;
            }
            else{
                //先卖平,再卖开
                PredictlongIf = 0;
                NeedSellCloseIf = longIf;
                PredictshortIf = -(longIf - pHD->HedgeNum);
                NeedSellOpenIf = -(longIf - pHD->HedgeNum);
            }
        }
        else{
            NeedSellA50 = -pHD->HedgeNum * MultiA50;
            if(shortIf + pHD->HedgeNum >= 0){
                //全部买平
                PredictshortIf = shortIf + pHD->HedgeNum;
                NeedBuyCloseIf = -pHD->HedgeNum;
            }
            else{
                //先买平,再买开
                PredictshortIf = 0;
                NeedBuyCloseIf = shortIf;
                PredictlongIf = -(shortIf + pHD->HedgeNum);
                NeedBuyOpenIf = -(shortIf + pHD->HedgeNum);
            }
        }
    }
    else{
        //平仓
        PredictPositionA50 = netPositionA50 - pHD->HedgeNum * MultiA50;
        //平仓时,IF的实际方向与HedgeNum一致;IF遵循先平仓再开仓的原则。
        if(pHD->HedgeNum > 0){
            NeedSellA50 = pHD->HedgeNum * MultiA50;
            if(shortIf - pHD->HedgeNum >= 0){
                //全部买平
                PredictshortIf = shortIf - pHD->HedgeNum;
                NeedBuyCloseIf = pHD->HedgeNum;
            }
            else{
                //先买平,再买开
                PredictshortIf = 0;
                NeedBuyCloseIf = shortIf;
                PredictlongIf = -(shortIf - pHD->HedgeNum);
                NeedBuyOpenIf = -(shortIf - pHD->HedgeNum);
            }
        }
        else{
            NeedBuyA50 = -pHD->HedgeNum * MultiA50;
            if(longIf + pHD->HedgeNum >= 0){
                //全部卖平
                PredictlongIf = longIf + pHD->HedgeNum;
                NeedSellCloseIf = -pHD->HedgeNum;
            }
            else{
                //先卖平,再卖开
                PredictlongIf = 0;
                NeedSellCloseIf = longIf;
                PredictshortIf = -(longIf + pHD->HedgeNum);
                NeedSellOpenIf = -(longIf + pHD->HedgeNum);
            }
        }
    }
    SYSTEMTIME sys;
    GetLocalTime(&sys);
    sprintf(buffer,_T("%2d:%2d:%2d:%3d\r\n"),sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds);hedgeStatusPrint = hedgeStatusPrint + buffer;
    sprintf(buffer,_T("操作:\r\n买A50--%d手\r\n,卖A50--%d手\r\n,买开IF---%d手\r\n,买平IF---%d手\r\n,卖开IF---%d手\r\n,卖平IF---%d手\r\n"),NeedBuyA50,NeedSellA50,NeedBuyOpenIf,NeedBuyCloseIf,NeedSellOpenIf,NeedSellCloseIf);
    hedgeStatusPrint = hedgeStatusPrint + buffer;
    sprintf(buffer,"A50指数%f,HS300指数%f, %f, %f, %f, %f, %f\r\n",A50Index,HS300Index,g_ifBid1,g_ifAsk1,g_a50Bid1,g_a50Ask1);hedgeStatusPrint = hedgeStatusPrint + buffer;
    sprintf(buffer,"premiumHigh%f,premiumLow%f\r\n",g_a50Ask1 - g_ifBid1 * A50Index / HS300Index,g_a50Bid1 - g_ifAsk1 * A50Index / HS300Index);hedgeStatusPrint = hedgeStatusPrint + buffer;
    ////////////////////////////////////////////////////////////////////////////////////
    //IB表示需要新的保证金
    if(abs(PredictPositionA50) > abs(netPositionA50)){
        NeedNewMarginA50 = pointValueA50 * MarginA50 * (abs(PredictPositionA50) - abs(netPositionA50));
        if(NeedNewMarginA50 - AvailIB > 1000.0){
            sprintf(buffer,_T("IB保证金不足,需要%f\r\n"),NeedNewMarginA50);
            hedgeStatusPrint = hedgeStatusPrint + buffer;
            sprintf(buffer,_T("======================END07======================\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
            return -1;
        }
    }
    //IF需要新的保证金
    if(PredictshortIf + PredictlongIf > longIf + shortIf){
        NeedNewMarginIf = (g_ifAsk1 + g_ifBid1) / 2.0 *  pointValueIf * MarginIf * (PredictshortIf + PredictlongIf - (longIf + shortIf));
        if(NeedNewMarginIf - AvailCtp > 6000.0){
            sprintf(buffer,_T("CTP保证金不足,需要%f\r\n"),NeedNewMarginIf);
            hedgeStatusPrint = hedgeStatusPrint + buffer;
            sprintf(buffer,_T("======================END08======================\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
            return -1;
        }
    }

    hedgeTaskStatus = WAITING_FOR_FINISHED;//标记等待状态，在等待状态下对冲循环不会有新的任务
    if(m_pHedgePostProcessing){
        m_pHedgePostProcessing->PostThreadMessage(WM_PREPARE_POST_PROCESSING,NULL,NULL);
    }
    ////////////////////////////////////////////////////////////
    //////千万注意要先清零,否则将会导致意想不到的错误。
    IfTask iftask;A50Task a50task;
    memset(&hedgetask,0,sizeof(hedgetask));memset(&iftask,0,sizeof(iftask));memset(&a50task,0,sizeof(a50task));
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
            iftask.ref = ((CHiStarApp*)AfxGetApp())->m_cT->ReqOrdLimit(*pInst,THOST_FTDC_D_Buy,kpp,g_ifAsk1 + 20.0,NeedBuyOpenIf);
            iftask.direction = THOST_FTDC_D_Buy;iftask.offset[0] = THOST_FTDC_OF_Open;iftask.priceRecord = g_ifAsk1;iftask.volumeRecord = NeedBuyOpenIf;
            hedgetask.ifalltask.push_back(iftask);
        }
    }
    if(NeedBuyCloseIf > 0){
        kpp[0] = THOST_FTDC_OF_Close;
        if(((CHiStarApp*)AfxGetApp())->m_cT){
            iftask.ref = ((CHiStarApp*)AfxGetApp())->m_cT->ReqOrdLimit(*pInst,THOST_FTDC_D_Buy,kpp,g_ifAsk1 + 20.0,NeedBuyCloseIf);
            iftask.direction = THOST_FTDC_D_Buy;iftask.offset[0] = THOST_FTDC_OF_Close;iftask.priceRecord = g_ifAsk1;iftask.volumeRecord = NeedBuyCloseIf;
            hedgetask.ifalltask.push_back(iftask);
        }
    }
    if(NeedSellOpenIf > 0){
        kpp[0] = THOST_FTDC_OF_Open;
        if(((CHiStarApp*)AfxGetApp())->m_cT){
            iftask.ref = ((CHiStarApp*)AfxGetApp())->m_cT->ReqOrdLimit(*pInst,THOST_FTDC_D_Sell,kpp,g_ifBid1 - 20.0,NeedSellOpenIf);
            iftask.direction = THOST_FTDC_D_Sell;iftask.offset[0] = THOST_FTDC_OF_Open;iftask.priceRecord = g_ifBid1;iftask.volumeRecord = NeedSellOpenIf;
            hedgetask.ifalltask.push_back(iftask);
        }
    }
    if(NeedSellCloseIf > 0){
        kpp[0] = THOST_FTDC_OF_Close;
        if(((CHiStarApp*)AfxGetApp())->m_cT){
            iftask.ref = ((CHiStarApp*)AfxGetApp())->m_cT->ReqOrdLimit(*pInst,THOST_FTDC_D_Sell,kpp,g_ifBid1 - 20.0,NeedSellCloseIf);
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
                ((CHiStarApp*)AfxGetApp())->m_IBOrder.lmtPrice = DealA50Price(true,g_a50Ask1 + 100.0);
                if(((CHiStarApp*)AfxGetApp())->m_IBOrder.lmtPrice < 1.0){
                    sprintf(buffer,_T("A50买价小于1.0,异常\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
					return -1;
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
                ((CHiStarApp*)AfxGetApp())->m_IBOrder.lmtPrice = DealA50Price(false,g_a50Bid1 - 100.0);
                if(((CHiStarApp*)AfxGetApp())->m_IBOrder.lmtPrice < 1.0){
                    sprintf(buffer,_T("A50买价小于1.0,异常\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
					return -1;
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
        m_pHedgePostProcessing->PostThreadMessage(WM_BEGIN_POST_PROCESSING,NULL,pHD->id);
    }
    return 0;
}

//后处理
void CHedgePostProcessing::PostProcessing(UINT t_wParam,LONG t_lParam){
	HWND hEdit = ::GetDlgItem(((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->m_basicPage.m_hWnd,IDC_RICHEDIT_STATUS);
	MSG msg;BOOL bRet;
	long idHedgeCurrent = -1;
	while((bRet = GetMessage(&msg,NULL,WM_BEGIN_POST_PROCESSING,WM_BEGIN_POST_PROCESSING)) != 0){
		if (!bRet){// handle the error and possibly exit
		}
		else{
			idHedgeCurrent = msg.lParam;
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
					sprintf(buffer,"收到WM_RTN_INSERT\r\n");hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
					CThostFtdcInputOrderField *pOrderInsert = (CThostFtdcInputOrderField *)msg.lParam;
					for(unsigned int i = 0;i < hedgetask.ifalltask.size();i++){
						if(hedgetask.ifalltask[i].ref == atoi(pOrderInsert->OrderRef)){
							hedgetask.ifalltask[i].bReceivedInsertRtn = true;
						}
					}
					delete (CThostFtdcInputOrderField*)msg.lParam;
					break;
				}
			case WM_RTN_ORDER:
				{
					sprintf(buffer,"收到WM_RTN_ORDER\r\n");hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
					CThostFtdcOrderField *pOrderRtn = (CThostFtdcOrderField *)msg.lParam;
					for(unsigned int i = 0;i < hedgetask.ifalltask.size();i++){
						if(hedgetask.ifalltask[i].ref == atoi(pOrderRtn->OrderRef)){
							if(pOrderRtn->OrderStatus == THOST_FTDC_OST_AllTraded || pOrderRtn->OrderStatus == THOST_FTDC_OST_Canceled 
								|| pOrderRtn->OrderStatus == THOST_FTDC_OST_NoTradeNotQueueing || pOrderRtn->OrderStatus == THOST_FTDC_OST_PartTradedNotQueueing){
									hedgetask.ifalltask[i].traded = pOrderRtn->VolumeTraded;
									hedgetask.ifalltask[i].sysid = atoi(pOrderRtn->OrderSysID);
									hedgetask.ifalltask[i].bReceivedAllOrder = true;
									sprintf(buffer,"Order,ref%d,最终状态%c\r\n",atoi(pOrderRtn->OrderRef),pOrderRtn->OrderStatus);hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
							}			
						}
					}
					delete (CThostFtdcOrderField*)msg.lParam;
					break;
				}
			}
			bool bBreakGetMsg = true;
			for(unsigned int i = 0;i < hedgetask.ifalltask.size();i++){
				if(!(hedgetask.ifalltask[i].bReceivedAllOrder || hedgetask.ifalltask[i].bReceivedInsertRtn)){
					bBreakGetMsg = false;break;
				}
			}
			if(bBreakGetMsg) break;
		}
	}
	//检查是否有成交量,可能都被取消而没有成交量
	bool bVolumeTraded = false;
	for(unsigned int i = 0;i < hedgetask.ifalltask.size();i++){
		if(hedgetask.ifalltask[i].traded > 0){
			bVolumeTraded = true;break;
		}
	}
	if(bVolumeTraded){
		while((bRet = GetMessage(&msg,NULL,WM_RTN_TRADE,WM_RTN_TRADE)) != 0){
			if (!bRet){
			}
			else{
				sprintf(buffer,"收到WM_RTN_TRADE\r\n");hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW
				CThostFtdcTradeField *pTradeRtn = (CThostFtdcTradeField *)msg.lParam;
				for(unsigned int i = 0;i < hedgetask.ifalltask.size();i++){
					if(hedgetask.ifalltask[i].sysid == atoi(pTradeRtn->OrderSysID)){
						hedgetask.ifalltask[i].receivedTradedVolume = hedgetask.ifalltask[i].receivedTradedVolume + pTradeRtn->Volume;
						hedgetask.ifalltask[i].receivedValue = hedgetask.ifalltask[i].receivedValue + pTradeRtn->Price * pTradeRtn->Volume;	
					}
				}
				delete (CThostFtdcTradeField*)msg.lParam;
				bool bBreakGetMsg = true;
				for(unsigned int i = 0;i < hedgetask.ifalltask.size();i++){
					if(hedgetask.ifalltask[i].receivedTradedVolume != hedgetask.ifalltask[i].traded){
						bBreakGetMsg = false;break;
					}
				}
				if(bBreakGetMsg) break;
			}
		}
		//计算平均价格
		for(unsigned int i = 0;i < hedgetask.ifalltask.size();i++){
			hedgetask.ifalltask[i].avgPrice = hedgetask.ifalltask[i].receivedValue / hedgetask.ifalltask[i].receivedTradedVolume;
		}
	}

	while((bRet = GetMessage(&msg,NULL,WM_RTN_ORDER_IB,WM_RTN_ORDER_IB)) != 0){
		if (!bRet){
		}
		else{
			sprintf(buffer,"收到WM_RTN_ORDER_IB\r\n");hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
			OrderStatus *pStatus = (OrderStatus *)msg.lParam; 
			for(unsigned int i = 0;i < hedgetask.a50alltask.size();i++){
				if(hedgetask.a50alltask[i].id == pStatus->orderId){
					if(pStatus->status == CString("Cancelled") || pStatus->status == CString("ApiCancelled") || hedgetask.a50alltask[i].volumeRecord == pStatus->filled){
						hedgetask.a50alltask[i].traded = pStatus->filled;
						hedgetask.a50alltask[i].avgPrice = pStatus->avgFillPrice;
						hedgetask.a50alltask[i].bReceivedAllStatus = true;
					}
				}
			}
			delete (OrderStatus*)msg.lParam;
			bool bBreakGetMsg = true;
			for(unsigned int i = 0;i < hedgetask.a50alltask.size();i++){
				if(!hedgetask.a50alltask[i].bReceivedAllStatus){
					bBreakGetMsg = false;
				}
			}
			if(bBreakGetMsg) break;
		}
	}
	//具体持仓统计
	double t_totalValueIf = 0.0;int t_totalTradedIf = 0;double t_avgPriceIf = 0.0;
	for(unsigned int i = 0;i < hedgetask.ifalltask.size();i++){
		if(hedgetask.ifalltask[i].direction == THOST_FTDC_D_Buy && hedgetask.ifalltask[i].offset[0] == THOST_FTDC_OF_Open){
			longIf = longIf + hedgetask.ifalltask[i].traded;
			t_totalTradedIf = t_totalTradedIf + hedgetask.ifalltask[i].traded;
			t_totalValueIf = t_totalValueIf + hedgetask.ifalltask[i].receivedValue;
		}
		else if(hedgetask.ifalltask[i].direction == THOST_FTDC_D_Buy && hedgetask.ifalltask[i].offset[0] == THOST_FTDC_OF_Close){
			shortIf = shortIf - hedgetask.ifalltask[i].traded;
			t_totalTradedIf = t_totalTradedIf + hedgetask.ifalltask[i].traded;
			t_totalValueIf = t_totalValueIf + hedgetask.ifalltask[i].receivedValue;
		}
		else if(hedgetask.ifalltask[i].direction == THOST_FTDC_D_Sell && hedgetask.ifalltask[i].offset[0] == THOST_FTDC_OF_Open){
			shortIf = shortIf + hedgetask.ifalltask[i].traded;
			t_totalTradedIf = t_totalTradedIf - hedgetask.ifalltask[i].traded;
			t_totalValueIf = t_totalValueIf - hedgetask.ifalltask[i].receivedValue;
		}
		else{
			longIf = longIf - hedgetask.ifalltask[i].traded;
			t_totalTradedIf = t_totalTradedIf - hedgetask.ifalltask[i].traded;
			t_totalValueIf = t_totalValueIf - hedgetask.ifalltask[i].receivedValue;
		}
	}
	if(t_totalTradedIf != 0){
		t_avgPriceIf = fabs(t_totalValueIf / t_totalTradedIf);
	}
	double t_totalValueA50 = 0.0;int t_totalTradedA50 = 0;double t_avgPriceA50 = 0.0;
	for(unsigned int i = 0;i < hedgetask.a50alltask.size();i++){
		if( hedgetask.a50alltask[i].direction == 'l'){
			netPositionA50 = netPositionA50 + hedgetask.a50alltask[i].traded;
			t_totalTradedA50 = t_totalTradedA50 + hedgetask.a50alltask[i].traded;
			t_totalValueA50 = t_totalValueA50 + hedgetask.a50alltask[i].traded * hedgetask.a50alltask[i].avgPrice;
		}
		else{
			netPositionA50 = netPositionA50 - hedgetask.a50alltask[i].traded;
			t_totalTradedA50 = t_totalTradedA50 - hedgetask.a50alltask[i].traded;
			t_totalValueA50 = t_totalValueA50 - hedgetask.a50alltask[i].traded * hedgetask.a50alltask[i].avgPrice;
		}
	}
	if(t_totalTradedA50 != 0){
		t_avgPriceA50 = fabs(t_totalValueA50 / t_totalTradedA50);
	}
	for(unsigned int i = 0;i < HedgeHoldTemp.size();i++){
		if(HedgeHoldTemp[i].id == idHedgeCurrent){
			if(fabs(t_avgPriceA50 - t_avgPriceIf * A50Index / HS300Index - HedgeHoldTemp[i].originalCost) < 10.0 && _isnan((t_avgPriceA50 - t_avgPriceIf * A50Index / HS300Index)) == 0){
				HedgeHoldTemp[i].originalCost = t_avgPriceA50 - t_avgPriceIf * A50Index / HS300Index;
			}
		}
	}
	SelectIndex();
	sprintf(buffer,_T("对冲绝对成本%f\r\n"),t_avgPriceA50 - t_avgPriceIf * A50Index / HS300Index);hedgeStatusPrint = hedgeStatusPrint + buffer;
	HedgeHold = HedgeHoldTemp;//更新Hold持仓
	hedgeTaskStatus = NEW_HEDGE;
	sprintf(buffer,_T("对冲结束\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
	Sleep(30000);//休息30秒,为了让系统有时间更新账户信息
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

void SelectIndex(){
	if(fabs(g_A50Index - g_A50IndexMSHQ) > 15.0){
		//TRACE("over 0.1,MS%f,SN%f\r\n",g_A50Index,g_A50Index);
		A50Index = g_A50Index;
	}
	else{
		A50Index = g_A50IndexMSHQ;
		TRACE("MS,SN,%f,%f\r\n",g_A50IndexMSHQ,g_A50Index);
	}
	if(fabs(g_HS300Index - g_HS300IndexMSHQ) > 5.0){
		//TRACE("over 0.1,MS%f,SN%f\r\n",g_HS300Index,g_HS300Index);
		HS300Index = g_HS300Index;
	}
	else{
		HS300Index = g_HS300IndexMSHQ;
		//TRACE("MS%f,SN%f\r\n",g_HS300IndexMSHQ,g_HS300Index);
	}
}

void CalcDeviation(){
    premium = (g_a50Bid1 + g_a50Ask1) / 2.0 - (g_ifAsk1 + g_ifBid1) / 2.0 * A50Index / HS300Index;
    premiumHigh = g_a50Ask1 - g_ifBid1 * A50Index / HS300Index;
    premiumLow = g_a50Bid1 - g_ifAsk1 * A50Index / HS300Index;
    deviation = premium - datumDiff;
    DeviationSell = premiumHigh - datumDiff;
    DeviationBuy = premiumLow - datumDiff;
}