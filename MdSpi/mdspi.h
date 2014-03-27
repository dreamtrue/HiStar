#ifndef MD_SPI_H_
#define MD_SPI_H_

#include "StdAfx.h"
#include "ThostFtdcMdApi.h"
#include <vector>
class CtpMdSpi : public CThostFtdcMdSpi
{
public:
	CtpMdSpi(CThostFtdcMdApi* api):pUserApi(api){ m_iRequestID = 0;}
	//~CtpMdSpi();
	///错误应答
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo,
		int nRequestID, bool bIsLast);
	///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	///@param nReason 错误原因
	///        0x1001 网络读失败
	///        0x1002 网络写失败
	///        0x2001 接收心跳超时
	///        0x2002 发送心跳失败
	///        0x2003 收到错误报文
	virtual void OnFrontDisconnected(int nReason);		
	///心跳超时警告。当长时间未收到报文时，该方法被调用。
	///@param nTimeLapse 距离上次接收报文的时间
	virtual void OnHeartBeatWarning(int nTimeLapse);
	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	virtual void OnFrontConnected();
	///登录请求响应
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///登出请求响应
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///订阅行情应答
	virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///取消订阅行情应答
	virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///深度行情通知
	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);
public:
	//LoginDlg* m_pDlg;
	//void ReqUserLogin(TThostFtdcBrokerIDType	appId);
	void ReqUserLogin(TThostFtdcBrokerIDType	vAppId,TThostFtdcUserIDType	vUserId,TThostFtdcPasswordType	vPasswd);
	void ReqUserLogout();
	//TThostFtdcUserIDType	userId,	TThostFtdcPasswordType	passwd);
	void SubscribeMarketData(char *pInst[], int nCount);
	void UnSubscribeMarketData(char *pInst[], int nCount);
	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);
public:
	int m_iRequestID;
	TThostFtdcBrokerIDType	m_sBkrID;
	TThostFtdcUserIDType m_sINVEST_ID;		// 投资者代码
private:
	CThostFtdcMdApi* pUserApi;
public:
	std::vector<CString> InstSubscribed;//提交报价的Inst
	std::vector<CString> InstMustSubscribe;//必须提交报价的Inst
	void SynchronizeMarket(std::vector<CString> &InstSubscribed,std::vector<CString> &InstMustSubscribe,std::vector<CThostFtdcInvestorPositionDetailField> &InvPosDetailVec);//同步市场，即将需要提交报价的提交，需要撤销报价的撤销
};

#endif