// DocTemplateBase.h: interface for the CDocTemplateBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DOCTEMPLATEBASE_H__F1C104AE_883D_41BE_A2BF_881717CE5D33__INCLUDED_)
#define AFX_DOCTEMPLATEBASE_H__F1C104AE_883D_41BE_A2BF_881717CE5D33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define WM_DOCMGRDOFILEPROMPT ::RegisterWindowMessage("WM_DOCMGRDOFILEPROMPT")

typedef struct _DOCMGRDOFILEPROMPT
{
	CString fileName;
	UINT nIDSTitle;
	DWORD lFlags;
	BOOL bOpenFileDialog;
	LPVOID pTemplate;
} DOCMGRDOFILEPROMPT;

class CDocumentBase;
class CDocument;
class CMDIFrameBase;

UINT AtlGetFileName(LPCTSTR lpszPathName, LPTSTR lpszTitle, UINT nMax);
UINT AtlGetFileTitle(LPCTSTR lpszPathName, LPTSTR lpszTitle, UINT nMax);
void AtlGetRoot(LPCTSTR lpszPath, CString& strRoot);
BOOL AtlFullPath(LPTSTR lpszPathOut, LPCTSTR lpszFileIn);
BOOL AtlResolveShortcut(HWND hWnd, LPCTSTR lpszFileIn, LPTSTR lpszFileOut, int cchPath);
BOOL ExtractSubString(CString& rString, LPCTSTR lpszFullString, int iSubString, TCHAR chSep = '\n');
void SendMessageToDescendants(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, BOOL bDeep);

class CDocTemplateBase  
{
public:
	virtual void LoadTemplate() = 0;
	CDocTemplateBase();
	virtual ~CDocTemplateBase();
	
	virtual CDocument* GetDocument(int pos) = 0;
	virtual CDocument* GetDocument(HWND hChildFrame)=0;
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszPathName, BOOL bMakeVisible = TRUE) = 0;
	virtual CDocument* OpenDocument(BOOL bNew = FALSE) = 0;
	virtual int GetDocumentsCount() const = 0;
	virtual BOOL SaveAllModified () = 0;
	virtual void CloseAllDocuments (BOOL bEndSession) = 0;
	virtual CDocument* RemoveDocument(CDocument* pDoc) = 0;
	virtual void InitialUpdateFrame (CMDIFrameBase* pFrame, CDocument* pDoc, BOOL bMakeVisible = TRUE)=0;
	
	bool m_bAutodelete;

	CString m_strDocStrings;    // '\n' separated names

	HWND     m_hWndClient; 
	HWND     m_hWnd; 

	enum Confidence
	{
		noAttempt,
		maybeAttemptForeign,
		maybeAttemptNative,
		yesAttemptForeign,
		yesAttemptNative,
		yesAlreadyOpen
	};
	enum DocStringIndex
	{
		windowTitle,        // default window title
		docName,            // user visible name for default document
		fileNewName,        // user visible name for FileNew
		// for file based documents:
		filterName,         // user visible name for FileOpen
		filterExt,          // user visible extension for FileOpen
		// for file based documents with Shell open support:
		regFileTypeId,      // REGEDIT visible registered file type identifier
		regFileTypeName,    // Shell visible registered file type name
	};

	virtual BOOL GetDocString(CString& rString, enum DocStringIndex i){ return FALSE; };
	virtual Confidence MatchDocType(LPCTSTR lpszPathName,CDocument*& rpDocMatch)=0;
};

#endif // !defined(AFX_DOCTEMPLATEBASE_H__F1C104AE_883D_41BE_A2BF_881717CE5D33__INCLUDED_)
