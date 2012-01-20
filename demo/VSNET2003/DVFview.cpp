// DVFView.cpp : implementation of the CDVFView class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "DVFDocument.h"
#include "DVFView.h"

BOOL CDVFView::PreTranslateMessage(MSG* pMsg)
{
	pMsg;
	return FALSE;
}

LRESULT CDVFView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CDVFDocument* pDoc = static_cast<CDVFDocument*>(GetDocument());
	CPaintDC dc(m_hWnd);

	//TODO: Add your drawing code here
	dc.TextOut(10,10,pDoc->text , lstrlen(pDoc->text) );

	return 0;
}

LRESULT CDVFView::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	UINT Flags = (UINT)wParam;
	CPoint Pt = CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	CDVFDocument* pDoc = static_cast<CDVFDocument*>(GetDocument());

	CString sWork;
	sWork =  ("Rodrigo Pinho");

	lstrcpy(pDoc->text, sWork);

	//Update(this, NULL,NULL);
	pDoc->UpdateAllViews(NULL);
	return 0;
}
