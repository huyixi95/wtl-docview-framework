// Document.cpp: implementation of the CDocument class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Document.h"
#include "View.h"
#include <atlframe.h>
#include <atlctrls.h>
#include <atlctrlx.h>
#include "DocTemplate.h"
#include "MDIChildFrameImpl.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDocument::CDocument() :
	m_bAutoDelete(TRUE),
	m_bModified(FALSE)
{

}

CDocument::~CDocument()
{
	if( m_pDocTemplate )
	{
		m_pDocTemplate->RemoveDocument(this);
	}
}

void CDocument::UpdateAllViews(CView* pSender, LPARAM lHint, LPVOID pHint)
{
	int count = m_arrViews.GetSize();
	for(int i = 0; i < count; i++)
	{
		CView * pView = m_arrViews[i];
		if(pView != static_cast<CView*>(pSender) )
		{
			pView->Update(pSender, lHint, pHint);
		}
	}
}

CView* CDocument::AddView(CView* pView)
{
	_ASSERTE(pView);
	pView->SetDocument(this);
	m_arrViews.Add(pView);
	OnChangedViewList();
	return pView;
}

void CDocument::RemoveView(CView* pView)
{
	int i = m_arrViews.Find(pView);
	m_arrViews.RemoveAt(i);
	int nCount = m_arrViews.GetSize();
	pView->m_pDocument = NULL;
	OnChangedViewList();    // must be the last thing done to the document
}

int CDocument::GetViewsCount() const
{
	return m_arrViews.GetSize();
}

CView* CDocument::GetView(int pos)
{

	_ASSERTE(pos < m_arrViews.GetSize());
	return m_arrViews[pos];
}

BOOL CDocument::IsModified() const
{
	return m_bModified;
}

void CDocument::SetModifiedFlag(BOOL bModified)
{
	m_bModified = bModified;
}

CDocTemplateBase* CDocument::GetDocTemplate()
{
	return m_pDocTemplate;
}

BOOL CDocument::OnNewDocument()
{ 
	DeleteContents();
	m_strPathName.Empty();      // no path name yet
	SetModifiedFlag(FALSE);     // make clean
	
	return TRUE;
};

	

void CDocument::OnChangedViewList()
{
	// if no more views on the document, delete ourself
	// not called if directly closing the document or terminating the app

	if (m_arrViews.GetSize()==0 && m_bAutoDelete)
	{
		OnCloseDocument();
		return;
	}

	UpdateFrameCounts();
}

BOOL CDocument::CanCloseFrame(CMDIWindow* pFrame)
{

	for(int i = 0; i<m_arrViews.GetSize(); i++)
	{
		// get frame attached to the view
		CView* pView = m_arrViews[i]; 
		_ASSERTE(pView);

		HWND hFrame = pView->GetParentFrame();

		if (hFrame != NULL)
		{
			return TRUE;        // more than one frame refering to us
		}
	}

	// otherwise only one frame that we know about
	return SaveModified();
}

void CDocument::OnCloseDocument()
{
	// destroy all frames viewing this document
	// the last destroy may destroy us
	BOOL bAutoDelete = m_bAutoDelete;
	m_bAutoDelete = TRUE;  // don't destroy document while closing views
	
	for(int i = 0; i<m_arrViews.GetSize(); i++)
	{
		// get frame attached to the view
		CView* pView = m_arrViews[i];
		pView->m_pDocument = NULL;
		_ASSERTE(pView);
		HWND hFrame = pView->GetParentFrame();
		//_ASSERTE(pFrame);
		
		// and close it
//		PreCloseFrame(pFrame);
		::SendMessage(hFrame, WM_DESTROY, 0, 0);
		// will destroy the view as well
	}
	m_bAutoDelete = bAutoDelete;
	
	// clean up contents of document before destroying the document itself
	DeleteContents();
	
	// delete the document if necessary
	if (m_bAutoDelete)
	{
		delete this;
	}
}

void CDocument::UpdateFrameCounts()
{
	// walk all frames of views (mark and sweep approach)
	
	for (int i=0; i<m_arrViews.GetSize(); i++)
	{
		CView* pView = m_arrViews[i];
		_ASSERTE(pView);
		if (pView->IsWindowVisible())   // Do not count invisible windows.
		{
			CMDIFrameBase* pFrame = pView->GetFrame();
			_ASSERTE(pFrame);
			pFrame->m_nWindow = -1;
		}
	}

	// now do it again counting the unique ones
	int nFrames = 0;
	for (i=0; i<m_arrViews.GetSize(); i++)
	{
		CView* pView = m_arrViews[i];
		_ASSERTE(pView);
		if (pView->IsWindowVisible())   // Do not count invisible windows.
		{
			CMDIFrameBase* pFrame = pView->GetFrame();
			if ( pFrame && pFrame->m_nWindow == -1)
			{
				// not yet counted (give it a 1 based number)
				pFrame->m_nWindow = ++nFrames;
			}
		}
	}
		
	// lastly walk the frames and update titles (assume same order)
	// go through frames updating the appropriate one
	int iFrame = 1;
	for (i=0; i<m_arrViews.GetSize(); i++)
	{
		CView* pView = m_arrViews[i];
		_ASSERTE(pView);
		if (pView->IsWindowVisible())   // Do not count invisible windows.
		{
			CMDIFrameBase* pFrame = pView->GetFrame();
			if (pFrame && pFrame->m_nWindow == iFrame)
			{
				if (nFrames == 1)
					pFrame->m_nWindow = 0;      // the only one of its kind
				pFrame->OnUpdateFrameTitle(TRUE);
				iFrame++;
			}
		}
	}
	_ASSERTE(iFrame == nFrames + 1);

}

void CDocument::DeleteContents()
{

}

void CDocument::PreCloseFrame(CMDIWindow *pMDIFrame)
{

}

BOOL CDocument::SaveModified()
{
	if (!IsModified())
		return TRUE;  

	CString prompt;
	prompt.Format(_T("Save changes to %s?"), m_strTitle);
	switch (::MessageBox(::GetActiveWindow(), prompt, _T("Save"), MB_YESNOCANCEL|MB_ICONQUESTION))
	{
	case IDCANCEL:
		return FALSE;       // don't continue
		
	case IDYES:
		// If so, either Save or Update, as appropriate
		if (!DoFileSave())
			return FALSE;       // don't continue
		break;
		
	case IDNO:
		// If not saving changes, revert the document
		break;
		
	default:
		_ASSERTE(FALSE);
		break;
	}
	return TRUE;    // keep going
}

LRESULT CDocument::OnFileSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	DoFileSave();	
	return TRUE;
}

LRESULT CDocument::OnFileSaveAs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (!DoSave(NULL))
		ATLTRACE("Warning: File save-as failed.\n");
	return TRUE;
}


BOOL CDocument::DoFileSave()
{
	DWORD dwAttrib = ::GetFileAttributes(m_strPathName);
	if (dwAttrib & FILE_ATTRIBUTE_READONLY)
	{
		// we do not have read-write access or the file does not (now) exist
		if (!DoSave(NULL))
		{
			ATLTRACE("Warning: File save with new name failed.\n");
			return FALSE;
		}
	}
	else
	{
		if (!DoSave(m_strPathName))
		{
			ATLTRACE("Warning: File save failed.\n");
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CDocument::DoSave(LPCTSTR lpszPathName, BOOL bReplace)
	// Save the document data to a file
	// lpszPathName = path name where to save document file
	// if lpszPathName is NULL then the user will be prompted (SaveAs)
	// note: lpszPathName can be different than 'm_strPathName'
	// if 'bReplace' is TRUE will change file name if successful (SaveAs)
	// if 'bReplace' is FALSE will not change path name (SaveCopyAs)
{
	CString newName = lpszPathName;
	if (newName.IsEmpty())
	{
		CDocTemplateBase* pTemplate = GetDocTemplate();
		_ASSERTE(pTemplate != NULL);

		newName = m_strPathName;
		if (bReplace && newName.IsEmpty())
		{
			int iBad = newName.FindOneOf(_T(":/\\<>|:*?\""));
			if (iBad != -1)
				newName.ReleaseBuffer(iBad);

			// append the default suffix if there is one
			CString strExt;
			if (pTemplate->GetDocString(strExt, CDocTemplateBase::filterExt) &&
				!strExt.IsEmpty())
			{
				_ASSERTE(strExt[0] == '.');
				int iStart = 0;
				int iEnd = strExt.Find(';');
				if (iEnd == -1)
				{
					iEnd = strExt.GetLength();
				}
				newName += strExt.Mid(iStart, iEnd);
			}
		}

		// In MFC, we just call  AfxGetApp()->DoPromptFileName, but 
		// in WTL, we do not have this function, and it's insane implement it.
		// This is workaround to fix that, just send message to window that implements
		// CDocManager.

		DOCMGRDOFILEPROMPT DocFilePromptParam;
		DocFilePromptParam.fileName = newName;
		DocFilePromptParam.nIDSTitle = bReplace ? ATL_IDS_SAVEFILE : ATL_IDS_SAVEFILECOPY;
		DocFilePromptParam.lFlags = FALSE;
		DocFilePromptParam.pTemplate = static_cast<LPVOID>(pTemplate);
		BOOL bResult = (BOOL)::SendMessage(pTemplate->m_hWnd, WM_DOCMGRDOFILEPROMPT, NULL, (LPARAM)(&DocFilePromptParam));

		if (!bResult)
		{
			return FALSE;
		}

		newName = DocFilePromptParam.fileName;
	}	


	CWaitCursor wait;

	if (!OnSaveDocument(newName))
	{
		if (lpszPathName == NULL)
		{
			// be sure to delete the file
			if (::DeleteFile(newName) == FALSE)
			{
				ATLTRACE("Warning: failed to delete file after failed SaveAs.\n");
			}
		}
		return FALSE;
	}

	// reset the title and change the document name
	if (bReplace)
		return SetPathName(newName);

	
	return TRUE;
}

BOOL CDocument::OnSaveDocument(LPCTSTR lpszPathName)
{ 
	// try to open file for write
	BOOL bRes = FALSE;

	bRes = Serialize(lpszPathName, FALSE);

	if (bRes) 
		SetModifiedFlag(FALSE);     // back to unmodified
	return bRes; 
};


BOOL CDocument::OnOpenDocument(LPCTSTR lpszPathName)
{
	BOOL bRes = FALSE;

	DeleteContents();
	SetModifiedFlag();  // dirty during de-serialize

	bRes = Serialize(lpszPathName, TRUE);

	if (bRes) 
		SetModifiedFlag(FALSE);     // back to unmodified
	return bRes; 

}

BOOL CDocument::SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU)
{
	// store the path fully qualified
	TCHAR szFullPath[_MAX_PATH];
	if ( lstrlen(lpszPathName) >= _MAX_PATH )
	{
		ATLTRACE("Invalid Path");
		_ASSERTE(FALSE);
		// requires paths with length < _MAX_PATH
		return FALSE;
	}

	if( AtlFullPath(szFullPath, lpszPathName) == FALSE )
	{
		ATLTRACE("Invalid Path");
		_ASSERTE(FALSE);
		// requires paths with length < _MAX_PATH
		return FALSE;
	}

	m_strPathName = szFullPath;
	_ASSERTE(!m_strPathName.IsEmpty());       // must be set to something
	//m_bEmbedded = FALSE;

	// set the document title based on path name
	TCHAR szTitle[_MAX_FNAME];
	if (AtlGetFileTitle(szFullPath, szTitle, _MAX_FNAME) == 0)
	SetTitle(szTitle);

//	// add it to the file MRU list
//	if (bAddToMRU)
//		AfxGetApp()->AddToRecentFileList(m_strPathName);

	return TRUE;
}

const CString& CDocument::GetPathName() const
{
	return m_strPathName;
}

void CDocument::SetTitle(LPCTSTR lpszTitle)
{
	m_strTitle = lpszTitle;
	UpdateFrameCounts();
}

const CString& CDocument::GetTitle() const
{
	return m_strTitle;
}
