#pragma once



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
};


