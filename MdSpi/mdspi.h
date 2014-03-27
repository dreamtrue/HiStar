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
	TThostFtdcUserIDType m_sINVEST_ID;		// Ͷ���ߴ���
private:
	CThostFtdcMdApi* pUserApi;
public:
	std::vector<CString> InstSubscribed;//�ύ���۵�Inst
	std::vector<CString> InstMustSubscribe;//�����ύ���۵�Inst
	void SynchronizeMarket(std::vector<CString> &InstSubscribed,std::vector<CString> &InstMustSubscribe,std::vector<CThostFtdcInvestorPositionDetailField> &InvPosDetailVec);//ͬ���г���������Ҫ�ύ���۵��ύ����Ҫ�������۵ĳ���
};

#endif