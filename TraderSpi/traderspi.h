
#pragma once

#include "StdAfx.h"
#include "global.h"
#include "CVector.h"
#include "mysql.h"
#include <vector>
struct WaitSettlementForClose{
	///��Լ����
	TThostFtdcInstrumentIDType	InstrumentID;
	///����������
	TThostFtdcExchangeIDType	ExchangeID;
	///��������
	TThostFtdcDirectionType	Direction;
	///�۸�
	TThostFtdcPriceType	OpenPrice;
	//�۸�
	TThostFtdcPriceType ClosePrice;
	///����
	TThostFtdcVolumeType	Volume;
};
class CtpTraderSpi : public CThostFtdcTraderSpi
{
public:
	CtpTraderSpi(CThostFtdcTraderApi* api):pUserApi(api){ m_iRequestID =0; m_ifrontId=-1; m_isessionId=-1; connctp = NULL;

	AcquireSRWLockExclusive(&g_srwLock_TradingAccount);//����TradingAccount�Ӷ�д����ʵ��������ģ�ͬʱ����ط��Ӹ��������壬��Ϊ���ʱ���д����û��ʼ����
	memset(&TradingAccount,0,sizeof(CThostFtdcTradingAccountField));
	ReleaseSRWLockExclusive(&g_srwLock_TradingAccount);

	memset(&m_TdAcc,0,sizeof(CThostFtdcTradingAccountField));
	ClearAllVectors();
	}
   //~CtpTraderSpi();

	///���ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
	virtual void OnFrontConnected();
	///��¼������Ӧ
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�ǳ�������Ӧ
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�����ѯ�ͻ�֪ͨ��Ӧ
	virtual void OnRspQryNotice(CThostFtdcNoticeField *pNotice, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryTradingNotice(CThostFtdcTradingNoticeField *pTradingNotice, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///Ͷ���߽�����ȷ����Ӧ
	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�����ѯ��Լ��Ӧ
	virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�����ѯ�ʽ��˻���Ӧ
	virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�����ѯ�ֲ���Ӧ
	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�����ѯ�ֲ���ϸ��Ӧ
	virtual void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryInvestorPositionCombineDetail(CThostFtdcInvestorPositionCombineDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryInvestor(CThostFtdcInvestorField *pInvestor, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryAccountregister(CThostFtdcAccountregisterField *pAccountregister, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryTransferBank(CThostFtdcTransferBankField *pTransferBank, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryContractBank(CThostFtdcContractBankField *pContractBank, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///�ڻ����������ʽ�ת�ڻ�֪ͨ
	virtual void OnRtnFromBankToFutureByFuture(CThostFtdcRspTransferField *pRspTransfer);	
	///�ڻ������ڻ��ʽ�ת����֪ͨ
	virtual void OnRtnFromFutureToBankByFuture(CThostFtdcRspTransferField *pRspTransfer);	
	///ϵͳ����ʱ�ڻ����ֹ������������ת�ڻ��������д�����Ϻ��̷��ص�֪ͨ
	virtual void OnRtnRepealFromBankToFutureByFutureManual(CThostFtdcRspRepealField *pRspRepeal);
	///ϵͳ����ʱ�ڻ����ֹ���������ڻ�ת�����������д�����Ϻ��̷��ص�֪ͨ
	virtual void OnRtnRepealFromFutureToBankByFutureManual(CThostFtdcRspRepealField *pRspRepeal);
	///�ڻ������ѯ�������֪ͨ
	virtual void OnRtnQueryBankBalanceByFuture(CThostFtdcNotifyQueryAccountField *pNotifyQueryAccount);
	///�ڻ����������ʽ�ת�ڻ�����ر�
	virtual void OnErrRtnBankToFutureByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo);
	///�ڻ������ڻ��ʽ�ת���д���ر�
	virtual void OnErrRtnFutureToBankByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo);
	///ϵͳ����ʱ�ڻ����ֹ������������ת�ڻ�����ر�
	virtual void OnErrRtnRepealBankToFutureByFutureManual(CThostFtdcReqRepealField *pReqRepeal, CThostFtdcRspInfoField *pRspInfo);
	///ϵͳ����ʱ�ڻ����ֹ���������ڻ�ת���д���ر�
	virtual void OnErrRtnRepealFutureToBankByFutureManual(CThostFtdcReqRepealField *pReqRepeal, CThostFtdcRspInfoField *pRspInfo);
	///�ڻ������ѯ����������ر�
	virtual void OnErrRtnQueryBankBalanceByFuture(CThostFtdcReqQueryAccountField *pReqQueryAccount, CThostFtdcRspInfoField *pRspInfo);
	///�ڻ������������ת�ڻ��������д�����Ϻ��̷��ص�֪ͨ
	virtual void OnRtnRepealFromBankToFutureByFuture(CThostFtdcRspRepealField *pRspRepeal);
	///�ڻ���������ڻ�ת�����������д�����Ϻ��̷��ص�֪ͨ
	virtual void OnRtnRepealFromFutureToBankByFuture(CThostFtdcRspRepealField *pRspRepeal);
	virtual void OnRspFromBankToFutureByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�ڻ������ڻ��ʽ�ת����Ӧ��
	virtual void OnRspFromFutureToBankByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�ڻ������ѯ�������Ӧ��
	virtual void OnRspQueryBankAccountMoneyByFuture(CThostFtdcReqQueryAccountField *pReqQueryAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�����ѯת����ˮ��Ӧ
	virtual void OnRspQryTransferSerial(CThostFtdcTransferSerialField *pTransferSerial, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�ͻ�����֤��Ӧ
	virtual void OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///////��ѯ���ױ�����Ӧ
	virtual void OnRspQryTradingCode(CThostFtdcTradingCodeField *pTradingCode, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///����¼��������Ӧ
	virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///��������������Ӧ
	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///����Ӧ��
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�û��������������Ӧ
	virtual void OnRspUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�ʽ��˻��������������Ӧ
	virtual void OnRspTradingAccountPasswordUpdate(CThostFtdcTradingAccountPasswordUpdateField *pTradingAccountPasswordUpdate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///���ͻ����뽻�׺�̨ͨ�����ӶϿ�ʱ���÷��������á���������������API���Զ��������ӣ��ͻ��˿ɲ�������
	virtual void OnFrontDisconnected(int nReason);	
	///������ʱ���档����ʱ��δ�յ�����ʱ���÷��������á�
	virtual void OnHeartBeatWarning(int nTimeLapse);
	///����֪ͨ
	virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);
	///�ɽ�֪ͨ
	virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);
	///��Լ����״̬֪ͨ
	virtual void OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus);
	virtual void OnRspQryCFMMCTradingAccountKey(CThostFtdcCFMMCTradingAccountKeyField *pCFMMCTradingAccountKey, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�����ѯ���͹�˾���ײ�����Ӧ
	virtual void OnRspQryBrokerTradingParams(CThostFtdcBrokerTradingParamsField *pBrokerTradingParams, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�����ѯ���͹�˾�����㷨��Ӧ
	virtual void OnRspQryBrokerTradingAlgos(CThostFtdcBrokerTradingAlgosField *pBrokerTradingAlgos, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	////////////////////////////////////////////////////////////////////////////////////////////////
	///Ԥ��¼��������Ӧ
	virtual void OnRspParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///Ԥ�񳷵�¼��������Ӧ
	virtual void OnRspParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�����ѯԤ����Ӧ
	virtual void OnRspQryParkedOrder(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///ɾ��Ԥ�񳷵���Ӧ
	virtual void OnRspRemoveParkedOrderAction(CThostFtdcRemoveParkedOrderActionField *pRemoveParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast); 	
	///ɾ��Ԥ����Ӧ
	virtual void OnRspRemoveParkedOrder(CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///����ɾ��Ԥ�񳷵���Ӧ
	virtual void OnRspQryParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///////////////////////////////////////////////////////////////////////////////////////////////
	virtual void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�����ѯ�ɽ���Ӧ
	virtual void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
public:
	///�û���¼����
	int ReqUserLogin(TThostFtdcBrokerIDType appId,TThostFtdcUserIDType	userId,	TThostFtdcPasswordType	passwd);
	int ReqUserLogout();
	const char* GetTradingDay();
	///Ͷ���߽�����ȷ��
	int ReqSettlementInfoConfirm();
	///�����ѯ������Ϣȷ��
	int ReqQrySettlementInfoConfirm();
	//��ѯ������Ϣ
	int ReqQrySettlementInfo(TThostFtdcDateType TradingDay);
	///�����ѯ�ͻ�֪ͨ
	int ReqQryNotice();
	///�����ѯ����֪ͨ
	int ReqQryTdNotice();
	///�����ѯ���ױ���
	int ReqQryTradingCode();
	//�����ѯ�û�����
	int ReqQryInvestor();
	//����֤��������Կ
	int ReqQryCFMMCTdAccKey();
	//��ѯ����������Ϣ
	int ReqQryAccreg();
	int ReqQryContractBk(TThostFtdcBankIDType BankID,TThostFtdcBankBrchIDType BankBrchID);
	int ReqQryTransBk(TThostFtdcBankIDType BankID,TThostFtdcBankBrchIDType BankBrchID);
	///��ѯת����ˮ
	int ReqQryTfSerial(TThostFtdcBankIDType BkID);
	///�ڻ�->�����ʽ�ת�ڻ�
	int ReqBk2FByF(TThostFtdcBankIDType BkID,TThostFtdcPasswordType BkPwd,TThostFtdcPasswordType Pwd,TThostFtdcTradeAmountType TdAmt);
	///�ڻ�->�ڻ��ʽ�ת����
	int ReqF2BkByF(TThostFtdcBankIDType BkID,TThostFtdcPasswordType BkPwd,
	 TThostFtdcPasswordType Pwd,TThostFtdcTradeAmountType TdAmt);
	///�ڻ�->��ѯ�������
	int ReqQryBkAccMoneyByF(TThostFtdcBankIDType BkID,TThostFtdcPasswordType BkPwd,TThostFtdcPasswordType Pwd);	
	///�����ѯ��Լ��֤����
	int ReqQryInstMgr(TThostFtdcInstrumentIDType instId);	
	///�����ѯ��Լ��������
	int ReqQryInstFee(TThostFtdcInstrumentIDType instId);
	///�����ѯ��Լ��Ϣ
	int ReqQryInst(TThostFtdcInstrumentIDType instId);
	///�����ѯ��Լ��֤����
	int ReqQryInstrumentMarginRate(TThostFtdcInstrumentIDType instId);
	///�����ѯ�ʽ��˻�
	int ReqQryTdAcc();
	///������֤
	int ReqAuthenticate(TThostFtdcProductInfoType UserProdInf,TThostFtdcAuthCodeType	AuthCode);
	//�����ѯ�ֲ�
	int ReqQryInvPos(TThostFtdcInstrumentIDType instId);
	int ReqQryInvPosEx(TThostFtdcInstrumentIDType instId);
	int ReqQryInvPosCombEx(TThostFtdcInstrumentIDType instId);	
	///��ͨ�޼۵�
	int ReqOrdLimit(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir, 
		TThostFtdcCombOffsetFlagType kpp,TThostFtdcPriceType price,TThostFtdcVolumeType vol);
	///�м۵�
	int ReqOrdAny(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir, 
		TThostFtdcCombOffsetFlagType kpp,TThostFtdcVolumeType vol);
	//FOK,FAK�޼۵�
	int ReqOrdFAOK(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir,TThostFtdcCombOffsetFlagType kpp,
		TThostFtdcPriceType price,/*TThostFtdcVolumeType vol,*/TThostFtdcVolumeConditionType volconType,TThostFtdcVolumeType minVol);
	//������
	int ReqOrdCondition(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp,
        TThostFtdcPriceType price,TThostFtdcVolumeType vol,TThostFtdcPriceType stopPrice,TThostFtdcContingentConditionType conType);
	//����
	int ReqOrderCancel(TThostFtdcSequenceNoType orderSeq);
	int ReqOrderCancel(TThostFtdcInstrumentIDType instId,TThostFtdcOrderRefType OrderRef);
	//���½�������
	int ReqUserPwdUpdate(TThostFtdcPasswordType szNewPass,TThostFtdcPasswordType szOldPass);
	//�����ʽ�����
	int ReqTdAccPwdUpdate(TThostFtdcPasswordType szNewPass,TThostFtdcPasswordType szOldPass);
	int ReqQryBkrTdParams();
	int ReqQryBkrTdAlgos(TThostFtdcExchangeIDType ExhID,TThostFtdcInstrumentIDType instID);
	///////////////////////////////////////////////////////////////////////
	///Ԥ��¼������
	int ReqParkedOrderInsert(CThostFtdcParkedOrderField *ParkedOrder);
	int ReqParkedOrderAction(CThostFtdcParkedOrderActionField *ParkedOrderAction);
	///�����ѯԤ��
	int ReqQryParkedOrder(TThostFtdcInstrumentIDType InstrumentID,TThostFtdcExchangeIDType ExchangeID);
	///�����ѯԤ�񳷵�
	int ReqQryParkedOrderAction(TThostFtdcInstrumentIDType InstrumentID,TThostFtdcExchangeIDType ExchangeID);
	///����ɾ��Ԥ��
	int ReqRemoveParkedOrder(TThostFtdcParkedOrderIDType ParkedOrder_ID);
	///����ɾ��Ԥ�񳷵�
	int ReqRemoveParkedOrderAction(TThostFtdcParkedOrderActionIDType ParkedOrderAction_ID);
	///�����ѯorder
	int ReqQryOrder(TThostFtdcInstrumentIDType instId);
	///�����ѯ�ɽ�
    int ReqQryTrade(TThostFtdcInstrumentIDType instId);
	//////////////////////////////////////////////////////////////////////
	// �Ƿ��յ��ɹ�����Ӧ
	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);
	int ShowErrTips(TThostFtdcErrorMsgType ErrorMsg);
  //void PrintOrders();
  //void PrintTrades();
public:
	int m_iRequestID;
	TThostFtdcBrokerIDType BROKER_ID;		// ���͹�˾
	TThostFtdcUserIDType INVEST_ID;		// Ͷ���ߴ���

	int	 m_ifrontId;
	int	 m_isessionId;
	char m_sOrdRef[13];
	char m_sTdday[9];
	char m_sTmBegin[20];
	////////////////////////////////////////////
	CVector<CThostFtdcInputOrderField> m_onRoadVec;
	CVector<CThostFtdcOrderField> m_orderVec;
	CVector<CThostFtdcTradeField> m_tradeVec;

	std::vector<CThostFtdcInstrumentFieldEx> m_InsinfVec;
	std::vector<CThostFtdcInstrumentCommissionRateField> FeeRateList;
	std::vector<WaitSettlementForClose> WaitingForSettlementIns;

	CVector<CThostFtdcSettlementInfoField> m_StmiVec;
	CVector<CThostFtdcAccountregisterField> m_AccRegVec;
	CVector<CThostFtdcTradingCodeField> m_TdCodeVec;
	CVector<CThostFtdcInvestorPositionField> m_InvPosVec;

	std::vector<CThostFtdcInstrumentMarginRateField> m_MargRateVec;
	std::vector<CThostFtdcInvestorPositionDetailField> m_InvPosDetailVec;//�ֲ���ϸ
	CThostFtdcTradingAccountField TradingAccount;//�����˻����

	CVector<CThostFtdcRspTransferField> m_BfTransVec;
	CTimeSpan m_tsEXnLocal[4];
	CThostFtdcInstrumentCommissionRateField m_FeeRateRev;
	//////////////////////////////////////////////////////
	CThostFtdcTradingAccountField m_TdAcc;
	CThostFtdcRspInfoField m_RspMsg;
private:
	CThostFtdcTraderApi* pUserApi;
  public:
	void ClearAllVectors();
	int FindOrdInOnRoadVec(TThostFtdcOrderRefType OrderRef);
	MYSQL *connctp;
	CString statusTableName;
	CString tradeTableName;
};
