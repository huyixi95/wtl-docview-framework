// Document.h: interface for the CDocument class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DOCUMENT_H__F4B7C81D_A738_4B5A_A36F_189294D98939__INCLUDED_)
#define AFX_DOCUMENT_H__F4B7C81D_A738_4B5A_A36F_189294D98939__INCLUDED_

#include <atlframe.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CView;
class CDocTemplateBase;

#define     HINT_UPDATE_ALL_DOCUMENTS     -1
#define     HINT_DOCUMENT_MODIFIED        -2

//template<class TFrame>
class CDocument :public CMessageMap
{
public:
	BEGIN_MSG_MAP(CDocument)
		COMMAND_ID_HANDLER(ID_FILE_SAVE, OnFileSave)
		COMMAND_ID_HANDLER(ID_FILE_SAVE_AS, OnFileSaveAs)
	END_MSG_MAP()

	
public:
	CDocument();
	virtual ~CDocument();

	virtual void UpdateAllViews(CView* pSender, LPARAM lHint = 0, LPVOID pHint = NULL);
	virtual CView* AddView(CView* pView);
	
	void RemoveView(CView* pView);
	
	int GetViewsCount() const;
	
	CView* GetView(int pos);
	
	virtual BOOL IsModified() const;
	
	virtual void SetModifiedFlag(BOOL bModified = TRUE);
	
    
    virtual BOOL Serialize(LPCTSTR lpszFileName, BOOL isLoading)
	{
		_ASSERT(FALSE);
		//You should derive this method.

		//return TRUE for success and FALSE for failure
		return FALSE;
	};
    	
	CDocTemplateBase* GetDocTemplate();
	
	virtual BOOL        OnNewDocument();
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	
protected:
	
	CSimpleArray<CView*> m_arrViews;

	BOOL                       m_bModified;
	CString m_strPathName;
	CString m_strTitle;
	
	
public:
	virtual BOOL DoSave(LPCTSTR lpszPathName, BOOL bReplace=TRUE);
	virtual BOOL DoFileSave();
	virtual BOOL SaveModified();
	virtual void PreCloseFrame(CMDIWindow* pMDIFrame);
	virtual void DeleteContents();
	virtual void UpdateFrameCounts();
	virtual void OnCloseDocument();
	virtual BOOL CanCloseFrame(CMDIWindow* pFrame);
	virtual void OnChangedViewList();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);

	const CString& GetTitle() const;
	virtual void SetTitle(LPCTSTR lpszTitle);
	const CString& GetPathName() const;
	virtual BOOL SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);
	
	CDocTemplateBase*           m_pDocTemplate;
	BOOL						m_bAutoDelete;

protected:

private:
	LRESULT OnFileSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFileSaveAs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

#endif // !defined(AFX_DOCUMENT_H__F4B7C81D_A738_4B5A_A36F_189294D98939__INCLUDED_)
