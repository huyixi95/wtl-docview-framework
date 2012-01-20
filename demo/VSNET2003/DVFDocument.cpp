// DVFDocument.cpp: implementation of the CDVFDocument class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DVFDocument.h"
#include <comdef.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDVFDocument::CDVFDocument()
{
	text[0] = '\0';
}

CDVFDocument::~CDVFDocument()
{

}

BOOL CDVFDocument::Serialize(LPCTSTR lpszFileName, BOOL isLoading)
{
	HRESULT hRes;
	CComPtr<IXMLDOMDocument> spDoc;

	hRes = spDoc.CoCreateInstance(CLSID_DOMDocument);
	if (FAILED(hRes))
	{
		MessageBox(m_pDocTemplate->m_hWnd, "Error creting instance of DOMDocument", "Error", MB_ICONASTERISK);
		return FALSE;
	}

	if (isLoading)
	{
		VARIANT_BOOL bSuccess;	
		spDoc->load(_variant_t(lpszFileName), &bSuccess);

	}
	else
	{
		CComPtr<IXMLDOMNode> spNode;
		CComPtr<IXMLDOMElement> spRootElement;
		CComPtr<IXMLDOMAttribute> spAttr;

		VARIANT_BOOL bLoadOk;
		hRes = spDoc->loadXML(_bstr_t("<root/>"), &bLoadOk);

		hRes = spDoc->get_documentElement(&spRootElement);
		
		hRes = spDoc->createNode(_variant_t((short)NODE_ELEMENT ), _bstr_t("Main"), _bstr_t(""), &spNode);

		CComPtr<IXMLDOMNode> spMainNode;
		hRes = spRootElement->appendChild(spNode, &spMainNode);

		hRes = spDoc->createAttribute(_bstr_t("Text"), &spAttr);
		spAttr->put_value(_variant_t(text));

		CComPtr<IXMLDOMNamedNodeMap> spNameNodeMap;
		hRes = spMainNode->get_attributes(&spNameNodeMap);

		CComPtr<IXMLDOMNode> sp_work;
		hRes = spNameNodeMap->setNamedItem(spAttr, &sp_work);

		hRes = spDoc->save(_variant_t(lpszFileName));
		if (FAILED(hRes))
		{
			MessageBox(m_pDocTemplate->m_hWnd, "Error saving document.", "Error", MB_ICONASTERISK);
			return FALSE;
		}

	}


	return TRUE;
};
