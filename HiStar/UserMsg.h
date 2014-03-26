#ifndef USERMSG_H
#define USERMSG_H
#define WM_CONNECT_IB WM_USER
#define WM_ORDER_STATUS WM_USER+1
#define WM_ERRORS WM_USER+2
#define WM_DISCONNECT_IB WM_USER+3
#define WM_LOGIN_CTP WM_USER+4
#define WM_LOGOUT_CTP WM_USER+5
#define WM_UPDATE_ACC_CTP WM_USER+6
#define WM_QRY_ACC_CTP WM_USER+7
#define WM_UPDATE_LSTCTRL WM_USER+8
#define WM_MD_REFRESH WM_USER+9
#define WM_BEGIN_POST_PROCESSING WM_USER+10 //开始进行后处理
//WM_RTN_INSERT,WM_RTN_ORDER消息值必须紧靠WM_RTN_INSERT < WM_RTN_ORDER以供处理消息时使用
#define WM_RTN_INSERT WM_USER+11//报单返回
#define WM_RTN_ORDER WM_USER+12 //命令返回
///////////////////////////////////////////////////////////////////////////////////////
#define WM_RTN_TRADE WM_USER+13 //成交返回
#define WM_RTN_ORDER_IB WM_USER+14 //IB命令返回
#define WM_PREPARE_POST_PROCESSING WM_USER+15 //为后处理做准备
#define WM_CONNECT_SQL WM_USER+16
#define WM_INI WM_USER+17
#define WM_NOTIFY_EVENT WM_USER+18 //替代原先event的通知信息
#endif