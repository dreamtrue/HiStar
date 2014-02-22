
#pragma once

#include "StdAfx.h"
#include "global.h"
#include "CVector.h"

class CtpTraderSpi : public CThostFtdcTraderSpi
{
public:
	CtpTraderSpi(CThostFtdcTraderApi* api):pUserApi(api){ m_iRequestID =0; m_ifrontId=-1; m_isessionId=-1;}
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


public:
	///�û���¼����
	void ReqUserLogin(TThostFtdcBrokerIDType appId,TThostFtdcUserIDType	userId,	TThostFtdcPasswordType	passwd);
	void ReqUserLogout();

	const char* GetTradingDay();

	///Ͷ���߽�����ȷ��
	void ReqSettlementInfoConfirm();
	///�����ѯ������Ϣȷ��
	void ReqQrySettlementInfoConfirm();
	//��ѯ������Ϣ
	void ReqQrySettlementInfo(TThostFtdcDateType TradingDay);

	///�����ѯ�ͻ�֪ͨ
	void ReqQryNotice();
	///�����ѯ����֪ͨ
	void ReqQryTdNotice();
	///�����ѯ���ױ���
	void ReqQryTradingCode();
	//�����ѯ�û�����
	void ReqQryInvestor();

	//����֤��������Կ
	void ReqQryCFMMCTdAccKey();

	//��ѯ����������Ϣ
	void ReqQryAccreg();
	void ReqQryContractBk(TThostFtdcBankIDType BankID,TThostFtdcBankBrchIDType BankBrchID);
	void ReqQryTransBk(TThostFtdcBankIDType BankID,TThostFtdcBankBrchIDType BankBrchID);

	///��ѯת����ˮ
	void ReqQryTfSerial(TThostFtdcBankIDType BkID);
	///�ڻ�->�����ʽ�ת�ڻ�
	void ReqBk2FByF(TThostFtdcBankIDType BkID,TThostFtdcPasswordType BkPwd,TThostFtdcPasswordType Pwd,TThostFtdcTradeAmountType TdAmt);
	
	///�ڻ�->�ڻ��ʽ�ת����
	void ReqF2BkByF(TThostFtdcBankIDType BkID,TThostFtdcPasswordType BkPwd,
	 TThostFtdcPasswordType Pwd,TThostFtdcTradeAmountType TdAmt);
	
	///�ڻ�->��ѯ�������
	void ReqQryBkAccMoneyByF(TThostFtdcBankIDType BkID,TThostFtdcPasswordType BkPwd,TThostFtdcPasswordType Pwd);
	
	///�����ѯ��Լ��֤����
	void ReqQryInstMgr(TThostFtdcInstrumentIDType instId);
	
	///�����ѯ��Լ��������
	void ReqQryInstFee(TThostFtdcInstrumentIDType instId);

	///�����ѯ��Լ��Ϣ
	void ReqQryInst(TThostFtdcInstrumentIDType instId);
	///�����ѯ�ʽ��˻�
	void ReqQryTdAcc();

	///������֤
	void ReqAuthenticate(TThostFtdcProductInfoType UserProdInf,TThostFtdcAuthCodeType	AuthCode);
	//�����ѯ�ֲ�
	void ReqQryInvPos(TThostFtdcInstrumentIDType instId);
	void ReqQryInvPosEx(TThostFtdcInstrumentIDType instId);
	void ReqQryInvPosCombEx(TThostFtdcInstrumentIDType instId);
	
	///��ͨ�޼۵�
	void ReqOrdLimit(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir, 
		TThostFtdcCombOffsetFlagType kpp,TThostFtdcPriceType price,TThostFtdcVolumeType vol);

	///�м۵�
	void ReqOrdAny(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir, 
		TThostFtdcCombOffsetFlagType kpp,TThostFtdcVolumeType vol);

	//FOK,FAK�޼۵�
	void ReqOrdFAOK(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir,TThostFtdcCombOffsetFlagType kpp,
		TThostFtdcPriceType price,/*TThostFtdcVolumeType vol,*/TThostFtdcVolumeConditionType volconType,TThostFtdcVolumeType minVol);

	//������
	void ReqOrdCondition(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp,
        TThostFtdcPriceType price,TThostFtdcVolumeType vol,TThostFtdcPriceType stopPrice,TThostFtdcContingentConditionType conType);

	//����
	void ReqOrderCancel(TThostFtdcSequenceNoType orderSeq);
	void ReqOrderCancel(TThostFtdcInstrumentIDType instId,TThostFtdcOrderRefType OrderRef);

	//���½�������
	void ReqUserPwdUpdate(TThostFtdcPasswordType szNewPass,TThostFtdcPasswordType szOldPass);
	//�����ʽ�����
	void ReqTdAccPwdUpdate(TThostFtdcPasswordType szNewPass,TThostFtdcPasswordType szOldPass);

	void ReqQryBkrTdParams();
	void ReqQryBkrTdAlgos(TThostFtdcExchangeIDType ExhID,TThostFtdcInstrumentIDType instID);

	///////////////////////////////////////////////////////////////////////
	///Ԥ��¼������
	void ReqParkedOrderInsert(CThostFtdcParkedOrderField *ParkedOrder);
	void ReqParkedOrderAction(CThostFtdcParkedOrderActionField *ParkedOrderAction);
	
	///�����ѯԤ��
	void ReqQryParkedOrder(TThostFtdcInstrumentIDType InstrumentID,TThostFtdcExchangeIDType ExchangeID);
	///�����ѯԤ�񳷵�
	void ReqQryParkedOrderAction(TThostFtdcInstrumentIDType InstrumentID,TThostFtdcExchangeIDType ExchangeID);
	///����ɾ��Ԥ��
	void ReqRemoveParkedOrder(TThostFtdcParkedOrderIDType ParkedOrder_ID);
	///����ɾ��Ԥ�񳷵�
	void ReqRemoveParkedOrderAction(TThostFtdcParkedOrderActionIDType ParkedOrderAction_ID);

	//////////////////////////////////////////////////////////////////////
	// �Ƿ��յ��ɹ�����Ӧ
	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);

	void ShowErrTips(TThostFtdcErrorMsgType ErrorMsg);
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
	CVector<CThostFtdcOrderField> m_orderVec;
	CVector<CThostFtdcTradeField> m_tradeVec;
	CVector<CThostFtdcInstrumentFieldEx> m_InsinfVec;
	CVector<CThostFtdcInstrumentMarginRateField> m_MargRateVec;
	CVector<CThostFtdcSettlementInfoField> m_StmiVec;
	CVector<CThostFtdcAccountregisterField> m_AccRegVec;
	CVector<CThostFtdcTradingCodeField> m_TdCodeVec;
	CVector<CThostFtdcInvestorPositionField> m_InvPosVec;
	CVector<CThostFtdcRspTransferField> m_BfTransVec;
	CTimeSpan m_tsEXnLocal[4];
	CThostFtdcInstrumentCommissionRateField m_FeeRateRev;
	//////////////////////////////////////////////////////
	CThostFtdcTradingAccountField m_TdAcc;
	CThostFtdcRspInfoField m_RspMsg;

private:
  CThostFtdcTraderApi* pUserApi;

};
