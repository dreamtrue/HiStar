#pragma once

struct HoldAndDirec{
	double price;
	int numDirec;//���з����Եĳֲ�����
};

// CHedgeLoop

class CHedgeLoop : public CWinThread
{
	DECLARE_DYNCREATE(CHedgeLoop)

protected:
	CHedgeLoop();           // ��̬������ʹ�õ��ܱ����Ĺ��캯��
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


