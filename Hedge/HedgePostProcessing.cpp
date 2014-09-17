#include "stdafx.h"
#include "afxsock.h"
#include <afxinet.h> 
#include "HiStar.h"
#include "HedgePostProcessing.h"
#include "UserMsg.h"
#include "MainDlg.h"
#include "calendar.h"
#include <math.h>
#define SHOW if(IsWindow(hEdit)){::SendMessage(hEdit,WM_SETTEXT,0,(LPARAM)(LPCTSTR)hedgeStatusPrint);}
int SendMsg(CString msg);
#define OPEN true
#define CLOSE false
extern bool isReal;
bool iSell = true,iBuy = true;
extern DWORD MainThreadId;
double datumDiff = 0.0;
double pointValueA50 = 1.0;double pointValueIf = 300.0;//合约每个点的价值
int MultiA50 = 16;//A50乘数
double MarginA50 = 416.0,MarginIf = 0.15;
double USDtoRMB = 6.07;//汇率
bool isHedgeLoopingPause = true;
bool iIfCanTrade = true,iA50CanTrade = true;
double premium = 0,premiumHigh = 0,premiumLow = 0;
double deviation = 0,DeviationSell = 0,DeviationBuy = 0;
extern double g_a50Bid1,g_a50Ask1;
extern double g_ifAsk1,g_ifBid1;
extern double g_A50Index,g_HS300Index;
extern double g_A50IndexMSHQ,g_HS300IndexMSHQ;
extern bool iAccountDownloadEnd;
extern bool iInitMarginOIfAddOneA50;
int maxhold = 2;//最大持仓
CString hedgeStatusPrint;
double profitBackTest = 0.0,feeBackTest = 0.0,NetProfitBackTest = 0.0;
void SelectIndex(double &A50Index,double &HS300Index);
int iBackTestTime(SYSTEMTIME & systime);
/////////////////////////////////Hedge变量///////////////////////////////////////////////
int run_mode = 1;//运行模式
int MultiPos = 1;//持仓乘数
//梯级，一共21个分割点,分割成22(=21+1)个区间
const double HedgeLadderRef01[21] = {   -95, -85, -75, -65, -55, -45, -35, -25, -15, -5,  0,  5,   15,  25,  35,  45,  55,  65,   75,   85,   95};
const int PositionAimRef01[22] = { 10,  9,   8,   7,   6,   5,    4,  3,   2,   1,  0,  0,  -1,  -2,  -3,  -4,  -5,  -6,  -7,   -8,   -9,  -10};
const double HedgeLadderRef02[21] = {   -200, -180, -160, -140, -120, -100, -80, -60, -40, -30, -10,  20,  40,  60,  80,  100,  120,  140,  160,  180,  200};
const int PositionAimRef02[22] = {11,    10,    9,    8,    7,    6,    5,   4,   3,   2,   1,   0,  -1,  -2,  -3,  -4,  -5,    -6,   -7,   -8,   -9,   -10};//默认持仓目标单位（没有乘以乘数）
double HedgeLadder[21];
int PositionAim[22];
double MaxProfitAim = 10.0,MinProfitAim = 10.0;//最小盈利目标，最大盈利目标（不分多空）
///////////////////////////////////////////////////////
int netPositionA50 = 0,longIf = 0,shortIf = 0;//净持仓,需要计算;IF分空头净持仓和多头净持仓
double AvailIB = 0.0,AvailCtp = 0.0;//可用资金，需要计算
////////////////////////////////////////////////////////
char buffer[1000];
CVector<HoldDetail> HedgeHoldTemp;//临时使用的，先用这个进行预计算，最后同步到HedgeHold
CVector<HoldDetail> HedgeHold;
long maxIdHold = 0;//最大持仓id
struct A50Task{
	int volumeRecord;char direction;//'l'表示长仓,'s'表示短舱
	double priceRecord;int id;int traded;double avgPrice;
	bool bReceivedAllStatus;
};
struct IfTask{
	int volumeRecord;TThostFtdcDirectionType direction;TThostFtdcCombOffsetFlagType offset;
	double priceRecord;int ref;int sysid;TThostFtdcExchangeIDType ExchangeID;int traded;double avgPrice;int sessionid;int frontid;
	bool bReceivedInsertRtn;bool bReceivedAllOrder;int receivedTradedVolume;double receivedValue;
};
class HedgeTask{
public:
	std::vector<A50Task> a50alltask;
	std::vector<IfTask>  ifalltask;
public:
	HedgeTask();
	~HedgeTask();
	void clear();
};
HedgeTask::HedgeTask(){}
HedgeTask::~HedgeTask(){}
void HedgeTask::clear(){a50alltask.clear();ifalltask.clear();}
HedgeTask hedgetask;
//对冲交易初始状态
#define NEW_HEDGE 'n'
#define WAITING_FOR_FINISHED 'w'
char hedgeTaskStatus = 'n';
double DealA50Price(bool isBuy, double A50Price);
void CalcDeviation(double &a50Bid1,double &a50Ask1,double &ifBid1,double &ifAsk1,double &A50IndexNow,double &HS300IndexNow);
void tradePermit(bool &iIfTrade,bool &iA50Trade);
int seconds(SYSTEMTIME &time);//计算某个时刻的秒数
bool iBackTest01 = false;
double A50IndexNow_BackTest = 0.0,HS300IndexNow_BackTest = 0.0,t_avgPriceA50_BackTest = 0.0,t_avgPriceIf_BackTest = 0.0;
SYSTEMTIME systime,time_09_10_10,time_09_15_00,time_11_29_50,time_13_00_00,time_15_14_50,time_09_40_01;
// CHedgePostProcessing

IMPLEMENT_DYNCREATE(CHedgePostProcessing, CWinThread)

	CHedgePostProcessing::CHedgePostProcessing()
{
	//0.25是经过验证过的数据
	if(run_mode == 1){
		for(unsigned int i = 0;i < 21;i++){
			HedgeLadder[i] = HedgeLadderRef01[i] * 0.25;
		}
	}
	else if(run_mode == 2){
		for(unsigned int i = 0;i < 21;i++){
			HedgeLadder[i] = HedgeLadderRef02[i];
		}
	}
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
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CHedgePostProcessing, CWinThread)
	ON_THREAD_MESSAGE(WM_PREPARE_POST_PROCESSING,Run_PostProcessing)
END_MESSAGE_MAP()

void CHiStarApp::OnHedgeLooping(WPARAM wParam,LPARAM lParam){
	static bool iFirst = true;static HWND hEdit;
	double a50Bid1 = 0.0,a50Ask1 = 0.0,ifBid1 = 0.0,ifAsk1 = 0.0;
	double A50IndexNow = 0.0,HS300IndexNow = 0.0;
	if(isReal){
		SelectIndex(A50IndexNow,HS300IndexNow);
		a50Bid1 = g_a50Bid1;a50Ask1 = g_a50Ask1;ifBid1 = g_ifBid1;ifAsk1 = g_ifAsk1;
	}
	else{
		if(iBackTest01){
			//创建Hedge持仓明细表格
			if(conn){
				static MYSQL_RES * res_set = NULL;MYSQL_ROW row;
				static bool iFirstSql = true;
				if(iFirstSql){
					if(mysql_query(conn,"select * from market_20140530")){
						TRACE("Error %u: %s\n", mysql_errno(conn), mysql_error(conn)); 
					}
					res_set = mysql_store_result(conn);
					iFirstSql = false;
				}
				if(res_set != NULL){
					if ((row = mysql_fetch_row(res_set))){
						SYSTEMTIME timeBT;
						CString datetime,hour,minute,second;
						memset(&timeBT,0,sizeof(timeBT));
						datetime = row[0];
						hour = datetime.Mid(11,2);minute = datetime.Mid(14,2);second = datetime.Mid(17,2);
						timeBT.wHour = atoi(hour.GetBuffer());timeBT.wMinute = atoi(minute.GetBuffer());timeBT.wSecond = atoi(second.GetBuffer());
						if(iBackTestTime(timeBT) != 1)return;
						A50IndexNow = atof(row[2]);
						a50Bid1 = atof(row[3]);
						a50Ask1 = atof(row[4]);
						HS300IndexNow = atof(row[5]);
						ifBid1 = atof(row[6]);
						ifAsk1 = atof(row[7]);
						//test
						A50IndexNow_BackTest = A50IndexNow,HS300IndexNow_BackTest = HS300IndexNow;
						if(A50IndexNow < 1.0 || HS300IndexNow < 1.0 || a50Bid1 < 1.0 || a50Ask1 < 1.0 || ifBid1 < 1.0 || ifAsk1< 1.0){
							return;
						}
					}
					else{
						return;
					}
				}
				else{
					return;
				}
			}
		}
	}
	if(!iAccountDownloadEnd || !iInitMarginOIfAddOneA50){
		return;
	}
	if(iFirst){
		//A50合约保证金
		MarginA50 = fabs(m_accountvalue.InitMarginOIfAddOneA50 - m_accountvalue.InitMarginReqO);
		hEdit = ::GetDlgItem(((CMainDlg*)m_pMainWnd)->m_basicPage.m_hWnd,IDC_RICHEDIT_STATUS);
		iFirst = false;
	}
	//计算A50可用资金以及持仓
	AvailIB = m_accountvalue.AvailableFunds;
	for(unsigned int i = 0;i < m_portfolio.size();i++){
		//暂时这样对比，日后还有期权，因为存在行权价，可能contract比较方法有异
		if( m_A50Contract.symbol == m_portfolio[i].contract.symbol
			&& m_A50Contract.currency == m_portfolio[i].contract.currency
			&& m_A50Contract.expiry.Left(6) == m_portfolio[i].contract.expiry.Left(6)
			&& m_A50Contract.secType == m_portfolio[i].contract.secType){
				netPositionA50 = m_portfolio[i].position;
		}
	}
	if(((CMainDlg*)m_pMainWnd)){
		((CMainDlg*)m_pMainWnd)->OnRefreshMdPane(NULL,NULL);
	}
	if(isReal){
		tradePermit(iIfCanTrade,iA50CanTrade);
		if(!iA50CanTrade && !iIfCanTrade) return;
	}
	//求最大持仓id
	for(unsigned int i = 0;i < HedgeHold.size();i++){
		maxIdHold = max(maxIdHold,HedgeHold[i].id);
	}
	if(hedgeTaskStatus == NEW_HEDGE){
		HedgeHoldTemp = HedgeHold;
		for(unsigned int i = 0;i < HedgeHoldTemp.size();i++){
			HedgeHoldTemp[i].adjustedCost = HedgeHoldTemp[i].originalCost - datumDiff;
		}
		for(int i= 0;i < 22;i++){
			if(run_mode == 1){
				PositionAim[i] = PositionAimRef01[i] * MultiPos;
			}
			else if(run_mode == 2){
				PositionAim[i] = PositionAimRef02[i] * MultiPos;
			}
		}
		//默认盈利目标,一共22个区间，包含左右两个无边界区间
		double DefaultProfitAimBuy[22],DefaultProfitAimSell[22],ProfitAimBuy[22],ProfitAimSell[22];
		int netPosition = 0;//净持仓
		int SupposedBuyOpen = 0,SupposedSellOpen = 0;
		int SupposedSectionBuyOpen = 0,SupposedSectionSellOpen = 0;
		//增加这两个变量主要是考虑两侧边界可能已经饱和，这时isSupposedBuy或isSupposedSell将仍然保持false，表示无需任何操作
		bool isSupposedBuyOpen = false,isSupposedSellOpen = false;
		int CurrentSectionBuy = 0,CurrentSectionSell = 0;//当前所在的区间,Buy和Sell分别表示以买价和卖价计算
		CalcDeviation(a50Bid1,a50Ask1,ifBid1,ifAsk1,A50IndexNow,HS300IndexNow);
		if(isHedgeLoopingPause){//暂停
			return;
		}
		if(isReal){
			if(_isnan(datumDiff) != 0 || _isnan(premium)!=0 ||_isnan(deviation)!=0){
				return;//判断非零值错误
			}
			if(a50Bid1 < 1 || a50Ask1 < 1 || ifAsk1 < 1 || ifBid1 < 1 || A50IndexNow < 1 || HS300IndexNow < 1){
				return;
			}
			if(fabs(premium) > 500 || fabs(premium) < 0.01){
				return;//排除开盘时有可能报价不全导致的错误溢价计算
			}
		}
		//统计净持仓
		for(unsigned int i = 0;i < HedgeHoldTemp.size();i++){
			netPosition = netPosition + HedgeHoldTemp[i].HedgeNum;
		}
		for(int i = 1;i < 21;i++){
			if(HedgeLadder[i - 1] > HedgeLadder[i]){
				return;
			}
			if(PositionAim[i - 1] < PositionAim[i]){
				return;
			}
		}
		if(MaxProfitAim < MinProfitAim){
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
		}
		//每次循环只进行一次开仓或者平仓操作,完成后即return,进入下一个循环;
		//这么做因为持仓和资金只有在每个循环的开始才计算，中途不计算,
		//所以完成一个操作后返回重新计算持仓和资金才能进行下一个操作。
		//平仓操作
		for(unsigned int i = 0;i < HedgeHoldTemp.size();i++){
			if(HedgeHoldTemp[i].HedgeNum > 0){//多头持仓
				if(HedgeHoldTemp[i].HedgeSection == 21){
					//需要平仓
					if(DeviationBuy >= HedgeLadder[20] + ProfitAimBuy[HedgeHoldTemp[i].HedgeSection] 
					&& DeviationBuy >= HedgeHoldTemp[i].adjustedCost + ProfitAimBuy[HedgeHoldTemp[i].HedgeSection]){
						//需要平仓
						ReqHedgeOrder(&HedgeHoldTemp[i],CLOSE,a50Bid1,a50Ask1,ifBid1,ifAsk1,A50IndexNow,HS300IndexNow);
						HedgeHoldTemp.erase(HedgeHoldTemp.begin() + i);
						i--;
						return;
					}
				}
				else{
					if(DeviationBuy >= HedgeLadder[HedgeHoldTemp[i].HedgeSection] + ProfitAimBuy[HedgeHoldTemp[i].HedgeSection]
					&& DeviationBuy >= HedgeHoldTemp[i].adjustedCost + ProfitAimBuy[HedgeHoldTemp[i].HedgeSection]){
						//需要平仓
						ReqHedgeOrder(&HedgeHoldTemp[i],CLOSE,a50Bid1,a50Ask1,ifBid1,ifAsk1,A50IndexNow,HS300IndexNow);
						HedgeHoldTemp.erase(HedgeHoldTemp.begin() + i);
						i--;
						return;
					}
				}
			}
			else if(HedgeHoldTemp[i].HedgeNum < 0){//空头持仓
				if(HedgeHoldTemp[i].HedgeSection == 0){
					if(DeviationSell <= HedgeLadder[0] - ProfitAimSell[HedgeHoldTemp[i].HedgeSection]
					&& DeviationSell <= HedgeHoldTemp[i].adjustedCost - ProfitAimSell[HedgeHoldTemp[i].HedgeSection]){	
						ReqHedgeOrder(&HedgeHoldTemp[i],CLOSE,a50Bid1,a50Ask1,ifBid1,ifAsk1,A50IndexNow,HS300IndexNow);
						HedgeHoldTemp.erase(HedgeHoldTemp.begin() + i);
						i--;
						return;
					}
				}
				else{
					if(DeviationSell <= HedgeLadder[HedgeHoldTemp[i].HedgeSection - 1] - ProfitAimSell[HedgeHoldTemp[i].HedgeSection]
					&& DeviationSell <= HedgeHoldTemp[i].adjustedCost - ProfitAimSell[HedgeHoldTemp[i].HedgeSection]){
						ReqHedgeOrder(&HedgeHoldTemp[i],CLOSE,a50Bid1,a50Ask1,ifBid1,ifAsk1,A50IndexNow,HS300IndexNow);
						HedgeHoldTemp.erase(HedgeHoldTemp.begin() + i);
						i--;
						return;
					}
				}
			}
		}
		for(int i = 0;i <= 21;i++){
			if(PositionAim[i] < 0 && netPosition <= 0){
				if(-abs(netPosition) > PositionAim[i]){
					SupposedSellOpen = -abs(netPosition) - PositionAim[i];
					SupposedSectionSellOpen = i;
					isSupposedSellOpen = true;
					break;
				}
			}
		}
		for(int i = 21;i >= 0;i--){
			if(PositionAim[i] > 0 && netPosition >= 0){
				if(abs(netPosition) < PositionAim[i]){
					SupposedBuyOpen = -(abs(netPosition) - PositionAim[i]);
					SupposedSectionBuyOpen = i;
					isSupposedBuyOpen = true;
					break;
				}
			}
		}
		if(abs(netPosition) >= abs(maxhold))return;
		//开仓操作
		if(isSupposedBuyOpen && iBuy){
			if(CurrentSectionSell <= SupposedSectionBuyOpen){
				//需要开仓
				HoldDetail newhold;
				newhold.HedgeNum = SupposedBuyOpen;
				newhold.adjustedCost = DeviationSell;
				newhold.originalCost = DeviationSell + datumDiff;
				newhold.priceIf = ifBid1;
				newhold.indexHS300 = HS300IndexNow;
				newhold.priceA50 = a50Ask1;
				newhold.indexA50 = A50IndexNow;
				newhold.HedgeSection = SupposedSectionBuyOpen;
				newhold.numA50 = SupposedBuyOpen * MultiA50;
				newhold.numIf = -SupposedBuyOpen;
				newhold.id = ++maxIdHold;
				HedgeHoldTemp.push_back(newhold);
				ReqHedgeOrder(&newhold,OPEN,a50Bid1,a50Ask1,ifBid1,ifAsk1,A50IndexNow,HS300IndexNow);
				return;
			}
		}
		//注意：这个地方千万不能将if改成else if！！
		if(isSupposedSellOpen && iSell){
			if(CurrentSectionBuy >= SupposedSectionSellOpen){
				//需要开仓
				HoldDetail newhold;
				newhold.HedgeNum = -SupposedSellOpen;
				newhold.adjustedCost = DeviationBuy;
				newhold.originalCost = DeviationBuy + datumDiff;
				newhold.priceIf = ifAsk1;
				newhold.indexHS300 = HS300IndexNow;
				newhold.priceA50 = a50Bid1;
				newhold.indexA50 = A50IndexNow;
				newhold.HedgeSection = SupposedSectionSellOpen;
				newhold.numA50 = -SupposedSellOpen * MultiA50;
				newhold.numIf = SupposedSellOpen;
				newhold.id = ++maxIdHold;
				HedgeHoldTemp.push_back(newhold);
				ReqHedgeOrder(&newhold,OPEN,a50Bid1,a50Ask1,ifBid1,ifAsk1,A50IndexNow,HS300IndexNow);
				return;
			}
		}
	}
	else if(hedgeTaskStatus == WAITING_FOR_FINISHED){

	}
}

int CHiStarApp::ReqHedgeOrder(HoldDetail *pHD,bool OffsetFlag,double &a50Bid1,double &a50Ask1,double &ifBid1,double &ifAsk1,double &A50IndexNow,double &HS300IndexNow){
	HWND hEdit = ::GetDlgItem(((CMainDlg*)m_pMainWnd)->m_basicPage.m_hWnd,IDC_RICHEDIT_STATUS);
	int NeedBuyA50 = 0,NeedSellA50 = 0;
	int NeedBuyOpenIf = 0,NeedBuyCloseIf = 0,NeedSellOpenIf = 0,NeedSellCloseIf = 0;
	int PredictPositionA50 = 0;
	int PredictBuyA50 = 0,PredictSellA50 = 0;
	double NeedNewMarginA50 = 0,NeedNewMarginIf = 0;
	int PredictlongIf = 0,PredictshortIf = 0;
	//IF预计要采取的操作包括买、卖、开、平
	int PredictBuyOpenIF = 0,PredictBuyCloseIF = 0,PredictSellOpenIF = 0,PredictSellCloseIF = 0;
	//统计长短持仓
	AcquireSRWLockShared(&g_srwLock_PosDetail);
	longIf = 0;shortIf = 0;
	for(unsigned int i = 0;i < m_cT->m_InvPosDetailVec.size();i++){
		if(strcmp(m_cT->m_InvPosDetailVec[i].InstrumentID,m_accountCtp.m_szInst) == 0){
			if(m_cT->m_InvPosDetailVec[i].Direction == THOST_FTDC_D_Buy){
				longIf = longIf + m_cT->m_InvPosDetailVec[i].Volume;
			}
			else{
				shortIf =  shortIf + m_cT->m_InvPosDetailVec[i].Volume;
			}
			//因为同样的合约可能是不同的日期，所以可能有两个持仓，故取消break
			//break;
		}
	}
	ReleaseSRWLockShared(&g_srwLock_PosDetail); 
	//
	if(OffsetFlag == OPEN){
		//开仓
		PredictPositionA50 = netPositionA50 + pHD->numA50;
		//注意IF的实际方向与HedgeNum相反;IF遵循先平仓再开仓的原则。
		if(pHD->HedgeNum > 0){
			NeedBuyA50 = pHD->numA50;
			if(longIf + pHD->numIf >= 0){
				//全部卖平
				PredictlongIf = longIf + pHD->numIf;
				PredictshortIf = shortIf;
				NeedSellCloseIf = -pHD->numIf;
			}
			else{
				//先卖平,再卖开
				PredictlongIf = 0;
				NeedSellCloseIf = longIf;
				PredictshortIf = shortIf + (-(longIf + pHD->numIf));
				NeedSellOpenIf = -(longIf + pHD->numIf);
			}
		}
		else{
			NeedSellA50 = -pHD->numA50;
			if(shortIf - pHD->numIf >= 0){
				//全部买平
				PredictshortIf = shortIf - pHD->numIf;
				PredictlongIf = longIf;
				NeedBuyCloseIf = pHD->numIf;
			}
			else{
				//先买平,再买开
				PredictshortIf = 0;
				NeedBuyCloseIf = shortIf;
				PredictlongIf = longIf + (-(shortIf - pHD->numIf));
				NeedBuyOpenIf = -(shortIf - pHD->numIf);
			}
		}
	}
	else{
		//平仓
		PredictPositionA50 = netPositionA50 - pHD->numA50;
		//平仓时,IF的实际方向与HedgeNum一致;IF遵循先平仓再开仓的原则。
		if(pHD->HedgeNum > 0){
			NeedSellA50 = pHD->numA50;
			if(shortIf + pHD->numIf >= 0){
				//全部买平
				PredictshortIf = shortIf + pHD->numIf;
				PredictlongIf = longIf;
				NeedBuyCloseIf = -pHD->numIf;
			}
			else{
				//先买平,再买开
				PredictshortIf = 0;
				NeedBuyCloseIf = shortIf;
				PredictlongIf = longIf + (-(shortIf + pHD->numIf));
				NeedBuyOpenIf = -(shortIf + pHD->numIf);
			}
		}
		else{
			NeedBuyA50 = -pHD->numA50;
			if(longIf - pHD->numIf >= 0){
				//全部卖平
				PredictlongIf = longIf - pHD->numIf;
				PredictshortIf = shortIf;
				NeedSellCloseIf = pHD->numIf;
			}
			else{
				//先卖平,再卖开
				PredictlongIf = 0;
				NeedSellCloseIf = longIf;
				PredictshortIf = shortIf + (-(longIf - pHD->numIf));
				NeedSellOpenIf = -(longIf - pHD->numIf);
			}
		}
	}
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	////////////////////////////////////////////////////////////////////////////////////
	//IB表示需要新的保证金
	if(abs(PredictPositionA50) > abs(netPositionA50)){
		NeedNewMarginA50 = pointValueA50 * MarginA50 * (abs(PredictPositionA50) - abs(netPositionA50));
		if(NeedNewMarginA50 - AvailIB > 300.0){
			sprintf_s(buffer,_T("IB保证金不足,需要%.02lf\r\n"),NeedNewMarginA50);
			hedgeStatusPrint = hedgeStatusPrint + buffer;
			sprintf_s(buffer,_T("======================END07======================\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
			return -1;
		}
	}
	//IF需要新的保证金
	if(PredictshortIf + PredictlongIf > longIf + shortIf){
		NeedNewMarginIf = (ifAsk1 + ifBid1) / 2.0 *  pointValueIf * MarginIf * (PredictshortIf + PredictlongIf - (longIf + shortIf));
		if(NeedNewMarginIf - AvailCtp > 2000.0){
			sprintf_s(buffer,_T("CTP保证金不足,需要%.02lf\r\n"),NeedNewMarginIf);
			hedgeStatusPrint = hedgeStatusPrint + buffer;
			sprintf_s(buffer,_T("======================END08======================\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
			return -1;
		}
	}
	hedgeTaskStatus = WAITING_FOR_FINISHED;//标记等待状态，在等待状态下对冲循环不会有新的任务
	if(m_pHedgePostProcessing){
		while(m_pHedgePostProcessing->PostThreadMessage(WM_PREPARE_POST_PROCESSING,NULL,NULL) == 0){
			Sleep(100);
		}
	}
	////////////////////////////////////////////////////////////
	//////千万注意要先清零,否则将会导致意想不到的错误。
	IfTask iftask;A50Task a50task;
	hedgetask.clear();/*禁止memset(&hedgetask,0,sizeof(hedgetask));*/
	memset(&iftask,0,sizeof(iftask));memset(&a50task,0,sizeof(a50task));
	////////////////////////////////////////////////////////////
	//A50下单
	if(abs(NeedBuyA50 - NeedSellA50) != 0){
		if(((CHiStarApp*)AfxGetApp())->m_pIBClient){
			if(NeedBuyA50 - NeedSellA50 > 0){
				((CHiStarApp*)AfxGetApp())->m_IBOrder.action = "BUY";
				((CHiStarApp*)AfxGetApp())->m_IBOrder.totalQuantity = NeedBuyA50 - NeedSellA50;
				((CHiStarApp*)AfxGetApp())->m_IBOrder.lmtPrice = DealA50Price(true,a50Ask1 + 500.0);
				((CHiStarApp*)AfxGetApp())->m_IBOrder.whatIf = false;
				if(((CHiStarApp*)AfxGetApp())->m_IBOrder.lmtPrice < 1.0){
					sprintf_s(buffer,_T("A50买价小于1.0,异常\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
					return -1;
				}
				if(((CHiStarApp*)AfxGetApp())->m_pIBClient){
					if(iA50CanTrade){
						if(iBackTest01){
							t_avgPriceA50_BackTest = a50Ask1;
						}
						else{
							((CHiStarApp*)AfxGetApp())->m_pIBClient->placeOrder(++((CHiStarApp*)AfxGetApp())->m_id,((CHiStarApp*)AfxGetApp())->m_A50Contract,((CHiStarApp*)AfxGetApp())->m_IBOrder);
							a50task.id = ((CHiStarApp*)AfxGetApp())->m_id;a50task.volumeRecord = NeedBuyA50 - NeedSellA50;a50task.direction = 'l';a50task.priceRecord = a50Ask1;
							hedgetask.a50alltask.push_back(a50task);
						}
					}
				}
			}
			else if(NeedBuyA50 - NeedSellA50 < 0){
				((CHiStarApp*)AfxGetApp())->m_IBOrder.action = "SELL";
				((CHiStarApp*)AfxGetApp())->m_IBOrder.totalQuantity = -(NeedBuyA50 - NeedSellA50);
				((CHiStarApp*)AfxGetApp())->m_IBOrder.lmtPrice = DealA50Price(false,a50Bid1 - 500.0);
				((CHiStarApp*)AfxGetApp())->m_IBOrder.whatIf = false;
				if(((CHiStarApp*)AfxGetApp())->m_IBOrder.lmtPrice < 1.0){
					sprintf_s(buffer,_T("A50买价小于1.0,异常\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
					return -1;
				}
				if(((CHiStarApp*)AfxGetApp())->m_pIBClient){
					if(iA50CanTrade){
						if(iBackTest01){
							t_avgPriceA50_BackTest = a50Bid1;
						}
						else{
							((CHiStarApp*)AfxGetApp())->m_pIBClient->placeOrder(++((CHiStarApp*)AfxGetApp())->m_id,((CHiStarApp*)AfxGetApp())->m_A50Contract,((CHiStarApp*)AfxGetApp())->m_IBOrder);
							a50task.id = ((CHiStarApp*)AfxGetApp())->m_id;a50task.volumeRecord = -(NeedBuyA50 - NeedSellA50);a50task.direction = 's';a50task.priceRecord = a50Bid1;
							hedgetask.a50alltask.push_back(a50task);
						}
					}
				}
			}
		}
	}
	//IF下单时间特殊控制
	if(isReal){
		while(!iIfCanTrade){
			tradePermit(iIfCanTrade,iA50CanTrade);
		}
	}
	TThostFtdcCombOffsetFlagType kpp;
	char szInst[MAX_PATH];
	uni2ansi(CP_ACP,((CHiStarApp*)AfxGetApp())->m_accountCtp.m_szInst,szInst);
	LPSTR* pInst = new LPSTR;
	pInst[0] = szInst;
	iftask.frontid = m_cT->m_ifrontId;iftask.sessionid = m_cT->m_isessionId;
	if(NeedBuyOpenIf > 0){
		kpp[0] = THOST_FTDC_OF_Open;
		if(((CHiStarApp*)AfxGetApp())->m_cT){
			if(iBackTest01){
				t_avgPriceIf_BackTest = ifAsk1;
			}
			else{
				iftask.ref = ((CHiStarApp*)AfxGetApp())->m_cT->ReqOrdLimit(*pInst,THOST_FTDC_D_Buy,kpp,ifAsk1 + 100.0,NeedBuyOpenIf);
				iftask.direction = THOST_FTDC_D_Buy;iftask.offset[0] = THOST_FTDC_OF_Open;iftask.priceRecord = ifAsk1;iftask.volumeRecord = NeedBuyOpenIf;
				hedgetask.ifalltask.push_back(iftask);
			}
		}
	}
	if(NeedBuyCloseIf > 0){
		if(isReal){
			kpp[0] = THOST_FTDC_OF_Close;
		}
		else{
			kpp[0] = THOST_FTDC_OF_CloseToday;
		}
		if(((CHiStarApp*)AfxGetApp())->m_cT){
			if(iBackTest01){
				t_avgPriceIf_BackTest = ifAsk1;
			}
			else{
				iftask.ref = ((CHiStarApp*)AfxGetApp())->m_cT->ReqOrdLimit(*pInst,THOST_FTDC_D_Buy,kpp,ifAsk1 + 100.0,NeedBuyCloseIf);
				iftask.direction = THOST_FTDC_D_Buy;iftask.offset[0] = THOST_FTDC_OF_Close;iftask.priceRecord = ifAsk1;iftask.volumeRecord = NeedBuyCloseIf;
				hedgetask.ifalltask.push_back(iftask);
			}
		}
	}
	if(NeedSellOpenIf > 0){
		kpp[0] = THOST_FTDC_OF_Open;
		if(((CHiStarApp*)AfxGetApp())->m_cT){
			if(iBackTest01){
				t_avgPriceIf_BackTest = ifBid1;
			}
			else{
				iftask.ref = ((CHiStarApp*)AfxGetApp())->m_cT->ReqOrdLimit(*pInst,THOST_FTDC_D_Sell,kpp,ifBid1 - 100.0,NeedSellOpenIf);
				iftask.direction = THOST_FTDC_D_Sell;iftask.offset[0] = THOST_FTDC_OF_Open;iftask.priceRecord = ifBid1;iftask.volumeRecord = NeedSellOpenIf;
				hedgetask.ifalltask.push_back(iftask);
			}
		}
	}
	if(NeedSellCloseIf > 0){
		if(isReal){
			kpp[0] = THOST_FTDC_OF_Close;
		}
		else{
			kpp[0] = THOST_FTDC_OF_CloseToday;
		}
		if(((CHiStarApp*)AfxGetApp())->m_cT){
			if(iBackTest01){
				t_avgPriceIf_BackTest = ifBid1;
			}
			else{
				iftask.ref = ((CHiStarApp*)AfxGetApp())->m_cT->ReqOrdLimit(*pInst,THOST_FTDC_D_Sell,kpp,ifBid1 - 100.0,NeedSellCloseIf);
				iftask.direction = THOST_FTDC_D_Sell;iftask.offset[0] = THOST_FTDC_OF_Close;iftask.priceRecord = ifBid1;iftask.volumeRecord = NeedSellCloseIf;
				hedgetask.ifalltask.push_back(iftask);
			}
		}
	}
	delete []pInst;//防止内存泄露
	//开始后处理
	if(m_pHedgePostProcessing){
		HoldDetail *pHDNew = new HoldDetail();
		*pHDNew = *pHD;
		while(m_pHedgePostProcessing->PostThreadMessage(WM_BEGIN_POST_PROCESSING,OffsetFlag,(LPARAM)pHDNew) == 0){
			Sleep(100);
		}
	}
	return 0;
}

void CHedgePostProcessing::Run_PostProcessing(WPARAM t_wParam,LPARAM t_lParam){
	HWND hEdit = ::GetDlgItem(((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->m_basicPage.m_hWnd,IDC_RICHEDIT_STATUS);
	double A50IndexNow = 0.0,HS300IndexNow = 0.0;
	double t_totalValueA50 = 0.0;int t_totalTradedA50 = 0;double t_avgPriceA50 = 0.0;
	double t_totalValueIf = 0.0;int t_totalTradedIf = 0;double t_avgPriceIf = 0.0;
	HoldDetail hd;bool OffsetFlag = OPEN; 
	MSG m_msgCur;
	while(true){
		if(::PeekMessage(&m_msgCur,NULL,WM_QUIT,WM_QUIT,PM_NOREMOVE)){
			while(::PeekMessage(&m_msgCur,NULL,NULL,NULL,PM_REMOVE)){};
			while(::PostThreadMessage(::GetCurrentThreadId(),WM_QUIT,0,0) == 0){
				Sleep(100);
			}
			return;
		}
		else if(::PeekMessage(&m_msgCur,NULL,WM_BEGIN_POST_PROCESSING,WM_BEGIN_POST_PROCESSING,PM_REMOVE)){
			hd = *((HoldDetail*)m_msgCur.lParam);OffsetFlag = (bool)(m_msgCur.wParam);
			delete (HoldDetail*)m_msgCur.lParam;
			break;//已经开始，往下正式进行处理。
		}
	}
	if(!iBackTest01){
		while(true && hd.numIf != 0){
			if(::PeekMessage(&m_msgCur,NULL,WM_QUIT,WM_QUIT,PM_NOREMOVE)){
				while(::PeekMessage(&m_msgCur,NULL,NULL,NULL,PM_REMOVE)){};
				while(::PostThreadMessage(::GetCurrentThreadId(),WM_QUIT,0,0) == 0){
					Sleep(100);
				}
				return;
			}
			else if(::PeekMessage(&m_msgCur,NULL,WM_RTN_INSERT,WM_RTN_ORDER,PM_REMOVE)){
				switch(m_msgCur.message)
				{
				case WM_RTN_INSERT:
					{//存在瑕疵，最好用requestid来区分
						sprintf_s(buffer,_T("收到WM_RTN_INSERT\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
						CThostFtdcInputOrderField *pOrderInsert = (CThostFtdcInputOrderField *)m_msgCur.lParam;
						for(unsigned int i = 0;i < hedgetask.ifalltask.size();i++){
							if(hedgetask.ifalltask[i].ref == atoi(pOrderInsert->OrderRef)){
								hedgetask.ifalltask[i].bReceivedInsertRtn = true;
							}
						}
						delete (CThostFtdcInputOrderField*)m_msgCur.lParam;
						break;
					}
				case WM_RTN_ORDER:
					{
						sprintf_s(buffer,_T("收到WM_RTN_ORDER\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
						CThostFtdcOrderField *pOrderRtn = (CThostFtdcOrderField *)m_msgCur.lParam;
						for(unsigned int i = 0;i < hedgetask.ifalltask.size();i++){
							if(hedgetask.ifalltask[i].ref == atoi(pOrderRtn->OrderRef)
								&&hedgetask.ifalltask[i].frontid == pOrderRtn->FrontID
								&&hedgetask.ifalltask[i].sessionid == pOrderRtn->SessionID){
									if(pOrderRtn->OrderStatus == THOST_FTDC_OST_AllTraded || pOrderRtn->OrderStatus == THOST_FTDC_OST_Canceled 
										|| pOrderRtn->OrderStatus == THOST_FTDC_OST_NoTradeNotQueueing || pOrderRtn->OrderStatus == THOST_FTDC_OST_PartTradedNotQueueing){
											hedgetask.ifalltask[i].traded = pOrderRtn->VolumeTraded;
											hedgetask.ifalltask[i].sysid = atoi(pOrderRtn->OrderSysID);
											strcpy_s(hedgetask.ifalltask[i].ExchangeID,pOrderRtn->ExchangeID);
											hedgetask.ifalltask[i].bReceivedAllOrder = true;
											sprintf_s(buffer,_T("Order,ref%d,最终状态%c\r\n"),atoi(pOrderRtn->OrderRef),pOrderRtn->OrderStatus);hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
									}			
							}
						}
						delete (CThostFtdcOrderField*)m_msgCur.lParam;
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
			while(true){
				if(::PeekMessage(&m_msgCur,NULL,WM_QUIT,WM_QUIT,PM_NOREMOVE)){
					while(::PeekMessage(&m_msgCur,NULL,NULL,NULL,PM_REMOVE)){};
					while(::PostThreadMessage(::GetCurrentThreadId(),WM_QUIT,0,0) == 0){
						Sleep(100);
					}
					return;
				}
				else if(::PeekMessage(&m_msgCur,NULL,WM_RTN_TRADE,WM_RTN_TRADE,PM_REMOVE)){
					sprintf_s(buffer,_T("收到WM_RTN_TRADE\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW
						CThostFtdcTradeField *pTradeRtn = (CThostFtdcTradeField *)m_msgCur.lParam;
					for(unsigned int i = 0;i < hedgetask.ifalltask.size();i++){
						if(hedgetask.ifalltask[i].sysid == atoi(pTradeRtn->OrderSysID) && strcmp(hedgetask.ifalltask[i].ExchangeID,pTradeRtn->ExchangeID) == 0){
							hedgetask.ifalltask[i].receivedTradedVolume = hedgetask.ifalltask[i].receivedTradedVolume + pTradeRtn->Volume;
							hedgetask.ifalltask[i].receivedValue = hedgetask.ifalltask[i].receivedValue + pTradeRtn->Price * pTradeRtn->Volume;	
						}
					}
					delete (CThostFtdcTradeField*)m_msgCur.lParam;
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
		while(true && hd.numA50 != 0){
			if(::PeekMessage(&m_msgCur,NULL,WM_QUIT,WM_QUIT,PM_NOREMOVE)){
				while(::PeekMessage(&m_msgCur,NULL,NULL,NULL,PM_REMOVE)){};
				while(::PostThreadMessage(::GetCurrentThreadId(),WM_QUIT,0,0) == 0){
					Sleep(100);
				}
				return;
			}
			else if(::PeekMessage(&m_msgCur,NULL,WM_RTN_ORDER_IB,WM_RTN_ORDER_IB,PM_REMOVE)){
				sprintf_s(buffer,_T("收到WM_RTN_ORDER_IB\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
				OrderStatus *pStatus = (OrderStatus *)m_msgCur.lParam; 
				for(unsigned int i = 0;i < hedgetask.a50alltask.size();i++){
					if(hedgetask.a50alltask[i].id == pStatus->orderId){
						if(pStatus->status == CString("Cancelled") || pStatus->status == CString("ApiCancelled") || hedgetask.a50alltask[i].volumeRecord == pStatus->filled){
							hedgetask.a50alltask[i].traded = pStatus->filled;
							hedgetask.a50alltask[i].avgPrice = pStatus->avgFillPrice;
							hedgetask.a50alltask[i].bReceivedAllStatus = true;
						}
					}
				}
				delete (OrderStatus*)m_msgCur.lParam;
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
		for(unsigned int i = 0;i < hedgetask.ifalltask.size();i++){
			if(hedgetask.ifalltask[i].direction == THOST_FTDC_D_Buy){
				t_totalTradedIf = t_totalTradedIf + hedgetask.ifalltask[i].traded;
				t_totalValueIf = t_totalValueIf + hedgetask.ifalltask[i].receivedValue;
			}
			else{
				t_totalTradedIf = t_totalTradedIf - hedgetask.ifalltask[i].traded;
				t_totalValueIf = t_totalValueIf - hedgetask.ifalltask[i].receivedValue;
			}
		}
		if(t_totalTradedIf != 0){
			t_avgPriceIf = fabs(t_totalValueIf / t_totalTradedIf);
		}
		for(unsigned int i = 0;i < hedgetask.a50alltask.size();i++){
			if( hedgetask.a50alltask[i].direction == 'l'){
				t_totalTradedA50 = t_totalTradedA50 + hedgetask.a50alltask[i].traded;
				t_totalValueA50 = t_totalValueA50 + hedgetask.a50alltask[i].traded * hedgetask.a50alltask[i].avgPrice;
			}
			else{
				t_totalTradedA50 = t_totalTradedA50 - hedgetask.a50alltask[i].traded;
				t_totalValueA50 = t_totalValueA50 - hedgetask.a50alltask[i].traded * hedgetask.a50alltask[i].avgPrice;
			}
		}
		if(t_totalTradedA50 != 0){
			t_avgPriceA50 = fabs(t_totalValueA50 / t_totalTradedA50);
		}
		SelectIndex(A50IndexNow,HS300IndexNow);
	}
	else{
		if(OffsetFlag == OPEN){
			t_totalTradedIf = hd.numIf;
			t_totalTradedA50 = hd.numA50;
			t_avgPriceA50 = t_avgPriceA50_BackTest;
			t_avgPriceIf = t_avgPriceIf_BackTest;
			A50IndexNow = A50IndexNow_BackTest;
			HS300IndexNow = HS300IndexNow_BackTest;
		}
		else{
			t_totalTradedIf = -hd.numIf;
			t_totalTradedA50 = -hd.numA50;
			t_avgPriceA50 = t_avgPriceA50_BackTest;
			t_avgPriceIf = t_avgPriceIf_BackTest;
			A50IndexNow = A50IndexNow_BackTest;
			HS300IndexNow = HS300IndexNow_BackTest;
		}
	}
	if(OffsetFlag == OPEN){
		for(unsigned int i = 0;i < HedgeHoldTemp.size();i++){
			if(HedgeHoldTemp[i].id == hd.id){
				//暂时不对成本和成交价格数据进行修正，仍然使用最初预填进去的数据。
				/*
				if(fabs(t_avgPriceA50 - t_avgPriceIf * A50IndexNow / HS300IndexNow - HedgeHoldTemp[i].originalCost) < 10.0 && _isnan((t_avgPriceA50 - t_avgPriceIf * A50IndexNow / HS300IndexNow)) == 0){
				HedgeHoldTemp[i].originalCost = t_avgPriceA50 - t_avgPriceIf * A50IndexNow / HS300IndexNow;
				HedgeHoldTemp[i].priceIf = t_avgPriceIf;HedgeHoldTemp[i].indexHS300 = HS300IndexNow;
				HedgeHoldTemp[i].priceA50 = t_avgPriceA50;HedgeHoldTemp[i].indexA50 = A50IndexNow;
				}
				*/
			}
		}
	}
	else{
		//统计盈利 
		if(iBackTest01){
			feeBackTest = feeBackTest + 1.5 * 2 * 6.2 * abs(hd.numA50) + 65 * 2 * abs(hd.numIf); 
			profitBackTest = profitBackTest + (t_avgPriceA50 - hd.priceA50) * 6.2 * hd.numA50 + (t_avgPriceIf - hd.priceIf) * 300.0 * hd.numIf;
			NetProfitBackTest = profitBackTest - feeBackTest;
		}
	}
	if((OffsetFlag == OPEN && (hd.numA50 != t_totalTradedA50 || hd.numIf != t_totalTradedIf))
		|| (OffsetFlag == CLOSE && (hd.numA50 != -t_totalTradedA50 || hd.numIf != -t_totalTradedIf))){
			isHedgeLoopingPause = true;
			if((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd)){
				((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->m_basicPage.m_btnRun.SetWindowText(_T("成交错误！"));
			}
			hedgeTaskStatus = NEW_HEDGE;
			sprintf_s(buffer,_T("HEDGE INFORMATION:TRADE ERROR OF NUM!\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
			if(isReal)SendMsg(buffer);
			return;
	}
	sprintf_s(buffer,_T("HEDGE INFORMATION:%.02lf,A50:%.02lf,IF:%.02lf,A50INDEX:%.02lf,HS300INDEX:%.02lf\r\n"),t_avgPriceA50 - t_avgPriceIf * A50IndexNow / HS300IndexNow,t_avgPriceA50,t_avgPriceIf,A50IndexNow,HS300IndexNow);hedgeStatusPrint = hedgeStatusPrint + buffer;
	if(isReal)SendMsg(buffer);
	HedgeHold = HedgeHoldTemp;//更新Hold持仓
	sprintf_s(buffer,_T("对冲结束\r\n=================================================\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
	if((CHiStarApp*)AfxGetApp()->m_pMainWnd){
		while(::PostMessage(((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->GetSafeHwnd(),WM_UPDATE_HEDGEHOLD,NULL,NULL) == 0){
			Sleep(100);
		}
	}
	if(!iBackTest01){
		if((CHiStarApp*)AfxGetApp()->m_pMainWnd){
			while(PostMessage(((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->GetSafeHwnd(),WM_REQACCOUNTUPDATES,NULL,0) == 0){
				Sleep(100);
			}
			while(PostMessage(((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->GetSafeHwnd(),WM_REQACCOUNTUPDATES,NULL,1) == 0){
				Sleep(100);
			}
			while(true){
				if(::PeekMessage(&m_msgCur,NULL,WM_QUIT,WM_QUIT,PM_NOREMOVE)){
					while(::PeekMessage(&m_msgCur,NULL,NULL,NULL,PM_REMOVE)){};
					while(::PostThreadMessage(::GetCurrentThreadId(),WM_QUIT,0,0) == 0){
						Sleep(100);
					}
					return;
				}
				else if(::PeekMessage(&m_msgCur,NULL,WM_REQACCOUNTUPDATES_NOTIFY,WM_REQACCOUNTUPDATES_NOTIFY,PM_REMOVE)){
					while(PostMessage(((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->GetSafeHwnd(),WM_REQACCOUNTUPDATES,NULL,0) == 0){
						Sleep(100);
					}
					break;
				}
			}
		}
		static int idSynchronize = 0;
		if((CHiStarApp*)AfxGetApp()->m_pMainWnd){
			while(PostMessage(((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->GetSafeHwnd(),WM_SYNCHRONIZE_MARKET,NULL,++idSynchronize) == 0){
				Sleep(100);
			}
		}
		while(true){
			if(::PeekMessage(&m_msgCur,NULL,WM_QUIT,WM_QUIT,PM_NOREMOVE)){
				while(::PeekMessage(&m_msgCur,NULL,NULL,NULL,PM_REMOVE)){};
				while(::PostThreadMessage(::GetCurrentThreadId(),WM_QUIT,0,0) == 0){
					Sleep(100);
				}
				return;
			}
			else if(::PeekMessage(&m_msgCur,NULL,WM_SYNCHRONIZE_NOTIFY,WM_SYNCHRONIZE_NOTIFY,PM_REMOVE)){
				if(idSynchronize == m_msgCur.lParam){
					break;
				}
			}
		}
	}
	hedgeTaskStatus = NEW_HEDGE;
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

void SelectIndex(double &A50Index,double &HS300Index){
	if(fabs(g_A50Index - g_A50IndexMSHQ) > 15.0){
		A50Index = g_A50Index;
	}
	else{
		A50Index = g_A50IndexMSHQ;
	}
	if(fabs(g_HS300Index - g_HS300IndexMSHQ) > 5.0){
		HS300Index = g_HS300Index;
	}
	else{
		HS300Index = g_HS300IndexMSHQ;
	}
}

void CalcDeviation(double &a50Bid1,double &a50Ask1,double &ifBid1,double &ifAsk1,double &A50IndexNow,double &HS300IndexNow){
	premium = (a50Bid1 + a50Ask1) / 2.0 - (ifAsk1 + ifBid1) / 2.0 * A50IndexNow / HS300IndexNow;
	premiumHigh = a50Ask1 - ifBid1 * A50IndexNow / HS300IndexNow;
	premiumLow = a50Bid1 - ifAsk1 * A50IndexNow / HS300IndexNow;
	deviation = premium - datumDiff;
	DeviationSell = premiumHigh - datumDiff;
	DeviationBuy = premiumLow - datumDiff;
}

int SendMsg(CString msg){
	CInternetSession Session(NULL,0);
	CHttpFile* HttpFile = NULL;
	CString URL;
	CString res;
	URL = "http://hedgemsg.duapp.com/index.php?msg=" + msg;
	try{
		HttpFile = (CHttpFile*)Session.OpenURL(URL,1,INTERNET_FLAG_RELOAD|INTERNET_FLAG_TRANSFER_ASCII);
	}
	catch(CInternetException*pException){
		pException->Delete();
		return -1;//读取失败,返回
	}
	if(HttpFile != NULL){
		HttpFile->ReadString(res);
	}
	HttpFile->Close();
	delete HttpFile;
	Session.Close();
	return 0;
}

int seconds(SYSTEMTIME &time){
	return 3600 * time.wHour + 60 * time.wMinute + time.wSecond;
}

void tradePermit(bool &iIfTrade,bool &iA50Trade){
	static bool iFirst = true;
	if(iFirst){
		time_09_10_10.wHour = 9;time_09_10_10.wMinute = 10;time_09_10_10.wSecond = 10;
		time_09_15_00.wHour = 9;time_09_15_00.wMinute = 15;
		time_11_29_50.wHour = 11;time_11_29_50.wMinute = 29;time_11_29_50.wSecond = 50;
		time_13_00_00.wHour = 13;time_13_00_00.wMinute = 0;
		time_15_14_50.wHour = 15;time_15_14_50.wMinute = 14;time_15_14_50.wSecond = 50;
		iFirst = false;
	}
	GetLocalTime(&systime);
	//非交易日
	if(!isTradeDay(systime.wYear,systime.wMonth,systime.wDay))
	{
		iIfTrade = false;iA50Trade = false;
	}
	else{
		if(seconds(systime) >= seconds(time_09_10_10) && seconds(systime) < seconds(time_09_15_00)){
			iIfCanTrade = false;iA50CanTrade = true;
		}
		else if(seconds(systime) >= seconds(time_09_15_00) && seconds(systime) < seconds(time_11_29_50)){
			iIfCanTrade = true;iA50CanTrade = true;
		}
		else if(seconds(systime) >= seconds(time_11_29_50) && seconds(systime) < seconds(time_13_00_00)){
			iIfCanTrade = false;iA50CanTrade = true;
		}
		else if(seconds(systime) >= seconds(time_13_00_00) && seconds(systime) < seconds(time_15_14_50)){
			iIfCanTrade = true;iA50CanTrade = true;
		}
		else{
			iIfCanTrade = false;iA50CanTrade = false;
		}
	}
}

int iBackTestTime(SYSTEMTIME & systime){
	static bool iFirst = true;
	if(iFirst){
		time_09_10_10.wHour = 9;time_09_10_10.wMinute = 10;time_09_10_10.wSecond = 10;
		time_09_15_00.wHour = 9;time_09_15_00.wMinute = 15;time_09_15_00.wSecond = 0;
		time_09_40_01.wHour = 9;time_09_40_01.wMinute = 40;time_09_40_01.wSecond = 1;
		time_11_29_50.wHour = 11;time_11_29_50.wMinute = 29;time_11_29_50.wSecond = 50;
		time_13_00_00.wHour = 13;time_13_00_00.wMinute = 0;time_13_00_00.wSecond = 0;
		time_15_14_50.wHour = 15;time_15_14_50.wMinute = 14;time_15_14_50.wSecond = 50;
		iFirst = false;
	}
	if(seconds(systime) >= seconds(time_09_15_00) && seconds(systime) <= seconds(time_15_14_50)){
		return 1;
	}
	else if(seconds(systime) > seconds(time_15_14_50)){
		return -1;
	}
	else{
		return 0;
	}
}