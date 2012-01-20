// View.h: interface for the CView class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIEW_H__D8B151BE_111E_4AA1_8EAB_E17B8312F4B4__INCLUDED_)
#define AFX_VIEW_H__D8B151BE_111E_4AA1_8EAB_E17B8312F4B4__INCLUDED_

#include <atlframe.h>
#include "Document.h"
#include "CreateContext.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDocTemplateBase;
class CDocument;
class CMDIFrameBase;

class CView  
{
	friend class CDocument;
public:

	CView(){ m_pFrame = NULL; m_pDocument=NULL; };
	virtual ~CView(){};
	
	CMDIFrameBase* m_pFrame;

protected:
	CDocument*    m_pDocument;
	
public:
	virtual HWND GetParentFrame()=0;
	virtual CMDIFrameBase* GetFrame(){ _ASSERTE(m_pFrame); return m_pFrame; };

	virtual void Update(CView* pSender, LPARAM lHint, LPVOID pHint) = 0;

	CDocument* GetDocument(){ return m_pDocument; };


	void SetDocument(CDocument* pDoc){ _ASSERTE(pDoc); m_pDocument = pDoc; };

	void UpdateAllDocs();

	virtual BOOL PreTranslateMessage(MSG* pMsg) = 0;

	virtual BOOL IsWindowVisible() = 0;

	virtual void OnActivateFrame(UINT nState, CMDIFrameBase* pFrameWnd){};
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) = 0;

protected:


};


/* 
*	base class for view implementations 
*/
template <class T>
class CAxFormViewImpl
	:	public CAxDialogImpl<T>, 
	public CView
{
public:
	BEGIN_MSG_MAP(CAxFormViewImpl)
		MESSAGE_HANDLER(WM_INITIALUPDATE, InitialUpdate)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

public:
	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
		DWORD dwStyle = 0, DWORD dwExStyle = 0,
		UINT nMenuID = 0, LPVOID lpCreateParam = NULL)
	{
		return CAxDialogImpl<T>::Create(hWndParent, *rect.m_lpRect, (LPARAM)lpCreateParam);
	}

	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
	{
		//UNUSED_PARAMETER (pActivateView);   // unused in release builds

		if (bActivate)
		{
			_ASSERTE(pActivateView == this);

			// take the focus if this frame/view/pane is now active
			//if (IsTopParentActive())
			SetFocus();				
		}
	}
	virtual void Update(CView* pSender, LPARAM lHint, LPVOID pHint)
	{
		T* pT = static_cast<T*>(this);
		pT->OnUpdate(pSender, lHint, pHint);
	}

	virtual void OnUpdate(CView* pSender, LPARAM Hint, LPVOID pHint)	
	{

	}

	virtual void OnInitialUpdate()
	{
		T* pT = static_cast<T*>(this);
		pT->OnUpdate(0, 0, 0);
	}

	virtual void OnFinalMessage(HWND hWnd)	
	{
		if(m_pDocument)
		{
			m_pDocument->RemoveView(this);
		}		

		delete this;	
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CCreateContext<T>* pContext = (CCreateContext<T>*)lParam;
		_ASSERTE(pContext->m_pCurrentDoc);
		_ASSERTE(pContext->m_pCurrentDoc->AddView(this));
		bHandled = TRUE;
		return 0;
	}

	LRESULT InitialUpdate(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		OnInitialUpdate();
		bHandled = TRUE;
		return 0;
	}

	virtual BOOL IsWindowVisible()
	{
		return CAxDialogImpl<T>::IsWindowVisible();
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if((pMsg->message < WM_KEYFIRST || pMsg->message > WM_KEYLAST) &&
			(pMsg->message < WM_MOUSEFIRST || pMsg->message > WM_MOUSELAST))
			return FALSE;

		HWND hWndCtl = ::GetFocus();
		if(IsChild(hWndCtl))
		{
			// find a direct child of the dialog from the window that has focus
			while(::GetParent(hWndCtl) != m_hWnd)
				hWndCtl = ::GetParent(hWndCtl);

			// give control a chance to translate this message
			if(::SendMessage(hWndCtl, WM_FORWARDMSG, 0, (LPARAM)pMsg) != 0)
				return TRUE;
		}
		return CWindow::IsDialogMessage(pMsg);
	}
};


/* 
*	base class for view implementations 
*/
template <class T>
class CViewImpl 
	:	public CWindowImpl<T>, 
		public CView
{
public:
	
	BEGIN_MSG_MAP(CViewImpl)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
	END_MSG_MAP()

protected:
	

public:
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
	{
		//UNUSED_PARAMETER (pActivateView);   // unused in release builds
		
		if (bActivate)
		{
			_ASSERTE(pActivateView == this);
			
			// take the focus if this frame/view/pane is now active
			//if (IsTopParentActive())
			SetFocus();				
		}
	}
	virtual void Update(CView* pSender, LPARAM lHint, LPVOID pHint)
	{
		T* pT = static_cast<T*>(this);
		pT->OnUpdate(pSender, lHint, pHint);
	}

	virtual void OnUpdate(CView* pSender, LPARAM Hint, LPVOID pHint)	
	{
		Invalidate(TRUE);
	}
	virtual void OnInitialUpdate()
	{
		T* pT = static_cast<T*>(this);
		pT->OnUpdate(0, 0, 0);
	}
	virtual void OnFinalMessage(HWND hWnd)	
	{
		if(m_pDocument)
		{
			m_pDocument->RemoveView(this);
		}		

		delete this;	
	}
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
		if( lpcs == NULL || lpcs->lpCreateParams == NULL)
		{
			_ASSERTE(FALSE);
			return -1;
		}
		CCreateContext<T>* pContext = (CCreateContext<T>*)lpcs->lpCreateParams;
		_ASSERTE(pContext->m_pCurrentDoc);
		_ASSERTE(pContext->m_pCurrentDoc->AddView(this));

		bHandled = TRUE;
		
		return 0;
	}
	HWND GetParentFrame()
	{
		T* pT = static_cast<T*>(this);
		HWND hWnd = pT->GetParent();
		while(!(::GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_MDICHILD))
		{
			hWnd = ::GetParent(hWnd);
		} 
		
		return hWnd;
	}

	virtual BOOL IsWindowVisible()
	{
		return CWindowImpl<T>::IsWindowVisible();
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CPaintDC dc(m_hWnd);
		
		//TODO: Add your drawing code here
		
		return 0;
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return TRUE;
	}

};


#endif // !defined(AFX_VIEW_H__D8B151BE_111E_4AA1_8EAB_E17B8312F4B4__INCLUDED_)
