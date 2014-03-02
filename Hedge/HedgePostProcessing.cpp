// E:\�½��ļ���\trunk\Hedge\HedgeLoop.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "HiStar.h"
#include "HedgePostProcessing.h"
#include "UserMsg.h"
#include "MainDlg.h"
#include <math.h>
#define OPEN true
#define CLOSE false
int MultiA50 = 12;//A50����
double MarginA50 = 625.0,MarginIF = 120000.0;
double USDtoRMB = 6.07;//����
double datumDiff = 0;
bool isHedgeLoopingPause = false;
double premium = 0,premiumHigh = 0,premiumLow = 0;
double deviation = 0,DeviationSell = 0,DeviationBuy = 0;
extern double g_a50Bid1,g_a50Ask1;
extern double g_ifAsk1,g_ifBid1;
extern double g_A50Index,g_HS300Index;
///////////////////////////////////////////////////////
int NetPositionA50 = 0,PositionIFb = 0,PositionIFs = 0;//���ֲ�,��Ҫ����;IF�ֿ�ͷ���ֲֺͶ�ͷ���ֲ�
double AvailIB = 17000.0,AvailCtp = 250000.0;//�����ʽ���Ҫ����
////////////////////////////////////////////////////////
char buffer[1000];
double DeviationSell_save,DeviationBuy_save;
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
	HWND hEdit = ::GetDlgItem(((CMainDlg*)m_pMainWnd)->m_basicPage.m_hWnd,IDC_HEDGE_OUT);
	::SendMessage(hEdit,WM_SETTEXT,0,(LPARAM)(LPCTSTR)m_HedgeStatusOut);
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
	DeviationSell = rand() % 120 - 60;
	DeviationBuy = DeviationSell - 6.5;
	sprintf(buffer,"��ǰ��ƫ��%f\r\n",DeviationBuy);m_HedgeStatusOut = m_HedgeStatusOut + buffer;
	sprintf(buffer,"��ǰ��ƫ��%f\r\n",DeviationSell);m_HedgeStatusOut = m_HedgeStatusOut + buffer;
	/////////////////////////////////////////////////
	DeviationSell_save = DeviationSell;
	DeviationBuy_save = DeviationBuy;	

	if(isHedgeLoopingPause){//��ͣ
		sprintf(buffer,_T("��ͣ\r\n"));m_HedgeStatusOut = m_HedgeStatusOut + buffer;
		return;
	}
	/*
	if(_isnan(datumDiff) != 0 || _isnan(premium)!=0 ||_isnan(deviation)!=0){
	return;//�жϷ���ֵ����
	}
	if(g_a50Bid1 < 1 || g_a50Ask1 < 1 || g_ifAsk1 < 1 || g_ifBid1 < 1 || g_A50Index < 1 || g_HS300Index < 1){
	return;
	}
	if(fabs(premium) > 300 || fabs(premium) < 0.01){
	return;//�ų�����ʱ�п��ܱ��۲�ȫ���µĴ�����ۼ���
	}
	*/
	//ͳ�ƾ��ֲ�
	for(int i = 0;i < HedgeHold.size();i++){
		netPosition = netPosition + HedgeHold[i].HedgeNum;
	}
	sprintf(buffer,"���ֲ�%d\r\n",netPosition);
	for(int i = 1;i < 21;i++){
		if(HedgeLadder[i - 1] > HedgeLadder[i]){
			sprintf(buffer,"������ݼ�\r\n");m_HedgeStatusOut = m_HedgeStatusOut + buffer;
			return;
		}
		if(PositionAimUnit[i - 1] < PositionAimUnit[i]){
			sprintf(buffer,"�����Ŀ��ֲ�\r\n");m_HedgeStatusOut = m_HedgeStatusOut + buffer;
			return;
		}
	}
	if(MaxProfitAim < MinProfitAim){
		sprintf(buffer,"MaxProfitAimС��MinProfitAim\r\n");m_HedgeStatusOut = m_HedgeStatusOut + buffer;
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
	sprintf(buffer,"��ǰ�������%d,��%f,��%f\r\n",CurrentSectionBuy,HedgeLadder[CurrentSectionBuy - 1],HedgeLadder[CurrentSectionBuy]);
	m_HedgeStatusOut = m_HedgeStatusOut + buffer;
	sprintf(buffer,"��ǰ��������%d,��%f,��%f\r\n",CurrentSectionSell,HedgeLadder[CurrentSectionSell - 1],HedgeLadder[CurrentSectionSell]);
	m_HedgeStatusOut = m_HedgeStatusOut + buffer;
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
		//sprintf(buffer,"����%d��Ŀ��ӯ��%f,���ӯ��%f,��Сӯ��%f\r\n",i,ProfitAimBuy[i],MaxProfitAim,MinProfitAim);
		//sprintf(buffer,"����%d��Ŀ��ӯ��%f,���ӯ��%f,��Сӯ��%f\r\n",i,ProfitAimSell[i],MaxProfitAim,MinProfitAim);
	}
	//ÿ��ѭ��ֻ����һ�ο��ֻ���ƽ�ֲ���,��ɺ�return,������һ��ѭ��;
	//��ô����Ϊ�ֲֺ��ʽ�ֻ����ÿ��ѭ���Ŀ�ʼ�ż��㣬��;������,
	//�������һ�������󷵻����¼���ֲֺ��ʽ���ܽ�����һ��������
	//ƽ�ֲ���
	for(int i = 0;i < HedgeHold.size();i++){
		if(HedgeHold[i].HedgeNum > 0){//��ͷ�ֲ�
			if(HedgeHold[i].HedgeSection == 21){
				//��Ҫƽ��
				if(DeviationBuy >= HedgeLadder[20] + ProfitAimBuy[HedgeHold[i].HedgeSection]){
					//��Ҫƽ��
					sprintf(buffer,"��Ҫƽ��%d��,�۸�%f,ӯ��%f\r\n",-HedgeHold[i].HedgeNum,DeviationBuy,DeviationBuy - HedgeHold[i].HedgePrice);
					m_HedgeStatusOut = m_HedgeStatusOut + buffer;
					ReqHedgeOrder(&HedgeHold[i],CLOSE);
					HedgeHold.erase(HedgeHold.begin() + i);
					i--;
					sprintf(buffer,"-------------------------------------------END--------------------------------------------\r\n");
					return;
				}
			}
			else{
				if(DeviationBuy >= HedgeLadder[HedgeHold[i].HedgeSection] + ProfitAimBuy[HedgeHold[i].HedgeSection]){
					//��Ҫƽ��
					sprintf(buffer,"��Ҫƽ��%d��,�۸�%f,ӯ��%f\r\n",-HedgeHold[i].HedgeNum,DeviationBuy,DeviationBuy - HedgeHold[i].HedgePrice);
					m_HedgeStatusOut = m_HedgeStatusOut + buffer;
					ReqHedgeOrder(&HedgeHold[i],CLOSE);
					HedgeHold.erase(HedgeHold.begin() + i);
					i--;
					sprintf(buffer,"-------------------------------------------END--------------------------------------------\r\n");
					m_HedgeStatusOut = m_HedgeStatusOut + buffer;
					return;
				}
			}
		}
		else if(HedgeHold[i].HedgeNum < 0){//��ͷ�ֲ�
			if(HedgeHold[i].HedgeSection == 0){
				if(DeviationSell <= HedgeLadder[0] - ProfitAimSell[HedgeHold[i].HedgeSection]){
					sprintf(buffer,"��Ҫƽ��%d��,�۸�%f,ӯ��%f\r\n",-HedgeHold[i].HedgeNum,DeviationSell,HedgeHold[i].HedgePrice - DeviationSell);
					m_HedgeStatusOut = m_HedgeStatusOut + buffer;
					ReqHedgeOrder(&HedgeHold[i],CLOSE);
					HedgeHold.erase(HedgeHold.begin() + i);
					i--;
					sprintf(buffer,"-------------------------------------------END--------------------------------------------\r\n");
					m_HedgeStatusOut = m_HedgeStatusOut + buffer;
					return;
				}
			}
			else{
				if(DeviationSell <= HedgeLadder[HedgeHold[i].HedgeSection - 1] - ProfitAimSell[HedgeHold[i].HedgeSection]){
					sprintf(buffer,"��Ҫƽ��%d��,�۸�%f,ӯ��%f\r\n",-HedgeHold[i].HedgeNum,DeviationSell,HedgeHold[i].HedgePrice - DeviationSell);
					m_HedgeStatusOut = m_HedgeStatusOut + buffer;
					ReqHedgeOrder(&HedgeHold[i],CLOSE);
					HedgeHold.erase(HedgeHold.begin() + i);
					i--;
					sprintf(buffer,"-------------------------------------------END--------------------------------------------\r\n");
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
			sprintf(buffer,"��������%d��������%d,��%f,��%f\r\n",SupposedSellOpen,i,HedgeLadder[i - 1],HedgeLadder[i]);
			m_HedgeStatusOut = m_HedgeStatusOut + buffer;
			break;
		}
	}
	for(int i = 21;i >= 0;i--){
		if(max(netPosition,0) < PositionAim[i]){
			SupposedBuyOpen = -(max(netPosition,0) - PositionAim[i]);
			SupposedSectionBuyOpen = i;
			isSupposedBuyOpen = true;
			sprintf(buffer,"������%d��������%d,��%f,��%f\r\n",SupposedBuyOpen,i,HedgeLadder[i - 1],HedgeLadder[i]);
			m_HedgeStatusOut = m_HedgeStatusOut + buffer;
			break;
		}
	}
	//���ֲ���
	if(isSupposedBuyOpen){
		if(CurrentSectionSell <= SupposedSectionBuyOpen){
			//��Ҫ����
			HoldDetail newhold;
			newhold.HedgeNum = SupposedBuyOpen;
			newhold.HedgePrice = DeviationSell;
			newhold.HedgeSection = CurrentSectionSell;
			sprintf(buffer,"��Ҫ����%d��,�۸�%f,��������%d,��%f,��%f\r\n",newhold.HedgeNum,newhold.HedgePrice,newhold.HedgeSection,HedgeLadder[newhold.HedgeSection - 1],HedgeLadder[newhold.HedgeSection]);
			m_HedgeStatusOut = m_HedgeStatusOut + buffer;
			HedgeHold.push_back(newhold);
			ReqHedgeOrder(&newhold,OPEN);
			sprintf(buffer,"-------------------------------------------END--------------------------------------------\r\n");
			m_HedgeStatusOut = m_HedgeStatusOut + buffer;
			return;
		}
	}
	if(isSupposedSellOpen){
		if(CurrentSectionBuy >= SupposedSectionSellOpen){
			//��Ҫ����
			HoldDetail newhold;
			newhold.HedgeNum = -SupposedSellOpen;
			newhold.HedgePrice = DeviationBuy;
			newhold.HedgeSection = CurrentSectionBuy;
			sprintf(buffer,"��Ҫ����%d��,�۸�%f,��������%d,��%f,��%f\r\n",newhold.HedgeNum,newhold.HedgePrice,newhold.HedgeSection,HedgeLadder[newhold.HedgeSection - 1],HedgeLadder[newhold.HedgeSection]);
			m_HedgeStatusOut = m_HedgeStatusOut + buffer;
			HedgeHold.push_back(newhold);
			ReqHedgeOrder(&newhold,OPEN);
			sprintf(buffer,"-------------------------------------------END--------------------------------------------\r\n");
			m_HedgeStatusOut = m_HedgeStatusOut + buffer;
			return;
		}
	}
	sprintf(buffer,"-------------------------------------------END--------------------------------------------\r\n");
	m_HedgeStatusOut = m_HedgeStatusOut + buffer;
}

int CHiStarApp::ReqHedgeOrder(HoldDetail *pHD,bool OffsetFlag){
	int NeedBuyA50 = 0,NeedSellA50 = 0;
	int NeedBuyOpenIf = 0,NeedBuyCloseIf = 0,NeedSellOpenIf = 0,NeedSellCloseIf = 0;
	int PredictPositionA50 = 0;
	int PredictBuyA50 = 0,PredictSellA50 = 0;
	double NeedNewMarginA50 = 0,NeedNewMarginIF = 0;
	int PredictPositionIFb = 0,PredictPositionIFs = 0;
	//IFԤ��Ҫ��ȡ�Ĳ�����������������ƽ
	int PredictBuyOpenIF = 0,PredictBuyCloseIF = 0,PredictSellOpenIF = 0,PredictSellCloseIF = 0;	
	if(OffsetFlag == OPEN){
		//����
		PredictPositionA50 = NetPositionA50 + pHD->HedgeNum * MultiA50;
		//ע��IF��ʵ�ʷ�����HedgeNum�෴;IF��ѭ��ƽ���ٿ��ֵ�ԭ��
		if(pHD->HedgeNum > 0){
			NeedBuyA50 = pHD->HedgeNum * MultiA50;
			if(PositionIFb - pHD->HedgeNum >= 0){
				//ȫ����ƽ
				PredictPositionIFb = PositionIFb - pHD->HedgeNum;
				NeedSellCloseIf = pHD->HedgeNum;
			}
			else{
				//����ƽ,������
				PredictPositionIFb = 0;
				NeedSellCloseIf = PositionIFb;
				PredictPositionIFs = -(PositionIFb - pHD->HedgeNum);
				NeedSellOpenIf = -(PositionIFb - pHD->HedgeNum);
			}
		}
		else{
			NeedSellA50 = -pHD->HedgeNum * MultiA50;
			if(PositionIFs + pHD->HedgeNum >= 0){
				//ȫ����ƽ
				PredictPositionIFs = PositionIFs + pHD->HedgeNum;
				NeedBuyCloseIf = -pHD->HedgeNum;
			}
			else{
				//����ƽ,����
				PredictPositionIFs = 0;
				NeedBuyCloseIf = PositionIFs;
				PredictPositionIFb = -(PositionIFs + pHD->HedgeNum);
				NeedBuyOpenIf = -(PositionIFs + pHD->HedgeNum);
			}
		}
	}
	else{
		//ƽ��
		PredictPositionA50 = NetPositionA50 - pHD->HedgeNum * MultiA50;
		//ƽ��ʱ,IF��ʵ�ʷ�����HedgeNumһ��;IF��ѭ��ƽ���ٿ��ֵ�ԭ��
		if(pHD->HedgeNum > 0){
			NeedSellA50 = pHD->HedgeNum * MultiA50;
			if(PositionIFs - pHD->HedgeNum >= 0){
				//ȫ����ƽ
				PredictPositionIFs = PositionIFs - pHD->HedgeNum;
				NeedBuyCloseIf = pHD->HedgeNum;
			}
			else{
				//����ƽ,����
				PredictPositionIFs = 0;
				NeedBuyCloseIf = PositionIFs;
				PredictPositionIFb = -(PositionIFs - pHD->HedgeNum);
				NeedBuyOpenIf = -(PositionIFs - pHD->HedgeNum);
			}
		}
		else{
			NeedBuyA50 = -pHD->HedgeNum * MultiA50;
			if(PositionIFb + pHD->HedgeNum >= 0){
				//ȫ����ƽ
				PredictPositionIFb = PositionIFb + pHD->HedgeNum;
				NeedSellCloseIf = -pHD->HedgeNum;
			}
			else{
				//����ƽ,������
				PredictPositionIFb = 0;
				NeedSellCloseIf = PositionIFb;
				PredictPositionIFs = -(PositionIFb + pHD->HedgeNum);
				NeedSellOpenIf = -(PositionIFb + pHD->HedgeNum);
			}
		}
	}
	sprintf(buffer,"��Ҫ:\r\n��A50--%d��\r\n,��A50--%d��\r\n,��IF---%d��\r\n,��ƽIF---%d��\r\n,����IF---%d��\r\n,��ƽIF---%d��\r\n",NeedBuyA50,NeedSellA50,NeedBuyOpenIf,NeedBuyCloseIf,NeedSellOpenIf,NeedSellCloseIf);
	m_HedgeStatusOut = m_HedgeStatusOut + buffer;
	//���ԣ�ͳ���³ֲ�
	PositionIFb = PositionIFb - NeedSellCloseIf + NeedBuyOpenIf;
	PositionIFs = PositionIFs - NeedBuyCloseIf + NeedSellOpenIf;
	NetPositionA50 = NetPositionA50 + NeedBuyA50 - NeedSellA50;
	////////////////////////////////////////////////////////////////////////////////////
	//IB��ʾ��Ҫ�µı�֤��
	if(abs(PredictPositionA50) > abs(NetPositionA50)){
		NeedNewMarginA50 = MarginA50 * (abs(PredictPositionA50) - abs(NetPositionA50));
		if(NeedNewMarginA50 - AvailIB > 1000.0){
			sprintf(buffer,"IB��֤����,��Ҫ%f\r\n",NeedNewMarginA50);
			m_HedgeStatusOut = m_HedgeStatusOut + buffer;
			sprintf(buffer,"-------------------------------------------END--------------------------------------------\r\n");
			m_HedgeStatusOut = m_HedgeStatusOut + buffer;
			return -1;
		}
	}
	//IF��Ҫ�µı�֤��
	if(PredictPositionIFs + PredictPositionIFb > PositionIFb + PositionIFs){
		NeedNewMarginIF = MarginIF * (PredictPositionIFs + PredictPositionIFb - (PositionIFb + PositionIFs));
		if(NeedNewMarginIF - AvailCtp > 6000.0){
			sprintf(buffer,"CTP��֤����,��Ҫ%f\r\n",NeedNewMarginIF);
			m_HedgeStatusOut = m_HedgeStatusOut + buffer;
			sprintf(buffer,"-------------------------------------------END--------------------------------------------\r\n");
			m_HedgeStatusOut = m_HedgeStatusOut + buffer;
			return -1;
		}
	}
	return 0;
}







void CHedgePostProcessing::PostProcessing(UINT wParam,LONG lParam){


}