#pragma once

// CHedgePostProcessing

class CHedgePostProcessing : public CWinThread
{
	DECLARE_DYNCREATE(CHedgePostProcessing)

protected:
	CHedgePostProcessing();           // ��̬������ʹ�õ��ܱ����Ĺ��캯��
	virtual ~CHedgePostProcessing();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
public:
	void PostProcessing(UINT wParam,LONG lParam);//����
protected:
	DECLARE_MESSAGE_MAP()
};


