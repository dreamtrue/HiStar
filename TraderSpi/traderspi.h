
#pragma once

#include "StdAfx.h"
#include "global.h"
#include "CVector.h"

class CtpTraderSpi : public CThostFtdcTraderSpi
{
public:
	CtpTraderSpi(CThostFtdcTraderApi* api):pUserApi(api){ m_iRequestID =0; m_ifrontId=-1; m_isessionId=-1;}
   //~CtpTraderSpi();

	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	virtual void OnFrontConnected();
	///登录请求响应
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///登出请求响应
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///请求查询客户通知响应
	virtual void OnRspQryNotice(CThostFtdcNoticeField *pNotice, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryTradingNotice(CThostFtdcTradingNoticeField *pTradingNotice, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///投资者结算结果确认响应
	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQrySettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///请求查询合约响应
	virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///请求查询资金账户响应
	virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///请求查询持仓响应
	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///请求查询持仓明细响应
	virtual void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryInvestorPositionCombineDetail(CThostFtdcInvestorPositionCombineDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryInvestor(CThostFtdcInvestorField *pInvestor, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryAccountregister(CThostFtdcAccountregisterField *pAccountregister, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryTransferBank(CThostFtdcTransferBankField *pTransferBank, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	virtual void OnRspQryContractBank(CThostFtdcContractBankField *pContractBank, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///期货发起银行资金转期货通知
	virtual void OnRtnFromBankToFutureByFuture(CThostFtdcRspTransferField *pRspTransfer);	
	///期货发起期货资金转银行通知
	virtual void OnRtnFromFutureToBankByFuture(CThostFtdcRspTransferField *pRspTransfer);	
	///系统运行时期货端手工发起冲正银行转期货请求，银行处理完毕后报盘发回的通知
	virtual void OnRtnRepealFromBankToFutureByFutureManual(CThostFtdcRspRepealField *pRspRepeal);
	///系统运行时期货端手工发起冲正期货转银行请求，银行处理完毕后报盘发回的通知
	virtual void OnRtnRepealFromFutureToBankByFutureManual(CThostFtdcRspRepealField *pRspRepeal);
	///期货发起查询银行余额通知
	virtual void OnRtnQueryBankBalanceByFuture(CThostFtdcNotifyQueryAccountField *pNotifyQueryAccount);
	///期货发起银行资金转期货错误回报
	virtual void OnErrRtnBankToFutureByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo);
	///期货发起期货资金转银行错误回报
	virtual void OnErrRtnFutureToBankByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo);
	///系统运行时期货端手工发起冲正银行转期货错误回报
	virtual void OnErrRtnRepealBankToFutureByFutureManual(CThostFtdcReqRepealField *pReqRepeal, CThostFtdcRspInfoField *pRspInfo);
	///系统运行时期货端手工发起冲正期货转银行错误回报
	virtual void OnErrRtnRepealFutureToBankByFutureManual(CThostFtdcReqRepealField *pReqRepeal, CThostFtdcRspInfoField *pRspInfo);
	///期货发起查询银行余额错误回报
	virtual void OnErrRtnQueryBankBalanceByFuture(CThostFtdcReqQueryAccountField *pReqQueryAccount, CThostFtdcRspInfoField *pRspInfo);
	///期货发起冲正银行转期货请求，银行处理完毕后报盘发回的通知
	virtual void OnRtnRepealFromBankToFutureByFuture(CThostFtdcRspRepealField *pRspRepeal);
	///期货发起冲正期货转银行请求，银行处理完毕后报盘发回的通知
	virtual void OnRtnRepealFromFutureToBankByFuture(CThostFtdcRspRepealField *pRspRepeal);
	virtual void OnRspFromBankToFutureByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///期货发起期货资金转银行应答
	virtual void OnRspFromFutureToBankByFuture(CThostFtdcReqTransferField *pReqTransfer, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///期货发起查询银行余额应答
	virtual void OnRspQueryBankAccountMoneyByFuture(CThostFtdcReqQueryAccountField *pReqQueryAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///请求查询转帐流水响应
	virtual void OnRspQryTransferSerial(CThostFtdcTransferSerialField *pTransferSerial, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///客户端认证响应
	virtual void OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///////查询交易编码响应
	virtual void OnRspQryTradingCode(CThostFtdcTradingCodeField *pTradingCode, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///报单录入请求响应
	virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///报单操作请求响应
	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///错误应答
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///用户口令更新请求响应
	virtual void OnRspUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///资金账户口令更新请求响应
	virtual void OnRspTradingAccountPasswordUpdate(CThostFtdcTradingAccountPasswordUpdateField *pTradingAccountPasswordUpdate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	virtual void OnFrontDisconnected(int nReason);	
	///心跳超时警告。当长时间未收到报文时，该方法被调用。
	virtual void OnHeartBeatWarning(int nTimeLapse);
	///报单通知
	virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);
	///成交通知
	virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);
	///合约交易状态通知
	virtual void OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus);
	virtual void OnRspQryCFMMCTradingAccountKey(CThostFtdcCFMMCTradingAccountKeyField *pCFMMCTradingAccountKey, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///请求查询经纪公司交易参数响应
	virtual void OnRspQryBrokerTradingParams(CThostFtdcBrokerTradingParamsField *pBrokerTradingParams, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///请求查询经纪公司交易算法响应
	virtual void OnRspQryBrokerTradingAlgos(CThostFtdcBrokerTradingAlgosField *pBrokerTradingAlgos, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	////////////////////////////////////////////////////////////////////////////////////////////////
	///预埋单录入请求响应
	virtual void OnRspParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///预埋撤单录入请求响应
	virtual void OnRspParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///请求查询预埋单响应
	virtual void OnRspQryParkedOrder(CThostFtdcParkedOrderField *pParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///删除预埋撤单响应
	virtual void OnRspRemoveParkedOrderAction(CThostFtdcRemoveParkedOrderActionField *pRemoveParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast); 	
	///删除预埋单响应
	virtual void OnRspRemoveParkedOrder(CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///请求删除预埋撤单响应
	virtual void OnRspQryParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///////////////////////////////////////////////////////////////////////////////////////////////
	virtual void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///请求查询成交响应
	virtual void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
public:
	///用户登录请求
	void ReqUserLogin(TThostFtdcBrokerIDType appId,TThostFtdcUserIDType	userId,	TThostFtdcPasswordType	passwd);
	void ReqUserLogout();
	const char* GetTradingDay();
	///投资者结算结果确认
	void ReqSettlementInfoConfirm();
	///请求查询结算信息确认
	void ReqQrySettlementInfoConfirm();
	//查询结算信息
	void ReqQrySettlementInfo(TThostFtdcDateType TradingDay);
	///请求查询客户通知
	void ReqQryNotice();
	///请求查询交易通知
	void ReqQryTdNotice();
	///请求查询交易编码
	void ReqQryTradingCode();
	//请求查询用户资料
	void ReqQryInvestor();
	//请求保证金中心密钥
	void ReqQryCFMMCTdAccKey();
	//查询开户银行信息
	void ReqQryAccreg();
	void ReqQryContractBk(TThostFtdcBankIDType BankID,TThostFtdcBankBrchIDType BankBrchID);
	void ReqQryTransBk(TThostFtdcBankIDType BankID,TThostFtdcBankBrchIDType BankBrchID);
	///查询转帐流水
	void ReqQryTfSerial(TThostFtdcBankIDType BkID);
	///期货->银行资金转期货
	void ReqBk2FByF(TThostFtdcBankIDType BkID,TThostFtdcPasswordType BkPwd,TThostFtdcPasswordType Pwd,TThostFtdcTradeAmountType TdAmt);
	///期货->期货资金转银行
	void ReqF2BkByF(TThostFtdcBankIDType BkID,TThostFtdcPasswordType BkPwd,
	 TThostFtdcPasswordType Pwd,TThostFtdcTradeAmountType TdAmt);
	///期货->查询银行余额
	void ReqQryBkAccMoneyByF(TThostFtdcBankIDType BkID,TThostFtdcPasswordType BkPwd,TThostFtdcPasswordType Pwd);	
	///请求查询合约保证金率
	void ReqQryInstMgr(TThostFtdcInstrumentIDType instId);	
	///请求查询合约手续费率
	void ReqQryInstFee(TThostFtdcInstrumentIDType instId);
	///请求查询合约信息
	void ReqQryInst(TThostFtdcInstrumentIDType instId);
	///请求查询资金账户
	void ReqQryTdAcc();
	///请求认证
	void ReqAuthenticate(TThostFtdcProductInfoType UserProdInf,TThostFtdcAuthCodeType	AuthCode);
	//请求查询持仓
	void ReqQryInvPos(TThostFtdcInstrumentIDType instId);
	void ReqQryInvPosEx(TThostFtdcInstrumentIDType instId);
	void ReqQryInvPosCombEx(TThostFtdcInstrumentIDType instId);	
	///普通限价单
	int ReqOrdLimit(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir, 
		TThostFtdcCombOffsetFlagType kpp,TThostFtdcPriceType price,TThostFtdcVolumeType vol);
	///市价单
	void ReqOrdAny(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir, 
		TThostFtdcCombOffsetFlagType kpp,TThostFtdcVolumeType vol);
	//FOK,FAK限价单
	void ReqOrdFAOK(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir,TThostFtdcCombOffsetFlagType kpp,
		TThostFtdcPriceType price,/*TThostFtdcVolumeType vol,*/TThostFtdcVolumeConditionType volconType,TThostFtdcVolumeType minVol);
	//条件单
	void ReqOrdCondition(TThostFtdcInstrumentIDType instId,TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp,
        TThostFtdcPriceType price,TThostFtdcVolumeType vol,TThostFtdcPriceType stopPrice,TThostFtdcContingentConditionType conType);
	//撤单
	void ReqOrderCancel(TThostFtdcSequenceNoType orderSeq);
	void ReqOrderCancel(TThostFtdcInstrumentIDType instId,TThostFtdcOrderRefType OrderRef);
	//更新交易密码
	void ReqUserPwdUpdate(TThostFtdcPasswordType szNewPass,TThostFtdcPasswordType szOldPass);
	//更新资金密码
	void ReqTdAccPwdUpdate(TThostFtdcPasswordType szNewPass,TThostFtdcPasswordType szOldPass);
	void ReqQryBkrTdParams();
	void ReqQryBkrTdAlgos(TThostFtdcExchangeIDType ExhID,TThostFtdcInstrumentIDType instID);
	///////////////////////////////////////////////////////////////////////
	///预埋单录入请求
	void ReqParkedOrderInsert(CThostFtdcParkedOrderField *ParkedOrder);
	void ReqParkedOrderAction(CThostFtdcParkedOrderActionField *ParkedOrderAction);
	///请求查询预埋单
	void ReqQryParkedOrder(TThostFtdcInstrumentIDType InstrumentID,TThostFtdcExchangeIDType ExchangeID);
	///请求查询预埋撤单
	void ReqQryParkedOrderAction(TThostFtdcInstrumentIDType InstrumentID,TThostFtdcExchangeIDType ExchangeID);
	///请求删除预埋单
	void ReqRemoveParkedOrder(TThostFtdcParkedOrderIDType ParkedOrder_ID);
	///请求删除预埋撤单
	void ReqRemoveParkedOrderAction(TThostFtdcParkedOrderActionIDType ParkedOrderAction_ID);
	///请求查询order
	void ReqQryOrder(TThostFtdcInstrumentIDType instId);
	///请求查询成交
    void ReqQryTrade(TThostFtdcInstrumentIDType instId);
	//////////////////////////////////////////////////////////////////////
	// 是否收到成功的响应
	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);
	void ShowErrTips(TThostFtdcErrorMsgType ErrorMsg);
  //void PrintOrders();
  //void PrintTrades();
public:
	int m_iRequestID;
	TThostFtdcBrokerIDType BROKER_ID;		// 经纪公司
	TThostFtdcUserIDType INVEST_ID;		// 投资者代码

	int	 m_ifrontId;
	int	 m_isessionId;
	char m_sOrdRef[13];
	char m_sTdday[9];
	char m_sTmBegin[20];
	////////////////////////////////////////////
	CVector<CThostFtdcInputOrderField> m_onRoadVec;
	CVector<CThostFtdcOrderField> m_orderVec;
	CVector<CThostFtdcTradeField> m_tradeVec;
	CVector<CThostFtdcInstrumentFieldEx> m_InsinfVec;
	CVector<CThostFtdcInstrumentMarginRateField> m_MargRateVec;
	CVector<CThostFtdcSettlementInfoField> m_StmiVec;
	CVector<CThostFtdcAccountregisterField> m_AccRegVec;
	CVector<CThostFtdcTradingCodeField> m_TdCodeVec;
	CVector<CThostFtdcInvestorPositionField> m_InvPosVec;
	CVector<CThostFtdcInvestorPositionDetailField> m_InvPosDetailVec;//新增加的持仓明细
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
};
