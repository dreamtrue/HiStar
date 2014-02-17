#include "StdAfx.h"
#include "traderspi.h"
#include "HiStar.h"
#include "UserMsg.h"
#pragma warning(disable :4996)
extern HANDLE g_hEvent;
BOOL g_bRecconnectT = FALSE;
BOOL g_bLoginCtpT = FALSE;
bool g_bOnceT = FALSE;//����ϵͳ�Ƿ�������½���������½������TRUE,����FALSE
//������ϻָ������� �Զ�����
void CtpTraderSpi::OnFrontConnected()
{
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	if (g_bOnceT)
	{
		g_bRecconnectT = TRUE;
		ReqUserLogin(pApp->m_accountCtp.m_sBROKER_ID,pApp->m_accountCtp.m_sINVESTOR_ID,pApp->m_accountCtp.m_sPASSWORD);
		SYSTEMTIME curTime;
		::GetLocalTime(&curTime);
		CString	szT;
		szT.Format(_T("%02d:%02d:%02d CTP�ص�¼"), curTime.wHour, curTime.wMinute, curTime.wSecond);	
	}	
	else
	{//��һ�ε�½
		ReqUserLogin(pApp->m_accountCtp.m_sBROKER_ID,pApp->m_accountCtp.m_sINVESTOR_ID,pApp->m_accountCtp.m_sPASSWORD);
		g_bOnceT = true;
	}
}

void CtpTraderSpi::ReqUserLogin(TThostFtdcBrokerIDType	vAppId,TThostFtdcUserIDType	vUserId,TThostFtdcPasswordType	vPasswd)
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, vAppId); strcpy(BROKER_ID, vAppId); 
	strcpy(req.UserID, vUserId);  strcpy(INVEST_ID, vUserId); 
	strcpy(req.Password, vPasswd);
	strcpy(req.UserProductInfo,PROD_INFO);
	int iRet = pUserApi->ReqUserLogin(&req, ++m_iRequestID);
}

#define TIME_NULL "--:--:--"

void CtpTraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
		if(pRspInfo){memcpy(&m_RspMsg,pRspInfo,sizeof(CThostFtdcRspInfoField));}
		if(!IsErrorRspInfo(pRspInfo) && pRspUserLogin){ 
			g_bLoginCtpT = true;
			// ����Ự����	
			m_ifrontId = pRspUserLogin->FrontID;
			m_isessionId = pRspUserLogin->SessionID;

			strcpy(m_sTdday,pRspUserLogin->TradingDay);

			int nextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
			sprintf(m_sOrdRef, "%d", ++nextOrderRef);

			SYSTEMTIME curTime;
			::GetLocalTime(&curTime);
			CTime tc(curTime);
			int i=0;
			int iHour[4],iMin[4],iSec[4];
			if (!strcmp(pRspUserLogin->DCETime,TIME_NULL) || !strcmp(pRspUserLogin->SHFETime,TIME_NULL)){
				for (i=0;i<4;i++){
					iHour[i]=curTime.wHour;
					iMin[i]=curTime.wMinute;
					iSec[i]=curTime.wSecond;
				}
			}
			else{
				sscanf(pRspUserLogin->SHFETime, "%d:%d:%d", &iHour[0], &iMin[0], &iSec[0]);
				sscanf(pRspUserLogin->DCETime, "%d:%d:%d", &iHour[1], &iMin[1], &iSec[1]);
				sscanf(pRspUserLogin->CZCETime, "%d:%d:%d", &iHour[2], &iMin[2], &iSec[2]);
				sscanf(pRspUserLogin->FFEXTime, "%d:%d:%d", &iHour[3], &iMin[3], &iSec[3]);
			}
			CTime t[4];
			for (i=0;i<4;i++){
				t[i] = CTime(curTime.wYear,curTime.wMonth,curTime.wDay,iHour[i],iMin[i],iSec[i]);
				m_tsEXnLocal[i] = t[i]-tc;
			}
			sprintf(m_sTmBegin,"%02d:%02d:%02d.%03d",curTime.wHour,curTime.wMinute,curTime.wSecond,curTime.wMilliseconds); 
		}
		if(bIsLast) SetEvent(g_hEvent);
}

const char* CtpTraderSpi::GetTradingDay()
{
	return m_sTdday;
}

void CtpTraderSpi::ReqUserLogout()
{
	CThostFtdcUserLogoutField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.UserID, INVEST_ID);
	int iResult = pUserApi->ReqUserLogout(&req, ++m_iRequestID);
	if(iResult == 0){
		TRACE("ctp����ϵͳ�ǳ�ָ��ͳɹ�\r\n");
	}
	else{
		TRACE("ctp����ϵͳ�ǳ�ָ���ʧ��\r\n");
	}
}

///�ǳ�������Ӧ
void CtpTraderSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pUserLogout){
		TRACE(_T("�Ѿ��ǳ�ctpT\n"));
		g_bLoginCtpT = FALSE;
	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqSettlementInfoConfirm()
{
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	pUserApi->ReqSettlementInfoConfirm(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspSettlementInfoConfirm(
	CThostFtdcSettlementInfoConfirmField  *pSettlementInfoConfirm, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{	
	if( !IsErrorRspInfo(pRspInfo) && pSettlementInfoConfirm){
	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQryNotice()
{
	CThostFtdcQryNoticeField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	pUserApi->ReqQryNotice(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspQryNotice(CThostFtdcNoticeField *pNotice, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) &&  pNotice){}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQryTdNotice()
{
	CThostFtdcQryTradingNoticeField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	pUserApi->ReqQryTradingNotice(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspQryTradingNotice(CThostFtdcTradingNoticeField *pTradingNotice, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) &&  pTradingNotice){}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQrySettlementInfoConfirm()
{
	CThostFtdcQrySettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	pUserApi->ReqQrySettlementInfoConfirm(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) &&  pSettlementInfoConfirm){}	
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQrySettlementInfo(TThostFtdcDateType TradingDay)
{
	CThostFtdcQrySettlementInfoField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	strcpy(req.TradingDay,TradingDay);

	pUserApi->ReqQrySettlementInfo(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) &&  pSettlementInfo)
	{
		CThostFtdcSettlementInfoField* pSi = new CThostFtdcSettlementInfoField();
		memcpy(pSi,pSettlementInfo,sizeof(CThostFtdcSettlementInfoField));
		m_StmiVec.push_back(pSi);
	}
	if(bIsLast) 
	{ 
		SetEvent(g_hEvent);
		//SendNotifyMessage(HWND_BROADCAST,WM_QRYSMI_MSG,0,0);		
	}
}

void CtpTraderSpi::ReqQryInst(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	if (instId != NULL)
	{ strcpy(req.InstrumentID, instId); }

	pUserApi->ReqQryInstrument(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{ 
	if ( !IsErrorRspInfo(pRspInfo) &&  pInstrument)
	{
		PINSINFEX pInsInf = new INSINFEX;
		ZeroMemory(pInsInf,sizeof(INSINFEX));
		memcpy(pInsInf,  pInstrument, sizeof(INSTINFO));
		m_InsinfVec.push_back(pInsInf);
	}
	if(bIsLast){
		TRACE(_T("��Լ��ѯ���\n"));
		SetEvent(g_hEvent);
	}
}

void CtpTraderSpi::ReqQryTdAcc()
{
	CThostFtdcQryTradingAccountField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	pUserApi->ReqQryTradingAccount(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspQryTradingAccount(
	CThostFtdcTradingAccountField *pTradingAccount, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{ 
	if (!IsErrorRspInfo(pRspInfo) &&  pTradingAccount)
	{
		CThostFtdcTradingAccountField* pAcc = new CThostFtdcTradingAccountField();
		memcpy(pAcc,pTradingAccount,sizeof(CThostFtdcTradingAccountField));
		m_TdAcc = *pAcc;
		if(AfxGetApp()->m_pMainWnd->m_hWnd){
			PostMessageA(AfxGetApp()->m_pMainWnd->m_hWnd,WM_UPDATE_ACC_CTP,NULL,(LPARAM)pAcc);
		}
	}
	if(bIsLast) SetEvent(g_hEvent);
}

//INSTRUMENT_ID��ɲ����ֶ�,����IF10,���ܲ������IF10��ͷ��ͷ��
void CtpTraderSpi::ReqQryInvPos(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInvestorPositionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	if (instId!=NULL)
	{strcpy(req.InstrumentID, instId);}		
	pUserApi->ReqQryInvestorPosition(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspQryInvestorPosition(
	CThostFtdcInvestorPositionField *pInvestorPosition, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{ 
	if( !IsErrorRspInfo(pRspInfo) &&  pInvestorPosition )
	{
		CThostFtdcInvestorPositionField* pInvPos = new CThostFtdcInvestorPositionField();
		memcpy(pInvPos,  pInvestorPosition, sizeof(CThostFtdcInvestorPositionField));
		m_InvPosVec.push_back(pInvPos);
	}
	if(bIsLast) SetEvent(g_hEvent);	
}

//INSTRUMENT_ID��ɲ����ֶ�,����IF10,���ܲ������IF10��ͷ��ͷ��
void CtpTraderSpi::ReqQryInvPosEx(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInvestorPositionDetailField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	if (instId!=NULL)
	{strcpy(req.InstrumentID, instId);}		
	pUserApi->ReqQryInvestorPositionDetail(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) &&  pInvestorPositionDetail)
	{

	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQryInvPosCombEx(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInvestorPositionCombineDetailField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID);
	if (instId!=NULL)
	{strcpy(req.CombInstrumentID, instId);}		
	pUserApi->ReqQryInvestorPositionCombineDetail(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspQryInvestorPositionCombineDetail(CThostFtdcInvestorPositionCombineDetailField *pInvestorPositionDetail, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) &&  pInvestorPositionDetail)
	{

	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqOrdLimit(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir,
	TThostFtdcCombOffsetFlagType kpp,TThostFtdcPriceType price,   TThostFtdcVolumeType vol)
{
	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));	
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID); 
	strcpy(req.InstrumentID, instId); 	
	strcpy(req.OrderRef, m_sOrdRef);
	int nextOrderRef = atoi(m_sOrdRef);
	sprintf(m_sOrdRef, "%d", ++nextOrderRef);

	req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;//�۸�����=�޼�	
	req.Direction = MapDirection(dir,true);  //��������	
	req.CombOffsetFlag[0] = MapOffset(kpp[0],true); //��Ͽ�ƽ��־:����
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;	  //���Ͷ���ױ���־
	req.LimitPrice = price;	//�۸�
	req.VolumeTotalOriginal = vol;	///����	
	req.TimeCondition = THOST_FTDC_TC_GFD;  //��Ч������:������Ч
	req.VolumeCondition = THOST_FTDC_VC_AV; //�ɽ�������:�κ�����
	req.MinVolume = 1;	//��С�ɽ���:1	
	req.ContingentCondition = THOST_FTDC_CC_Immediately;  //��������:����

	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;	//ǿƽԭ��:��ǿƽ	
	req.IsAutoSuspend = 0;  //�Զ������־:��	
	req.UserForceClose = 0;   //�û�ǿ����־:��

	pUserApi->ReqOrderInsert(&req, ++m_iRequestID);

}

///�м۵�
void CtpTraderSpi::ReqOrdAny(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp,TThostFtdcVolumeType vol)
{
	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));	
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID); 
	strcpy(req.InstrumentID, instId); 	
	strcpy(req.OrderRef, m_sOrdRef);
	int nextOrderRef = atoi(m_sOrdRef);
	sprintf(m_sOrdRef, "%d", ++nextOrderRef);

	req.OrderPriceType = THOST_FTDC_OPT_AnyPrice;//�м�
	req.Direction = MapDirection(dir,true);  //��������	
	req.CombOffsetFlag[0] = MapOffset(kpp[0],true); //��Ͽ�ƽ��־:����
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;	  //���Ͷ���ױ���־
	//req.LimitPrice = price;	//�۸�
	req.VolumeTotalOriginal = vol;	///����	
	req.TimeCondition = THOST_FTDC_TC_IOC;;  //������Ч
	req.VolumeCondition = THOST_FTDC_VC_AV; //�ɽ�������:�κ�����
	req.MinVolume = 1;	//��С�ɽ���:1	
	req.ContingentCondition = THOST_FTDC_CC_Immediately;  //��������:����

	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;	//ǿƽԭ��:��ǿƽ	
	req.IsAutoSuspend = 0;  //�Զ������־:��	
	req.UserForceClose = 0;   //�û�ǿ����־:��

	pUserApi->ReqOrderInsert(&req, ++m_iRequestID);	
}

void CtpTraderSpi::ReqOrdCondition(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp,
	TThostFtdcPriceType price,TThostFtdcVolumeType vol,TThostFtdcPriceType stopPrice,TThostFtdcContingentConditionType conType)
{
	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));	
	strcpy(req.BrokerID, BROKER_ID);  //���͹�˾����	
	strcpy(req.InvestorID, INVEST_ID); //Ͷ���ߴ���	
	strcpy(req.InstrumentID, instId); //��Լ����	
	strcpy(req.OrderRef, m_sOrdRef);  //��������
	int nextOrderRef = atoi(m_sOrdRef);
	sprintf(m_sOrdRef, "%d", ++nextOrderRef);

	req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;	
	req.Direction = MapDirection(dir,true);  //��������	
	req.CombOffsetFlag[0] = MapOffset(kpp[0],true); //��Ͽ�ƽ��־:����
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	req.LimitPrice = price;	//�۸�
	req.VolumeTotalOriginal = vol;	///����	
	req.TimeCondition = THOST_FTDC_TC_GFD;  //������Ч
	req.VolumeCondition = THOST_FTDC_VC_AV; //�ɽ�������:�κ�����
	req.MinVolume = 1;	//��С�ɽ���:1	
	req.ContingentCondition = conType;  //��������

	req.StopPrice = stopPrice;  //ֹ���
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;	//ǿƽԭ��:��ǿƽ	
	req.IsAutoSuspend = 0;  //�Զ������־:��	
	req.UserForceClose = 0;   //�û�ǿ����־:��

	pUserApi->ReqOrderInsert(&req, ++m_iRequestID);
}

/*
FAK(Fill And Kill)ָ����ǽ���������Ч����ΪTHOST_FTDC_TC_IOC,ͬʱ,�ɽ���������ΪTHOST_FTDC_VC_AV,����������;
FOK(Fill Or Kill)ָ���ǽ���������Ч����������ΪTHOST_FTDC_TC_IOC,ͬʱ���ɽ�����������ΪTHOST_FTDC_VC_CV,��ȫ������.
����,��FAKָ����,����ָ����С�ɽ���,����ָ����λ��������С�ɽ��������ϳɽ�,ʣ�ඩ����ϵͳ����,����ϵͳȫ������.����״����,
��Ч����������ΪTHOST_FTDC_TC_IOC,����������ΪTHOST_FTDC_VC_MV,ͬʱ�趨MinVolume�ֶ�.
*/
void CtpTraderSpi::ReqOrdFAOK(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir,TThostFtdcCombOffsetFlagType kpp,
	TThostFtdcPriceType price,/*TThostFtdcVolumeType vol,*/TThostFtdcVolumeConditionType volconType,TThostFtdcVolumeType minVol)
{
	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));	
	strcpy(req.BrokerID, BROKER_ID);
	strcpy(req.InvestorID, INVEST_ID); 
	strcpy(req.InstrumentID, instId); 	
	strcpy(req.OrderRef, m_sOrdRef);
	int nextOrderRef = atoi(m_sOrdRef);
	sprintf(m_sOrdRef, "%d", ++nextOrderRef);

	req.OrderPriceType = THOST_FTDC_OPT_LimitPrice; //�޼�
	req.Direction = MapDirection(dir,true);  //��������	
	req.CombOffsetFlag[0] = MapOffset(kpp[0],true); //��Ͽ�ƽ��־
	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;	//���Ͷ���ױ���־
	req.LimitPrice = price;	//�۸�
	//req.VolumeTotalOriginal = vol;	///����	
	req.TimeCondition = THOST_FTDC_TC_IOC;  //����
	req.VolumeCondition = volconType; //THOST_FTDC_VC_AV,THOST_FTDC_VC_MV;THOST_FTDC_VC_CV
	req.MinVolume = minVol;	//FAK��THOST_FTDC_VC_MVʱ���ָ��MinVol,���������0
	req.ContingentCondition = THOST_FTDC_CC_Immediately;  //��������:����

	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;	//ǿƽԭ��:��ǿƽ	
	req.IsAutoSuspend = 0;  //�Զ������־:��	
	req.UserForceClose = 0;   //�û�ǿ����־:��

	pUserApi->ReqOrderInsert(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( IsErrorRspInfo(pRspInfo) || (pInputOrder==NULL) )
	{
		TCHAR szErr[MAX_PATH];
		//ansi2uni(CP_ACP,pRspInfo->ErrorMsg,szErr);

		/////////////////////////////////////////////////////////////////////
		CString szItems[ORDER_ITMES],szStat;

		szItems[0].Empty();
		//ansi2uni(CP_ACP,pInputOrder->InstrumentID,szItems[1].GetBuffer(MAX_PATH));
		szItems[2]=JgBsType(pInputOrder->Direction);
		szItems[3]=JgOcType(pInputOrder->CombOffsetFlag[0]);

		szItems[4]=_GERR;

		szItems[5].Format(_T("%f"),pInputOrder->LimitPrice);
		szItems[5].TrimRight('0');
		int iLen = szItems[5].GetLength();
		if (szItems[5].Mid(iLen-1,1)==_T(".")) {szItems[5].TrimRight(_T("."));}

		szItems[6].Format(_T("%d"),pInputOrder->VolumeTotalOriginal);

		////////////////////////////////////////////////////////
		szStat.Format(_T("ʧ��:%s,%s,%s"),szItems[1],szItems[2],szItems[3]);

		szItems[7].Format(_T("%d"),pInputOrder->VolumeTotalOriginal);
		szItems[8] = _T("0");


		szItems[9] = szItems[5];
		getCurTime(szItems[10]);
		szItems[11] =  _T("�D");
		szItems[12] =  _T("0");
		szItems[13] =  _T("0");
		szItems[14]= szErr;
	}
	if(bIsLast) SetEvent(g_hEvent);	
}

void CtpTraderSpi::ReqOrderCancel(TThostFtdcSequenceNoType orderSeq)
{
	bool found=false; UINT i=0;
	for(i=0;i<m_orderVec.size();i++){
		if(m_orderVec[i]->BrokerOrderSeq == orderSeq){ found = true; break;}
	}
	if(!found)
	{
		////////�����ѱ��ɽ��򲻴���///////////
		return;
	} 

	CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	
	strcpy(req.InvestorID, INVEST_ID); //Ͷ���ߴ���
	strcpy(req.ExchangeID, m_orderVec[i]->ExchangeID);
	strcpy(req.OrderSysID, m_orderVec[i]->OrderSysID);
	req.ActionFlag = THOST_FTDC_AF_Delete;  //������־ 

	pUserApi->ReqOrderAction(&req, ++m_iRequestID);
	//cerr<< " ���� | ���ͳ���..." <<((ret == 0)?"�ɹ�":"ʧ��") << endl;
}

void CtpTraderSpi::ReqOrderCancel(TThostFtdcInstrumentIDType instId,TThostFtdcOrderRefType OrderRef)
{
	// FrontID + SessionID + OrdRef + InstrumentID
	CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	
	strcpy(req.InvestorID, INVEST_ID); //Ͷ���ߴ���
	strcpy(req.InstrumentID, instId); //��Լ����
	strcpy(req.OrderRef, m_sOrdRef); //��������	
	req.FrontID = m_ifrontId;           //ǰ�ñ��	
	req.SessionID = m_isessionId;       //�Ự���

	req.ActionFlag = THOST_FTDC_AF_Delete;  //������־ 

	pUserApi->ReqOrderAction(&req, ++m_iRequestID);
	//cerr<< " ���� | ���ͳ���..." <<((ret == 0)?"�ɹ�":"ʧ��") << endl;
}

void CtpTraderSpi::OnRspOrderAction(
	CThostFtdcInputOrderActionField *pInputOrderAction, 
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{	
	if (IsErrorRspInfo(pRspInfo) || (pInputOrderAction==NULL))
	{
		if (true)
		{
			//////////////////////////////////////////
			//pDlg->SetStatusTxt(_T("����ʧ��!"),2);
		}
	}
	if(bIsLast) SetEvent(g_hEvent);	
}

///�����ر�
void CtpTraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder){	
	CThostFtdcOrderField* order = new CThostFtdcOrderField();
	memcpy(order,  pOrder, sizeof(CThostFtdcOrderField));
	bool founded = false;UINT i = 0;
	for(i = 0;i<m_orderVec.size();i++){
		if(m_orderVec[i]->BrokerOrderSeq == order->BrokerOrderSeq) { 
			founded = true;
			break;
		}
	}
	//////�޸��ѷ����ı���״̬
	if(founded){
		m_orderVec[i] = order; 
		if (!g_bRecconnectT){
			///////////////////////////ˢ�¹ҵ��б�/////////////////////
			//δ����ҵ��б�
			if (true){
				//�����б�
				if (order->OrderStatus == '1' || order->OrderStatus == '3'){
				}
			}
			else{
				//�Ѿ����б� ����� ��ɾ��
				if (order->OrderStatus != '1' && order->OrderStatus != '3'){
				}
				else{
				//���ڵĹҵ� �޸�״̬
				}
			}
			/////////////////////////ˢ�³ֲ�////////////////////////////////
			bool bExist = false;
			for(i = 0;i<m_InvPosVec.size();i++){
				if (!strcmp(order->InstrumentID,m_InvPosVec[i]->InstrumentID)){ 
					bExist = true; 
					break;
				}	
			}
			if (bExist){
			}
			else{
			}
	     /////////////////////////////////////////////////////////////////
		}
	} 
	///////������ί�е�
	else{
		m_orderVec.push_back(order);
		if (true){
			//////////////////////////////////////////////////////////
		}
	}
	SetEvent(g_hEvent);
}

///�ɽ�֪ͨ
void CtpTraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade){
	CHiStarApp* pApp = (CHiStarApp*)AfxGetApp();
	CThostFtdcTradeField* trade = new CThostFtdcTradeField();
	memcpy(trade,pTrade,sizeof(CThostFtdcTradeField));
	bool founded = false;     
	unsigned int i = 0;
	for(i = 0;i<m_tradeVec.size();i++){
		//strcmp����0ʱ��ʾ���
		if(strcmp(m_tradeVec[i]->TradeID,trade->TradeID) == 0){
			founded = true;   
			break;
		}
	}
	//////�޸ĳɽ���״̬
	if(founded){
		TRACE(_T("�޸ĳɽ���״̬"));
		//���������¸��ǳɽ���Ϣ
		m_tradeVec[i]= trade; 
		if (!g_bRecconnectT){
			//������������Ҫ��ʾ�ĳɽ���Ϣ,������������ȡ������ʾ��
		} 
	}
	///////�������ѳɽ��� 
	else 
	{
		m_tradeVec.push_back(trade);
		if (true){
			/////////////////////////ˢ�³ֲ�////////////////////////////////
			bool bExist = false;//�Ƿ���ڸóֲ�
			for(i = 0;i<m_InvPosVec.size();i++)
			{
				if (strcmp(trade->InstrumentID,m_InvPosVec[i]->InstrumentID) == 0){ 
					bExist = true; 
					break;
				}	
			}
			CThostFtdcInvestorPositionField* newInvPos = new CThostFtdcInvestorPositionField();
			ZeroMemory(newInvPos,sizeof(CThostFtdcInvestorPositionField));
			if (bExist){
				if (trade->Direction == THOST_FTDC_D_Buy){
					switch(trade->OffsetFlag){
					case THOST_FTDC_OF_Open:
						break;
					case THOST_FTDC_OF_Close:
						break;
					case THOST_FTDC_OF_CloseToday:
						break;
					case THOST_FTDC_OF_CloseYesterday:
						break;
					case THOST_FTDC_OF_ForceOff:
						break;
					case THOST_FTDC_OF_LocalForceClose:
						break;			
					}
				}
				if (trade->Direction == THOST_FTDC_D_Sell)
				{
					switch(trade->OffsetFlag){
					case THOST_FTDC_OF_Open:
						break;
					case THOST_FTDC_OF_Close:
						break;
					case THOST_FTDC_OF_CloseToday:
						break;
					case THOST_FTDC_OF_CloseYesterday:
						break;
					case THOST_FTDC_OF_ForceOff:
						break;
					case THOST_FTDC_OF_LocalForceClose:
						break;			
					}
				}
			}
			else{
				int iMul = pApp->FindInstMul(trade->InstrumentID);
				strcpy(newInvPos->InstrumentID,trade->InstrumentID);
				strcpy(newInvPos->BrokerID,trade->BrokerID);
				strcpy(newInvPos->InvestorID,trade->InvestorID);
				newInvPos->PosiDirection = trade->Direction + 2;
				newInvPos->HedgeFlag = trade->HedgeFlag;
				newInvPos->PositionDate = (strcmp(m_sTdday,trade->TradeDate)==0)?THOST_FTDC_PSD_Today:THOST_FTDC_PSD_History;
				newInvPos->Position = trade->Volume;
				newInvPos->OpenVolume = trade->Volume;
				newInvPos->OpenAmount = trade->Volume * trade->Price * iMul;
				newInvPos->PositionCost = trade->Volume * trade->Price * iMul;
			}
			/////////////////////////////////////////////////////////////////
		}
	}
	SetEvent(g_hEvent);
}

void CtpTraderSpi::OnFrontDisconnected(int nReason){
	if (true){
		g_bLoginCtpT = FALSE;
		SYSTEMTIME curTime;
		::GetLocalTime(&curTime);
		CString	szT;
		szT.Format(_T("%02d:%02d:%02d CTP�жϵȴ�����"), curTime.wHour, curTime.wMinute, curTime.wSecond);
		//ShowErroTips(IDS_DISCONTIPS,IDS_STRTIPS);
	}
}
void CtpTraderSpi::OnHeartBeatWarning(int nTimeLapse){
	TRACE(_T("OnHeartBeatWarningT\n"));
}
///�����ѯ���ױ���
void CtpTraderSpi::ReqQryTradingCode()
{
	CThostFtdcQryTradingCodeField req;
	memset(&req, 0, sizeof(req));
	req.ClientIDType = THOST_FTDC_CIDT_Speculation;
	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	
	strcpy(req.InvestorID, INVEST_ID); //Ͷ���ߴ���
	pUserApi->ReqQryTradingCode(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQryTradingCode(CThostFtdcTradingCodeField *pTradingCode, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pTradingCode ){
		CThostFtdcTradingCodeField* pTdCode = new CThostFtdcTradingCodeField();
		memcpy(pTdCode, pTradingCode, sizeof(CThostFtdcTradingCodeField));
		m_TdCodeVec.push_back(pTdCode);
	}
	if(bIsLast) SetEvent(g_hEvent);
}

///�����ѯ��Լ��֤����
void CtpTraderSpi::ReqQryInstMgr(TThostFtdcInstrumentIDType instId){
	CThostFtdcQryInstrumentMarginRateField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	
	strcpy(req.InvestorID, INVEST_ID); //Ͷ���ߴ���
	if (instId!=NULL)
	{strcpy(req.InstrumentID, instId);}	
	req.HedgeFlag = '1';
	pUserApi->ReqQryInstrumentMarginRate(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pInstrumentMarginRate )
	{

		CThostFtdcInstrumentMarginRateField* pMaginRate = new CThostFtdcInstrumentMarginRateField();
		memcpy(pMaginRate,  pInstrumentMarginRate, sizeof(CThostFtdcInstrumentMarginRateField));
		m_MargRateVec.push_back(pMaginRate);
	}
	if(bIsLast) SetEvent(g_hEvent);

}

///�����ѯ��Լ��������
void CtpTraderSpi::ReqQryInstFee(TThostFtdcInstrumentIDType instId)
{
	CThostFtdcQryInstrumentCommissionRateField req;

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	
	strcpy(req.InvestorID, INVEST_ID); //Ͷ���ߴ���
	if (instId!=NULL)
	{strcpy(req.InstrumentID, instId);}	
	pUserApi->ReqQryInstrumentCommissionRate(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pInstrumentCommissionRate )
	{
		//CThostFtdcInstrumentCommissionRateField* pFeeRate = new CThostFtdcInstrumentCommissionRateField();
		memcpy(&m_FeeRateRev,  pInstrumentCommissionRate, sizeof(CThostFtdcInstrumentCommissionRateField)); 
		//FeeRateList.push_back(pFeeRate);

	}
	if(bIsLast) SetEvent(g_hEvent);

}

//////////////////�����ѯ�û�����/////////////
void CtpTraderSpi::ReqQryInvestor()
{
	CThostFtdcQryInvestorField req;

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	
	strcpy(req.InvestorID, INVEST_ID); //Ͷ���ߴ���

	pUserApi->ReqQryInvestor(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspQryInvestor(CThostFtdcInvestorField *pInvestor, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pInvestor )
	{
		CThostFtdcInvestorField *pInv = new CThostFtdcInvestorField;
		memcpy(pInv,pInvestor,sizeof(CThostFtdcInvestorField));

		//SendNotifyMessage(((CXTraderDlg*)g_pCWnd)->m_hWnd,WM_QRYUSER_MSG,0,(LPARAM)pInv);
	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqUserPwdUpdate(TThostFtdcPasswordType szNewPass,TThostFtdcPasswordType szOldPass)
{
	CThostFtdcUserPasswordUpdateField req;

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	
	strcpy(req.UserID, INVEST_ID); //�û�����
	strcpy(req.NewPassword, szNewPass);    	
	strcpy(req.OldPassword,szOldPass);  

	pUserApi->ReqUserPasswordUpdate(&req,++m_iRequestID);
}

///�û��������������Ӧ
void CtpTraderSpi::OnRspUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pUserPasswordUpdate )
	{
		//ShowErroTips(IDS_MODPASSOK,IDS_STRTIPS);
	}
	else
	{
		ShowErrTips(pRspInfo->ErrorMsg);
	}
	if(bIsLast) SetEvent(g_hEvent);
}

//�ʽ��˻�����
void CtpTraderSpi::ReqTdAccPwdUpdate(TThostFtdcPasswordType szNewPass,TThostFtdcPasswordType szOldPass)
{
	CThostFtdcTradingAccountPasswordUpdateField req;

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	
	strcpy(req.AccountID, INVEST_ID); //�û�����
	strcpy(req.NewPassword, szNewPass);    	
	strcpy(req.OldPassword,szOldPass);  

	pUserApi->ReqTradingAccountPasswordUpdate(&req,++m_iRequestID);
}

///�ʽ��˻��������������Ӧ
void CtpTraderSpi::OnRspTradingAccountPasswordUpdate(CThostFtdcTradingAccountPasswordUpdateField *pTradingAccountPasswordUpdate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pTradingAccountPasswordUpdate )
	{
		//ShowErroTips(IDS_MODPASSOK,IDS_STRTIPS);
	}
	else
	{
		ShowErrTips(pRspInfo->ErrorMsg);
	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqAuthenticate(TThostFtdcProductInfoType UserProdInf,TThostFtdcAuthCodeType	AuthCode)
{
	CThostFtdcReqAuthenticateField req;

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	
	strcpy(req.UserID, INVEST_ID); //�û�����
	strcpy(req.UserProductInfo, UserProdInf);    	
	strcpy(req.AuthCode,AuthCode);  

	pUserApi->ReqAuthenticate(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pRspAuthenticateField )
	{
		//memcpy(&g_InvInf,pInvestor,sizeof(CThostFtdcInvestorField));
		//AfxMessageBox(tName);
	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQryAccreg()
{
	CThostFtdcQryAccountregisterField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	
	strcpy(req.AccountID, INVEST_ID); //�û�����

	pUserApi->ReqQryAccountregister(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQryAccountregister(CThostFtdcAccountregisterField *pAccountregister, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pAccountregister)
	{
		CThostFtdcAccountregisterField* pAccReg = new CThostFtdcAccountregisterField();
		memcpy(pAccReg,  pAccountregister, sizeof(CThostFtdcAccountregisterField));
		m_AccRegVec.push_back(pAccReg);

	}
	if(bIsLast) SetEvent(g_hEvent);
}


void CtpTraderSpi::ReqQryTransBk(TThostFtdcBankIDType BankID,TThostFtdcBankBrchIDType BankBrchID)
{
	CThostFtdcQryTransferBankField req;
	memset(&req, 0, sizeof(req));
	if(BankID != NULL)
		strcpy(req.BankID,BankID);
	if(BankBrchID != NULL)
		strcpy(req.BankBrchID,BankBrchID);

	pUserApi->ReqQryTransferBank(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQryTransferBank(CThostFtdcTransferBankField *pTransferBank, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pTransferBank)
	{
		//memcpy(&g_InvInf,pInvestor,sizeof(CThostFtdcInvestorField));
		//AfxMessageBox(tName);
	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQryContractBk(TThostFtdcBankIDType BankID,TThostFtdcBankBrchIDType BankBrchID)
{
	CThostFtdcQryContractBankField req;

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	

	if(BankID != NULL)
		strcpy(req.BankID,BankID);
	if(BankBrchID != NULL)
		strcpy(req.BankBrchID,BankBrchID);

	pUserApi->ReqQryContractBank(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQryContractBank(CThostFtdcContractBankField *pContractBank, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pContractBank)
	{
		//TCHAR szBankID[20],szBankBrchID[20],szBankName[MAX_PATH];
		//ansi2uni(CP_ACP,pContractBank->BankID,szBankID);
		//ansi2uni(CP_ACP,pContractBank->BankBrchID,szBankBrchID);
		//ansi2uni(CP_ACP,pContractBank->BankName,szBankName);

		//CString str;
		//str.Format(_T("BankID: %s; BankBrchID: %s; BankName: %s"),szBankID,szBankBrchID,szBankName);
		//memcpy(&g_InvInf,pInvestor,sizeof(CThostFtdcInvestorField));
		//AfxMessageBox(str);
	}
	if(bIsLast) SetEvent(g_hEvent);
}

//////////////////////////////////////////�ڻ����������ʽ�ת�ڻ�����///////////////////////////////////////
void CtpTraderSpi::ReqBk2FByF(TThostFtdcBankIDType BkID,TThostFtdcPasswordType BkPwd,
	TThostFtdcPasswordType Pwd,TThostFtdcTradeAmountType TdAmt)
{
	CThostFtdcReqTransferField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	 
	strcpy(req.AccountID, INVEST_ID); //�û�����
	strcpy(req.TradeCode,"202001");
	strcpy(req.BankBranchID,"0000");
	strcpy(req.CurrencyID,"RMB");
	strcpy(req.BankID,BkID);
	strcpy(req.BankPassWord,BkPwd);
	strcpy(req.Password,Pwd);
	req.TradeAmount=TdAmt;
	req.SecuPwdFlag = THOST_FTDC_BPWDF_BlankCheck;

	pUserApi->ReqFromBankToFutureByFuture(&req,++m_iRequestID);
}

///�ڻ����������ʽ�ת�ڻ�Ӧ��
void CtpTraderSpi::OnRspFromBankToFutureByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	//AfxMessageBox(_T("OnRspFromBankToFutureByFuture"));
	if( IsErrorRspInfo(pRspInfo) || (pReqTransfer==NULL))
	{

		ShowErrTips(pRspInfo->ErrorMsg);
	}
	if(bIsLast) SetEvent(g_hEvent);	 
}
///�ڻ����������ʽ�ת�ڻ�֪ͨ
void CtpTraderSpi::OnRtnFromBankToFutureByFuture(CThostFtdcRspTransferField *pRspTransfer)
{
	CThostFtdcRspTransferField* bfTrans = new CThostFtdcRspTransferField();
	memcpy(bfTrans,  pRspTransfer, sizeof(CThostFtdcRspTransferField));
	bool founded=false;    UINT i=0;
	for(i=0; i<m_BfTransVec.size(); i++)
	{
		if(m_BfTransVec[i]->FutureSerial==bfTrans->FutureSerial) 
		{ founded=true;    break;}
	}
	//////����
	if(founded) 
	{
		m_BfTransVec[i]= bfTrans; 
	} 
	///////�������ڷ���
	else 
	{
		m_BfTransVec.push_back(bfTrans);
		if(true && !g_bRecconnectT)
		{
			if(pRspTransfer->ErrorID!=0)
			{
				ShowErrTips(pRspTransfer->ErrorMsg);
			}
			else
			{
				//ShowErroTips(IDS_BFTRANS_OK,IDS_STRTIPS);
			}
		}

	}

	//SetEvent(g_hEvent);
}

///�ڻ����������ʽ�ת�ڻ�����ر�
void CtpTraderSpi::OnErrRtnBankToFutureByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo)
{
	//AfxMessageBox(_T("OnErrRtnBankToFutureByFuture"));
	/*
	CThostFtdcRspTransferField* bfTrans = new CThostFtdcRspTransferField();
	ZeroMemory(bfTrans,sizeof(CThostFtdcRspTransferField));

	bfTrans->FutureSerial = pReqTransfer->FutureSerial;
	strcpy(bfTrans->TradeCode,pReqTransfer->TradeCode);
	bfTrans->TradeAmount = pReqTransfer->TradeAmount;
	strcpy(bfTrans->TradeTime,pReqTransfer->TradeTime);
	strcpy(bfTrans->ErrorMsg,pRspInfo->ErrorMsg);

	BfTransVec.push_back(bfTrans);
	*/
}


///////////////////////////////////////////�ڻ������ڻ��ʽ�ת��������///////////////////////////////////////////
void CtpTraderSpi::ReqF2BkByF(TThostFtdcBankIDType BkID,TThostFtdcPasswordType BkPwd,
	TThostFtdcPasswordType Pwd,TThostFtdcTradeAmountType TdAmt)
{
	CThostFtdcReqTransferField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	 
	strcpy(req.AccountID, INVEST_ID); //�û�����
	strcpy(req.TradeCode,"202002");
	strcpy(req.BankBranchID,"0000");
	strcpy(req.CurrencyID,"RMB");
	strcpy(req.BankID,BkID);
	strcpy(req.BankPassWord,BkPwd);
	strcpy(req.Password,Pwd);
	req.TradeAmount=TdAmt;
	req.SecuPwdFlag = THOST_FTDC_BPWDF_BlankCheck;

	pUserApi->ReqFromFutureToBankByFuture(&req,++m_iRequestID);
}


///�ڻ������ڻ��ʽ�ת����Ӧ��
void CtpTraderSpi::OnRspFromFutureToBankByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( IsErrorRspInfo(pRspInfo) || (pReqTransfer==NULL))
	{

		ShowErrTips(pRspInfo->ErrorMsg);
	}

	if(bIsLast) SetEvent(g_hEvent);	
}


///�ڻ������ڻ��ʽ�ת����֪ͨ
void CtpTraderSpi::OnRtnFromFutureToBankByFuture(CThostFtdcRspTransferField *pRspTransfer)
{
	//AfxMessageBox(_T("OnRtnFromFutureToBankByFuture"));

	CThostFtdcRspTransferField* bfTrans = new CThostFtdcRspTransferField();
	memcpy(bfTrans,  pRspTransfer, sizeof(CThostFtdcRspTransferField));
	bool founded=false;    UINT i=0;
	for(i=0; i<m_BfTransVec.size(); i++)
	{
		if(m_BfTransVec[i]->FutureSerial==bfTrans->FutureSerial)
		{ founded=true;    break;}
	}
	//////����
	if(founded) 
	{
		m_BfTransVec[i]= bfTrans; 

	} 
	///////�������ڷ���
	else 
	{
		m_BfTransVec.push_back(bfTrans);
		if(true && !g_bRecconnectT)
		{
			if(pRspTransfer->ErrorID!=0)
			{
				//TCHAR szMsg[MAX_PATH];
				//ansi2uni(CP_ACP,pRspTransfer->ErrorMsg,szMsg);

				//ShowErroTips(szMsg,MY_TIPS);
			}
			else
			{
				//ShowErroTips(IDS_BFTRANS_OK,IDS_STRTIPS);
			}
		}
	}

	//SetEvent(g_hEvent);
}

///�ڻ������ڻ��ʽ�ת���д���ر�
void CtpTraderSpi::OnErrRtnFutureToBankByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo)
{

}

///////////////////////////////////////////////////�ڻ������ѯ�����������///////////////////////////////////////////////
void CtpTraderSpi::ReqQryBkAccMoneyByF(TThostFtdcBankIDType BkID,TThostFtdcPasswordType BkPwd,
	TThostFtdcPasswordType Pwd)
{
	CThostFtdcReqQueryAccountField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	 
	strcpy(req.AccountID, INVEST_ID); //�û�����
	strcpy(req.TradeCode,"204002");
	strcpy(req.BankBranchID,"0000");
	strcpy(req.CurrencyID,"RMB");
	strcpy(req.BankID,BkID);
	strcpy(req.BankPassWord,BkPwd);
	strcpy(req.Password,Pwd);

	req.SecuPwdFlag = THOST_FTDC_BPWDF_BlankCheck;
	pUserApi->ReqQueryBankAccountMoneyByFuture(&req,++m_iRequestID);
}

///�ڻ������ѯ�������Ӧ��
void CtpTraderSpi::OnRspQueryBankAccountMoneyByFuture(CThostFtdcReqQueryAccountField *pReqQueryAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

	if( IsErrorRspInfo(pRspInfo) || (pReqQueryAccount==NULL))
	{
		CThostFtdcRspTransferField* bfTrans = new CThostFtdcRspTransferField();
		ZeroMemory(bfTrans,sizeof(CThostFtdcRspTransferField));

		bfTrans->FutureSerial = pReqQueryAccount->FutureSerial;
		strcpy(bfTrans->TradeCode,pReqQueryAccount->TradeCode);
		//bfTrans->TradeAmount = pReqQueryAccount->TradeAmount;
		strcpy(bfTrans->TradeTime,pReqQueryAccount->TradeTime);
		strcpy(bfTrans->ErrorMsg,pRspInfo->ErrorMsg);

		m_BfTransVec.push_back(bfTrans);

		ShowErrTips(pRspInfo->ErrorMsg);
	}
	//if(bIsLast) SetEvent(g_hEvent);	

}

///�ڻ������ѯ�������֪ͨ
void CtpTraderSpi::OnRtnQueryBankBalanceByFuture(CThostFtdcNotifyQueryAccountField *pNotifyQueryAccount)
{
	if(pNotifyQueryAccount->ErrorID ==0)
	{
		if (true && !g_bRecconnectT)
		{

			CThostFtdcNotifyQueryAccountField *pNotify = new CThostFtdcNotifyQueryAccountField();
			memcpy(pNotify,pNotifyQueryAccount,sizeof(CThostFtdcNotifyQueryAccountField));

			//::PostMessage(g_pCWnd->m_hWnd,WM_QRYBKYE_MSG,0,(LPARAM)pNotify);
			/*
			HWND hwnd = g_pCWnd->m_hWnd;

			////////////////////////////////////
			{
			COPYDATASTRUCT cpd;
			cpd.dwData = 0x10000;		//��ʶ
			cpd.cbData = sizeof(CThostFtdcNotifyQueryAccountField);
			cpd.lpData = (PVOID)pNotify;
			::SendMessage( hwnd, WM_COPYDATA, NULL, (LPARAM)&cpd );
			}
			*/

		}

	}

	SetEvent(g_hEvent);	
}

///�ڻ������ѯ����������ر�
void CtpTraderSpi::OnErrRtnQueryBankBalanceByFuture(CThostFtdcReqQueryAccountField *pReqQueryAccount, CThostFtdcRspInfoField *pRspInfo)
{

}

///////////////////////////////////////��ѯת����ˮ////////////////////////////////////////////
/// "204005"
void CtpTraderSpi::ReqQryTfSerial(TThostFtdcBankIDType BkID)
{
	CThostFtdcQryTransferSerialField req;
	memset(&req, 0, sizeof(req));

	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	 
	strcpy(req.AccountID, INVEST_ID); //�û�����

	strcpy(req.BankID,BkID);

	pUserApi->ReqQryTransferSerial(&req,++m_iRequestID);
}
///�����ѯת����ˮ��Ӧ
void CtpTraderSpi::OnRspQryTransferSerial(CThostFtdcTransferSerialField *pTransferSerial, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( IsErrorRspInfo(pRspInfo) || (pTransferSerial==NULL))
	{
		ShowErrTips(pRspInfo->ErrorMsg);	
	}
	if(bIsLast) SetEvent(g_hEvent);	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
///ϵͳ����ʱ�ڻ����ֹ������������ת�ڻ��������д�����Ϻ��̷��ص�֪ͨ
void CtpTraderSpi::OnRtnRepealFromBankToFutureByFutureManual(CThostFtdcRspRepealField *pRspRepeal)
{
	//AfxMessageBox(_T("0"));
}


///ϵͳ����ʱ�ڻ����ֹ���������ڻ�ת�����������д�����Ϻ��̷��ص�֪ͨ
void CtpTraderSpi::OnRtnRepealFromFutureToBankByFutureManual(CThostFtdcRspRepealField *pRspRepeal)
{
	//AfxMessageBox(_T("1"));
}


///ϵͳ����ʱ�ڻ����ֹ������������ת�ڻ�����ر�
void CtpTraderSpi::OnErrRtnRepealBankToFutureByFutureManual(CThostFtdcReqRepealField *pReqRepeal, CThostFtdcRspInfoField *pRspInfo)
{
	//AfxMessageBox(_T("2"));
}

///ϵͳ����ʱ�ڻ����ֹ���������ڻ�ת���д���ر�
void CtpTraderSpi::OnErrRtnRepealFutureToBankByFutureManual(CThostFtdcReqRepealField *pReqRepeal, CThostFtdcRspInfoField *pRspInfo)
{
	//AfxMessageBox(_T("3"));
}


///�ڻ������������ת�ڻ��������д�����Ϻ��̷��ص�֪ͨ
void CtpTraderSpi::OnRtnRepealFromBankToFutureByFuture(CThostFtdcRspRepealField *pRspRepeal)
{
	//AfxMessageBox(_T("4"));
}


///�ڻ���������ڻ�ת�����������д�����Ϻ��̷��ص�֪ͨ
void CtpTraderSpi::OnRtnRepealFromFutureToBankByFuture(CThostFtdcRspRepealField *pRspRepeal)
{
	//AfxMessageBox(_T("5"));
}



void CtpTraderSpi::ReqQryCFMMCTdAccKey()
{
	CThostFtdcQryCFMMCTradingAccountKeyField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, BROKER_ID);   //���͹�˾����	 
	strcpy(req.InvestorID, INVEST_ID); //�û�����

	pUserApi->ReqQryCFMMCTradingAccountKey(&req,++m_iRequestID);
}

void CtpTraderSpi::OnRspQryCFMMCTradingAccountKey(CThostFtdcCFMMCTradingAccountKeyField *pCFMMCTradingAccountKey, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pCFMMCTradingAccountKey)
	{
		//memcpy(&g_Cfmmc,pCFMMCTradingAccountKey,sizeof(CThostFtdcCFMMCTradingAccountKeyField));

		char strMsg[1000];
		sprintf(strMsg,CFMMC_TMPL,pCFMMCTradingAccountKey->ParticipantID,pCFMMCTradingAccountKey->AccountID,
			pCFMMCTradingAccountKey->KeyID,pCFMMCTradingAccountKey->CurrentKey);
		ShellExecuteA(NULL,"open",strMsg,NULL, NULL, SW_SHOW);

	}
	//if(bIsLast) SetEvent(g_hEvent);	
}

void CtpTraderSpi::ReqQryBkrTdParams()
{
	CThostFtdcQryBrokerTradingParamsField  req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID,BROKER_ID);
	strcpy(req.InvestorID,INVEST_ID);

	pUserApi->ReqQryBrokerTradingParams(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspQryBrokerTradingParams(CThostFtdcBrokerTradingParamsField *pBrokerTradingParams, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pBrokerTradingParams)
	{

	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::ReqQryBkrTdAlgos(TThostFtdcExchangeIDType ExhID,TThostFtdcInstrumentIDType instID)
{
	CThostFtdcQryBrokerTradingAlgosField  req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID,BROKER_ID);
	if(ExhID != NULL)
		strcpy(req.ExchangeID, ExhID);
	if(instID != NULL)
		strcpy(req.InstrumentID, instID);

	pUserApi->ReqQryBrokerTradingAlgos(&req, ++m_iRequestID);
}

void CtpTraderSpi::OnRspQryBrokerTradingAlgos(CThostFtdcBrokerTradingAlgosField *pBrokerTradingAlgos, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pBrokerTradingAlgos)
	{

	}
	if(bIsLast) SetEvent(g_hEvent);
}

///Ԥ��¼������
void CtpTraderSpi::ReqParkedOrderInsert(CThostFtdcParkedOrderField *ParkedOrder)
{
	pUserApi->ReqParkedOrderInsert(ParkedOrder,++m_iRequestID);
}
///Ԥ��¼��������Ӧ
void CtpTraderSpi::OnRspParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pParkedOrder)
	{

	}
	if(bIsLast) SetEvent(g_hEvent);
}

///Ԥ�񳷵�¼������
void CtpTraderSpi::ReqParkedOrderAction(CThostFtdcParkedOrderActionField *ParkedOrderAction)
{
	pUserApi->ReqParkedOrderAction(ParkedOrderAction,++m_iRequestID);
}
///Ԥ�񳷵�¼��������Ӧ
void CtpTraderSpi::OnRspParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pParkedOrderAction)
	{

	}
	if(bIsLast) SetEvent(g_hEvent);
}

///�����ѯԤ��
void CtpTraderSpi::ReqQryParkedOrder(TThostFtdcInstrumentIDType InstrumentID,TThostFtdcExchangeIDType ExchangeID)
{
	CThostFtdcQryParkedOrderField  req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID,BROKER_ID);
	strcpy(req.InvestorID,INVEST_ID);
	if(InstrumentID != NULL)
		strcpy(req.InstrumentID,InstrumentID);
	if(ExchangeID != NULL)
		strcpy(req.ExchangeID,ExchangeID);
	pUserApi->ReqQryParkedOrder(&req, ++m_iRequestID);
}

///�����ѯԤ����Ӧ
void CtpTraderSpi::OnRspQryParkedOrder(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pParkedOrder)
	{

	}
	if(bIsLast) SetEvent(g_hEvent);
}

///������Ԥ��
void CtpTraderSpi::ReqQryParkedOrderAction(TThostFtdcInstrumentIDType InstrumentID,TThostFtdcExchangeIDType ExchangeID)
{
	CThostFtdcQryParkedOrderActionField  req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID,BROKER_ID);
	strcpy(req.InvestorID,INVEST_ID);
	if(InstrumentID != NULL)
		strcpy(req.InstrumentID,InstrumentID);
	if(ExchangeID != NULL)
		strcpy(req.ExchangeID,ExchangeID);
	pUserApi->ReqQryParkedOrderAction(&req, ++m_iRequestID);
}

///ɾ��Ԥ�񳷵���Ӧ
void CtpTraderSpi::OnRspRemoveParkedOrderAction(CThostFtdcRemoveParkedOrderActionField *pRemoveParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if( !IsErrorRspInfo(pRspInfo) && pRemoveParkedOrderAction)
	{

	}
	if(bIsLast) SetEvent(g_hEvent);
}

///����ɾ��Ԥ��
void CtpTraderSpi::ReqRemoveParkedOrder(TThostFtdcParkedOrderIDType ParkedOrder_ID)
{
	CThostFtdcRemoveParkedOrderField  req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID,BROKER_ID);
	strcpy(req.InvestorID,INVEST_ID);
	strcpy(req.ParkedOrderID,ParkedOrder_ID);
	pUserApi->ReqRemoveParkedOrder(&req, ++m_iRequestID);
}

///ɾ��Ԥ����Ӧ
void CtpTraderSpi::OnRspRemoveParkedOrder(CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pRemoveParkedOrder)
	{

	}
	if(bIsLast) SetEvent(g_hEvent);
}

///����ɾ��Ԥ�񳷵�
void CtpTraderSpi::ReqRemoveParkedOrderAction(TThostFtdcParkedOrderActionIDType ParkedOrderAction_ID)
{
	CThostFtdcRemoveParkedOrderActionField  req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID,BROKER_ID);
	strcpy(req.InvestorID,INVEST_ID);
	strcpy(req.ParkedOrderActionID,ParkedOrderAction_ID);
	pUserApi->ReqRemoveParkedOrderAction(&req, ++m_iRequestID);
}

///����ɾ��Ԥ�񳷵���Ӧ
void CtpTraderSpi::OnRspQryParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	if( !IsErrorRspInfo(pRspInfo) && pParkedOrderAction)
	{

	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpTraderSpi::OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus)
{
	if(pInstrumentStatus && true)
	{
		//CXTraderDlg* pDlg = (CXTraderDlg*)g_pCWnd;
		CString szStat,szMsg,szExh;
		//TCHAR szTm[30];
		//ansi2uni(CP_ACP,pInstrumentStatus->EnterTime,szTm);
		JgTdStatus(szStat,pInstrumentStatus->InstrumentStatus);
		szExh = JgExchage(pInstrumentStatus->ExchangeID);

		//szMsg.Format(_T("%s %s:%s"),szTm,(LPCTSTR)szExh,(LPCTSTR)szStat);
		//pDlg->SetStatusTxt(szMsg,2);	
	}
}

void CtpTraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	TRACE(_T("OnRspErrorT\n"));
	IsErrorRspInfo(pRspInfo);
}

bool CtpTraderSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	// ���ErrorID != 0, ˵���յ��˴������Ӧ
	bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	return ret;
}

void CtpTraderSpi::ShowErrTips(TThostFtdcErrorMsgType ErrorMsg)
{
	//TCHAR szMsg[MAX_PATH];
	//ansi2uni(CP_ACP,ErrorMsg,szMsg);
	//ShowErroTips(szMsg,MY_TIPS);
}
