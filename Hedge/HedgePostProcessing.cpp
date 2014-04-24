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
extern DWORD MainThreadId;
double datumDiff = 0.0;
double pointValueA50 = 1.0;double pointValueIf = 300.0;//��Լÿ����ļ�ֵ
int MultiA50 = 16;//A50����
double MarginA50 = 416.0,MarginIf = 0.15;
double USDtoRMB = 6.07;//����
bool isHedgeLoopingPause = true;
double premium = 0,premiumHigh = 0,premiumLow = 0;
double deviation = 0,DeviationSell = 0,DeviationBuy = 0;
extern double g_a50Bid1,g_a50Ask1;
extern double g_ifAsk1,g_ifBid1;
extern double g_A50Index,g_HS300Index;
extern double g_A50IndexMSHQ,g_HS300IndexMSHQ;
double A50Index = 0.0,HS300Index = 0.0;//������ʵ������ָ��
CString hedgeStatusPrint;
void SelectIndex();
/////////////////////////////////Hedge����///////////////////////////////////////////////
int MultiPos = 1;//�ֲֳ���
//�ݼ���һ��21���ָ��,�ָ��22(=21+1)������
double HedgeLadder[21] = {   -200, -180, -160, -140, -120, -100, -80, -60, -40, -30, -10,  20,  40,  60,  80,  100,  120,  140,  160,  180,  200};
int PositionAimUnit[22] = {11,    10,    9,    8,    7,    6,    5,   4,   3,   2,   1,   0,  -1,  -2,  -3,  -4,  -5,    -6,   -7,   -8,   -9,   -10};//Ĭ�ϳֲ�Ŀ�굥λ��û�г��Գ�����
int PositionAim[22];
double MaxProfitAim = 20.0,MinProfitAim = 20.0;//��Сӯ��Ŀ�꣬���ӯ��Ŀ�꣨���ֶ�գ�
///////////////////////////////////////////////////////
int netPositionA50 = 0,longIf = 0,shortIf = 0;//���ֲ�,��Ҫ����;IF�ֿ�ͷ���ֲֺͶ�ͷ���ֲ�
double AvailIB = 0.0,AvailCtp = 0.0;//�����ʽ���Ҫ����
////////////////////////////////////////////////////////
char buffer[1000];
CVector<HoldDetail> HedgeHoldTemp;//��ʱʹ�õģ������������Ԥ���㣬���ͬ����HedgeHold
CVector<HoldDetail> HedgeHold;
long maxIdHold = 0;//���ֲ�id
struct A50Task{
	int volumeRecord;char direction;//'l'��ʾ����,'s'��ʾ�̲�
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
//�Գ彻�׳�ʼ״̬
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
	TRACE(_T("�˳�Hedge�߳�\n"));
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CHedgePostProcessing, CWinThread)
	ON_THREAD_MESSAGE(WM_PREPARE_POST_PROCESSING,PostProcessing)
END_MESSAGE_MAP()

void CHiStarApp::OnHedgeLooping(WPARAM wParam,LPARAM lParam){
	static bool iFirst = true;static HWND hEdit;
	if(((CMainDlg*)m_pMainWnd)){
		((CMainDlg*)m_pMainWnd)->OnRefreshMdPane(NULL,NULL);
	}
	if(iFirst){
		hEdit = ::GetDlgItem(((CMainDlg*)m_pMainWnd)->m_basicPage.m_hWnd,IDC_RICHEDIT_STATUS);
		iFirst = false;
	}
	//��ȡϵͳʱ��
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	if(isReal){
		//�ǽ����շ���
		if(!isTradeDay(sys.wYear,sys.wMonth,sys.wDay))
		{
			return;
		}
		//�ǳ��潻��ʱ��
		if((sys.wHour == 9 && sys.wMinute < 15) || 
			(sys.wHour == 15 && sys.wMinute > 15) ||
			(sys.wHour == 11 && sys.wMinute > 30) ||
			(sys.wHour == 12)||
			(sys.wHour < 9) || (sys.wHour > 15)){
				return;
		}
	}
	//�����ֲ�id
	for(unsigned int i = 0;i < HedgeHold.size();i++){
		maxIdHold = max(maxIdHold,HedgeHold[i].id);
	}
	SelectIndex();
	if(hedgeTaskStatus == NEW_HEDGE){
		HedgeHoldTemp = HedgeHold;
		for(unsigned int i = 0;i < HedgeHoldTemp.size();i++){
			HedgeHoldTemp[i].adjustedCost = HedgeHoldTemp[i].originalCost - datumDiff;
		}
		for(int i= 0;i < 22;i++){
			PositionAim[i] = PositionAimUnit[i] * MultiPos;
		}
		//Ĭ��ӯ��Ŀ��,һ��22�����䣬�������������ޱ߽�����
		double DefaultProfitAimBuy[22],DefaultProfitAimSell[22],ProfitAimBuy[22],ProfitAimSell[22];
		int netPosition = 0;//���ֲ�
		int SupposedBuyOpen = 0,SupposedSellOpen = 0;
		int SupposedSectionBuyOpen = 0,SupposedSectionSellOpen = 0;
		//����������������Ҫ�ǿ�������߽�����Ѿ����ͣ���ʱisSupposedBuy��isSupposedSell����Ȼ����false����ʾ�����κβ���
		bool isSupposedBuyOpen = false,isSupposedSellOpen = false;
		int CurrentSectionBuy = 0,CurrentSectionSell = 0;//��ǰ���ڵ�����,Buy��Sell�ֱ��ʾ����ۺ����ۼ���
		CalcDeviation();
		if(isHedgeLoopingPause){//��ͣ
			return;
		}
		if(_isnan(datumDiff) != 0 || _isnan(premium)!=0 ||_isnan(deviation)!=0){
			return;//�жϷ���ֵ����
		}
		if(g_a50Bid1 < 1 || g_a50Ask1 < 1 || g_ifAsk1 < 1 || g_ifBid1 < 1 || A50Index < 1 || HS300Index < 1){
			return;
		}
		if(fabs(premium) > 700 || fabs(premium) < 0.01){
			return;//�ų�����ʱ�п��ܱ��۲�ȫ���µĴ�����ۼ���
		}
		//ͳ�ƾ��ֲ�
		for(unsigned int i = 0;i < HedgeHoldTemp.size();i++){
			netPosition = netPosition + HedgeHoldTemp[i].HedgeNum;
		}
		for(int i = 1;i < 21;i++){
			if(HedgeLadder[i - 1] > HedgeLadder[i]){
				sprintf(buffer,_T("������ݼ�\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
				return;
			}
			if(PositionAimUnit[i - 1] < PositionAimUnit[i]){
				sprintf(buffer,_T("�����Ŀ��ֲ�\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
				return;
			}
		}
		if(MaxProfitAim < MinProfitAim){
			sprintf(buffer,_T("MaxProfitAimС��MinProfitAim\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
			return;
		}
		//���㵱ǰ����
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
				DefaultProfitAimBuy[i] = 20.0;//�߽��
			}
			if(i > 1){
				DefaultProfitAimSell[i] = HedgeLadder[i - 1] - HedgeLadder[i - 2];
			}
			else{
				DefaultProfitAimSell[i] = 20.0;//�߽��
			}
			ProfitAimBuy[i] = max(DefaultProfitAimBuy[i],MinProfitAim);
			ProfitAimBuy[i] = min(ProfitAimBuy[i],MaxProfitAim);
			ProfitAimSell[i] = max(DefaultProfitAimSell[i],MinProfitAim);
			ProfitAimSell[i] = min(ProfitAimSell[i],MaxProfitAim);
		}
		//ÿ��ѭ��ֻ����һ�ο��ֻ���ƽ�ֲ���,��ɺ�return,������һ��ѭ��;
		//��ô����Ϊ�ֲֺ��ʽ�ֻ����ÿ��ѭ���Ŀ�ʼ�ż��㣬��;������,
		//�������һ�������󷵻����¼���ֲֺ��ʽ���ܽ�����һ��������
		//ƽ�ֲ���
		for(unsigned int i = 0;i < HedgeHoldTemp.size();i++){
			if(HedgeHoldTemp[i].HedgeNum > 0){//��ͷ�ֲ�
				if(HedgeHoldTemp[i].HedgeSection == 21){
					//��Ҫƽ��
					if(DeviationBuy >= HedgeLadder[20] + ProfitAimBuy[HedgeHoldTemp[i].HedgeSection] 
					&& DeviationBuy >= HedgeHoldTemp[i].adjustedCost + ProfitAimBuy[HedgeHoldTemp[i].HedgeSection]){
						//��Ҫƽ��
						sprintf(buffer,_T("��Ҫƽ��%d��,��Լ۸�%.02lf,ӯ��%.02lf\r\n"),-HedgeHoldTemp[i].HedgeNum,DeviationBuy,DeviationBuy - HedgeHoldTemp[i].adjustedCost);
						hedgeStatusPrint = hedgeStatusPrint + buffer;
						ReqHedgeOrder(&HedgeHoldTemp[i],CLOSE);
						HedgeHoldTemp.erase(HedgeHoldTemp.begin() + i);
						i--;
						sprintf(buffer,_T("======================END01======================\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
						return;
					}
				}
				else{
					if(DeviationBuy >= HedgeLadder[HedgeHoldTemp[i].HedgeSection] + ProfitAimBuy[HedgeHoldTemp[i].HedgeSection]
					&& DeviationBuy >= HedgeHoldTemp[i].adjustedCost + ProfitAimBuy[HedgeHoldTemp[i].HedgeSection]){
						//��Ҫƽ��
						sprintf(buffer,_T("��Ҫƽ��%d��,��Լ۸�%.02lf,ӯ��%.02lf\r\n"),-HedgeHoldTemp[i].HedgeNum,DeviationBuy,DeviationBuy - HedgeHoldTemp[i].adjustedCost);
						hedgeStatusPrint = hedgeStatusPrint + buffer;
						ReqHedgeOrder(&HedgeHoldTemp[i],CLOSE);
						HedgeHoldTemp.erase(HedgeHoldTemp.begin() + i);
						i--;
						sprintf(buffer,_T("======================END02======================\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
						return;
					}
				}
			}
			else if(HedgeHoldTemp[i].HedgeNum < 0){//��ͷ�ֲ�
				if(HedgeHoldTemp[i].HedgeSection == 0){
					if(DeviationSell <= HedgeLadder[0] - ProfitAimSell[HedgeHoldTemp[i].HedgeSection]
					&& DeviationSell <= HedgeHoldTemp[i].adjustedCost - ProfitAimSell[HedgeHoldTemp[i].HedgeSection]){
						sprintf(buffer,_T("��Ҫƽ��%d��,��Լ۸�%.02lf,ӯ��%.02lf\r\n"),-HedgeHoldTemp[i].HedgeNum,DeviationSell,HedgeHoldTemp[i].adjustedCost - DeviationSell);
						hedgeStatusPrint = hedgeStatusPrint + buffer;
						ReqHedgeOrder(&HedgeHoldTemp[i],CLOSE);
						HedgeHoldTemp.erase(HedgeHoldTemp.begin() + i);
						i--;
						sprintf(buffer,_T("======================END03======================\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
						return;
					}
				}
				else{
					if(DeviationSell <= HedgeLadder[HedgeHoldTemp[i].HedgeSection - 1] - ProfitAimSell[HedgeHoldTemp[i].HedgeSection]
					&& DeviationSell <= HedgeHoldTemp[i].adjustedCost - ProfitAimSell[HedgeHoldTemp[i].HedgeSection]){
						sprintf(buffer,_T("��Ҫƽ��%d��,��Լ۸�%.02lf,ӯ��%.02lf\r\n"),-HedgeHoldTemp[i].HedgeNum,DeviationSell,HedgeHoldTemp[i].adjustedCost - DeviationSell);
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
			if(PositionAim[i] < 0){
				if(-abs(netPosition) > PositionAim[i]){
					SupposedSellOpen = -abs(netPosition) - PositionAim[i];
					SupposedSectionSellOpen = i;
					isSupposedSellOpen = true;
					break;
				}
			}
		}
		for(int i = 21;i >= 0;i--){
			if(PositionAim[i] > 0){
				if(abs(netPosition) < PositionAim[i]){
					SupposedBuyOpen = -(abs(netPosition) - PositionAim[i]);
					SupposedSectionBuyOpen = i;
					isSupposedBuyOpen = true;
					break;
				}
			}
		}
		//���ֲ���
		if(isSupposedBuyOpen){
			if(CurrentSectionSell <= SupposedSectionBuyOpen){
				//��Ҫ����
				HoldDetail newhold;
				newhold.HedgeNum = SupposedBuyOpen;
				newhold.adjustedCost = DeviationSell;
				newhold.originalCost = DeviationSell + datumDiff;
				newhold.HedgeSection = SupposedSectionBuyOpen;//CurrentSectionSell;
				sprintf(buffer,_T("��Ҫ����%d��,��Լ۸�%.02lf,��������%d,��%.02lf,��%.02lf\r\n"),newhold.HedgeNum,newhold.adjustedCost,newhold.HedgeSection,HedgeLadder[newhold.HedgeSection - 1],HedgeLadder[newhold.HedgeSection]);
				hedgeStatusPrint = hedgeStatusPrint + buffer;
				newhold.id = ++maxIdHold;
				HedgeHoldTemp.push_back(newhold);
				ReqHedgeOrder(&newhold,OPEN);
				sprintf(buffer,_T("======================END05======================\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
				return;
			}
		}
		if(isSupposedSellOpen){
			if(CurrentSectionBuy >= SupposedSectionSellOpen){
				//��Ҫ����
				HoldDetail newhold;
				newhold.HedgeNum = -SupposedSellOpen;
				newhold.adjustedCost = DeviationBuy;
				newhold.originalCost = DeviationBuy + datumDiff;
				newhold.HedgeSection = SupposedSectionSellOpen;//CurrentSectionBuy;
				sprintf(buffer,_T("��Ҫ����%d��,��Լ۸�%.02lf,��������%d,��%.02lf,��%.02lf\r\n"),newhold.HedgeNum,newhold.adjustedCost,newhold.HedgeSection,HedgeLadder[newhold.HedgeSection - 1],HedgeLadder[newhold.HedgeSection]);
				hedgeStatusPrint = hedgeStatusPrint + buffer;
				newhold.id = ++maxIdHold;
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
	//IFԤ��Ҫ��ȡ�Ĳ�����������������ƽ
	int PredictBuyOpenIF = 0,PredictBuyCloseIF = 0,PredictSellOpenIF = 0,PredictSellCloseIF = 0;	
	if(OffsetFlag == OPEN){
		//����
		PredictPositionA50 = netPositionA50 + pHD->HedgeNum * MultiA50;
		//ע��IF��ʵ�ʷ�����HedgeNum�෴;IF��ѭ��ƽ���ٿ��ֵ�ԭ��
		if(pHD->HedgeNum > 0){
			NeedBuyA50 = pHD->HedgeNum * MultiA50;
			if(longIf - pHD->HedgeNum >= 0){
				//ȫ����ƽ
				PredictlongIf = longIf - pHD->HedgeNum;
				PredictshortIf = shortIf;
				NeedSellCloseIf = pHD->HedgeNum;
			}
			else{
				//����ƽ,������
				PredictlongIf = 0;
				NeedSellCloseIf = longIf;
				PredictshortIf = shortIf + (-(longIf - pHD->HedgeNum));
				NeedSellOpenIf = -(longIf - pHD->HedgeNum);
			}
		}
		else{
			NeedSellA50 = -pHD->HedgeNum * MultiA50;
			if(shortIf + pHD->HedgeNum >= 0){
				//ȫ����ƽ
				PredictshortIf = shortIf + pHD->HedgeNum;
				PredictlongIf = longIf;
				NeedBuyCloseIf = -pHD->HedgeNum;
			}
			else{
				//����ƽ,����
				PredictshortIf = 0;
				NeedBuyCloseIf = shortIf;
				PredictlongIf = longIf + (-(shortIf + pHD->HedgeNum));
				NeedBuyOpenIf = -(shortIf + pHD->HedgeNum);
			}
		}
	}
	else{
		//ƽ��
		PredictPositionA50 = netPositionA50 - pHD->HedgeNum * MultiA50;
		//ƽ��ʱ,IF��ʵ�ʷ�����HedgeNumһ��;IF��ѭ��ƽ���ٿ��ֵ�ԭ��
		if(pHD->HedgeNum > 0){
			NeedSellA50 = pHD->HedgeNum * MultiA50;
			if(shortIf - pHD->HedgeNum >= 0){
				//ȫ����ƽ
				PredictshortIf = shortIf - pHD->HedgeNum;
				PredictlongIf = longIf;
				NeedBuyCloseIf = pHD->HedgeNum;
			}
			else{
				//����ƽ,����
				PredictshortIf = 0;
				NeedBuyCloseIf = shortIf;
				PredictlongIf = longIf + (-(shortIf - pHD->HedgeNum));
				NeedBuyOpenIf = -(shortIf - pHD->HedgeNum);
			}
		}
		else{
			NeedBuyA50 = -pHD->HedgeNum * MultiA50;
			if(longIf + pHD->HedgeNum >= 0){
				//ȫ����ƽ
				PredictlongIf = longIf + pHD->HedgeNum;
				PredictshortIf = shortIf;
				NeedSellCloseIf = -pHD->HedgeNum;
			}
			else{
				//����ƽ,������
				PredictlongIf = 0;
				NeedSellCloseIf = longIf;
				PredictshortIf = shortIf + (-(longIf + pHD->HedgeNum));
				NeedSellOpenIf = -(longIf + pHD->HedgeNum);
			}
		}
	}
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	sprintf(buffer,_T("%2d:%2d:%2d:%3d\r\n"),sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds);hedgeStatusPrint = hedgeStatusPrint + buffer;
	sprintf(buffer,_T("����:\r\n��A50--%d��\r\n,��A50--%d��\r\n,��IF---%d��\r\n,��ƽIF---%d��\r\n,����IF---%d��\r\n,��ƽIF---%d��\r\n"),NeedBuyA50,NeedSellA50,NeedBuyOpenIf,NeedBuyCloseIf,NeedSellOpenIf,NeedSellCloseIf);
	hedgeStatusPrint = hedgeStatusPrint + buffer;
	sprintf(buffer,"A50ָ��%.02lf,HS300ָ��%.02lf, %.02lf, %.02lf, %.02lf, %.02lf\r\n",A50Index,HS300Index,g_ifBid1,g_ifAsk1,g_a50Bid1,g_a50Ask1);hedgeStatusPrint = hedgeStatusPrint + buffer;
	sprintf(buffer,"premiumHigh%.02lf,premiumLow%.02lf\r\n",g_a50Ask1 - g_ifBid1 * A50Index / HS300Index,g_a50Bid1 - g_ifAsk1 * A50Index / HS300Index);hedgeStatusPrint = hedgeStatusPrint + buffer;
	////////////////////////////////////////////////////////////////////////////////////
	//IB��ʾ��Ҫ�µı�֤��
	if(abs(PredictPositionA50) > abs(netPositionA50)){
		NeedNewMarginA50 = pointValueA50 * MarginA50 * (abs(PredictPositionA50) - abs(netPositionA50));
		if(NeedNewMarginA50 - AvailIB > 1000.0){
			sprintf(buffer,_T("IB��֤����,��Ҫ%.02lf\r\n"),NeedNewMarginA50);
			hedgeStatusPrint = hedgeStatusPrint + buffer;
			sprintf(buffer,_T("======================END07======================\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
			return -1;
		}
	}
	//IF��Ҫ�µı�֤��
	if(PredictshortIf + PredictlongIf > longIf + shortIf){
		NeedNewMarginIf = (g_ifAsk1 + g_ifBid1) / 2.0 *  pointValueIf * MarginIf * (PredictshortIf + PredictlongIf - (longIf + shortIf));
		if(NeedNewMarginIf - AvailCtp > 6000.0){
			sprintf(buffer,_T("CTP��֤����,��Ҫ%.02lf\r\n"),NeedNewMarginIf);
			hedgeStatusPrint = hedgeStatusPrint + buffer;
			sprintf(buffer,_T("======================END08======================\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
			return -1;
		}
	}

	hedgeTaskStatus = WAITING_FOR_FINISHED;//��ǵȴ�״̬���ڵȴ�״̬�¶Գ�ѭ���������µ�����
	if(m_pHedgePostProcessing){
		m_pHedgePostProcessing->PostThreadMessage(WM_PREPARE_POST_PROCESSING,NULL,NULL);
	}
	////////////////////////////////////////////////////////////
	//////ǧ��ע��Ҫ������,���򽫻ᵼ�����벻���Ĵ���
	IfTask iftask;A50Task a50task;
	hedgetask.clear();/*��ֹmemset(&hedgetask,0,sizeof(hedgetask));*/
	memset(&iftask,0,sizeof(iftask));memset(&a50task,0,sizeof(a50task));
	////////////////////////////////////////////////////////////
	//IF�µ�
	TThostFtdcCombOffsetFlagType kpp;
	char szInst[MAX_PATH];
	uni2ansi(CP_ACP,((CHiStarApp*)AfxGetApp())->m_accountCtp.m_szInst,szInst);
	LPSTR* pInst = new LPSTR;
	pInst[0] = szInst;
	iftask.frontid = m_cT->m_ifrontId;iftask.sessionid = m_cT->m_isessionId;
	if(NeedBuyOpenIf > 0){
		kpp[0] = THOST_FTDC_OF_Open;
		if(((CHiStarApp*)AfxGetApp())->m_cT){
			iftask.ref = ((CHiStarApp*)AfxGetApp())->m_cT->ReqOrdLimit(*pInst,THOST_FTDC_D_Buy,kpp,g_ifAsk1 + 100.0,NeedBuyOpenIf);
			iftask.direction = THOST_FTDC_D_Buy;iftask.offset[0] = THOST_FTDC_OF_Open;iftask.priceRecord = g_ifAsk1;iftask.volumeRecord = NeedBuyOpenIf;
			hedgetask.ifalltask.push_back(iftask);
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
			iftask.ref = ((CHiStarApp*)AfxGetApp())->m_cT->ReqOrdLimit(*pInst,THOST_FTDC_D_Buy,kpp,g_ifAsk1 + 100.0,NeedBuyCloseIf);
			iftask.direction = THOST_FTDC_D_Buy;iftask.offset[0] = THOST_FTDC_OF_Close;iftask.priceRecord = g_ifAsk1;iftask.volumeRecord = NeedBuyCloseIf;
			hedgetask.ifalltask.push_back(iftask);
		}
	}
	if(NeedSellOpenIf > 0){
		kpp[0] = THOST_FTDC_OF_Open;
		if(((CHiStarApp*)AfxGetApp())->m_cT){
			iftask.ref = ((CHiStarApp*)AfxGetApp())->m_cT->ReqOrdLimit(*pInst,THOST_FTDC_D_Sell,kpp,g_ifBid1 - 100.0,NeedSellOpenIf);
			iftask.direction = THOST_FTDC_D_Sell;iftask.offset[0] = THOST_FTDC_OF_Open;iftask.priceRecord = g_ifBid1;iftask.volumeRecord = NeedSellOpenIf;
			hedgetask.ifalltask.push_back(iftask);
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
			iftask.ref = ((CHiStarApp*)AfxGetApp())->m_cT->ReqOrdLimit(*pInst,THOST_FTDC_D_Sell,kpp,g_ifBid1 - 100.0,NeedSellCloseIf);
			iftask.direction = THOST_FTDC_D_Sell;iftask.offset[0] = THOST_FTDC_OF_Close;iftask.priceRecord = g_ifBid1;iftask.volumeRecord = NeedSellCloseIf;
			hedgetask.ifalltask.push_back(iftask);
		}
	}
	delete []pInst;//��ֹ�ڴ�й¶
	//A50�µ�
	if(abs(NeedBuyA50 - NeedSellA50) != 0){
		if(((CHiStarApp*)AfxGetApp())->m_pIBClient){
			if(NeedBuyA50 - NeedSellA50 > 0){
				((CHiStarApp*)AfxGetApp())->m_IBOrder.action = "BUY";
				((CHiStarApp*)AfxGetApp())->m_IBOrder.totalQuantity = NeedBuyA50 - NeedSellA50;
				((CHiStarApp*)AfxGetApp())->m_IBOrder.lmtPrice = DealA50Price(true,g_a50Ask1 + 500.0);
				if(((CHiStarApp*)AfxGetApp())->m_IBOrder.lmtPrice < 1.0){
					sprintf(buffer,_T("A50���С��1.0,�쳣\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
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
				((CHiStarApp*)AfxGetApp())->m_IBOrder.lmtPrice = DealA50Price(false,g_a50Bid1 - 500.0);
				if(((CHiStarApp*)AfxGetApp())->m_IBOrder.lmtPrice < 1.0){
					sprintf(buffer,_T("A50���С��1.0,�쳣\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
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
	//��ʼ����
	if(m_pHedgePostProcessing){
		m_pHedgePostProcessing->PostThreadMessage(WM_BEGIN_POST_PROCESSING,NULL,pHD->id);
	}
	return 0;
}

//����
void CHedgePostProcessing::PostProcessing(WPARAM t_wParam,LPARAM t_lParam){
	HWND hEdit = ::GetDlgItem(((CMainDlg*)((CHiStarApp*)AfxGetApp()->m_pMainWnd))->m_basicPage.m_hWnd,IDC_RICHEDIT_STATUS);
	MSG msg;BOOL bRet;
	long idHedgeCurrent = -1;
	while((bRet = GetMessage(&msg,NULL,WM_BEGIN_POST_PROCESSING,WM_BEGIN_POST_PROCESSING)) != 0){
		if (!bRet){// handle the error and possibly exit
		}
		else{
			idHedgeCurrent = msg.lParam;
			break;//�Ѿ���ʼ��������ʽ���д���
		}
	}
	//����������Ϣ���ֱ���WM_RTN_INSERT��WM_RTN_ORDER
	while((bRet = GetMessage(&msg,NULL,WM_RTN_INSERT,WM_RTN_ORDER)) != 0){
		if (!bRet){
		}
		else{
			switch(msg.message)
			{
			case WM_RTN_INSERT:
				{//����覴ã������requestid������
					sprintf(buffer,"�յ�WM_RTN_INSERT\r\n");hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
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
					sprintf(buffer,"�յ�WM_RTN_ORDER\r\n");hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
					CThostFtdcOrderField *pOrderRtn = (CThostFtdcOrderField *)msg.lParam;
					for(unsigned int i = 0;i < hedgetask.ifalltask.size();i++){
						if(hedgetask.ifalltask[i].ref == atoi(pOrderRtn->OrderRef)
							&&hedgetask.ifalltask[i].frontid == pOrderRtn->FrontID
							&&hedgetask.ifalltask[i].sessionid == pOrderRtn->SessionID){
								if(pOrderRtn->OrderStatus == THOST_FTDC_OST_AllTraded || pOrderRtn->OrderStatus == THOST_FTDC_OST_Canceled 
									|| pOrderRtn->OrderStatus == THOST_FTDC_OST_NoTradeNotQueueing || pOrderRtn->OrderStatus == THOST_FTDC_OST_PartTradedNotQueueing){
										hedgetask.ifalltask[i].traded = pOrderRtn->VolumeTraded;
										hedgetask.ifalltask[i].sysid = atoi(pOrderRtn->OrderSysID);
										strcpy(hedgetask.ifalltask[i].ExchangeID,pOrderRtn->ExchangeID);
										hedgetask.ifalltask[i].bReceivedAllOrder = true;
										sprintf(buffer,"Order,ref%d,����״̬%c\r\n",atoi(pOrderRtn->OrderRef),pOrderRtn->OrderStatus);hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
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
	//����Ƿ��гɽ���,���ܶ���ȡ����û�гɽ���
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
				sprintf(buffer,"�յ�WM_RTN_TRADE\r\n");hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW
					CThostFtdcTradeField *pTradeRtn = (CThostFtdcTradeField *)msg.lParam;
				for(unsigned int i = 0;i < hedgetask.ifalltask.size();i++){
					if(hedgetask.ifalltask[i].sysid == atoi(pTradeRtn->OrderSysID) && strcmp(hedgetask.ifalltask[i].ExchangeID,pTradeRtn->ExchangeID) == 0){
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
		//����ƽ���۸�
		for(unsigned int i = 0;i < hedgetask.ifalltask.size();i++){
			hedgetask.ifalltask[i].avgPrice = hedgetask.ifalltask[i].receivedValue / hedgetask.ifalltask[i].receivedTradedVolume;
		}
	}

	while((bRet = GetMessage(&msg,NULL,WM_RTN_ORDER_IB,WM_RTN_ORDER_IB)) != 0){
		if (!bRet){
		}
		else{
			sprintf(buffer,"�յ�WM_RTN_ORDER_IB\r\n");hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
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
	//����ֲ�ͳ��
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
	sprintf(buffer,_T("HEDGE INFORMATION:%.02lf,A50:%.02lf,IF:%.02lf,A50INDEX:%.02lf,HS300INDEX:%.02lf\r\n"),t_avgPriceA50 - t_avgPriceIf * A50Index / HS300Index,t_avgPriceA50,t_avgPriceIf,A50Index,HS300Index);hedgeStatusPrint = hedgeStatusPrint + buffer;SendMsg(buffer);
	HedgeHold = HedgeHoldTemp;//����Hold�ֲ�
	hedgeTaskStatus = NEW_HEDGE;
	sprintf(buffer,_T("�Գ����\r\n=================================================\r\n"));hedgeStatusPrint = hedgeStatusPrint + buffer;SHOW;
	::PostThreadMessage(MainThreadId,WM_UPDATE_HEDGEHOLD,NULL,NULL);//�����˻�
	::PostThreadMessage(MainThreadId,WM_QRY_ACC_CTP,NULL,NULL);//�����˻�
	static int idSynchronize = 0;
	::PostThreadMessage(MainThreadId,WM_SYNCHRONIZE_MARKET,NULL,++idSynchronize);//ͬ���г�
	while((bRet = GetMessage(&msg,NULL,WM_SYNCHRONIZE_NOTIFY,WM_SYNCHRONIZE_NOTIFY)) != 0){
		if (!bRet){
		}
		else{
			if(idSynchronize == msg.lParam){
				break;
			}
		}
	}
	Sleep(3000);
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
		//TRACE("over 0.1,MS%.02lf,SN%.02lf\r\n",g_A50IndexMSHQ,g_A50Index);
		A50Index = g_A50Index;
	}
	else{
		A50Index = g_A50IndexMSHQ;
		//TRACE("MS,SN,%.02lf,%.02lf\r\n",g_A50IndexMSHQ,g_A50Index);
	}
	if(fabs(g_HS300Index - g_HS300IndexMSHQ) > 5.0){
		//TRACE("over 0.1,MS%.02lf,SN%.02lf\r\n",g_HS300Index,g_HS300Index);
		HS300Index = g_HS300Index;
	}
	else{
		HS300Index = g_HS300IndexMSHQ;
		//TRACE("MS%.02lf,SN%.02lf\r\n",g_HS300IndexMSHQ,g_HS300Index);
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
		return -1;//��ȡʧ��,����
	}
	if(HttpFile != NULL){
		HttpFile->ReadString(res);
		TRACE("%s\n",res);
	}
	HttpFile->Close();
	delete HttpFile;
	Session.Close();
	return 0;
}