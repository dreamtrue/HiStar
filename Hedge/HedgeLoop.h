#pragma once

struct HoldAndDirec{
	double price;
	int numDirec;//带有方向性的持仓数量
};

// CHedgeLoop

class CHedgeLoop : public CWinThread
{
	DECLARE_DYNCREATE(CHedgeLoop)

protected:
	CHedgeLoop();           // 动态创建所使用的受保护的构造函数
	virtual ~CHedgeLoop();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
public:
	void BeginHedge(UINT wParam,LONG lParam);
public:
	HoldAndDirec tempIni;
	double step;
	int multiply;
	int aimOfLadder;
protected:
	DECLARE_MESSAGE_MAP()
public:
	bool m_isPause;
};


