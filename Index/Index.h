#pragma once
#include "afxsock.h"
#include <afxinet.h> 


// CIndex

class CIndex : public CWinThread
{
	DECLARE_DYNCREATE(CIndex)

protected:
	CIndex();           // ��̬������ʹ�õ��ܱ����Ĺ��캯��
	virtual ~CIndex();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

protected:
	DECLARE_MESSAGE_MAP()
public:
	UINT_PTR _timerID;
	static VOID CALLBACK UpdateIndexData(HWND wnd, UINT msg, UINT_PTR id, DWORD d);
	static CInternetSession mySession;
	static CHttpFile* myHttpFile;
	static CString myData;
	static CString myURL;
	static CString myURL_code;
	static CString m_URL;
	static CString m_hexunA50;
	static CString m_hexunHS300;
};


