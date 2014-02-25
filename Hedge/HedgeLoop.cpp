// E:\�½��ļ���\trunk\Hedge\HedgeLoop.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "HiStar.h"
#include "HedgeLoop.h"
#include "UserMsg.h"
int netHold = 0;//������
double datumDiff = 0;
int ladder = 0;
int needHold = 0;
double profitTarget = 0;//ӯ��Ŀ��
double premium = 0,premiumHigh = 0,premiumLow = 0;
double deviation = 0,deviationHigh = 0,deviationLow = 0;
std::vector<HoldAndDirec>::iterator itHedge;
extern double g_a50Bid1,g_a50Ask1;
extern double g_ifAsk1,g_ifBid1;
extern double g_A50Index,g_HS300Index;
double deviationHigh_save,deviationLow_save;
std::vector<HoldAndDirec> holdHedge;//��ֻ����һ��A50�����̵߳������,�ݲ�����ͬ��
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
	//��ʼ��,step/multiply/aimOfLadder/datumDiff/tempIni����Ҫ�ӽ����ȡ��ʼ��ֵ
	step = 20.0;
	multiply = 12;
	aimOfLadder = -1;
	datumDiff = 0;
	tempIni.price = 0;//�ֲ���Լ۸�
	tempIni.numDirec = 0;//���з���ĳֲ�
	holdHedge.push_back(tempIni);
	netHold = tempIni.numDirec;
	return TRUE;
}

int CHedgeLoop::ExitInstance()
{
	TRACE("�˳�Hedge�߳�\n");
	holdHedge.clear();
	netHold = 0;
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CHedgeLoop, CWinThread)
	ON_THREAD_MESSAGE(WM_MD_REFRESH,BeginHedge)
END_MESSAGE_MAP()


// CHedgeLoop ��Ϣ�������
void CHedgeLoop::BeginHedge(UINT wParam,LONG lParam){

	CString PREMIUM;	
	CalDeviation();
	deviationHigh_save = deviationHigh;
	deviationLow_save = deviationLow;	

	if(m_isPause){//��ͣ
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
	needHold = aimOfLadder * ladder;//��Ҫ�ĺ�Լ��ƫ��Ĺ�ϵ,���Ը����ʽ��������޸�
	profitTarget = step;

	double profitUnit = 0;
	int needBuyClose = 0,needSellClose = 0;//ʼ�մ���0
	double totalTradingValue = 0;//׼�����׵��ܼ�ֵ
	for(unsigned int j = 0;j < holdHedge.size();j++){
		itHedge = holdHedge.begin() + j;
		//if(itHedge->numDirec < 0){//��ʵֻҪnetholdΪ�����ͱ�֤�����е�numDirecΪ���������ڼ���numDirecΪ��������Ϊ�������
		if(netHold < 0){
			profitUnit = itHedge->price - deviationHigh;
			if(profitUnit >= profitTarget){
				needBuyClose = needBuyClose - itHedge->numDirec;
				totalTradingValue = totalTradingValue + itHedge->price * (-itHedge->numDirec);
				//���¼�¼
				netHold = netHold - itHedge->numDirec;
				holdHedge.erase(itHedge);
				j--;//����ɾ������ʱ,��Ҫ��ָ����ǰ����
			}
		}
		//else if(itHedge->numDirec > 0)
		else if(netHold > 0){
			profitUnit = -(itHedge->price - deviationLow);
			if(profitUnit >= profitTarget){
				needSellClose = needSellClose + itHedge->numDirec;
				totalTradingValue = totalTradingValue + itHedge->price * itHedge->numDirec;
				//���¼�¼
				netHold = netHold - itHedge->numDirec;
				holdHedge.erase(itHedge);
				j--;
			}
		}
		else{
			//����0ʱʲôҲ����
		}
	}
	//ƽ�ֲ���,,��ʵ��ƽ����ƽһ��ֻ��һ���õ�ִ�еģ���������������
	CloseOrOpen(needBuyClose,true,true);
	CloseOrOpen(needSellClose,false,true);
	//���ֲ���
	BS(needHold,netHold);
}

int CloseOrOpen(int num,bool isbuy,bool isflat){
	return 0;
}

int BS(int needHold_temp,int netHold_temp){
	return 0;
}