#ifndef USERMSG_H
#define USERMSG_H
#define WM_ORDER_STATUS WM_USER+1
#define WM_ERRORS WM_USER+2
#define WM_DISCONNECT_IB WM_USER+3
#define WM_LOGIN_CTP WM_USER+4
#define WM_LOGOUT_CTP WM_USER+5
#define WM_QRY_ACC_CTP WM_USER+7
#define WM_UPDATE_LSTCTRL WM_USER+8
#define WM_MD_REFRESH WM_USER+9
#define WM_BEGIN_POST_PROCESSING WM_USER+10 //��ʼ���к���
//WM_RTN_INSERT,WM_RTN_ORDER��Ϣֵ�������WM_RTN_INSERT < WM_RTN_ORDER�Թ�������Ϣʱʹ��
#define WM_RTN_INSERT WM_USER+11//��������
#define WM_RTN_ORDER WM_USER+12 //�����
///////////////////////////////////////////////////////////////////////////////////////
#define WM_RTN_TRADE WM_USER+13 //�ɽ�����
#define WM_RTN_ORDER_IB WM_USER+14 //IB�����
#define WM_CONNECT_SQL WM_USER+16
#define WM_INI WM_USER+17
#define WM_NOTIFY_EVENT WM_USER+18 //traderspi֪ͨ��Ϣ
#define WM_LOGIN_TD WM_USER+20 
#define WM_LOGIN_MD WM_USER+21
#define WM_UPDATE_HEDGEHOLD WM_USER+22
#define WM_REQ_MSHQ WM_USER+23 //MSHQ��������
#define WM_NOTIFY_EVENT_MD WM_USER+24 //����֪ͨ��Ϣ
#define WM_SYNCHRONIZE_MARKET WM_USER+25
#define WM_SYNCHRONIZE_NOTIFY WM_USER+26
#define WM_UPDATE_INDEX_REF WM_USER+27
#define WM_CONNECT_IB WM_USER+28
#define WM_REQACCOUNTUPDATES WM_USER+29
#define WM_REQACCOUNTUPDATES_NOTIFY WM_USER+30
#define WM_PREPARE_POST_PROCESSING WM_USER+31
#endif