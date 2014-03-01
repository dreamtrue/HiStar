// E:\�½��ļ���\trunk\Hedge\HedgeLoop.cpp : ʵ���ļ�
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
	int HedgeSection;//����ʱ���ڵ�Section
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
	TRACE("�˳�Hedge�߳�\n");
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CHedgePostProcessing, CWinThread)

END_MESSAGE_MAP()


void CHiStarApp::OnHedgeLooping(UINT wParam,LONG lParam){
	int MultiPos = 1;//�ֲֳ���
	//�ݼ���һ��21���ָ��,�ָ��22(=21+1)������
	double HedgeLadder[21] = {   -200, -180, -160, -140, -120, -100, -80, -60, -40, -20, -10,  20,  40,  60,  80,  100,  120,  140,  160,  180,  200};
	int PositionAimUnit[22] = {10,    9,    8,    7,    6,    5,    4,   3,   2,   1,   0,   0,  -1,  -2,  -3,  -4,  -5,    -6,   -7,   -8,   -9,   -10};//Ĭ�ϳֲ�Ŀ�굥λ��û�г��Գ�����
	int PositionAim[22];
	//Ĭ��ӯ��Ŀ��,һ��22�����䣬�������������ޱ߽�����
	double DefaultProfitAimBuy[22],DefaultProfitAimSell[22],ProfitAimBuy[22],ProfitAimSell[22];
	double MaxProfitAim = 20.0,MinProfitAim = 20.0;//��Сӯ��Ŀ�꣬���ӯ��Ŀ�꣨���ֶ�գ�
	int netPosition = 0;//���ֲ�
	int SupposedBuyOpen = 0,SupposedSellOpen = 0;
	int SupposedSectionBuyOpen = 0,SupposedSectionSellOpen = 0;
	//����������������Ҫ�ǿ�������߽�����Ѿ����ͣ�����isSupposedBuy��isSupposedSell����Ȼ����false����ʾ�����κβ���
	bool isSupposedBuyOpen = false,isSupposedSellOpen = false;
	int CurrentSectionBuy = 0,CurrentSectionSell = 0;//��ǰ���ڵ�����,Buy��Sell�ֱ��ʾ����ۺ����ۼ���
	CString PREMIUM;	
	CalcDeviation();
	//�����ã���һ�������DeviationSell��DeviationBuy
	DeviationSell = rand() % 60 - 30;
	DeviationBuy = DeviationSell - 6.5;
	TRACE("��ǰ��ƫ��%f\n",DeviationBuy);
	TRACE("��ǰ��ƫ��%f\n",DeviationSell);
	/////////////////////////////////////////////////
	DeviationSell_save = DeviationSell;
	DeviationBuy_save = DeviationBuy;	

	if(isHedgeLoopingPause){//��ͣ
	TRACE(_T("��ͣ\n"));
	return;
	}
	if(_isnan(datumDiff) != 0 || _isnan(premium)!=0 ||_isnan(deviation)!=0){
	return;//�жϷ���ֵ����
	}
	if(g_a50Bid1 < 1 || g_a50Ask1 < 1 || g_ifAsk1 < 1 || g_ifBid1 < 1 || g_A50Index < 1 || g_HS300Index < 1){
	return;
	}
	if(fabs(premium) > 300 || fabs(premium) < 0.01){
	return;//�ų�����ʱ�п��ܱ��۲�ȫ���µĴ�����ۼ���
	}

	//ͳ�ƾ��ֲ�
	for(int i = 0;i < HedgeHold.size();i++){
		netPosition = netPosition + HedgeHold[i].HedgeNum;
	}
	TRACE("���ֲ�%d\n",netPosition);
	for(int i = 1;i < 21;i++){
		if(HedgeLadder[i - 1] > HedgeLadder[i]){
			TRACE("������ݼ�\n");
			return;
		}
		if(PositionAimUnit[i - 1] < PositionAimUnit[i]){
			TRACE("�����Ŀ��ֲ�\n");
			return;
		}
	}
	if(MaxProfitAim < MinProfitAim){
		TRACE("MaxProfitAimС��MinProfitAim\n");
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
	TRACE("��ǰ�������%d,��%f,��%f\n",CurrentSectionBuy,HedgeLadder[CurrentSectionBuy - 1],HedgeLadder[CurrentSectionBuy]);
	TRACE("��ǰ��������%d,��%f,��%f\n",CurrentSectionSell,HedgeLadder[CurrentSectionSell - 1],HedgeLadder[CurrentSectionSell]);
	for(int i= 0;i < 22;i++){
		PositionAim[i] = PositionAimUnit[i] * MultiPos;
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
		//TRACE("����%d��Ŀ��ӯ��%f,���ӯ��%f,��Сӯ��%f\n",i,ProfitAimBuy[i],MaxProfitAim,MinProfitAim);
		//TRACE("����%d��Ŀ��ӯ��%f,���ӯ��%f,��Сӯ��%f\n",i,ProfitAimSell[i],MaxProfitAim,MinProfitAim);
	}
	for(int i = 0;i <= 21;i++){
		if(min(netPosition,0) > PositionAim[i]){
			SupposedSellOpen = min(netPosition,0) - PositionAim[i];
			SupposedSectionSellOpen = i;
			isSupposedSellOpen = true;
			TRACE("��������%d��������%d,��%f,��%f\n",SupposedSellOpen,i,HedgeLadder[i - 1],HedgeLadder[i]);
			break;
		}
	}
	for(int i = 21;i >= 0;i--){
		if(max(netPosition,0) < PositionAim[i]){
			SupposedBuyOpen = -(max(netPosition,0) - PositionAim[i]);
			SupposedSectionBuyOpen = i;
			isSupposedBuyOpen = true;
			TRACE("������%d��������%d,��%f,��%f\n",SupposedBuyOpen,i,HedgeLadder[i - 1],HedgeLadder[i]);
			break;
		}
	}
	//���ֲ���
	if(isSupposedBuyOpen){
		if(CurrentSectionSell <= SupposedSectionBuyOpen){
			//��Ҫ����
			//Open(SupposedBuy,BUY);
			HoldDetail newhold;
			newhold.HedgeNum = SupposedBuyOpen;
			newhold.HedgePrice = DeviationSell;
			newhold.HedgeSection = CurrentSectionSell;
			TRACE("��Ҫ����%d��,�۸�%f,��������%d,��%f,��%f\n",newhold.HedgeNum,newhold.HedgePrice,newhold.HedgeSection,HedgeLadder[newhold.HedgeSection - 1],HedgeLadder[newhold.HedgeSection]);
			HedgeHold.push_back(newhold);
		}
	}
	if(isSupposedSellOpen){
		if(CurrentSectionBuy >= SupposedSectionSellOpen){
			//��Ҫ����
			//Open(SupposedSell,SELL);
			HoldDetail newhold;
			newhold.HedgeNum = -SupposedSellOpen;
			newhold.HedgePrice = DeviationBuy;
			newhold.HedgeSection = CurrentSectionBuy;
			TRACE("��Ҫ����%d��,�۸�%f,��������%d,��%f,��%f\n",newhold.HedgeNum,newhold.HedgePrice,newhold.HedgeSection,HedgeLadder[newhold.HedgeSection - 1],HedgeLadder[newhold.HedgeSection]);
			HedgeHold.push_back(newhold);
		}
	}
	//ƽ�ֲ���
	for(int i = 0;i < HedgeHold.size();i++){
		if(HedgeHold[i].HedgeNum > 0){//��ͷ�ֲ�
			if(HedgeHold[i].HedgeSection == 21){
				//��Ҫƽ��
				//Close(HedgeHold[i].HedgeNum,SELL);
				if(DeviationBuy >= HedgeLadder[20] + ProfitAimBuy[HedgeHold[i].HedgeSection]){
					//��Ҫƽ��
					//Close(HedgeHold[i].HedgeNum,SELL);
					TRACE("��Ҫƽ��%d��,�۸�%f\n",-HedgeHold[i].HedgeNum,DeviationBuy);
					HedgeHold.erase(HedgeHold.begin() + i);
					i--;
				}
			}
			else{
				if(DeviationBuy >= HedgeLadder[HedgeHold[i].HedgeSection] + ProfitAimBuy[HedgeHold[i].HedgeSection]){
					//��Ҫƽ��
					//Close(HedgeHold[i].HedgeNum,SELL);
					TRACE("��Ҫƽ��%d��,�۸�%f\n",-HedgeHold[i].HedgeNum,DeviationBuy);
					HedgeHold.erase(HedgeHold.begin() + i);
					i--;
				}
			}
		}
		else if(HedgeHold[i].HedgeNum < 0){//��ͷ�ֲ�
			if(HedgeHold[i].HedgeSection == 0){
				if(DeviationSell <= HedgeLadder[0] - ProfitAimSell[HedgeHold[i].HedgeSection]){
					//Close(HedgeHold[i].HedgeNum,BUY);
					TRACE("��Ҫƽ��%d��,�۸�%f\n",-HedgeHold[i].HedgeNum,DeviationSell);
					HedgeHold.erase(HedgeHold.begin() + i);
					i--;
				}
			}
			else{
				if(DeviationSell <= HedgeLadder[HedgeHold[i].HedgeSection - 1] - ProfitAimSell[HedgeHold[i].HedgeSection]){
					//Close(HedgeHold[i].HedgeNum,BUY);
					TRACE("��Ҫƽ��%d��,�۸�%f\n",-HedgeHold[i].HedgeNum,DeviationSell);
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