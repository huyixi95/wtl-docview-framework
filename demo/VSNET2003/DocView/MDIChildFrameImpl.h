// MDIChildFrameImpl.h: interface for the CMDIChildFrameImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MDICHILDFRAMEIMPL_H__F4F3EB88_AA68_4FE6_8DAE_C0A06734C568__INCLUDED_)
#define AFX_MDICHILDFRAMEIMPL_H__F4F3EB88_AA68_4FE6_8DAE_C0A06734C568__INCLUDED_

#include <atlframe.h>
#include "CreateContext.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMDIFrameBase
{
protected:
	CView* m_pViewActive;
public:
	CMDIFrameBase() : m_nWindow(-1), m_pViewActive(NULL)
	{

	}

	virtual void OnUpdateFrameTitle(BOOL bAddToTitle) = 0;
	virtual void UpdateFrameTitleForDocument(LPCTSTR lpszDocName) = 0;
	virtual void InitialUpdateFrame (CDocument* pDoc, BOOL bMakeVisible)=0;

	virtual void BringToTop(int nCmdShow)=0;

	int m_nWindow;
	CString m_strTitle;

protected:
	virtual CView* GetActiveView();

	virtual CDocument* GetActiveDocument();

	void SetActiveView(CView* pViewNew, BOOL bNotify = TRUE);
	virtual void ActivateFrame(int nCmdShow = -1)=0;

	
};

template <class TFrame, class TView, int nID>
class CMDIChildFrameImpl  : 
			public CMDIChildWindowImpl<TFrame>,
			public CMDIFrameBase
{
	
public:
	CMDIChildFrameImpl()
	{
		m_pViewActive = NULL;
	}
	virtual ~CMDIChildFrameImpl()
	{
	}

	virtual HWND GetMDIFrame()
	{
		return ::GetParent(m_hWndClient);
	}

	DECLARE_FRAME_WND_CLASS(NULL, nID)
		
		
	BEGIN_MSG_MAP(CMDIChildFrameImpl)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_FORWARDMSG, OnForwardMsg)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		CHAIN_MSG_MAP(CMDIChildWindowImpl<TFrame>)
		if (uMsg == WM_COMMAND)
			if (GetActiveDocument())
				CHAIN_MSG_MAP_MEMBER((*GetActiveDocument()))
	END_MSG_MAP()

	virtual void InitialUpdateFrame (CDocument* pDoc, BOOL bMakeVisible)
	{
		// if the frame does not have an active view, set to first pane
		CView* pView = GetActiveView();
		_ASSERTE(pView);

		if (bMakeVisible)
		{

			// send initial update to all views (and other controls) in the frame
			SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE);
			
			// give view a chance to save the focus (CFormView needs this)
			if (pView != NULL)
				pView->OnActivateFrame(WA_INACTIVE, this);
			
			int nCmdShow = -1; 
			ActivateFrame(nCmdShow);

			if (pView != NULL)
				pView->OnActivateView(TRUE, pView, pView);

		}
		
		// update frame counts and frame title (may already have been visible)
		if (pDoc != NULL)
			pDoc->UpdateFrameCounts();
		OnUpdateFrameTitle(TRUE);
	}

	void BringToTop(int nCmdShow)
	{
		// place the window on top except for certain nCmdShow
		if (nCmdShow != SW_HIDE &&
			nCmdShow != SW_MINIMIZE && nCmdShow != SW_SHOWMINNOACTIVE &&
			nCmdShow != SW_SHOWNA && nCmdShow != SW_SHOWNOACTIVATE)
		{
			// if no last active popup, it will return m_hWnd
			HWND hWndLastPop = ::GetLastActivePopup(m_hWnd);
			::BringWindowToTop(hWndLastPop);
		}
	}

	virtual void OnUpdateFrameTitle(BOOL bAddToTitle)
	{
		CDocument* pDocument = GetActiveDocument();
		if (bAddToTitle)
		{
			TCHAR szText[256+_MAX_PATH];
			if (pDocument == NULL)
				lstrcpy(szText, m_strTitle);
			else
				lstrcpy(szText, pDocument->GetTitle());
			if (m_nWindow > 0)
				wsprintf(szText + lstrlen(szText), _T(":%d"), m_nWindow);
			
			// set title if changed, but don't remove completely
			::SetWindowText(m_hWnd, szText);
		}
	}

	virtual void UpdateFrameTitleForDocument(LPCTSTR lpszDocName)
	{
		// copy first part of title loaded at time of frame creation
		TCHAR szText[256+_MAX_PATH];
		
		// get name of currently active view
		lstrcpy(szText, m_strTitle);
		if (lpszDocName != NULL)
		{
			lstrcat(szText, _T(" - "));
			lstrcat(szText, lpszDocName);
			// add current window # if needed
			if (m_nWindow > 0)
				wsprintf(szText + lstrlen(szText), _T(":%d"), m_nWindow);
		}
		// set title if changed, but don't remove completely
		// Note: will be excessive for MDI Frame with maximized child
		::SetWindowText(m_hWnd, szText);
	}

	virtual void ActivateFrame(int nCmdShow = -1)
	{
		BOOL bVisibleThen = (GetStyle() & WS_VISIBLE) != 0;
		CMDIWindow frameWnd = GetMDIFrame();
		frameWnd.m_hWndMDIClient = m_hWndMDIClient;
		_ASSERTE(frameWnd.m_hWnd);

		// determine default show command
		if (nCmdShow == -1)
		{
			// get maximized state of frame window (previously active child)
			BOOL bMaximized;			
			frameWnd.MDIGetActive(&bMaximized);		

			// convert show command based on current style
			DWORD dwStyle = GetStyle();
			if (bMaximized || (dwStyle & WS_MAXIMIZE))
				nCmdShow = SW_SHOWMAXIMIZED;
			else if (dwStyle & WS_MINIMIZE)
				nCmdShow = SW_SHOWMINIMIZED;
		}

		// finally, show the window
		//CMDIFrameBase::ActivateFrame(nCmdShow);
		if (nCmdShow == -1)
		{
			if (!IsWindowVisible())
				nCmdShow = SW_SHOWNORMAL;
			else if (IsIconic())
				nCmdShow = SW_RESTORE;
		}

		// bring to top before showing
		BringToTop(nCmdShow);

		if (nCmdShow != -1)
		{
			// show the window as specified
			ShowWindow(nCmdShow);

			// and finally, bring to top after showing
			BringToTop(nCmdShow);
		}

		// update the Window menu to reflect new child window
		::SendMessage(m_hWndMDIClient, WM_MDIREFRESHMENU, 0, 0);

		// Note: Update the m_bPseudoInactive flag.  This is used to handle the
		//  last MDI child getting hidden.  Windows provides no way to deactivate
		//  an MDI child window.

		BOOL bVisibleNow = (GetStyle() & WS_VISIBLE) != 0;
		if (bVisibleNow == bVisibleThen)
			return;

		if (!bVisibleNow)
		{
			// get current active window according to Windows MDI
			HWND hWnd = (HWND)::SendMessage(m_hWndMDIClient, WM_MDIGETACTIVE, 0, 0);
			if (hWnd != m_hWnd)
			{
				// not active any more -- window must have been deactivated
				//_ASSERTE(!m_bPseudoInactive);
				return;
			}

			// check next window
			_ASSERTE(hWnd != NULL);
			frameWnd.MDINext(0);

			// see if it has been deactivated now...
			hWnd = (HWND)::SendMessage(m_hWndMDIClient,
				WM_MDIGETACTIVE, 0, 0);
			if (hWnd == m_hWnd)
			{
				// still active -- fake deactivate it
				_ASSERTE(hWnd != NULL);
				BOOL bHandled;
				OnMDIActivate(0, NULL, NULL, bHandled);
			}
		}
	}

protected:
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		
		LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
		LPMDICREATESTRUCT lpms = (LPMDICREATESTRUCT)lpcs->lpCreateParams;
		
		CCreateContext<TView>* pContext = (CCreateContext<TView>*)lpms->lParam;		
		if( pContext )
		{
			_ASSERTE(pContext->m_pCurrentView == NULL);	
			m_pViewActive = pContext->m_pCurrentView = new TView;
			m_pViewActive->m_pFrame = this;
			m_hWndClient = pContext->m_pCurrentView->Create(m_hWnd, RECT(), NULL, 0, 0, 0U, pContext);
			
			bHandled = TRUE;
		}
		else
		{
			bHandled = FALSE;
		}
		
		return 1;
	}

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;	
	}
	
	LRESULT OnForwardMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LPMSG pMsg = (LPMSG)lParam;
		
		if(CMDIChildWindowImpl<TFrame>::PreTranslateMessage(pMsg))
			return TRUE;

		bHandled = FALSE;
		
		return m_pViewActive != NULL ? m_pViewActive->PreTranslateMessage(pMsg) : FALSE;
	}

	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// Note: only queries the active document
		CDocument* pDocument = GetActiveDocument();
		if (pDocument != NULL && !pDocument->CanCloseFrame(this))
		{
			// document can't close right now -- don't close it
			bHandled = TRUE;
			return 1;
		}

		// detect the case that this is the last frame on the document and
		// shut down with OnCloseDocument instead.
		if (pDocument != NULL && pDocument->m_bAutoDelete)
		{
			BOOL bOtherFrame = FALSE;
			for (int i=0; i<pDocument->GetViewsCount(); i++)
			{
				CView* pView = pDocument->GetView(i);
				_ASSERTE(pView);
				if (pView->GetParentFrame() != m_hWnd)
				{
					bOtherFrame = TRUE;
					break;
				}
			}
			if (!bOtherFrame)
			{
				pDocument->OnCloseDocument();
				bHandled = FALSE;
				return 0;
			}
			
			// allow the document to cleanup before the window is destroyed
			pDocument->PreCloseFrame(this);
		}

		bHandled = FALSE;
		return 0;
	}
};

#endif // !defined(AFX_MDICHILDFRAMEIMPL_H__F4F3EB88_AA68_4FE6_8DAE_C0A06734C568__INCLUDED_)
