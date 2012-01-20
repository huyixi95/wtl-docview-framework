// MDIChildFrameImpl.cpp: implementation of the CMDIChildFrameImpl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MDIChildFrameImpl.h"
#include "View.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CView* CMDIFrameBase::GetActiveView()
{
	return m_pViewActive;
}

CDocument* CMDIFrameBase::GetActiveDocument()
{
	CView* pView = GetActiveView();
	return pView != NULL ? pView->GetDocument() : NULL;
}

void CMDIFrameBase::SetActiveView(CView* pViewNew, BOOL bNotify)
{
	CView* pViewOld = m_pViewActive;
	if (pViewNew == pViewOld)
		return;     // do not re-activate if SetActiveView called more than once
	
	// deactivate the old one
	if (pViewOld != NULL)
		pViewOld->OnActivateView(FALSE, pViewNew, pViewOld);
	
	// if the OnActivateView moves the active window,
	//    that will veto this change
	if (m_pViewActive != NULL)
		return;     // already set
	m_pViewActive = pViewNew;
	
	// activate
	if (pViewNew != NULL && bNotify)
		pViewNew->OnActivateView(TRUE, pViewNew, pViewOld);
}
