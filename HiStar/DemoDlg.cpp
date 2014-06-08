// DemoDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "HiStar.h"
#include "DemoDlg.h"
#include "afxdialogex.h"
#include "me.h"
#include <algorithm>
CVector<HoldDetail> HedgeHoldDemo;
extern int iBackTestTime(SYSTEMTIME & systime);
extern sqldb m_db;
//const double HedgeLadderDemoRef[21] = {   -200, -180, -160, -140, -120, -100, -80, -60, -40, -30, -10,  20,  40,  60,  80,  100,  120,  140,  160,  180,  200};
//const int PositionAimUnitDemo[22] = {11,    10,    9,    8,    7,    6,    5,   4,   3,   2,   1,   0,  -1,  -2,  -3,  -4,  -5,    -6,   -7,   -8,   -9,   -10};
const double HedgeLadderDemoRef[21] = {   -95, -85, -75, -65, -55, -45, -35, -25, -15, -5,  0,  5,   15,  25,  35,  45,  55,  65,   75,   85,   95};
const int PositionAimUnitDemo[22] = { 10,  9,   8,   7,   6,   5,    4,  3,   2,   1,  0,  0,  -1,  -2,  -3,  -4,  -5,  -6,  -7,   -8,   -9,  -10};
double HedgeLadderDemo[21];
extern void CalcDeviation(double &a50Bid1,double &a50Ask1,double &ifBid1,double &ifAsk1,double &A50IndexNow,double &HS300IndexNow);
bool CmpByTimeDemo(const CString first,const CString second) 
{    
	if(strcmp(first,second) <= 0){
		return true;
	}
	else{
		return false;
	}
}
// CDemoDlg �Ի���

IMPLEMENT_DYNAMIC(CDemoDlg, CDialogEx)

	CDemoDlg::CDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDemoDlg::IDD, pParent),m_pHiStarApp(NULL),conndemo(NULL),res_set(NULL),row(NULL)
	, m_tableName(_T("")),A50IndexDemo(0.0),a50Bid1Demo(0.0),a50Ask1Demo(0.0),HS300IndexDemo(0.0)
	,ifBid1Demo(0.0),ifAsk1Demo(0.0)
	, maxIdHoldDemo(0)
	, datumDiffDemo(0)
	, MultiPosDemo(1)
	, premiumDemo(0)
	, premiumHighDemo(0)
	, premiumLowDemo(0)
	, deviationDemo(0)
	, DeviationSellDemo(0)
	, DeviationBuyDemo(0)
	, MaxProfitAim(20.0)
	, MinProfitAim(20.0)
	, maxIdHold(0)
	, m_MultiA50(16)
	, fee(0)
	, profit(0)
	, NetProfit(0)
	, hedgenum(0)
	, numif(0l)
	, numA50(0l)
	, datetime(_T(""))
	, maxhedgehold(2)
	, m_mLadder(1.0)
{
	//��Ҫ��ֵ�ı��� 
	datumDiffDemo = 0.0;
	MultiPosDemo = 1;
	MaxProfitAim = 20.0;
	MinProfitAim = 20.0;
	m_MultiA50 = 0;
	numif = 0l;
	numA50 = 0l;
	m_pHiStarApp = (CHiStarApp*)AfxGetApp();
}

CDemoDlg::~CDemoDlg()
{
	m_pHiStarApp = NULL;
	conndemo = NULL;
	res_set = NULL;
	row = NULL;
	A50IndexDemo = 0.0,a50Bid1Demo = 0.0,a50Ask1Demo = 0.0,HS300IndexDemo = 0.0,ifBid1Demo = 0.0,ifAsk1Demo = 0.0;
}

void CDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_tableList);
	DDX_Control(pDX, IDC_RUN_DEMO, m_runDemo);
	DDX_Control(pDX, IDC_DEMOSHOW, m_demoShow);
	DDX_Control(pDX, IDC_LIST7, m_demoList);
	DDX_Control(pDX, IDC_POSITION_DEMO, m_positionShow);
	DDX_Control(pDX, IDC_EDIT2, m_bDatumdiff);
	DDX_Text(pDX, IDC_EDIT2,datumDiffDemo);
	DDX_Control(pDX, IDC_EDIT3, m_bMaxhold);
	DDX_Text(pDX, IDC_EDIT3, maxhedgehold);
	DDX_Control(pDX, IDC_LIST1, m_profitList);
	DDX_Text(pDX, IDC_LADDE_M, m_mLadder);
	DDX_Control(pDX, IDC_LADDE_M, m_bMLadder);
}


BEGIN_MESSAGE_MAP(CDemoDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CDemoDlg::OnOpenDB01)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CDemoDlg::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDC_RUN_DEMO, &CDemoDlg::OnBnClickedRunDemo)
	ON_BN_CLICKED(IDC_CLEAR, &CDemoDlg::OnBnClickedClear)
	ON_BN_CLICKED(IDC_BUTTON5, &CDemoDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_OPEN_DB_02, &CDemoDlg::OnBnClickedOpenDb02)
	ON_BN_CLICKED(IDC_UPDATE_DEMO, &CDemoDlg::OnBnClickedUpdateDemo)
END_MESSAGE_MAP()


// CDemoDlg ��Ϣ�������

void CDemoDlg::OnOpenDB01()
{
	char filter[1000] = "\"%%market_%%\"";
	OpenDB(filter);
}

void CDemoDlg::OnCbnSelchangeCombo1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}

void CDemoDlg::OnBnClickedRunDemo()
{
	CString date,hour,minute,second;
	m_runDemo.EnableWindow(false);
	m_runDemo.SetWindowText(_T("RUNNING..."));
	for(unsigned int i = 0;i < 21;i++){
		HedgeLadderDemo[i] = HedgeLadderDemoRef[i] * m_mLadder;
	}
	for(unsigned int i =0;i < m_csTableList.size();i++){
		char sql[1000];memset(sql,0,sizeof(sql));
		sprintf_s(sql,"select * from %s",m_csTableList[i].GetBuffer());
		if(conndemo){
			if(mysql_query(conndemo,sql)){
				TRACE("Error %u: %s\n", mysql_errno(conndemo), mysql_error(conndemo)); 
			}
			res_set = mysql_store_result(conndemo);
		}
		////////////////////
		if(res_set != NULL){
			while((row = mysql_fetch_row(res_set))){
				SYSTEMTIME timeBT;
				memset(&timeBT,0,sizeof(timeBT));
				datetime = row[0];
				date = datetime.Left(10);hour = datetime.Mid(11,2);minute = datetime.Mid(14,2);second = datetime.Mid(17,2);
				timeBT.wHour = atoi(hour.GetBuffer());timeBT.wMinute = atoi(minute.GetBuffer());timeBT.wSecond = atoi(second.GetBuffer());
				if(iBackTestTime(timeBT) != 1 && iBackTestTime(timeBT) != -1){
					continue;
				}
				else if(iBackTestTime(timeBT) == -1){
					//һ�����ʱͳ�Ƴֲָ���ӯ��
					double activeProfit = 0;char text[1000];memset(text,0,sizeof(text));
					for(unsigned int i = 0;i < HedgeHoldDemo.size();i++){
						activeProfit = activeProfit + HedgeHoldDemo[i].numIf * 300.0 * ((ifBid1Demo + ifAsk1Demo) /  2.0 - HedgeHoldDemo[i].priceIf)
							+ HedgeHoldDemo[i].numA50 * 6.2 * ((a50Bid1Demo + a50Ask1Demo) / 2.0 - HedgeHoldDemo[i].priceA50);
					}
					sprintf_s(text,"%s,total %12.02f,active %12.02f,close %12.02f,if %ld,A50 %ld\r\n",date,activeProfit+NetProfit,activeProfit,NetProfit,numif,numA50);
					m_profitList.AddString(text);
					break;
				}
				A50IndexDemo = atof(row[2]);
				a50Bid1Demo = atof(row[3]);
				a50Ask1Demo = atof(row[4]);
				HS300IndexDemo = atof(row[5]);
				ifBid1Demo = atof(row[6]);
				ifAsk1Demo = atof(row[7]);
				DemoTaskRun(datetime);
			}
			mysql_free_result(res_set);
			res_set = NULL;
		}
		if(date == "2014-03-20" || date == "2014-03-28" || date == "2014-04-17" || date == "2014-04-29" || date == "2014-05-15" || date == "2014-05-28"){
			ColseAllDemo();
		}
		PrintProfit();
	}
	m_runDemo.SetWindowText(_T("continue"));
	m_runDemo.EnableWindow(true);
}

void CDemoDlg::DemoTaskRun(CString datetime)
{
	char list[1000];
	if(A50IndexDemo < 1.0 || HS300IndexDemo < 1.0 || a50Bid1Demo < 1.0 || a50Ask1Demo < 1.0 || ifBid1Demo < 1.0 || ifAsk1Demo < 1.0){
		return;
	}
	//ִ��
	//�����ֲ�id
	for(unsigned int i = 0;i < HedgeHoldDemo.size();i++){
		maxIdHoldDemo = max(maxIdHoldDemo,HedgeHoldDemo[i].id);
	}
	///////////////////////
	for(unsigned int i = 0;i < HedgeHoldDemo.size();i++){
		HedgeHoldDemo[i].adjustedCost = HedgeHoldDemo[i].originalCost - datumDiffDemo;
	}
	for(int i= 0;i < 22;i++){
		PositionAimDemo[i] = PositionAimUnitDemo[i] * MultiPosDemo;
	}
	//Ĭ��ӯ��Ŀ��,һ��22�����䣬�������������ޱ߽�����
	double DefaultProfitAimBuy[22],DefaultProfitAimSell[22],ProfitAimBuy[22],ProfitAimSell[22];
	int netPosition = 0;//���ֲ�
	int SupposedBuyOpen = 0,SupposedSellOpen = 0;
	int SupposedSectionBuyOpen = 0,SupposedSectionSellOpen = 0;
	//����������������Ҫ�ǿ�������߽�����Ѿ����ͣ���ʱisSupposedBuy��isSupposedSell����Ȼ����false����ʾ�����κβ���
	bool isSupposedBuyOpen = false,isSupposedSellOpen = false;
	int CurrentSectionBuy = 0,CurrentSectionSell = 0;//��ǰ���ڵ�����,Buy��Sell�ֱ��ʾ����ۺ����ۼ��� 
	premiumDemo = (a50Bid1Demo + a50Ask1Demo) / 2.0 - (ifAsk1Demo + ifBid1Demo) / 2.0 * A50IndexDemo / HS300IndexDemo;
	premiumHighDemo = a50Ask1Demo - ifBid1Demo * A50IndexDemo / HS300IndexDemo;
	premiumLowDemo = a50Bid1Demo - ifAsk1Demo * A50IndexDemo / HS300IndexDemo;
	deviationDemo = premiumDemo - datumDiffDemo;
	DeviationSellDemo = premiumHighDemo - datumDiffDemo;
	DeviationBuyDemo = premiumLowDemo - datumDiffDemo;
	if(_isnan(datumDiffDemo) != 0 || _isnan(premiumDemo)!=0 || _isnan(deviationDemo)!=0){
		return;//�жϷ���ֵ����
	}
	if(a50Bid1Demo < 1 || a50Ask1Demo < 1 || ifAsk1Demo < 1 || ifBid1Demo < 1 || A50IndexDemo < 1 || HS300IndexDemo < 1){
		return;
	}
	if(fabs(premiumDemo) > 500 || fabs(premiumDemo) < 0.01){
		return;//�ų�����ʱ�п��ܱ��۲�ȫ���µĴ�����ۼ���
	}
	//ͳ�ƾ��ֲ�
	for(unsigned int i = 0;i < HedgeHoldDemo.size();i++){
		netPosition = netPosition + HedgeHoldDemo[i].HedgeNum;
	}
	for(int i = 1;i < 21;i++){
		if(HedgeLadderDemo[i - 1] > HedgeLadderDemo[i]){
			return;
		}
		if(PositionAimUnitDemo[i - 1] < PositionAimUnitDemo[i]){
			return;
		}
	}
	if(MaxProfitAim < MinProfitAim){
		return;
	}
	//���㵱ǰ����
	for(int i = 0;i < 22;i++){
		if(i == 0){
			if(DeviationBuyDemo < HedgeLadderDemo[0]){
				CurrentSectionBuy = 0;
			}
			if(DeviationSellDemo < HedgeLadderDemo[0]){
				CurrentSectionSell = 0;
			}
		}
		else if(i == 21){
			if(DeviationBuyDemo >= HedgeLadderDemo[20]){
				CurrentSectionBuy = 21;
			}
			if(DeviationSellDemo >= HedgeLadderDemo[20]){
				CurrentSectionSell = 21;
			}
		}
		else{
			if(DeviationBuyDemo >= HedgeLadderDemo[i] && DeviationBuyDemo < HedgeLadderDemo[i + 1]){
				CurrentSectionBuy = i + 1;
			}
			if(DeviationSellDemo >= HedgeLadderDemo[i] && DeviationSellDemo < HedgeLadderDemo[i + 1]){
				CurrentSectionSell = i + 1;
			}
		}
	}
	for(int i = 0;i < 22;i++){
		if(i < 20){
			DefaultProfitAimBuy[i] = HedgeLadderDemo[i + 1] -  HedgeLadderDemo[i];
		}
		else{
			DefaultProfitAimBuy[i] = 20.0;//�߽��
		}
		if(i > 1){
			DefaultProfitAimSell[i] = HedgeLadderDemo[i - 1] - HedgeLadderDemo[i - 2];
		}
		else{
			DefaultProfitAimSell[i] = 20.0;//�߽��
		}
		ProfitAimBuy[i] = max(DefaultProfitAimBuy[i],MinProfitAim);
		ProfitAimBuy[i] = min(ProfitAimBuy[i],MaxProfitAim);
		ProfitAimSell[i] = max(DefaultProfitAimSell[i],MinProfitAim);
		ProfitAimSell[i] = min(ProfitAimSell[i],MaxProfitAim);
	}
	for(unsigned int i = 0;i < HedgeHoldDemo.size();i++){
		if(HedgeHoldDemo[i].HedgeNum > 0){//��ͷ�ֲ�
			if(HedgeHoldDemo[i].HedgeSection == 21){
				//��Ҫƽ��
				if(DeviationBuyDemo >= HedgeLadderDemo[20] + ProfitAimBuy[HedgeHoldDemo[i].HedgeSection] 
				&& DeviationBuyDemo >= HedgeHoldDemo[i].adjustedCost + ProfitAimBuy[HedgeHoldDemo[i].HedgeSection]){
					//��Ҫƽ��
					fee = fee + 1.5 * 2 * 6.2 * abs(HedgeHoldDemo[i].numA50) + 65 * 2 * abs(HedgeHoldDemo[i].numIf); 
					profit = profit + (a50Bid1Demo - HedgeHoldDemo[i].priceA50) * 6.2 * HedgeHoldDemo[i].numA50 + (ifAsk1Demo - HedgeHoldDemo[i].priceIf) * 300.0 * HedgeHoldDemo[i].numIf;
					NetProfit = profit - fee;
					sprintf_s(list,"%s,�ֲܳ�%2d,if %2ld,A50 %2ld,��ƽ%2d��,%8.02f,%8.02lf,%8.02lf\r\n",datetime,hedgenum,numif,numA50,HedgeHoldDemo[i].HedgeNum,DeviationBuyDemo,a50Bid1Demo,ifAsk1Demo);
					m_demoList.AddString(list);
					HedgeHoldDemo.erase(HedgeHoldDemo.begin() + i);
					i--;
					PrintPosition();
					return;
				}
			}
			else{
				if(DeviationBuyDemo >= HedgeLadderDemo[HedgeHoldDemo[i].HedgeSection] + ProfitAimBuy[HedgeHoldDemo[i].HedgeSection]
				&& DeviationBuyDemo >= HedgeHoldDemo[i].adjustedCost + ProfitAimBuy[HedgeHoldDemo[i].HedgeSection]){
					//��Ҫƽ��
					fee = fee + 1.5 * 2 * 6.2 * abs(HedgeHoldDemo[i].numA50) + 65 * 2 * abs(HedgeHoldDemo[i].numIf); 
					profit = profit + (a50Bid1Demo - HedgeHoldDemo[i].priceA50) * 6.2 * HedgeHoldDemo[i].numA50 + (ifAsk1Demo - HedgeHoldDemo[i].priceIf) * 300.0 * HedgeHoldDemo[i].numIf;
					NetProfit = profit - fee;
					sprintf_s(list,"%s,�ֲܳ�%2d,if %2ld,A50 %2ld,��ƽ%2d��,%8.02f,%8.02lf,%8.02lf\r\n",datetime,hedgenum,numif,numA50,HedgeHoldDemo[i].HedgeNum,DeviationBuyDemo,a50Bid1Demo,ifAsk1Demo);
					m_demoList.AddString(list);
					HedgeHoldDemo.erase(HedgeHoldDemo.begin() + i);
					i--;
					PrintPosition();
					return;
				}
			}
		}
		else if(HedgeHoldDemo[i].HedgeNum < 0){//��ͷ�ֲ�
			if(HedgeHoldDemo[i].HedgeSection == 0){
				if(DeviationSellDemo <= HedgeLadderDemo[0] - ProfitAimSell[HedgeHoldDemo[i].HedgeSection]
				&& DeviationSellDemo <= HedgeHoldDemo[i].adjustedCost - ProfitAimSell[HedgeHoldDemo[i].HedgeSection]){
					fee = fee + 1.5 * 2 * 6.2 * abs(HedgeHoldDemo[i].numA50) + 65 * 2 * abs(HedgeHoldDemo[i].numIf); 
					profit = profit + (a50Ask1Demo - HedgeHoldDemo[i].priceA50) * 6.2 * HedgeHoldDemo[i].numA50 + (ifBid1Demo - HedgeHoldDemo[i].priceIf) * 300.0 * HedgeHoldDemo[i].numIf;
					NetProfit = profit - fee;
					sprintf_s(list,"%s,�ֲܳ�%2d,if %2ld,A50 %2ld,��ƽ%2d��,%8.02f,%8.02lf,%8.02lf\r\n",datetime,hedgenum,numif,numA50,HedgeHoldDemo[i].HedgeNum,DeviationSellDemo,a50Ask1Demo,ifBid1Demo);
					m_demoList.AddString(list);
					HedgeHoldDemo.erase(HedgeHoldDemo.begin() + i);
					i--;
					PrintPosition();
					return;
				}
			}
			else{
				if(DeviationSellDemo <= HedgeLadderDemo[HedgeHoldDemo[i].HedgeSection - 1] - ProfitAimSell[HedgeHoldDemo[i].HedgeSection]
				&& DeviationSellDemo <= HedgeHoldDemo[i].adjustedCost - ProfitAimSell[HedgeHoldDemo[i].HedgeSection]){
					fee = fee + 1.5 * 2 * 6.2 * abs(HedgeHoldDemo[i].numA50) + 65 * 2 * abs(HedgeHoldDemo[i].numIf); 
					profit = profit + (a50Ask1Demo - HedgeHoldDemo[i].priceA50) * 6.2 * HedgeHoldDemo[i].numA50 + (ifBid1Demo - HedgeHoldDemo[i].priceIf) * 300.0 * HedgeHoldDemo[i].numIf;
					NetProfit = profit - fee;
					sprintf_s(list,"%s,�ֲܳ�%2d,if %2ld,A50 %2ld,��ƽ%2d��,%8.02f,%8.02lf,%8.02lf\r\n",datetime,hedgenum,numif,numA50,HedgeHoldDemo[i].HedgeNum,DeviationSellDemo,a50Ask1Demo,ifBid1Demo);
					m_demoList.AddString(list);
					HedgeHoldDemo.erase(HedgeHoldDemo.begin() + i);
					i--;
					PrintPosition();
					return;
				}
			}
		}
	}
	for(int i = 0;i <= 21;i++){
		if(PositionAimDemo[i] < 0){
			if(-abs(netPosition) > PositionAimDemo[i]){
				SupposedSellOpen = -abs(netPosition) - PositionAimDemo[i];
				SupposedSectionSellOpen = i;
				isSupposedSellOpen = true;
				break;
			}
		}
	}
	for(int i = 21;i >= 0;i--){
		if(PositionAimDemo[i] > 0){
			if(abs(netPosition) < PositionAimDemo[i]){
				SupposedBuyOpen = -(abs(netPosition) - PositionAimDemo[i]);
				SupposedSectionBuyOpen = i;
				isSupposedBuyOpen = true;
				break;
			}
		}
	}
	if(abs(hedgenum) >= abs(maxhedgehold))return;
	//���ֲ���
	if(isSupposedBuyOpen){
		if(CurrentSectionSell <= SupposedSectionBuyOpen){
			//��Ҫ����
			HoldDetail newhold;
			newhold.HedgeNum = SupposedBuyOpen;
			newhold.adjustedCost = DeviationSellDemo;
			newhold.originalCost = DeviationSellDemo + datumDiffDemo;
			newhold.priceIf = ifBid1Demo;
			newhold.indexHS300 = HS300IndexDemo;
			newhold.priceA50 = a50Ask1Demo;
			newhold.indexA50 = A50IndexDemo;
			newhold.HedgeSection = SupposedSectionBuyOpen;
			newhold.numA50 = SupposedBuyOpen * m_MultiA50;
			newhold.numIf = -SupposedBuyOpen;
			newhold.id = ++maxIdHold;
			sprintf_s(list,"%s,�ֲܳ�%2d,if %2ld,A50 %2ld,��%2d��,%8.02f,%8.02lf,%8.02lf\r\n",datetime,hedgenum,numif,numA50,newhold.HedgeNum,DeviationSellDemo,a50Ask1Demo,ifBid1Demo);
			m_demoList.AddString(list);
			HedgeHoldDemo.push_back(newhold);
			PrintPosition();
			return;
		}
	}
	else if(isSupposedSellOpen){
		if(CurrentSectionBuy >= SupposedSectionSellOpen){
			//��Ҫ����
			HoldDetail newhold;
			newhold.HedgeNum = -SupposedSellOpen;
			newhold.adjustedCost = DeviationBuyDemo;
			newhold.originalCost = DeviationBuyDemo + datumDiffDemo;
			newhold.priceIf = ifAsk1Demo;
			newhold.indexHS300 = HS300IndexDemo;
			newhold.priceA50 = a50Bid1Demo;
			newhold.indexA50 = A50IndexDemo;
			newhold.HedgeSection = SupposedSectionSellOpen;
			newhold.numA50 = -SupposedSellOpen * m_MultiA50;
			newhold.numIf = SupposedSellOpen;
			newhold.id = ++maxIdHold;
			sprintf_s(list,"%s,�ֲܳ�%2d,if %2ld,A50 %2ld,����%2d��,%8.02f,%8.02lf,%8.02lf\r\n",datetime,hedgenum,numif,numA50,newhold.HedgeNum,DeviationBuyDemo,a50Bid1Demo,ifAsk1Demo);
			m_demoList.AddString(list);
			HedgeHoldDemo.push_back(newhold);
			PrintPosition();
			return;
		}
	}
}

void CDemoDlg::OnBnClickedClear()
{
	fee = 0.0;
	profit = 0.0;
	NetProfit = 0.0;
	hedgenum = 0;
	numif = 0;
	numA50 = 0;
	HedgeHoldDemo.clear();
	m_demoList.ResetContent();
	m_profitList.ResetContent();
}

void CDemoDlg::PrintPosition(void)
{
	char poList[1000];
	hedgenum = 0;numif = 0;numA50 = 0;
	for(unsigned int i = 0;i < HedgeHoldDemo.size();i++){
		hedgenum = hedgenum + HedgeHoldDemo[i].HedgeNum;
		numif = numif + HedgeHoldDemo[i].numIf;
		numA50 = numA50 + HedgeHoldDemo[i].numA50;
	}
	sprintf_s(poList,"Hedge %2d,If %2ld,A50 %2ld\r\n",hedgenum,numif,numA50);
	m_positionShow.SetWindowText(poList);
}

void CDemoDlg::OnBnClickedButton5()
{
	ColseAllDemo();
}

void CDemoDlg::PrintProfit(void)
{
	char demoShow[1000];memset(demoShow,0,sizeof(demoShow));
	sprintf_s(demoShow,"profit %8.02lf,fee %8.02lf,NetProfit %8.02lf\r\n",profit,fee,NetProfit);
	m_demoShow.SetWindowText(demoShow);
}

void CDemoDlg::OnBnClickedOpenDb02()
{
	char filter[1000] = "\"%%market\"";
	OpenDB(filter);
}

void CDemoDlg::OnBnClickedUpdateDemo()
{
	UpdateData();
}

BOOL CDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_bDatumdiff.SetWindowText(_T("0.0"));
	m_bMaxhold.SetWindowText(_T("2"));
	m_bMLadder.SetWindowText(_T("1.0"));
	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}

void CDemoDlg::ColseAllDemo(void)
{
	char list[1000];memset(list,0,sizeof(list));
	for(unsigned int i = 0;i < HedgeHoldDemo.size();i++){
		if(HedgeHoldDemo[i].HedgeNum > 0){
			fee = fee + 1.5 * 2 * 6.2 * abs(HedgeHoldDemo[i].numA50) + 65 * 2 * abs(HedgeHoldDemo[i].numIf); 
			profit = profit + (a50Bid1Demo - HedgeHoldDemo[i].priceA50) * 6.2 * HedgeHoldDemo[i].numA50 + (ifAsk1Demo - HedgeHoldDemo[i].priceIf) * 300.0 * HedgeHoldDemo[i].numIf;
			NetProfit = profit - fee;
			sprintf_s(list,"%s,�ֲܳ�%2d,if %2ld,A50 %2ld,��ƽ%2d��,%8.02f,%8.02lf,%8.02lf\r\n",datetime,hedgenum,numif,numA50,HedgeHoldDemo[i].HedgeNum,DeviationBuyDemo,a50Bid1Demo,ifAsk1Demo);
			m_demoList.AddString(list);
		}
		else{
			fee = fee + 1.5 * 2 * 6.2 * abs(HedgeHoldDemo[i].numA50) + 65 * 2 * abs(HedgeHoldDemo[i].numIf);
			profit = profit + (a50Ask1Demo - HedgeHoldDemo[i].priceA50) * 6.2 * HedgeHoldDemo[i].numA50 + (ifBid1Demo - HedgeHoldDemo[i].priceIf) * 300.0 * HedgeHoldDemo[i].numIf;
			NetProfit = profit - fee;
			sprintf_s(list,"%s,�ֲܳ�%2d,if %2ld,A50 %2ld,��ƽ%2d��,%8.02f,%8.02lf,%8.02lf\r\n",datetime,hedgenum,numif,numA50,HedgeHoldDemo[i].HedgeNum,DeviationSellDemo,a50Ask1Demo,ifBid1Demo);
			m_demoList.AddString(list);
		}
	}
	HedgeHoldDemo.clear();
	PrintPosition();
	PrintProfit();
}

void CDemoDlg::OpenDB(char *filter)
{
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	char sql[1000];memset(sql,0,sizeof(sql));
	conndemo = mysql_init(NULL); 
	if(conndemo == NULL) {
		TRACE("Error %u: %s\n", mysql_errno(conndemo), mysql_error(conndemo)); 
	}  
	if(conndemo){
		if(mysql_real_connect(conndemo,m_db.host.c_str(),m_db.user.c_str(),m_db.passwd.c_str(),m_db.db.c_str(),0,NULL,0) == NULL)
		{      
			TRACE("Error %u: %s\n", mysql_errno(conndemo), mysql_error(conndemo));
		}
		sprintf_s(sql,"show tables like %s",filter);
		if(mysql_query(conndemo,sql)) 
		{      
			TRACE("Error %u: %s\n", mysql_errno(conndemo), mysql_error(conndemo));      
		}
		unsigned int num_fields;
		res_set = mysql_store_result(conndemo);
		if(res_set != NULL){
			num_fields = mysql_num_fields(res_set);
			m_tableList.ResetContent();
			m_csTableList.clear();
			while ((row = mysql_fetch_row(res_set))){
				m_tableList.AddString(*row);
				m_csTableList.push_back(*row);
			}
			sort(m_csTableList.begin(),m_csTableList.end(),CmpByTimeDemo);//����
			mysql_free_result(res_set);
			res_set = NULL;
		}
		row = NULL;
	}	
}
