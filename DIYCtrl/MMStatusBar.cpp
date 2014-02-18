// MMStatusBar.cpp : implementation file
//
#include "stdafx.h"
#include "MMStatusBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// MMStatusBar

MMStatusBar::MMStatusBar()
{
}

MMStatusBar::~MMStatusBar()
{
  //cleanup.. remove all pane infos added
  for(int j = 0 ; j < m_oDrawStructArray.GetSize(); j++)
  {
    MMStatusDrawStruct* pDrawStruct = (MMStatusDrawStruct*)m_oDrawStructArray.GetAt(j);
    delete pDrawStruct;
  }
  m_oDrawStructArray.RemoveAll();
}


BEGIN_MESSAGE_MAP(MMStatusBar, CStatusBar)
	//{{AFX_MSG_MAP(MMStatusBar)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MMStatusBar message handlers

//overridden here just to create a tooltip control
int MMStatusBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CStatusBar::OnCreate(lpCreateStruct) == -1)
		return -1;
  
  //Create the tooltip
	m_oToolTip.Create(this,TTS_ALWAYSTIP);
  //Activate the tooltip
	m_oToolTip.Activate(TRUE);
	return 0;
}

void MMStatusBar::OnDestroy() 
{
	CStatusBar::OnDestroy();
	
	// TODO: Add your message handler code here
	if(::IsWindow(m_oToolTip.GetSafeHwnd()))
		m_oToolTip.DestroyWindow();
}

//////////////////////////////////////////////////////////
//              SetIndicators                           //
//                                                      //
// Used to set the indicators for the pane              //
// lpIDArray & nIDCount are same as what CStatusBar     //
// class expects.                                       //
// lpIDToolTipArray will point to an array of resource  //
// lphIconArray will point to an array of hIcons        //
//                                                      //
//////////////////////////////////////////////////////////
BOOL MMStatusBar::SetIndicators(const UINT* lpIDArray,int nIDCount,const UINT* lpIDToolTipArray,const HICON* lphIconArray)
{
  // defaults to base behaviour if called without third param / fourth param specifying text resources for tooltips
  ASSERT(lpIDToolTipArray==NULL || 
         AfxIsValidAddress(lpIDToolTipArray,sizeof(UINT)* nIDCount,FALSE));
  ASSERT(lphIconArray==NULL || 
         AfxIsValidAddress(lphIconArray,sizeof(HICON)* nIDCount,FALSE));

  if(!CStatusBar::SetIndicators(lpIDArray,nIDCount))	// trouble with standard bar
    return FALSE;					// abort

  if(lpIDArray != NULL &&			      // have indicators and
     lpIDToolTipArray != NULL)			// want tooltiptexts
  {
    for(int i =0;i<nIDCount;i++)		// each (new) indicator
    {
      CRect oRect;
      GetItemRect(i,&oRect);
      UINT uID  = lpIDArray[i];
      UINT uIDText = lpIDToolTipArray[i];
      if (uID != ID_SEPARATOR &&			// have indicator and 
	        uIDText != 0)				        // want tooltiptext for it
        VERIFY(m_oToolTip.AddTool(this,uIDText,oRect,uID));
    }
  }
  if(lpIDArray != NULL &&
     lphIconArray != NULL)
  {
    for(int i =0;i<nIDCount;i++)		// each (new) indicator
    {
      UINT uID  = lpIDArray[i];
      if (uID != ID_SEPARATOR &&     // have indicator and
          lphIconArray[i] != NULL)   // want icon
      {
        GetStatusBarCtrl().SetIcon(i,lphIconArray[i]);
        int nWidth;
        UINT nID,nStyle;
	      GetPaneInfo(i,nID,nStyle,nWidth);
	      SetPaneInfo(i,nID,nStyle,nWidth + GetSystemMetrics(SM_CXSMICON));
      }
    }
  }
  return TRUE;
}

void MMStatusBar::OnSize(UINT nType, int cx, int cy) 
{
	CStatusBar::OnSize(nType, cx, cy);
	
	if(!IsWindow(m_oToolTip.GetSafeHwnd()))
		return ;

	// TODO: Add your message handler code here
  //ask the statusbar to recompute the new tooltip 
  //rectangles since the size has changed
	Redraw();
}

//////////////////////////////////////////////////////////
//              Redraw                                  //
//                                                      //
// Used to recompute the rects for the tools for panes  //
// Normally , one would have to call this method        //
// if something in the panes has changed which would    //
// result in a new pane with and hence a new tool rect  //
//                                                      //
//////////////////////////////////////////////////////////
void MMStatusBar::Redraw()
{
  CToolInfo oToolInfo;
	memset(&oToolInfo,0,sizeof(TOOLINFO));
	oToolInfo.cbSize = sizeof(TOOLINFO) ;

	CRect oRect;
	UINT uId;
	for(int i = 0 ; i < m_nCount ; i++)
	{
		uId = GetItemID(i);
		GetItemRect(i,&oRect);
		
		oToolInfo.hwnd = this->GetSafeHwnd();
		oToolInfo.uId = uId;
		oToolInfo.rect.left = oRect.left;
		oToolInfo.rect.right = oRect.right;
		oToolInfo.rect.top = oRect.top;
		oToolInfo.rect.bottom = oRect.bottom;

    m_oToolTip.SendMessage(TTM_NEWTOOLRECT,0,(LPARAM)(TOOLINFO*)&oToolInfo);
    MMStatusDrawStruct* pstDrawStruct = GetPaneStruct(i);

    if(pstDrawStruct)
      SendMessage(SB_SETTEXT,pstDrawStruct->m_nIndex | SBT_OWNERDRAW,(LPARAM)pstDrawStruct);
	}
}

void MMStatusBar::UpdateTipText(LPCTSTR lpszTxt,UINT nIDTool)
{
	//if (uIDTxt != 0)				    
	m_oToolTip.UpdateTipText(lpszTxt,this->GetParent(),nIDTool);
    
}
//////////////////////////////////////////////////////////
//              SetDelayTime                            //
//                                                      //
// Used to set the delay time for the tooltips.         //
// Sets the delay times to default proportions          //
// To have more granularity for the times, please use   //
// CToolTipCtrl::SetDelayTime(Flags,Time)               //
//                                                      //
//////////////////////////////////////////////////////////
void MMStatusBar::SetDelayTime(UINT uDelayTime)
{
  m_oToolTip.SetDelayTime(uDelayTime);
}

//////////////////////////////////////////////////////////
//              Enable                                  //
//                                                      //
// Used to enable/disable the pane. Does not draw pane  //
// text.                                                //
// TRUE to enable, FALSE to disable                     //
//                                                      //
//////////////////////////////////////////////////////////
void MMStatusCmdUI::Enable(BOOL bOn)
{
	m_bEnableChanged = TRUE;
	CStatusBar* pStatusBar = (CStatusBar*)m_pOther;
	ASSERT(pStatusBar != NULL);
	ASSERT_KINDOF(CStatusBar, pStatusBar);
	ASSERT(m_nIndex < m_nIndexMax);

	UINT nNewStyle = pStatusBar->GetPaneStyle(m_nIndex) & ~SBPS_DISABLED;
	if (!bOn)
		nNewStyle |= SBPS_DISABLED;
	pStatusBar->SetPaneStyle(m_nIndex, nNewStyle);
}

//////////////////////////////////////////////////////////
//              SetCheck                                //
//                                                      //
// Used to make the pane text appear popped up          //
// nCheck is non-zero to popup text, zero to not pop-up //
//                                                      //
//////////////////////////////////////////////////////////
void MMStatusCmdUI::SetCheck(int nCheck)
{
	CStatusBar* pStatusBar = (CStatusBar*)m_pOther;
	ASSERT(pStatusBar != NULL);
	ASSERT_KINDOF(CStatusBar, pStatusBar);
	ASSERT(m_nIndex < m_nIndexMax);

	UINT nNewStyle = pStatusBar->GetPaneStyle(m_nIndex) & ~SBPS_POPOUT;
	if (nCheck != 0)
		nNewStyle |= SBPS_POPOUT;
	pStatusBar->SetPaneStyle(m_nIndex, nNewStyle);
}

//////////////////////////////////////////////////////////
//              SetText                                 //
//                                                      //
// Set the text for the pane.                           //
// lpszText is the new text                             //
//                                                      //
//////////////////////////////////////////////////////////
void MMStatusCmdUI::SetText(LPCTSTR lpszText)
{
	MMStatusBar* pStatusBar = (MMStatusBar*)m_pOther;
	ASSERT(pStatusBar != NULL);
	ASSERT_KINDOF(CStatusBar, pStatusBar);
	ASSERT(m_nIndex < m_nIndexMax);

  MMStatusDrawStruct* pstDrawStruct = pStatusBar->GetPaneStruct(m_nIndex);

  //if the pstDrawStruct is non-NULL, it implies that
  //this is an owner drawn pane... So, we need to send
  //SB_TEXT message with SBT_OWNERDRAW set
  if(pstDrawStruct && pstDrawStruct->m_szText.Compare(lpszText))
  {
    pstDrawStruct->m_szText = lpszText;
    pStatusBar->GetStatusBarCtrl().SendMessage(SB_SETTEXT,m_nIndex | SBT_OWNERDRAW,(LPARAM)pstDrawStruct);
    pStatusBar->Invalidate();
    pStatusBar->UpdateWindow();
  }
  else
  {
    //not owner drawn pane
	  pStatusBar->SetPaneText(m_nIndex, lpszText);
  }

  //now that the text has changed, we'll have 
  //to recompute the pane widths using the current font
  int cxExtra = 0;
  HICON hIcon = (HICON)pStatusBar->GetStatusBarCtrl().SendMessage(SB_GETICON,(WPARAM)m_nIndex,0);
  if(hIcon)
    cxExtra = GetSystemMetrics(SM_CXSMICON);//get icon width

  if(pstDrawStruct)
  {
    //if it is owner drawn, we have to set the correct 
    //icon width, so that the drawing routine is able 
    //to leave that much space before drawing the text
    if(hIcon)
      pstDrawStruct->m_cxExtra = cxExtra;
    else
      pstDrawStruct->m_cxExtra = 0;
  }

  CClientDC dc(NULL);
  HFONT hFont,hOldFont;
  //get current font
	hFont = (HFONT)pStatusBar->SendMessage(WM_GETFONT);
	hOldFont = (HFONT)dc.SelectObject(hFont);
  //set width to ( width of the text in current font + width for icon if any )
	SetWidth(dc.GetTextExtent(lpszText).cx + cxExtra);
  //cleanup
	dc.SelectObject(hOldFont);
}

//////////////////////////////////////////////////////////
//              SetToolTipText                          //
//                                                      //
// Set the tooltip text for the pane.                   //
// lpszText is the new text                             //
//                                                      //
//////////////////////////////////////////////////////////
void MMStatusCmdUI::SetToolTipText(LPCTSTR lpszText)
{
	MMStatusBar* pStatusBar = (MMStatusBar*)m_pOther;
	ASSERT(pStatusBar != NULL);
	ASSERT_KINDOF(CStatusBar, pStatusBar);
	ASSERT(m_nIndex < m_nIndexMax);

	pStatusBar->m_oToolTip.UpdateTipText(lpszText,m_pOther,m_nID);
}

//////////////////////////////////////////////////////////
//              SetWidth                                //
//                                                      //
// Set the width for the pane                           //
// cxWidth is the new width.                            //
// When the width is changed, the tooltip is updated    //
// with the new resulting rectangle boundaries for      //
// the pane by call to redraw                           //
//                                                      //
//////////////////////////////////////////////////////////
void MMStatusCmdUI::SetWidth(int cxWidth)
{
	MMStatusBar* pStatusBar = (MMStatusBar*)m_pOther;
	ASSERT(pStatusBar != NULL);
	ASSERT_KINDOF(CStatusBar, pStatusBar);
	ASSERT(m_nIndex < m_nIndexMax);
	UINT nID;
	UINT nStyle;
	int nWidth;
	pStatusBar->GetPaneInfo(m_nIndex,nID,nStyle,nWidth);
	pStatusBar->SetPaneInfo(m_nIndex,m_nID,nStyle,cxWidth);

  //since the width of the pane changed, we'll have to recompute the 
  //tool rectangles
  pStatusBar->Redraw();
}

//////////////////////////////////////////////////////////
//              SetIcon                                 //
//                                                      //
// Set the icon to be shown in the pane                 //
// hIcon is the handle to the icon                      //
//                                                      //
//////////////////////////////////////////////////////////
void MMStatusCmdUI::SetIcon(HICON hIcon)
{
	MMStatusBar* pStatusBar = (MMStatusBar*)m_pOther;
	ASSERT(pStatusBar != NULL);
	ASSERT_KINDOF(CStatusBar, pStatusBar);
	ASSERT(m_nIndex < m_nIndexMax);
	pStatusBar->GetStatusBarCtrl().SetIcon(m_nIndex,hIcon);

  CString szPaneText;
  pStatusBar->GetPaneText(m_nIndex,szPaneText);
  int cxExtra = 0;
  if(hIcon)
    cxExtra = GetSystemMetrics(SM_CXSMICON);

  MMStatusDrawStruct* pstDrawStruct = pStatusBar->GetPaneStruct(m_nIndex);
  if(pstDrawStruct)
  {
    //if it is owner drawn, we have to set the correct 
    //icon width, so that the drawing routine is able 
    //to leave that much space before drawing the text
    if(hIcon)
      pstDrawStruct->m_cxExtra = cxExtra;
    else
      pstDrawStruct->m_cxExtra = 0;
  }
  CClientDC dc(NULL);
  HFONT hFont,hOldFont;
  //get current font
	hFont = (HFONT)pStatusBar->SendMessage(WM_GETFONT);
	hOldFont = (HFONT)dc.SelectObject(hFont);
  //set width to ( width of the text in current font + width for icon if any )
	SetWidth(dc.GetTextExtent(szPaneText).cx + cxExtra);
  //cleanup
	dc.SelectObject(hOldFont);
}

//////////////////////////////////////////////////////////
//              OnUpdateCmdUI                           //
//                                                      //
// This member function is called by the framework to   //
// update the status of the toolbar or status bar.      //
// We use this as a place to send UPDATE_COMMAND_UI     //
// Msg to the framework so as to let the handlers       //
// Note: we set the m_pMenu to MM_STATUS_PANE_UPDATE    //
// This is a hint to the handlers that it is from the   //
// status bar panes                                     //
// change the status of the pane texts or properties    //
//                                                      //
//////////////////////////////////////////////////////////
void MMStatusBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	MMStatusCmdUI state;
	state.m_pOther = this;
	state.m_pMenu = (CMenu*)MM_STATUS_PANE_UPDATE;
	state.m_nIndexMax = (UINT)m_nCount;
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
		state.m_nIndex++)
	{
		state.m_nID = GetItemID(state.m_nIndex);

		// allow the statusbar itself to have update handlers
		if (CWnd::OnCmdMsg(state.m_nID, CN_UPDATE_COMMAND_UI, &state, NULL))
			continue;

		// allow target (owner) to handle the remaining updates
		state.DoUpdate(pTarget, FALSE);
	}

	// update the dialog controls added to the status bar
	UpdateDialogControls(pTarget, bDisableIfNoHndler);
}

//////////////////////////////////////////////////////////
// Call Relay event on tooltip control so it can handle //
// update it's state related to mouse events            //
//////////////////////////////////////////////////////////
BOOL MMStatusBar::PreTranslateMessage(MSG *pMsg)
{
	switch(pMsg->message)
	{
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		m_oToolTip.RelayEvent (pMsg);
		default :break;
	}
	return CStatusBar::PreTranslateMessage(pMsg);
}

//////////////////////////////////////////////////////////
// OnChildNotify is overridden so as to allow us to     //
// intercept the NM_DBLCLK notifications so that we can //
// post a WM_COMMAND message with the corresponding     //
// pane's command ID                                    //
//////////////////////////////////////////////////////////
BOOL MMStatusBar::OnChildNotify(UINT message,WPARAM wParam,LPARAM lParam,LRESULT* pResult)
{
	switch(message)
	{
	case WM_NOTIFY:
		{
			NMHDR* pNMHDR = (NMHDR*)lParam;
			if(NM_DBLCLK == pNMHDR->code)
			{
				NMMOUSE* pNMMouse = (NMMOUSE*)lParam;
        if(pNMMouse->dwItemSpec >= 0 && 
           pNMMouse->dwItemSpec < (unsigned int)m_nCount)
        {
				  UINT uCommandId = GetItemID(pNMMouse->dwItemSpec);
				  this->GetParent()->SendMessage(WM_COMMAND,uCommandId,0);
        }
			}
		}
		break;
	default:break;
	}
	return CStatusBar::OnChildNotify(message,wParam,lParam,pResult);
}

//////////////////////////////////////////////////////////
// DrawItem is overridden so as to allow us to do       //
// drawing of text for owner drawn panes                //
//////////////////////////////////////////////////////////
void MMStatusBar::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
  MMStatusDrawStruct* pstDrawStruct = (MMStatusDrawStruct*)GetPaneStruct(lpDrawItemStruct->itemID);

  ASSERT(pstDrawStruct);

  CDC oDC;
  oDC.Attach(lpDrawItemStruct->hDC);
  oDC.SetTextColor(pstDrawStruct->m_crTextColor);
  if(DEFAULT_PANE_COLOR == pstDrawStruct->m_crBkColor)
    oDC.SetBkMode(TRANSPARENT);
  else
    oDC.SetBkColor(pstDrawStruct->m_crBkColor);
  lpDrawItemStruct->rcItem.left += 2;
  lpDrawItemStruct->rcItem.left += pstDrawStruct->m_cxExtra;
  oDC.DrawText(pstDrawStruct->m_szText,&lpDrawItemStruct->rcItem,DT_LEFT);
  oDC.Detach();
}

//////////////////////////////////////////////////////////
// SetPaneTextColor is used to set the pane text color  //
// of any pane. 
//////////////////////////////////////////////////////////
void MMStatusBar::SetPaneTextColor(int nIndex,COLORREF crTextColor)
{
  MMStatusDrawStruct* pstDrawStruct = GetPaneStruct(nIndex);
  if(pstDrawStruct && crTextColor != pstDrawStruct->m_crTextColor)
  {
    //it is owner drawn and there is a request to 
    //change the text color
    pstDrawStruct->m_crTextColor = crTextColor;
  }
  else if(NULL == pstDrawStruct)
  {
    //it is a new owner drawn pane.. so create a new struct
    pstDrawStruct = new MMStatusDrawStruct;
    pstDrawStruct->m_cxExtra = 0;
    pstDrawStruct->m_nIndex = nIndex;
    pstDrawStruct->m_crTextColor = crTextColor;
    pstDrawStruct->m_crBkColor = DEFAULT_PANE_COLOR;
    GetPaneText(nIndex,pstDrawStruct->m_szText);
    m_oDrawStructArray.Add(pstDrawStruct);
  }

  //just so as to trigger a repaint
  SendMessage(SB_SETTEXT,nIndex | SBT_OWNERDRAW,(LPARAM)pstDrawStruct);
} 

void MMStatusBar::SetPaneTextBkColor(int nIndex,COLORREF crTextBkColor)
{
  MMStatusDrawStruct* pstDrawStruct = GetPaneStruct(nIndex);
  if(pstDrawStruct && crTextBkColor != pstDrawStruct->m_crBkColor)
  {
    //it is owner drawn and there is a request to 
    //change the text color
    pstDrawStruct->m_crBkColor = crTextBkColor;
  }
  else if(NULL == pstDrawStruct)
  {
    //it is a new owner drawn pane.. so create a new struct
    pstDrawStruct = new MMStatusDrawStruct;
    pstDrawStruct->m_cxExtra = 0;
    pstDrawStruct->m_nIndex = nIndex;
    pstDrawStruct->m_crTextColor = GetSysColor(COLOR_WINDOWTEXT);
    pstDrawStruct->m_crBkColor = crTextBkColor;
    GetPaneText(nIndex,pstDrawStruct->m_szText);
    m_oDrawStructArray.Add(pstDrawStruct);
  }

  //just so as to trigger a repaint
  SendMessage(SB_SETTEXT,nIndex | SBT_OWNERDRAW,(LPARAM)pstDrawStruct);
}

//private method to get the pane struct given pane index
MMStatusDrawStruct* MMStatusBar::GetPaneStruct(int nIndex)
{
  for(int j = 0 ; j < m_oDrawStructArray.GetSize(); j++)
  {
    MMStatusDrawStruct* pDrawStruct = (MMStatusDrawStruct*)m_oDrawStructArray.GetAt(j);
    if(pDrawStruct && pDrawStruct->m_nIndex == nIndex)
      return pDrawStruct;
  }
  return NULL;
}