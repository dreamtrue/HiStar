#ifndef GLOBAL_H
#define GLOBAL_H
#include <string.h>
#include "ThostFtdcMdApi.h"
#include "ThostFtdcTraderApi.h"
#pragma comment(lib,"..\\api_libs\\thostmduserapi.lib")
#pragma comment(lib,"..\\api_libs\\thosttraderapi.lib")

#include "Color.h"

#include <vector>

#define WAIT_MS	10000

///��Լ��֤����
typedef CThostFtdcInstrumentMarginRateField MGRATE,*PMGRATE;
///��Լ��������
typedef CThostFtdcInstrumentCommissionRateField FEERATE,*PFEERATE;

typedef CThostFtdcInstrumentField INSTINFO,*PINSTINFO;

typedef struct CThostFtdcInstrumentFieldEx
{
	INSTINFO iinf;
	TThostFtdcRatioType	OpenRatioByMoney;
	TThostFtdcRatioType	OpenRatioByVolume;
	TThostFtdcRatioType	CloseRatioByMoney;
	TThostFtdcRatioType	CloseRatioByVolume;
	TThostFtdcRatioType	CloseTodayRatioByMoney;
	TThostFtdcRatioType	CloseTodayRatioByVolume;
}INSINFEX,*PINSINFEX;

typedef std::vector<CThostFtdcInputOrderField> ::iterator VOrd;
typedef std::vector<CThostFtdcInvestorPositionField> ::iterator VInvP;
typedef std::vector<CThostFtdcInstrumentFieldEx>::iterator VIT_if;
typedef std::vector<CThostFtdcInstrumentMarginRateField>::iterator VIT_mr;
typedef std::vector<CThostFtdcInstrumentCommissionRateField>::iterator VIT_cf;

struct SAccountIB{
	CString  m_accountName;
	SAccountIB(){
		m_accountName = _T("U1032950");
	}
};
struct SAccountCtp{
	TThostFtdcBrokerIDType	m_sBROKER_ID;
	TThostFtdcInvestorIDType m_sINVESTOR_ID;
	TThostFtdcPasswordType  m_sPASSWORD;

	CString m_szInst;
	//ǰ�õ�ַ
	CStringArray m_szArTs,m_szArMd;
	//�˻���Ϣ
	INSINFEX *m_InstInf;
	CThostFtdcDepthMarketDataField *m_pDepthMd;
	CThostFtdcTradingAccountField *m_pTdAcc;
	CThostFtdcNotifyQueryAccountField *m_pNotifyBkYe;
	CThostFtdcInvestorField *m_pInvInf;
	SAccountCtp(){
		//��ʼ��
		m_InstInf = new CThostFtdcInstrumentFieldEx();
	    m_pDepthMd = new CThostFtdcDepthMarketDataField();
		m_pTdAcc = new CThostFtdcTradingAccountField();
		m_pInvInf = new CThostFtdcInvestorField();
		m_pNotifyBkYe = new CThostFtdcNotifyQueryAccountField();
	}
	~SAccountCtp(){
		delete m_InstInf;delete m_pDepthMd;delete m_pTdAcc;delete m_pInvInf;delete m_pNotifyBkYe;
	}
};
#define _REAL_CTP_

#define PROD_INFO "Q7 8117"
#define DBL_MAX 1.7976931348623158e+308 
#define DBL_MIN 2.2250738585072014e-308
#define CFMMC_TMPL "https://investorservice.cfmmc.com/loginByKey.do?companyID=%s&userid=%s&keyid=%d&passwd=%s"
#define CFG_FILE _T("config.xml")
#define SVR_FILE _T("servers.xml")
#define FEE_XML "fee_%s.xml"
#define MGR_XML "mgr_%s.xml"
#define SIDS _T("sids.xml")
#define AUTHOR _T("t0trader")
#define MY_TIPS _T("��ʾ��Ϣ")
#define _QNA _T("���˵��")
#define _GERR _T("����")
#define  ACC_DETAILS _T("�ڻ��ʽ��˻�����")
#define  BKACC_LEFT _T("�����ʽ��˻�")
#define BFTRANS_TITLE _T("����ת��@%s")
#define CONN_ERR _T("����ʧ��")
#define  ACC_FBINFO _T("������Ϣ")
#define USER_ID "LoginUserID"
#define SV_GRP "ServerGroup"
#define SV_INF "SaveInfo"
#define USER_PW "UserPassword"
#define NTP_SVR "NtpServer"
#define INS_LST "InstList"
#define WND_INF "WndInfo"
#define ROOT "root"
#define SESRT "sessions"
#define SESITEM "session"

#define _CZCE _T("֣����")
#define _DCE _T("������")
#define _SHFE _T("������")
#define _CFFEX _T("�н���")

#define REFRESH_TIMER 100
#define SYNC_TIMER 101

#define DIR_BUY _T("��")
#define DIR_SELL _T("��")

#define ORD_O _T("����")
#define ORD_C _T("ƽ��")
#define ORD_CT _T("ƽ��")

#define INVPOS_ITMES 9
#define ONROAD_ITMES 9
#define TRADE_ITMES 12
#define ORDER_ITMES 15
#define ALLINST_ITMES 10

#define UNCOMP _T("�����")

#ifdef _UNICODE
#define BTPERCHAR 2
#else
#define BTPERCHAR 1
#endif

const UINT WM_TASKBARCREATED = ::RegisterWindowMessage(_T("TaskbarCreated"));
const UINT WM_SYNCTIME = ::RegisterWindowMessage(_T("SyncTime_Client"));
const UINT WM_QRYACC_MSG = ::RegisterWindowMessage(_T("QryAcc_Msg"));
//const UINT WM_UPDATEMD_MSG = ::RegisterWindowMessage(_T("UpdateMd_Msg"));
const UINT WM_QRYUSER_MSG = ::RegisterWindowMessage(_T("QryUser_Msg"));
const UINT WM_QRYBKYE_MSG = ::RegisterWindowMessage(_T("QryBkYe_Msg"));
const UINT WM_QRYSMI_MSG = ::RegisterWindowMessage(_T("QrySmi_Msg"));

inline COLORREF CmpPriceColor(double d1,double d2)
{
	//double delta= d1-d2;

	COLORREF clorf = WHITE;
	if (d1>d2) { clorf=RED; }
	if (d1<d2) { clorf=GREEN; }

	return clorf;
}

//  ��������:	����ַ���
//  ˵��:������strDelimits����ַ����str������һ��CStringArray ���ö���
//  ��bTrimΪ�棬�򲻱����ָ��Ŀմ�(ע�ⲻ�ǿհ��ַ�)�����磺
//  Tokenize( "a,bc;,d,", ",;", &out_list, TRUE),�᷵��3������a��bc��d 
//	��bTrimΪFALSE������nullSubst��������ָ��Ŀմ������磺
//	Tokenize( "a,bc;,d;", ",;", &out_list, FALSE,"[null]" )	
//  �᷵��5������a��bc��[null]��d��[null]��
//  bTrimĬ��ΪTRUE��nullSubstĬ��Ϊ�մ��� 
inline void  Tokenize(CString str, CString strDelimits, CStringArray& strArray,BOOL bTrim, CString nullSubst)
{
	ASSERT( !str.IsEmpty() && !strDelimits.IsEmpty() );

	strArray.RemoveAll();

	str += strDelimits[0];
	for( long index = -1; (index=str.FindOneOf((LPCTSTR)strDelimits))!=-1; )
	{
		if(index != 0) 
			strArray.Add( str.Left(index) );
		else if(!bTrim) 
			strArray.Add(nullSubst);
		str = str.Right(str.GetLength()-index-1);
	}
}  


inline void getCurTime(CString& strIn)
{
	SYSTEMTIME tm;

	::GetLocalTime(&tm);
	strIn.Format(_T("%02d:%02d:%02d"), tm.wHour, tm.wMinute, tm.wSecond);
}

inline void Fee2String(CString& szIn,double dOpenByM,double dOpenByV,double dCloseByM,
	double dCloseByV,double dClosetByM,double dClosetByV)
{
	CString szOut =_T("��0.0");


	if ((dOpenByV>0) && (dOpenByV==dCloseByV))
	{
		if (dCloseByV==dClosetByV)
		{
			szOut.Format(_T("��%.2f/��"),dOpenByV);
		}
		else if (dClosetByV==0)
		{
			szOut.Format(_T("��ƽ��%.2f/��,ƽ����"),dOpenByV);
		}

	}
	else if ((dOpenByM>0) && (dOpenByM==dCloseByM))
	{
		if (dCloseByM==dClosetByM)
		{
			szOut.Format(_T("%.2f%%%%"),dOpenByM*10000);
		}
		else if ((dOpenByM>0) && (dOpenByM==dCloseByM) && (dClosetByM>0))
		{
			szOut.Format(_T("��ƽ%.2f%%%%,ƽ��%.2f%%%%"),dOpenByM*10000,dClosetByM*10000);
		}
		else
			szOut.Format(_T("��ƽ%.2f%%%%,ƽ����"),dOpenByM*10000);

	}

	szIn = szOut;
}

inline void JgTdStatus(CString& szIn,BYTE bType)
{
	szIn =_T("");

	switch(bType)
	{
	case THOST_FTDC_IS_BeforeTrading:
		szIn = _T("����ǰ");
		break;
	case THOST_FTDC_IS_NoTrading:
		szIn = _T("�ǽ���");
		break;
	case THOST_FTDC_IS_Continous:
		szIn = _T("��������");
		break;
	case THOST_FTDC_IS_AuctionOrdering:
		szIn = _T("���Ͼ��۱���");
		break;
	case THOST_FTDC_IS_AuctionBalance:
		szIn = _T("���Ͼ��ۼ۸�ƽ��");
		break;
	case THOST_FTDC_IS_AuctionMatch:
		szIn = _T("���Ͼ��۴��");
		break;
	case THOST_FTDC_IS_Closed:
		szIn = _T("����");
		break;
	default:
		szIn = _T("δ֪");
		break;
	}

}

inline BOOL SetHighPriority()
{
	if(!SetPriorityClass( GetCurrentProcess(), HIGH_PRIORITY_CLASS))
	{
		return TRUE;
	}
	return FALSE;
}

inline CString FormatLine(CString szItem,CString szValue,CString szFill,int iTotalLen)
{
	CString szOut = _T("");
	if (szFill.IsEmpty()) { return _T("");}

	int iLen = szItem.GetLength()+szValue.GetLength()+szFill.GetLength();
	if (iLen>=iTotalLen) { return _T(""); }

	int iFill=(iTotalLen-3*szItem.GetLength()/2-szValue.GetLength())/(szFill.GetLength());

	szOut += szItem;
	for (int i=0;i<iFill;i++) {szOut += szFill;}
	szOut += szValue;
	szOut += _T("\r\n");

	return szOut;
}

inline int D2Int(double dIn)
{
	int iOut = int(dIn);
	if (dIn>1e-7) iOut = int(dIn+0.5);
	if (dIn<-1e-7) iOut = int(dIn-0.5);

	return iOut;
}
/*
inline char *strsep(char **stringp, const char *delim)
{
char *s;
const char *spanp;
int c, sc;
char *tok;
if ((s = *stringp)== NULL)
return (NULL);
for (tok = s;;) {
c = *s++;
spanp = delim;
do {
if ((sc =*spanp++) == c) {
if (c == 0)
s = NULL;
else
s[-1] = 0;
*stringp = s;
return (tok);
}
} while (sc != 0);
}
// NOTREACHED
}
*/
inline int JudgeDigit(double dTick)
{
	int iRes;
	if (dTick<0.01) {iRes=3;}
	else if (dTick<0.1)
	{iRes=2;}
	else if (dTick<1)
	{iRes=1;}
	else
	{iRes = 0;}

	return iRes;		
}

inline int outStrAs4(CString& szIn)
{
	CString szTemp = szIn;

	int iLen = szTemp.GetLength();
	int iDotPos = szTemp.ReverseFind('.');

	CString szDigEnd=_T("");
	if (iDotPos>0)
	{
		szDigEnd = szTemp.Mid(iDotPos+1,iLen-iDotPos-1);
		szTemp = szTemp.Mid(0,iDotPos);
	}
	szTemp.MakeReverse();

	int iLen2 = szTemp.GetLength();
	int iTimes = iLen2 /4;
	int iLeft = iLen2 %4;

	CString szOut =_T("");
	for (int i=0;i<iTimes;i++)
	{
		szOut += szTemp.Mid(i*4,4);
		if ((i!=(iTimes-1))||(iLeft!=1)||(*(szTemp.GetBuffer(0) + iLen2-1)!='-'))
		{
			szOut += ',';
		}

	}

	if (iLeft>0)
	{
		szOut += szTemp.Mid(iLen2-iLeft,iLeft);
	}
	else
	{
		szOut.TrimRight(',');
	}

	szOut.MakeReverse();
	szIn = szOut;

	if (!szDigEnd.IsEmpty())
	{
		szIn.Format(_T("%s.%s"),szOut,szDigEnd);
	}

	return 0;

}

inline CString JgProType(char bType)
{
	CString szTmp ;

	switch(bType)
	{
	case '1':
		szTmp = _T("�ڻ�");
		break;
	case '2':
		szTmp = _T("��Ȩ");
		break;
	case '3':
		szTmp = _T("���");
		break;
	case '4':
		szTmp = _T("����");
		break;
	case '5':
		szTmp = _T("��ת��");
		break;
	default:
		szTmp=_T("δ֪");
		break;
	}
	return szTmp;
}

inline CString JgCardType(char bType)
{
	CString szTmp;

	switch(bType)
	{
	case THOST_FTDC_ICT_EID:
		szTmp = _T("��֯��������");
		break;
	case THOST_FTDC_ICT_IDCard:
		szTmp = _T("���֤");
		break;
	case THOST_FTDC_ICT_OfficerIDCard:
		szTmp = _T("����֤");
		break;
	case THOST_FTDC_ICT_PoliceIDCard:
		szTmp = _T("����֤");
		break;
	case THOST_FTDC_ICT_SoldierIDCard:
		szTmp = _T("ʿ��֤");
		break;
	case THOST_FTDC_ICT_HouseholdRegister:
		szTmp = _T("���ڲ�");
		break;
	case THOST_FTDC_ICT_Passport:
		szTmp = _T("����");
		break;
	case THOST_FTDC_ICT_TaiwanCompatriotIDCard:
		szTmp = _T("̨��֤");
		break;
	case THOST_FTDC_ICT_HomeComingCard:
		szTmp = _T("����֤");
		break;
	case THOST_FTDC_ICT_LicenseNo:
		szTmp = _T("Ӫҵִ�պ�");
		break;
	case THOST_FTDC_ICT_TaxNo:
		szTmp = _T("˰��ǼǺ�");
		break;
	case THOST_FTDC_ICT_OtherCard:
		szTmp = _T("����֤��");
		break;
	}
	return szTmp;
}

inline CString JgTdType(char bType)
{
	CString szTmp;

	switch(bType)
	{
	case 0:
		szTmp = _T("��ͨ�ɽ�");
		break;
	case 1:
		szTmp = _T("��Ȩִ��");
		break;
	case 2:
		szTmp = _T("OTC�ɽ�");
		break;
	case 3:
		szTmp = _T("��ת�������ɽ�");
		break;
	case 4:
		szTmp = _T("��������ɽ�");
		break;
	default:
		szTmp=_T("δ֪");
		break;
	}
	return szTmp;
}

inline CString JgOcType(char bType)
{
	CString szTmp = _T("δ֪");

	switch(bType)
	{
	case THOST_FTDC_OF_Open:
		szTmp = _T("����");
		break;
	case THOST_FTDC_OF_Close:
		szTmp = _T("ƽ��");
		break;
	case THOST_FTDC_OF_ForceClose:
		szTmp = _T("ǿƽ");
		break;
	case THOST_FTDC_OF_CloseToday:
		szTmp = _T("ƽ��");
		break;
	case THOST_FTDC_OF_CloseYesterday:
		szTmp = _T("ƽ��");
		break;
	case THOST_FTDC_OF_ForceOff:
		szTmp=_T("ǿ��");
		break;
	case THOST_FTDC_OF_LocalForceClose:
		szTmp=_T("����ǿƽ");
		break;
	default:
		szTmp=_T("δ֪");
		break;
	}

	return szTmp;
}

inline CString JgOrdSubmitStat(char bType)
{
	CString szTmp = _T("δ֪");

	switch(bType)
	{
	case THOST_FTDC_OSS_InsertSubmitted:
		szTmp = _T("ί�����ύ");
		break;
	case THOST_FTDC_OSS_CancelSubmitted:
		szTmp = _T("�������ύ");
		break;
	case THOST_FTDC_OSS_Accepted:
		szTmp = _T("�Ѿ�����");
		break;
	case THOST_FTDC_OSS_InsertRejected:
		szTmp = _T("�����ѱ���");
		break;
	case THOST_FTDC_OSS_CancelRejected:
		szTmp = _T("�����ѱ���");
		break;
	case THOST_FTDC_OSS_ModifyRejected:
		szTmp=_T("�ĵ��ѱ���");
		break;
		szTmp=_T("δ֪");
		break;
	}

	return szTmp;
}

inline CString JgOrdStatType(char bType)
{
	CString szTmp = _T("����");

	switch(bType)
	{
	case THOST_FTDC_OST_AllTraded:
		szTmp = _T("ȫ���ɽ�");
		break;
	case THOST_FTDC_OST_PartTradedQueueing:
		szTmp = _T("���ֳɽ�");
		break;
	case THOST_FTDC_OST_PartTradedNotQueueing:
		szTmp = _T("���ֳɽ����ڶ�����");
		break;
	case THOST_FTDC_OST_NoTradeQueueing:
		szTmp = _T("δ�ɽ�");
		break;
	case THOST_FTDC_OST_NoTradeNotQueueing:
		szTmp = _T("δ�ɽ����ڶ�����");
		break;
	case THOST_FTDC_OST_Canceled:
		szTmp=_T("�ѳ���");
		break;
	case THOST_FTDC_OST_Unknown:
		szTmp=_T("δ֪");
		break;
	case THOST_FTDC_OST_NotTouched:
		szTmp=_T("��δ����");
		break;
	case THOST_FTDC_OST_Touched:
		szTmp=_T("�Ѵ���");
		break;
	}

	return szTmp;
}

inline CString JgPosBsType(char bType)
{
	CString szTmp ;

	switch(bType)
	{
	case THOST_FTDC_PD_Net:
		szTmp = _T("��");
		break;
	case THOST_FTDC_PD_Long:
		szTmp = _T("��ͷ");
		break;
	case THOST_FTDC_PD_Short:
		szTmp = _T("��ͷ");
		break;
	default:
		szTmp=_T("δ֪");
		break;
	}
	return szTmp;
}

inline CString JgBsType(char bType)
{
	CString szTmp ;

	switch(bType)
	{
	case THOST_FTDC_D_Buy:
		szTmp = _T("��");
		break;
	case THOST_FTDC_D_Sell:
		szTmp = _T("��");
		break;
	default:
		szTmp=_T("δ֪");
		break;
	}
	return szTmp;
}

inline CString JgTbType(char bType)
{
	CString szTmp ;

	switch(bType)
	{
	case THOST_FTDC_HF_Speculation:
		szTmp = _T("Ͷ��");
		break;
	case THOST_FTDC_HF_Arbitrage:
		szTmp = _T("����");
		break;
	case THOST_FTDC_HF_Hedge:
		szTmp = _T("�ױ�");
		break;
	default:
		szTmp=_T("δ֪");
		break;
	}
	return szTmp;
}

inline CString JgExchage(TThostFtdcExchangeIDType ExID)
{
	CString szTmp = _T("δ֪");
	if (!strcmp(ExID,"CZCE"))
	{
		szTmp = _CZCE;
	}
	if (!strcmp(ExID,"DCE"))
	{
		szTmp = _DCE;
	}
	if (!strcmp(ExID,"SHFE"))
	{
		szTmp = _SHFE;
	}
	if (!strcmp(ExID,"CFFEX"))
	{
		szTmp = _CFFEX;
	}

	return szTmp;
}

inline CString JgBfTdType(TThostFtdcTradeCodeType TdCode)
{
	CString szTmp = _T("δ֪");
	if (!strcmp(TdCode,"202001"))
	{
		szTmp = _T("�����ʽ�ת�ڻ�");
	}
	if (!strcmp(TdCode,"202002"))
	{
		szTmp = _T("�ڻ��ʽ�ת����");
	}
	if (!strcmp(TdCode,"203001"))
	{
		szTmp = _T("��������ת�ڻ�");
	}
	if (!strcmp(TdCode,"203002"))
	{
		szTmp = _T("�����ڻ�ת����");
	}
	if (!strcmp(TdCode,"204002"))
	{
		szTmp = _T("��ѯ�������");
	}
	if (!strcmp(TdCode,"204004"))
	{
		szTmp = _T("��ѯ����ֱͨ��");
	}
	if (!strcmp(TdCode,"204005"))
	{
		szTmp = _T("��ѯת����ϸ");
	}
	if (!strcmp(TdCode,"204006"))
	{
		szTmp = _T("��ѯ����״̬");
	}
	if (!strcmp(TdCode,"204009"))
	{
		szTmp = _T("��ѯ������ˮ");
	}
	if (!strcmp(TdCode,"206001"))
	{
		szTmp = _T("���֪ͨ");
	}
	if (!strcmp(TdCode,"206002"))
	{
		szTmp = _T("����֪ͨ");
	}
	if (!strcmp(TdCode,"901001"))
	{
		szTmp = _T("��ͨ����ֱͨ��");
	}
	if (!strcmp(TdCode,"901002"))
	{
		szTmp = _T("�������ֱͨ��");
	}
	if (!strcmp(TdCode,"905001"))
	{
		szTmp = _T("����ǩ��");
	}
	if (!strcmp(TdCode,"905002"))
	{
		szTmp = _T("����ǩ��");
	}
	if (!strcmp(TdCode,"905003"))
	{
		szTmp = _T("ͬ����Կ");
	}
	return szTmp;
}

inline CString GetSpecFilePath(LPCTSTR lpsz)
{
	CString strPath = __targv[0]; 
	strPath = strPath.Left(strPath.ReverseFind('\\'));
	strPath += '\\';
	strPath += lpsz;
	return strPath;
}


inline CString LoadString(UINT nID)
{
	CString str;
	str.LoadString(nID);
	return str;
}

inline void ShowErroTips(UINT nID1, UINT nID2)
{
	//CDelayMsgBox mbox(g_pCWnd);
	//mbox.MessageBox(LoadString(nID1),LoadString(nID2),1,true,MB_ICONINFORMATION);
}

inline void ShowErroTips(LPCTSTR lpMsg, LPCTSTR lpTitle)
{
	//CDelayMsgBox mbox(g_pCWnd);
	//mbox.MessageBox(lpMsg,lpTitle,1,true,MB_ICONINFORMATION);
}

inline int res2file(LPCTSTR lpName,LPCTSTR lpType,LPCTSTR filename)
{
	HRSRC myres = FindResource (NULL,lpName,lpType);
	HGLOBAL gl = LoadResource (NULL,myres);
	LPVOID lp = LockResource(gl);
	HANDLE fp = CreateFile(filename ,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
	if (!fp)
		return false;

	DWORD a;
	if (!WriteFile (fp,lp,SizeofResource (NULL,myres),&a,NULL))
		return false;

	CloseHandle (fp);
	FreeResource (gl);
	return true;

}

inline int uni2ansi(UINT uCodepage,WCHAR *unicodestr, char *ansistr )
{
	int result = 0;
	try
	{
		int needlen = WideCharToMultiByte( uCodepage, 0, unicodestr, -1, NULL, 0, NULL, NULL );
		if( needlen < 0 )
		{
			return needlen;
		}

		result = WideCharToMultiByte( uCodepage, 0, unicodestr, -1, ansistr, needlen + 1, NULL, NULL );
		if( result < 0 )
		{
			return result;
		}
		return strlen( ansistr );
	}
	catch( ... )
	{
		////ShowErroTips(_T("unicode2ansi ERROR!"),MY_TIPS);
	}
	return result;
}
inline int uni2ansi(UINT uCodepage,CString str, char *ansistr )
{
	int result = 0;
	BSTR unicodestr = str.AllocSysString();
	try
	{
		int needlen = WideCharToMultiByte( uCodepage, 0, unicodestr, -1, NULL, 0, NULL, NULL );
		if( needlen < 0 )
		{
			SysFreeString(unicodestr);// �����ͷ�
			return needlen;
		}

		result = WideCharToMultiByte( uCodepage, 0, unicodestr, -1, ansistr, needlen + 1, NULL, NULL );
		if( result < 0 )
		{
			SysFreeString(unicodestr);// �����ͷ�
			return result;
		}
		SysFreeString(unicodestr);// �����ͷ�
		return strlen( ansistr );
	}
	catch( ... )
	{
		////ShowErroTips(_T("unicode2ansi ERROR!"),MY_TIPS);
	}
	SysFreeString(unicodestr);// �����ͷ�
	return result;
}
inline int ansi2uni(UINT uCodepage, char *ansistr,WCHAR *unicodestr)
{
	int result = 0;
	try
	{
		int needlen = MultiByteToWideChar( uCodepage, 0, ansistr, -1, NULL, 0);
		if( needlen < 0 )
		{
			return needlen;
		}

		result = MultiByteToWideChar( uCodepage, 0, ansistr, -1, unicodestr, needlen + 1);
		if( result < 0 )
		{
			return result;
		}
		return wcslen( unicodestr );
	}
	catch( ... )
	{
		////ShowErroTips(_T("ansi2unicode ERROR!"),MY_TIPS);
	}
	return result;
}
/*
inline void List2Csv(CColorListCtrl* pList,LPCTSTR lpName)
{
CMemFile mmf;
BYTE bBom[3]={0xEF,0xBB,0xBF};
BYTE bDot = ',';
BYTE bEnter = '\n';
mmf.Write(&bBom,3);

int nItem = pList->GetItemCount();
int nColumns = pList->GetHeaderCtrl()->GetItemCount();
HDITEM hd;
TCHAR pItem[MAX_PATH];
char utf8[3*MAX_PATH];


CString str;
int nCol = 0; 
for (nCol = 0; nCol < nColumns; nCol++)
{
memset(pItem,0,_tcslen(pItem));
hd.pszText=pItem;
hd.cchTextMax=MAX_PATH;
hd.mask = HDI_TEXT;
pList->GetHeaderCtrl()->GetItem(nCol, &hd);

str.Format(_T("\"%s\""),pItem);
uni2ansi(CP_UTF8,pItem,utf8);
mmf.Write(utf8,strlen(utf8));
if (nCol < nColumns-1)
{
mmf.Write(&bDot,1);
}
if (nCol == nColumns-1)
{
mmf.Write(&bEnter,1);
}

}

CString str2;
for (int nRow = 0; nRow < nItem; nRow++)
{
for (nCol = 0; nCol < nColumns; nCol++)
{
str2 = pList->GetItemText(nRow,nCol);
str.Format(_T("\"%s\""),(LPCTSTR)str2);
uni2ansi(CP_UTF8,str.GetBuffer(MAX_PATH),utf8);
str.ReleaseBuffer();

mmf.Write(utf8,strlen(utf8));
if (nCol < nColumns-1)
{
mmf.Write(&bDot,1);
}
if (nCol == nColumns-1)
{
mmf.Write(&bEnter,1);
}
}
}

CFile fLog(lpName, CFile::modeReadWrite | CFile::modeCreate | CFile::typeText);

DWORD dwLen = mmf.GetLength();
PBYTE pMemF = mmf.Detach();
fLog.Write(pMemF,dwLen);
fLog.Close();
mmf.Close();

}
*/
inline CString GenDef(LPCTSTR lpPre,LPCTSTR lpExt)
{
	CString szDef,szT;
	SYSTEMTIME tm;

	::GetLocalTime(&tm);
	szT.Format(_T("%s_%04d%02d%02d%02d%02d%02d"), lpPre,tm.wYear,tm.wMonth,tm.wDay,tm.wHour, tm.wMinute, tm.wSecond);

	szDef.Format(_T("%s.%s"),(LPCTSTR)szT,lpExt);
	return szDef;
}
/*
inline BOOL AddSvr2Ar(CStringArray& szArTs,CStringArray& szArMd,CString szTitle)
{
	xml_document doc;
	xml_parse_result result = doc.load_file(SVR_FILE);

	if (result.status == status_ok)
	{
		///////////����������//////////////////
		LPCSTR szSvrRt="//broker/Servers/Server",sNmae="Name",sTrading="Trading",sMData="MarketData";
		char strName[32];
		CString tName,tTrading,tMData;

		szArTs.RemoveAll();
		szArMd.RemoveAll();

		xpath_node_set sVrs = doc.select_nodes(szSvrRt);
		for (xpath_node_set::const_iterator it = sVrs.begin(); it !=  sVrs.end(); ++it)
		{
			xpath_node node = *it;
			strcpy(strName,node.node().child(sNmae).child_value());//
			ansi2uni(CP_UTF8,strName,tName.GetBuffer(MAX_PATH));

			if (tName.Compare(szTitle)==0)
			{
				xml_node tool;

				for (tool = node.node().child(sTrading).first_child(); tool; tool = tool.next_sibling())
				{
					ansi2uni(CP_UTF8,(char*)tool.child_value(),tTrading.GetBuffer(MAX_PATH));
					tTrading.ReleaseBuffer();

					szArTs.Add(tTrading);
				}

				for (tool = node.node().child(sMData).first_child(); tool; tool = tool.next_sibling())
				{
					ansi2uni(CP_UTF8,(char*)tool.child_value(),tMData.GetBuffer(MAX_PATH));
					tMData.ReleaseBuffer();

					szArMd.Add(tMData);
				}

				break;
			}	
		}

	}
	else
		return FALSE;

	return TRUE;
}
*/
inline CString TransTime(TThostFtdcTradeTimeType TdTm)
{
	CString szTm=_T("");
	//ansi2uni(CP_ACP,TdTm,szTm.GetBuffer(MAX_PATH));
	//szTm.ReleaseBuffer();
	return szTm;
}

inline char MapDirection(char src, bool toOrig=true)
{
	if(toOrig)
	{
		if('b'==src||'B'==src)
		{src='0';}
		else if('s'==src||'S'==src)
		{src='1';}
	}
	else
	{
		if('0'==src){src='B';}else if('1'==src){src='S';}
	}
	return src;
}

inline char MapOffset(char src, bool toOrig=true)
{
	if(toOrig)
	{
		if('o'==src||'O'==src){src='0';}
		else if('c'==src||'C'==src){src='1';}
		else if('j'==src||'J'==src) {src='3';}
	}
	else
	{
		if('0'==src){src='O';}
		else if('1'==src){src='C';}
		else if('3'==src){src='J';}
	}
	return src;
}
//��ÿ��ʱ�仯Ϊ������
inline int TThostFtdcTimeTypeToSeconds(TThostFtdcTimeType time)
{
	char hur[2],min[2],sec[2];
	int ihur,imin,isec;
	hur[0] = time[0];
	hur[1] = time[1];
	min[0] = time[3];
	min[1] = time[4];
	sec[0] = time[6];
	sec[1] = time[7];
	ihur = atoi(hur);
	imin = atoi(min);
	isec = atoi(sec);
	return 3600 * ihur + 60 * imin + isec;
}
#endif