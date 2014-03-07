#pragma once

///////////////////////////////////////////////

// CHedgePostProcessing

class CHedgePostProcessing : public CWinThread
{
	DECLARE_DYNCREATE(CHedgePostProcessing)

protected:
	CHedgePostProcessing();           // 动态创建所使用的受保护的构造函数
	virtual ~CHedgePostProcessing();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
public:
	void PostProcessing(WPARAM t_wParam,LPARAM t_lParam);//后处理
protected:
	DECLARE_MESSAGE_MAP()
};


