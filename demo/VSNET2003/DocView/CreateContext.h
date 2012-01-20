// CreateContext.h: interface for the CCreateContext class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CREATECONTEXT_H__2EF4F539_8D99_4E7F_A4FC_344BDACD40B5__INCLUDED_)
#define AFX_CREATECONTEXT_H__2EF4F539_8D99_4E7F_A4FC_344BDACD40B5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDocTemplateBase;
class CView;
class CDocument;
class CMDIFrameBase;

template<class TView>
class CCreateContext  
{
public:
	CDocument* m_pCurrentDoc;
	TView* m_pCurrentView;
	CDocTemplateBase* m_pNewDocTemplate;
	CMDIFrameBase*		m_pCurrentFrame;
	
	CCreateContext() :
		m_pCurrentDoc(NULL),
		m_pCurrentView(NULL),
		m_pNewDocTemplate(NULL),
		m_pCurrentFrame(NULL)
	{ 
	
	}
	
	
	/* constructor based on a different context handles more than
	one view per document (splitter windows) */     
	CCreateContext(CCreateContext* pContext)
	{ 
		m_pCurrentDoc = pContext->m_pCurrentDoc;
		m_pNewDocTemplate = pContext->m_pNewDocTemplate;
		m_pCurrentView = pContext->m_pCurrentView;
		m_pCurrentFrame = pContext->m_pCurrentFrame;
	}
	
	

};

#endif // !defined(AFX_CREATECONTEXT_H__2EF4F539_8D99_4E7F_A4FC_344BDACD40B5__INCLUDED_)
