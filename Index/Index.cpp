// Index.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Index.h"
#include "UserMsg.h"
extern int GetIndexData(void);
VOID CALLBACK timerFun(HWND wnd, UINT msg, UINT_PTR id, DWORD d);
// CIndex

IMPLEMENT_DYNCREATE(CIndex, CWinThread)

CIndex::CIndex()
{
}

CIndex::~CIndex()
{
}

BOOL CIndex::InitInstance()
{
	_timerID = SetTimer(NULL,0,(UINT)3000,timerFun); 
	return TRUE;
}

int CIndex::ExitInstance()
{
	TRACE("�˳�ָ����ȡ�߳�\r\n");
	KillTimer(NULL, _timerID);
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CIndex, CWinThread)
END_MESSAGE_MAP()
VOID CALLBACK timerFun(HWND wnd, UINT msg, UINT_PTR id, DWORD d){
	GetIndexData();
}
