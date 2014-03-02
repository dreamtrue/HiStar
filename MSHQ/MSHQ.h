#pragma once
#pragma pack(1)
typedef struct tagHQHead		//����ͷ��
{
	char	Mark[20];		//��ʶ
	long	Update;			//����ʱ��
	ULONG	Length;			//�ܳ���
	UINT	SZCount;		//���м�¼��
	UINT	SHCount;		//���м�¼��
	long	SZOffset;		//���������ƫ��
	long	SHOffset;		//���������ƫ��
	BYTE	Protect[128];	//����
}HQ_HEAD,*PHQ_HEAD;

typedef struct tagStockHQ		//��Ʊ����
{
	char	Code[10];			//��Ʊ����
	char	Name[10];			//��Ʊ����
	UINT	Type;				//��Ʊ���
	long	Protect1;			//��ʶ
	float	Close;				//�������̼�
	float	Open;				//���տ��̼�
	float	High;				//������߼�
	float	Low;				//������ͼ�
	float	Current;			//���յ�ǰ��
	long	Update;				//����ʱ��
	long	Protect2;			//����
	long	Volum;				//�ɽ���
	long	Vol1;				//����
	float	Amount;				//�ɽ���
	long	Imp_vol;			//������
	long	Ext_vol;			//������
	long	Protect3;			//����
	long	Protect4;			//����
	float	Buy_Price[5];		//5�����
	long	Buy_Volum[5];		//5������
	float	Sell_Price[5];		//5������
	long	Sell_Volum[5];		//5������
	long	Protect5;			//����
	BYTE	Protect6[6];		//����
}STOCK_HQ,*PSTOCK_HQ;			//�ܳ�174
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
	CMSHQ();           // ��̬������ʹ�õ��ܱ����Ĺ��캯��
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


