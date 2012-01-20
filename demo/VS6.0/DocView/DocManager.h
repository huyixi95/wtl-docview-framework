// DocManager.h: interface for the CDocManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DOCMANAGER_H__06A6B883_D4F1_47D1_8EE7_1523D7E17A66__INCLUDED_)
#define AFX_DOCMANAGER_H__06A6B883_D4F1_47D1_8EE7_1523D7E17A66__INCLUDED_

#include "resource.h"
#include "DocTemplateBase.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDocTemplateBase;
class CDocument;

/* the doc manager */
template <class TMainFrame>
class CDocManager : public CMessageMap
{
	BEGIN_MSG_MAP(CDocManager<TMainFrame>)
		MESSAGE_HANDLER(WM_DOCMGRDOFILEPROMPT, OnDoPromptFileName)
		COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
		COMMAND_ID_HANDLER(ID_FILE_OPEN, OnFileOpen)
	END_MSG_MAP()
public:
	CDocManager()
	{
		
	}

	virtual ~CDocManager()
	{
		for (int i =0; i< m_arrTemplates.GetSize(); i++)
		{
			CDocTemplateBase* pTemplate = m_arrTemplates[i];
			_ASSERTE(pTemplate);
			if( pTemplate->m_bAutodelete )
			{
				delete pTemplate;
			}
		}
	}

	template <class TDocTemplate>
	int AddDocTemplate(TDocTemplate* pDocTemplate)
	{
		_ASSERTE(pDocTemplate != NULL);
		pDocTemplate->m_hWndClient = static_cast<TMainFrame*>(this)->m_hWndClient;
		pDocTemplate->m_hWnd = static_cast<TMainFrame*>(this)->m_hWnd;
		CDocTemplateBase* pDocBase = static_cast<CDocTemplateBase*>(pDocTemplate);
		m_arrTemplates.Add(pDocBase);
		return m_arrTemplates.GetSize() - 1;
	}

	int GetTemplatesCount() const
	{
		return m_arrTemplates.GetSize();
	}



	CDocTemplateBase* GetDocTemplate(int pos)
	{
		_ASSERTE(pos < m_arrTemplates.GetSize());
		return m_arrTemplates[pos];
	}

	LRESULT OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if (m_arrTemplates.GetSize()==0)
		{
			ATLTRACE("Error: no document templates registered with CWinApp.\n");
			_ASSERTE(FALSE);
			return FALSE;
		}

		CDocTemplateBase* pTemplate = m_arrTemplates[0];
		if (m_arrTemplates.GetSize() > 1)
		{
			// more than one document template to choose from
			// bring up dialog prompting user
			/*
			CNewTypeDlg dlg(&m_templateList);
			int nID = dlg.DoModal();
			if (nID == IDOK)
				pTemplate = dlg.m_pSelectedTemplate;
			else
				return FALSE;     // none - cancel operation
			*/
		}

		_ASSERTE(pTemplate != NULL);

		pTemplate->OpenDocumentFile(NULL);
		// if returns NULL, the user has already been alerted

		return TRUE;
	}

	LRESULT OnFileOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CString newName;
		if (!DoPromptFileName(newName, ATL_IDS_OPENFILE,
			OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, TRUE, NULL))
			return FALSE; // open cancelled

		OpenDocumentFile(newName);

		return TRUE;
	}

	// open named file
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName) 
	{
		TMainFrame* pMainFrame = static_cast<TMainFrame*>(this);
		CDocTemplateBase::Confidence bestMatch = CDocTemplateBase::noAttempt;
		CDocTemplateBase* pBestTemplate = NULL;
		CDocument* pOpenDocument = NULL;

		// Finds the best template
		TCHAR szPath[_MAX_PATH];
		_ASSERTE(lstrlen(lpszFileName) < (sizeof(szPath)/sizeof(szPath[0]) ));
		TCHAR szTemp[_MAX_PATH];
		if (lpszFileName[0] == '\"')
			++lpszFileName;
		lstrcpyn(szTemp, lpszFileName, _MAX_PATH);
		LPTSTR lpszLast = _tcsrchr(szTemp, '\"');
		if (lpszLast != NULL)
			*lpszLast = 0;

		if( AtlFullPath(szPath, szTemp) == FALSE )
		{
			_ASSERTE(FALSE);
			return NULL; // We won't open the file. DocView requires paths with length < _MAX_PATH
		}

		TCHAR szLinkName[_MAX_PATH];
		if ( AtlResolveShortcut(pMainFrame->m_hWnd ,szPath, szLinkName, _MAX_PATH))
			lstrcpy(szPath, szLinkName);

		int i;
		for(i=0; i < m_arrTemplates.GetSize(); i++)
		{
			CDocTemplateBase* pTemplate = m_arrTemplates[i];
			_ASSERTE(pOpenDocument == NULL);

			CDocTemplateBase::Confidence match;
			match = pTemplate->MatchDocType(szPath, pOpenDocument);
			if (match > bestMatch)
			{
				bestMatch = match;
				pBestTemplate = pTemplate;
			}
			if (match == CDocTemplateBase::yesAlreadyOpen)
				break;      // stop here
		}

		if (pOpenDocument != NULL)
		{
			if (pOpenDocument->GetViewsCount() == 0)
			{
				ATLTRACE("Error: Can not find a view for document to activate.\n");
			}
			for(i=0; i < pOpenDocument->GetViewsCount(); i++)
			{
				CView* pView = pOpenDocument->GetView(i); 
				HWND hFrame = pView->GetParentFrame();

				if (hFrame == NULL)
					ATLTRACE("Error: Can not find a frame for document to activate.\n");

				HWND hWndLastPop = ::GetLastActivePopup(hFrame);
				::BringWindowToTop(hWndLastPop);
			}

			return pOpenDocument;
		}

		if (pBestTemplate == NULL)
		{
			MessageBox(pMainFrame->m_hWnd, "Failed to open document.", "Error", MB_ICONEXCLAMATION);
			return NULL;
		}

		return pBestTemplate->OpenDocumentFile(szPath);	
	}

	// save before exit
	virtual BOOL SaveAllModified()
	{
		for(int i=0; i < m_arrTemplates.GetSize(); i++)
		{
			CDocTemplateBase* pTempl = m_arrTemplates[i];
			if( !pTempl->SaveAllModified() )
			{
				return FALSE;
			}
		}

		return TRUE;
	}
	// close documents before exiting
	virtual void CloseAllDocuments(BOOL bEndSession)
	{
		for(int i=0; i < m_arrTemplates.GetSize(); i++)
		{
			CDocTemplateBase* pTempl = m_arrTemplates[i];
			pTempl->CloseAllDocuments(bEndSession);
		}
	}
	virtual int GetOpenDocumentCount()
	{
		int nCount=0;
		for(int i=0; i < m_arrTemplates.GetSize(); i++)
		{
			CDocTemplateBase* pTempl = m_arrTemplates[i];
			nCount += pTempl->GetDocumentsCount();
		}

		return nCount;
	}

	LRESULT OnDoPromptFileName(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		DOCMGRDOFILEPROMPT* pDocFilePromptParam = (DOCMGRDOFILEPROMPT*)(lParam);
		CDocTemplateBase* pTemplate = static_cast<CDocTemplateBase*>(pDocFilePromptParam->pTemplate);
		return DoPromptFileName(pDocFilePromptParam->fileName, pDocFilePromptParam->nIDSTitle, pDocFilePromptParam->lFlags, pDocFilePromptParam->bOpenFileDialog, pTemplate);
	};
	BOOL DoPromptFileName(CString& fileName, UINT nIDSTitle, DWORD lFlags, BOOL bOpenFileDialog, CDocTemplateBase* pTemplate)
	{
		CFileDialog dlgFile(bOpenFileDialog, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, NULL);

		CString title;
		_ASSERTE(title.LoadString(nIDSTitle));

		dlgFile.m_ofn.Flags |= lFlags;

		CString strFilter;
		CString strDefault;
		if (pTemplate != NULL)
		{
			_AppendFilterSuffix(strFilter, dlgFile.m_ofn, pTemplate, &strDefault);
		}
		else
		{
			BOOL bFirst = TRUE;
			for(int i=0; i < m_arrTemplates.GetSize(); i++)
			{
				CDocTemplateBase* pTempl = m_arrTemplates[i];
				_AppendFilterSuffix(strFilter, dlgFile.m_ofn, pTempl,
					bFirst ? &strDefault : NULL);
				bFirst = FALSE;
			}
		}

		// append the "*.*" all files filter
		CString allFilter;
		_ASSERTE(allFilter.LoadString(ATL_IDS_ALLFILTER));
		strFilter += allFilter;
		strFilter += (TCHAR)'\0';   // next string please
		strFilter += _T("*.*");
		strFilter += (TCHAR)'\0';   // last string
		dlgFile.m_ofn.nMaxCustFilter++;

		dlgFile.m_ofn.lpstrFilter = strFilter;
		dlgFile.m_ofn.lpstrTitle = title;
		dlgFile.m_ofn.lpstrFile = fileName.GetBuffer(_MAX_PATH);

		INT_PTR nResult = dlgFile.DoModal();
		fileName.ReleaseBuffer();
		return nResult == IDOK;
	}


protected:
	CSimpleArray< CDocTemplateBase * > m_arrTemplates;

	static BOOL _SaveAllModified(CDocTemplateBase* pTemplate)
	{
		_ASSERTE(pTemplate);
		return pTemplate->SaveAllModified();
	}


private:
	void _AppendFilterSuffix(CString& filter, OPENFILENAME& ofn,
		CDocTemplateBase* pTemplate, CString* pstrDefaultExt)
	{
		_ASSERTE(pTemplate);

		CString strFilterExt, strFilterName;
		if (pTemplate->GetDocString(strFilterExt, CDocTemplateBase::filterExt) &&
			!strFilterExt.IsEmpty() &&
			pTemplate->GetDocString(strFilterName, CDocTemplateBase::filterName) &&
			!strFilterName.IsEmpty())
		{
			if (pstrDefaultExt != NULL)
				pstrDefaultExt->Empty();

			// add to filter
			filter += strFilterName;
			_ASSERTE(!filter.IsEmpty());  // must have a file type name
			filter += (TCHAR)'\0';  // next string please

			int iStart = 0;
			int iEnd = -1;
			int iWork = -1;
			do
			{
				iWork = strFilterExt.Find(';');
				iEnd = iWork;
				if (iWork == -1)
					iEnd = strFilterExt.GetLength();
				
				CString strExtension = strFilterExt.Mid(iStart, iEnd);

				if (!(strExtension.IsEmpty()))
				{

					// a file based document template - add to filter list

					// If you hit the following ASSERT, your document template 
					// string is formatted incorrectly.  The section of your 
					// document template string that specifies the allowable file
					// extensions should be formatted as follows:
					//    .<ext1>;.<ext2>;.<ext3>
					// Extensions may contain wildcards (e.g. '?', '*'), but must
					// begin with a '.' and be separated from one another by a ';'.
					// Example:
					//    .jpg;.jpeg
					_ASSERTE(strExtension[0] == '.');
					if ((pstrDefaultExt != NULL) && pstrDefaultExt->IsEmpty())
					{
						// set the default extension
						*pstrDefaultExt = strExtension.Mid( 1 );  // skip the '.'
						ofn.lpstrDefExt = const_cast< LPTSTR >((LPCTSTR)(*pstrDefaultExt));
						ofn.nFilterIndex = ofn.nMaxCustFilter + 1;  // 1 based number
					}

					filter += (TCHAR)'*';
					filter += strExtension;
					filter += (TCHAR)';';  // Always append a ';'.  The last ';' will get replaced with a '\0' later.
				}
			} while (iWork != -1);

			filter.SetAt( filter.GetLength()-1, '\0' );;  // Replace the last ';' with a '\0'
			ofn.nMaxCustFilter++;
		}
	}
};

#endif // !defined(AFX_DOCMANAGER_H__06A6B883_D4F1_47D1_8EE7_1523D7E17A66__INCLUDED_)
