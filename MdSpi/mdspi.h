#ifndef MD_SPI_H_
#define MD_SPI_H_

#include "StdAfx.h"
#include "ThostFtdcMdApi.h"
#include <vector>
#include "CVector.h"
class CtpMdSpi : public CThostFtdcMdSpi
{
public:
	CtpMdSpi(CThostFtdcMdApi* api):m_iRequestID(0),pUserApi(api){
		memset(m_sBkrID,0,sizeof(m_sBkrID));
		memset(m_sINVEST_ID,0,sizeof(m_sINVEST_ID));
	}
	CtpMdSpi():m_iRequestID(0),pUserApi(NULL){
		memset(m_sBkrID,0,sizeof(m_sBkrID));
		memset(m_sINVEST_ID,0,sizeof(m_sINVEST_ID));
	}
public:
	~CtpMdSpi(){
		pUserApi = NULL;
	}
	///����Ӧ��
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo,
		int nRequestID, bool bIsLast);
	///���ͻ����뽻�׺�̨ͨ�����ӶϿ�ʱ���÷��������á���������������API���Զ��������ӣ��ͻ��˿ɲ�������
	///@param nReason ����ԭ��
	///        0x1001 �����ʧ��
	///        0x1002 ����дʧ��
	///        0x2001 ����������ʱ
	///        0x2002 ��������ʧ��
	///        0x2003 �յ�������
	virtual void OnFrontDisconnected(int nReason);		
	///������ʱ���档����ʱ��δ�յ�����ʱ���÷��������á�
	///@param nTimeLapse �����ϴν��ձ��ĵ�ʱ��
	virtual void OnHeartBeatWarning(int nTimeLapse);
	///���ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
	virtual void OnFrontConnected();
	///��¼������Ӧ
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�ǳ�������Ӧ
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///��������Ӧ��
	virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///ȡ����������Ӧ��
	virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///�������֪ͨ
	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);
public:
	int ReqUserLogin(TThostFtdcBrokerIDType	vAppId,TThostFtdcUserIDType	vUserId,TThostFtdcPasswordType	vPasswd);
	int ReqUserLogout();
	int SubscribeMarketData(char *pInst[], int nCount);
	int UnSubscribeMarketData(char *pInst[], int nCount);
	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);
	double CalcPositionProfit();
protected:
	int m_iRequestID;
	TThostFtdcBrokerIDType	m_sBkrID;
	TThostFtdcUserIDType m_sINVEST_ID;		// Ͷ���ߴ���
	CVector<CThostFtdcDepthMarketDataField> Marketdata;
public:
	std::vector<CString> InstSubscribed;//�ύ���۵�Inst
	std::vector<CString> InstMustSubscribe;//�����ύ���۵�Inst
private:
	CThostFtdcMdApi* pUserApi;
public:
	void SynchronizeMarket(std::vector<CString> &InstSubscribed,std::vector<CString> &InstMustSubscribe,std::vector<CThostFtdcInvestorPositionDetailField> &InvPosDetailVec);//ͬ���г���������Ҫ�ύ���۵��ύ����Ҫ�������۵ĳ���
};

#endif
