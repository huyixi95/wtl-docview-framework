// DocTemplateBase.cpp: implementation of the CDocTemplateBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DocTemplateBase.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
UINT AtlGetFileName(LPCTSTR lpszPathName, LPTSTR lpszTitle, UINT nMax)
{
	// always capture the complete file name including extension (if present)
	LPTSTR lpszTemp = ::PathFindFileName(lpszPathName);

	// lpszTitle can be NULL which just returns the number of bytes
	if (lpszTitle == NULL)
		return lstrlen(lpszTemp)+1;

	// otherwise copy it into the buffer provided
	lstrcpyn(lpszTitle, lpszTemp, nMax);
	return 0;
}

UINT AtlGetFileTitle(LPCTSTR lpszPathName, LPTSTR lpszTitle, UINT nMax)
{
	// use a temporary to avoid bugs in ::GetFileTitle when lpszTitle is NULL
	TCHAR szTemp[_MAX_PATH];
	LPTSTR lpszTemp = lpszTitle;
	if (lpszTemp == NULL)
	{
		lpszTemp = szTemp;
		nMax = sizeof(szTemp)/sizeof(szTemp[0]);
	}
	if (::GetFileTitle(lpszPathName, lpszTemp, (WORD)nMax) != 0)
	{
		// when ::GetFileTitle fails, use cheap imitation
		return AtlGetFileName(lpszPathName, lpszTitle, nMax);
	}
	return lpszTitle == NULL ? lstrlen(lpszTemp)+1 : 0;
}
void AtlGetRoot(LPCTSTR lpszPath, CString& strRoot)
{
	_ASSERTE(lpszPath != NULL);

	LPTSTR lpszRoot = strRoot.GetBuffer(_MAX_PATH);
	lstrcpyn(lpszRoot, lpszPath, _MAX_PATH);
	PathStripToRoot(lpszRoot);
	strRoot.ReleaseBuffer();
};
BOOL AtlFullPath(LPTSTR lpszPathOut, LPCTSTR lpszFileIn)
{
	// first, fully qualify the path name
	LPTSTR lpszFilePart;
	DWORD dwRet = GetFullPathName(lpszFileIn, _MAX_PATH, lpszPathOut, &lpszFilePart);
	if (dwRet == 0)
	{
#ifdef _DEBUG
		if (lpszFileIn[0] != '\0')
			ATLTRACE("Warning: could not parse the path.\n");
#endif
		lstrcpyn(lpszPathOut, lpszFileIn, _MAX_PATH); // take it literally
		return FALSE;
	}
	else if (dwRet >= _MAX_PATH)
	{
#ifdef _DEBUG
		if (lpszFileIn[0] != '\0')
			ATLTRACE("Warning: could not parse the path. Path is too long.\n");
#endif
		return FALSE; // long path won't fit in buffer
	}

	CString strRoot;
	// determine the root name of the volume
	AtlGetRoot(lpszPathOut, strRoot);

	if (!::PathIsUNC( strRoot ))
	{
		// get file system information for the volume
		DWORD dwFlags, dwDummy;
		if (!GetVolumeInformation(strRoot, NULL, 0, NULL, &dwDummy, &dwFlags,
			NULL, 0))
		{
			ATLTRACE("Warning: could not get volume information.\n");
			return FALSE;   // preserving case may not be correct
		}

		// not all characters have complete uppercase/lowercase
		if (!(dwFlags & FS_CASE_IS_PRESERVED))
			CharUpper(lpszPathOut);

		// assume non-UNICODE file systems, use OEM character set
		if (!(dwFlags & FS_UNICODE_STORED_ON_DISK))
		{
			WIN32_FIND_DATA data;
			HANDLE h = FindFirstFile(lpszFileIn, &data);
			if (h != INVALID_HANDLE_VALUE)
			{
				FindClose(h);
				if(lpszFilePart != NULL && lpszFilePart > lpszPathOut)
				{
					int nFileNameLen = lstrlen(data.cFileName);
					int nIndexOfPart = (int)(lpszFilePart - lpszPathOut);
					if ((nFileNameLen + nIndexOfPart) < _MAX_PATH)
						lstrcpy(lpszFilePart, data.cFileName);
					else
						return FALSE; // Path doesn't fit in the buffer.
				}
				else
					return FALSE;
			}
		}
	}
	return TRUE;
};

BOOL AtlResolveShortcut(HWND hWnd, LPCTSTR lpszFileIn, LPTSTR lpszFileOut, int cchPath)
{
	USES_CONVERSION;
	CComPtr<IShellLink> spShellLink;
	IShellLink* psl = NULL;
	*lpszFileOut = 0;   // assume failure

	SHFILEINFO info;
	if ((SHGetFileInfo(lpszFileIn, 0, &info, sizeof(info),
		SHGFI_ATTRIBUTES) == 0) || !(info.dwAttributes & SFGAO_LINK))
	{
		return FALSE;
	}

	if (FAILED(spShellLink.CoCreateInstance(CLSID_ShellLink)))
	{
		return FALSE;
	}

	IPersistFile *pPersistFile = NULL;
	if (SUCCEEDED(spShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile)))
	{
		if (pPersistFile != NULL && SUCCEEDED(pPersistFile->Load(T2COLE(lpszFileIn), STGM_READ)))
		{
			/* Resolve the link, this may post UI to find the link */
			if (SUCCEEDED(spShellLink->Resolve(hWnd,SLR_ANY_MATCH)))
			{
				spShellLink->GetPath(lpszFileOut, cchPath, NULL, 0);
				pPersistFile->Release();
				return TRUE;
			}
		}
		if (pPersistFile != NULL)
			pPersistFile->Release();
	}
	return FALSE;
}

BOOL ExtractSubString(CString& rString, LPCTSTR lpszFullString, int iSubString, TCHAR chSep)
{
	if (lpszFullString == NULL)
		return FALSE;
	
	while (iSubString--)
	{
		lpszFullString = _tcschr(lpszFullString, chSep);
		if (lpszFullString == NULL)
		{
			rString.Empty();        // return empty string as well
			return FALSE;
		}
		lpszFullString++;       // point past the separator
	}
	LPCTSTR lpchEnd = _tcschr(lpszFullString, chSep);
	int nLen = (lpchEnd == NULL) ?
		lstrlen(lpszFullString) : (int)(lpchEnd - lpszFullString);
	_ASSERTE(nLen >= 0);
	lstrcpyn(rString.GetBufferSetLength(nLen), lpszFullString, nLen*sizeof(TCHAR)+1);
	return TRUE;
}

void SendMessageToDescendants(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, BOOL bDeep)
{
	// walk through HWNDs to avoid creating temporary CWnd objects
	// unless we need to call this function recursively
	for (HWND hWndChild = ::GetTopWindow(hWnd); hWndChild != NULL;
	hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
	{
		// send message with Windows SendMessage API
		::SendMessage(hWndChild, message, wParam, lParam);
		if (bDeep && ::GetTopWindow(hWndChild) != NULL)
		{
			// send to child windows after parent
			SendMessageToDescendants(hWndChild, message, wParam, lParam, bDeep);
		}
	}
}

CDocTemplateBase::CDocTemplateBase() : m_bAutodelete(true)
{

}

CDocTemplateBase::~CDocTemplateBase()
{

}
