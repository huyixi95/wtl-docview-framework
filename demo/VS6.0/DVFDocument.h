// DVFDocument.h: interface for the CDVFDocument class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DVFDOCUMENT_H__0534945C_54E8_4176_8EF0_34FF98F7F7FB__INCLUDED_)
#define AFX_DVFDOCUMENT_H__0534945C_54E8_4176_8EF0_34FF98F7F7FB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDVFDocument : public CDocument  
{
public:
	CDVFDocument();
	virtual ~CDVFDocument();

    BOOL Serialize(LPCTSTR lpszFileName, BOOL isLoading);

	char text[255];
};

#endif // !defined(AFX_DVFDOCUMENT_H__0534945C_54E8_4176_8EF0_34FF98F7F7FB__INCLUDED_)
