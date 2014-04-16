#include "stdafx.h"
#include "HiStar.h"
#include "MSHQ.h"
#include "DealIndex.h"
double g_totalA50Value = 0;//A50总值
double g_totalHS300Value = 0;//HS300总值
double g_A50IndexMSHQ = 0;
double g_HS300IndexMSHQ = 0;
struct stock{
	std::string exch;
	std::string code;
	int volume;
};
extern std::vector<stock> g_hs300;
extern std::vector<stock> g_a50;
extern double A50IndexRef,A50totalValueRef,HS300IndexRef,HS300totalValueRef;
//自定义的股票数据结构
typedef struct tagStockSlf		//股票行情
{
	char	Code[10];			//股票代码
	float	Current;			//今日当前价
	float   Close;              //昨日收盘价
	long	Update;				//更新时间
	int     index;              //在镜像文件中的位置
	int     volume;             //在指数中的持仓数目
	char    exchange;           //交易所
}STOCK_SLF,*PSTOCK_SLF;

typedef CArray<STOCK_SLF,STOCK_SLF>	STOCK_SLF_LIST;
STOCK_SLF_LIST StockSlfList;

CString	GetTimeStr(long nBCDTime)
{
	long	nHour	=nBCDTime/10000;
	long	nMinute	=(nBCDTime%10000)/100;
	long	nSecond	=(nBCDTime%10000)%100;
	CString nText;

	nText.Format("%02d:%02d:%02d",nHour,nMinute,nSecond);
	return nText;
}

CString GetLoadPath(void)
	//功能：获取系统载入路径
{
	TCHAR	pszFullPath[128];
	int		nPos;
	CString SourcePath;

	GetModuleFileName(NULL,pszFullPath,128);
	SourcePath=pszFullPath;
	nPos=SourcePath.ReverseFind('\\');
	return SourcePath.Left(nPos);
}

// CMSHQ

IMPLEMENT_DYNCREATE(CMSHQ, CWinThread)

	CMSHQ::CMSHQ()
{
	m_HQSelect =0;
	m_HQFileName = _T("MXHQ_X.DAT");
	m_pHQHead = NULL;
	m_HQFileHandle = NULL;
	//初始化股票数组
	STOCK_SLF stock;
	stock.index = -1;//默认值-1;
	memset(stock.Code,0,sizeof(stock.Code));
	for(unsigned int i = 0;i < 50;i++){
		if(i < g_a50.size()){
			strcpy(stock.Code,g_a50[i].code.c_str());
			stock.volume = g_a50[i].volume;
			StockSlfList.Add(stock);
		}
	}
	for(unsigned int i = 0;i < 300;i++){
		if(i < g_hs300.size()){
			strcpy(stock.Code,g_hs300[i].code.c_str());
			stock.volume = g_hs300[i].volume;
			StockSlfList.Add(stock);
		}
	}
}

CMSHQ::~CMSHQ()
{
}

BOOL CMSHQ::InitInstance()
{
	LoadOption();
	MSG msg;BOOL bRet;
	for (;true;)
	{
		if(bRet = ::PeekMessage(&msg,NULL,WM_QUIT,WM_QUIT,PM_NOREMOVE) == TRUE){
			if(!bRet){}else{
				break;
			}
		}
		else{
			UpdateHQ();
			::Sleep(2000);
		}
	}
	return TRUE;
}

int CMSHQ::ExitInstance()
{
	// TODO: 在此执行任意逐线程清理
	return CWinThread::ExitInstance();
}

void CMSHQ::LoadOption(){
	CFile		nFile;
	CString		nOptionPath=GetLoadPath()+"\\ChgTime.ini";
	CString		nFeild;
	char		nText[256];

	if (!nFile.Open(nOptionPath,CFile::modeRead))
	{
		::WritePrivateProfileString("HQ", "NAME", "MXHQ_X.DAT", nOptionPath);
		::WritePrivateProfileString("HQ", "WAYS", "0", nOptionPath);
		m_HQFileName = "MXHQ_X.DAT";
		m_HQSelect = 0;
		return;
	}

	nFile.Close();

	::GetPrivateProfileString("HQ", "NAME", "MXHQ.DAT",nText, sizeof(nText), nOptionPath);
	m_HQFileName = nText;

	::GetPrivateProfileString("HQ", "WAYS", "0",nText, sizeof(nText), nOptionPath);
	m_HQSelect= atoi(nText);
}

int CMSHQ::OpenHQFile()
	//功能:
{
	STOCK_HQ nHQItem;
	long	 nFileLength=sizeof(HQ_HEAD);
	UINT	 i;

	if (m_HQFileName.IsEmpty())
	{
		TRACE("未指定行情镜象文件名,请改名后重新启动!");
		return	1;
	}

	m_HQFileHandle = OpenFileMapping(FILE_MAP_READ,TRUE,m_HQFileName);
	if (m_HQFileHandle)
	{
		LPBYTE	pBase=(BYTE*)MapViewOfFile(m_HQFileHandle,FILE_MAP_READ,0,0,0);
		int		nLen=0;

		m_pHQHead=(PHQ_HEAD)pBase;
		if (strcmp(m_pHQHead->Mark,"530102196905033018"))
		{
			TRACE("发现同名镜象文件,但不是行情的镜象文件,请改名后重新启动!");
			return 3;
		}
		nFileLength=m_pHQHead->SHOffset-m_pHQHead->SZOffset;
		m_SZHQFile.Attach(pBase+m_pHQHead->SZOffset,nFileLength);
		nFileLength=m_pHQHead->Length-m_pHQHead->SHOffset;
		m_SHHQFile.Attach(pBase+m_pHQHead->SHOffset,nFileLength);

		for (i=0;i<m_pHQHead->SZCount;i++)
		{
			m_SZHQFile.Read(&nHQItem,sizeof(STOCK_HQ));
			for(int j = 0;j < StockSlfList.GetCount();j++){
				if(!strcmp(StockSlfList[j].Code,nHQItem.Code) && CString(nHQItem.Code).Left(1) == '0'){
					StockSlfList[j].exchange = 'Z';
					StockSlfList[j].index = i;
				}
			}
			m_SZHQList.Add(nHQItem);
		}

		for (i=0;i<m_pHQHead->SHCount;i++)
		{
			m_SHHQFile.Read(&nHQItem,sizeof(STOCK_HQ));
			for(int j = 0;j < StockSlfList.GetCount();j++){
				if(!strcmp(StockSlfList[j].Code,nHQItem.Code) && CString(nHQItem.Code).Left(1) == '6'){
					StockSlfList[j].exchange = 'H';
					StockSlfList[j].index = i;
				}
			}
			m_SHHQList.Add(nHQItem);
		}
		//自定义数组检查，看是否完全初始化
		for(int j = 0;j < StockSlfList.GetCount();j++){
			//TRACE("股票%s，位置%d,交易所%c\r\n",StockSlfList[j].Code,StockSlfList[j].index,StockSlfList[j].exchange);
			if(StockSlfList[j].index < 0){
				TRACE("%s未完成初始化\r\n",StockSlfList[j].Code);
				return 2;
			}
		}
		TRACE("已发现行情镜象文件!\n");
		return 0;
	}
	//TRACE("未发现行情镜象文件!\n");
	return 2;
}

void CMSHQ::UpdateHQ()
{
	if (m_pHQHead == NULL)//表示还没有打开文件
	{
		int nError= OpenHQFile();
		if(nError == 2){
			g_A50IndexMSHQ = 0.0;
			g_HS300IndexMSHQ = 0.0;
			//TRACE("股票数组初始化失败\r\n");
		}
		return;
	}
	UpdateSZHQ();
	UpdateSHHQ();
	CalcTotalValue();//计算总价值
}

void CMSHQ::CalcTotalValue(){
	g_totalA50Value = 0;
	g_totalHS300Value = 0;
	for(int j = 0;j < 50;j++){
		if(j < StockSlfList.GetCount()){
			if(StockSlfList[j].Current > 0.000001){
				g_totalA50Value = g_totalA50Value + StockSlfList[j].Current * StockSlfList[j].volume;
			}
			else{
				g_totalA50Value = g_totalA50Value + StockSlfList[j].Close * StockSlfList[j].volume;
			}
		}
		//TRACE("%s , %.02lf\r\n",StockSlfList[j].Code,StockSlfList[j].Current);
	}
	for(int j = 50;j < 350;j++){
		//TRACE("%s , %.02lf\r\n",StockSlfList[j].Code,StockSlfList[j].Current);
		if(j < StockSlfList.GetCount()){
			if(StockSlfList[j].Current > 0.000001){
				g_totalHS300Value = g_totalHS300Value + StockSlfList[j].Current * StockSlfList[j].volume;
			}
			else{
				g_totalHS300Value = g_totalHS300Value + StockSlfList[j].Close * StockSlfList[j].volume;
			}
		}
	}
	g_A50IndexMSHQ = g_totalA50Value / A50totalValueRef * A50IndexRef;
	g_HS300IndexMSHQ = g_totalHS300Value / HS300totalValueRef * HS300IndexRef;
	//TRACE(_T("A50指数%.02lf,HS300指数%.02lf\r\n"),g_A50IndexMSHQ,g_HS300IndexMSHQ);
}

void CMSHQ::UpdateSZHQ()
{
	PSTOCK_HQ	pItem;
	STOCK_HQ	nHQ;
	int			i,nNum=m_SZHQList.GetSize();

	m_SZHQFile.SeekToBegin();
	for (i=0;i < nNum;)
	{
		m_SZHQFile.Read(&nHQ,sizeof(STOCK_HQ));

		pItem=&m_SZHQList[i];
		if (strcmp(pItem->Code,nHQ.Code))
		{
			TRACE("Detect defferent HQ Item at SZ: %s---%s\n",pItem->Code,nHQ.Code);
			continue;
		}

		if (pItem->Update!= nHQ.Update)
		{
			memcpy(pItem,&nHQ,sizeof(STOCK_HQ));
		}
		i++;
	}
	for(int j = 0;j < StockSlfList.GetCount();j++){
		if(StockSlfList[j].exchange == 'Z'){
			StockSlfList[j].Current = m_SZHQList[StockSlfList[j].index].Current;
			StockSlfList[j].Close = m_SZHQList[StockSlfList[j].index].Close;
		}
	}
}

void CMSHQ::UpdateSHHQ()
{
	PSTOCK_HQ	pItem;
	STOCK_HQ	nHQ;
	int			i,nNum=m_SHHQList.GetSize();

	m_SHHQFile.SeekToBegin();
	for (i=0;i<nNum;)
	{
		m_SHHQFile.Read(&nHQ,sizeof(STOCK_HQ));

		pItem=&m_SHHQList[i];
		if (strcmp(pItem->Code,nHQ.Code))
		{
			TRACE("Detect defferent HQ Item from SH: %s---%s\n",pItem->Code,nHQ.Code);
			continue;
		}
		if (pItem->Update!=nHQ.Update)
		{
			memcpy(pItem,&nHQ,sizeof(STOCK_HQ));
		}
		i++;
	}
	for(int j = 0;j < StockSlfList.GetCount();j++){
		if(StockSlfList[j].exchange == 'H'){
			StockSlfList[j].Current = m_SHHQList[StockSlfList[j].index].Current;
			StockSlfList[j].Close = m_SHHQList[StockSlfList[j].index].Close;
		}
	}
}

BOOL CMSHQ::IsIndex(LPCTSTR pCode)
{
	CString 	nCode=CString(pCode).Left(3);

	return (nCode==_T("399")||nCode==_T("999")||nCode==_T("888"));
}

BEGIN_MESSAGE_MAP(CMSHQ, CWinThread)
END_MESSAGE_MAP()


// CMSHQ 消息处理程序
