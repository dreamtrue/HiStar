// Index.cpp : 实现文件
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
	_timerID = SetTimer(NULL,0,(UINT)1000,timerFun); 
	return TRUE;
}

int CIndex::ExitInstance()
{
	TRACE("退出指数获取线程\r\n");
	KillTimer(NULL, _timerID);
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CIndex, CWinThread)
END_MESSAGE_MAP()
VOID CALLBACK timerFun(HWND wnd, UINT msg, UINT_PTR id, DWORD d){
	GetIndexData();
}
