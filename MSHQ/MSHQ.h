#pragma once
#pragma pack(1)
typedef struct tagHQHead		//行情头部
{
	char	Mark[20];		//标识
	long	Update;			//更新时间
	ULONG	Length;			//总长度
	UINT	SZCount;		//深市记录数
	UINT	SHCount;		//沪市记录数
	long	SZOffset;		//深市行情表偏移
	long	SHOffset;		//沪市行情表偏移
	BYTE	Protect[128];	//保留
}HQ_HEAD,*PHQ_HEAD;

typedef struct tagStockHQ		//股票行情
{
	char	Code[10];			//股票代码
	char	Name[10];			//股票名称
	UINT	Type;				//股票类别
	long	Protect1;			//标识
	float	Close;				//昨日收盘价
	float	Open;				//今日开盘价
	float	High;				//今日最高价
	float	Low;				//今日最低价
	float	Current;			//今日当前价
	long	Update;				//更新时间
	long	Protect2;			//保留
	long	Volum;				//成交量
	long	Vol1;				//现量
	float	Amount;				//成交额
	long	Imp_vol;			//内盘量
	long	Ext_vol;			//外盘量
	long	Protect3;			//保留
	long	Protect4;			//保留
	float	Buy_Price[5];		//5档买价
	long	Buy_Volum[5];		//5档买量
	float	Sell_Price[5];		//5档卖价
	long	Sell_Volum[5];		//5档卖量
	long	Protect5;			//保留
	BYTE	Protect6[6];		//保留
}STOCK_HQ,*PSTOCK_HQ;			//总长174
#pragma pack()

typedef CArray<STOCK_HQ,STOCK_HQ>	HQ_LIST;

#define ISBLANK(a)		BOOL(fabs((double)(a))<0.001)
#define ISEQUAL(a,b)	BOOL(fabs((double)(a-b))<0.001)
#define NOEQUAL(a,b)	BOOL((fabs)((double)(a-b))>=0.001)


// CMSHQ

class CMSHQ : public CWinThread
{
	DECLARE_DYNCREATE(CMSHQ)

protected:
	CMSHQ();           // 动态创建所使用的受保护的构造函数
	virtual ~CMSHQ();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
public:
	void LoadOption();
	void UpdateHQ();
	void UpdateSZHQ();
	void UpdateSHHQ();
	int  OpenHQFile();
	void CalcTotalValue();
	BOOL IsIndex(LPCTSTR pCode);
	int			m_HQSelect;
	CString		m_HQFileName;
	PHQ_HEAD	m_pHQHead;
	HANDLE		m_HQFileHandle;
	CMemFile	m_SZHQFile;
	CMemFile	m_SHHQFile;
	HQ_LIST		m_SZHQList;
	HQ_LIST		m_SHHQList;
	PSTOCK_HQ	m_pCurSelect;
protected:
	DECLARE_MESSAGE_MAP()
};


