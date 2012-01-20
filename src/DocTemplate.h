// DocTemplate.h: interface for the CDocTemplate class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DOCTEMPLATE_H__6BBF7989_8F5E_456A_B88F_9757F27AB516__INCLUDED_)
#define AFX_DOCTEMPLATE_H__6BBF7989_8F5E_456A_B88F_9757F27AB516__INCLUDED_

#include "DocTemplateBase.h"
#include <atlctrls.h>
#include <atlctrlx.h>
#include "CreateContext.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/* the doc template */
template <class TDoc, class TView, class TFrame, int nID>
class CDocTemplate : public CDocTemplateBase
{
	CSimpleArray<CDocument*> m_arrDocuments;

	UINT m_nUntitledCount;   // start at 0, for "Document1" title

public:
	CDocTemplate()
	{
		m_nUntitledCount = 0;
		LoadTemplate();
	}

	~CDocTemplate()
	{
		for (int i = 0; i< m_arrDocuments.GetSize(); i++)
		{
			CDocument* pDoc = m_arrDocuments[i];
			_ASSERTE(pDoc);

			delete pDoc;
		}

		m_arrDocuments.RemoveAll();
	}

 	virtual void LoadTemplate()
	{
		if( m_strDocStrings.IsEmpty() )
		{
			m_strDocStrings.LoadString(nID);
		}
	}

	virtual TFrame* CreateNewFrame(CDocument* pDoc)
	{		
		TFrame::GetWndClassInfo().m_uCommonResourceID = nID;
		TFrame* pFrame = new TFrame;

		CCreateContext<TView> context;
		context.m_pCurrentDoc = pDoc;
		context.m_pNewDocTemplate = this;
		context.m_pCurrentFrame = pFrame;
		pFrame->CreateEx(m_hWndClient, NULL, NULL, 0, 0, &context); 

		return pFrame;
	}

	CDocument* CreateNewDocument()
	{
		CDocument* pDoc = new TDoc;
		AddDocument(pDoc);
		return pDoc;
	}

	virtual CDocument* OpenDocumentFile(LPCTSTR lpszPathName,BOOL bMakeVisible = TRUE)
	{
		CDocument* pDoc = CreateNewDocument();
		if (pDoc == NULL)
		{
			ATLTRACE("CDocTemplate::CreateNewDocument returned NULL.\n");
			MessageBox(m_hWnd,"Failed to create document", "Error", MB_ICONEXCLAMATION);
			return NULL;
		}
		TFrame* pFrame = CreateNewFrame(pDoc);     
		if (pFrame == NULL)
		{
			delete pDoc;
			ATLTRACE("CDocTemplate::CreateNewFrame returned NULL.\n");
			MessageBox(m_hWnd,"Failed to create document", "Error", MB_ICONEXCLAMATION);
			return NULL;
		}

		if( lpszPathName == NULL )
		{
			// create a new document - with default document name
			SetDefaultTitle(pDoc);

			if(!pDoc->OnNewDocument())
			{
				ATLTRACE("CDocument::OnNewDocument returned FALSE.\n");
				pFrame->DestroyWindow();
				return NULL;
			}

			// it worked, now bump untitled count
			m_nUntitledCount++;
		}
		else
		{
			// open an existing document
			CWaitCursor wait;
			if (!pDoc->OnOpenDocument(lpszPathName))
			{
				// user has be alerted to what failed in OnOpenDocument
				ATLTRACE("CDocument::OnOpenDocument returned FALSE.\n");
				pFrame->DestroyWindow();
				return NULL;
			}
			if (!pDoc->SetPathName(lpszPathName))
			{
				MessageBox(m_hWnd,"Failed to open file", "Error", MB_ICONEXCLAMATION);
				pFrame->DestroyWindow();
				return NULL;
			}
		}

		InitialUpdateFrame(pFrame, pDoc, bMakeVisible);
		return pDoc;
	}

	virtual void InitialUpdateFrame (CMDIFrameBase* pFrame, CDocument* pDoc, BOOL bMakeVisible = TRUE)
	{ 
		// just delagate to implementation in CFrameWnd
		pFrame->InitialUpdateFrame(pDoc, bMakeVisible);
	}

	virtual CDocument* OpenDocument(BOOL bNew = FALSE)
	{
		return NULL;
	}

	CDocument* GetDocument(int pos)
	{
		return m_arrDocuments[pos];
	}

	virtual CDocument* GetDocument(HWND hChildFrame)
	{
		for (int i = 0; i< m_arrDocuments.GetSize(); i++)
		{
			CDocument* pDoc = m_arrDocuments[i];
			_ASSERTE(pDoc);

			int nViewsCount = pDoc->GetViewsCount();
			for(int j=0; j<nViewsCount; j++)
			{
				CView* pView = pDoc->GetView(j);
				if( pView->GetParentFrame() == hChildFrame )
				{
					return pDoc;
				}
			}
		}

		return NULL;
	}

	virtual void CloseAllDocuments (BOOL bEndSession)
	{
		
	}

	virtual void AddDocument(CDocument* pDoc)
	{
		pDoc->m_pDocTemplate = this;
		m_arrDocuments.Add(pDoc);
	}

	virtual CDocument* RemoveDocument(CDocument* pDoc)
	{
		_ASSERTE(m_arrDocuments.Remove(pDoc));
		int nCount = m_arrDocuments.GetSize();
		pDoc->m_pDocTemplate = NULL;
		return pDoc;
	}

	virtual Confidence MatchDocType(LPCTSTR lpszPathName,CDocument*& rpDocMatch)
	{
		_ASSERTE(lpszPathName != NULL);
		rpDocMatch = NULL;

		// go through all documents
		for (int i = 0; i< m_arrDocuments.GetSize(); i++)
		{
			CDocument* pDoc = m_arrDocuments[i];
			_ASSERTE(pDoc);
			CString sDocFileName = pDoc->GetPathName();
			if(sDocFileName.CompareNoCase(lpszPathName) == 0)
			{
				// already open
				rpDocMatch = pDoc;
				return yesAlreadyOpen;
			}
		}

		// see if it matches our default suffix
		CString strFilterExt;
		if (GetDocString(strFilterExt, CDocTemplateBase::filterExt) && !strFilterExt.IsEmpty())
		{
			// see if extension matches
			_ASSERTE(strFilterExt[0] == '.');
			LPCTSTR lpszDot = ::PathFindExtension(lpszPathName);
			if (lpszDot != NULL && lstrcmpi(lpszDot, strFilterExt) == 0)
				return yesAttemptNative; // extension matches, looks like ours
		}

		// otherwise we will guess it may work
		return yesAttemptForeign;
	}


	int GetDocumentsCount() const
	{
		return m_arrDocuments.GetSize();
	}

	virtual BOOL SaveAllModified ()
	{
		for (int i = 0; i< m_arrDocuments.GetSize(); i++)
		{
			CDocument* pDoc = m_arrDocuments[i];
			_ASSERTE(pDoc);

			if(!(pDoc->SaveModified()))
			{
				return FALSE;
			}
		}

		return TRUE;
	}

	virtual BOOL GetDocString(CString& rString, enum DocStringIndex i)
	{
		return ExtractSubString(rString, m_strDocStrings, (int)i);
	}

	virtual void SetDefaultTitle (CDocument* pDocument)
	{
		CString strDocName;
		if (GetDocString(strDocName, CDocTemplate::docName) &&
			!strDocName.IsEmpty())
		{
			TCHAR szNum[8];
			wsprintf(szNum, _T("%d"), m_nUntitledCount+1);
			strDocName += szNum;
		}
		else
		{
			// use generic 'untitled' - ignore untitled count
			strDocName = _T("Untitled");
		}
		pDocument->SetTitle(strDocName);
	}

};

#endif // !defined(AFX_DOCTEMPLATE_H__6BBF7989_8F5E_456A_B88F_9757F27AB516__INCLUDED_)
