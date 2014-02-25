#pragma once



// CIndex

class CIndex : public CWinThread
{
	DECLARE_DYNCREATE(CIndex)

protected:
	CIndex();           // 动态创建所使用的受保护的构造函数
	virtual ~CIndex();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

protected:
	DECLARE_MESSAGE_MAP()
public:
	UINT_PTR _timerID;
};


