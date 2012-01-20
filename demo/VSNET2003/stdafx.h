// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__AB14C95B_0947_42A2_AA97_532B4ABEB099__INCLUDED_)
#define AFX_STDAFX_H__AB14C95B_0947_42A2_AA97_532B4ABEB099__INCLUDED_

// Change these values to use different versions
#define WINVER		0x0400
#define _WIN32_WINNT	0x0400
#define _WIN32_IE	0x0400
#define _RICHEDIT_VER	0x0100

#define _ATL_APARTMENT_THREADED

#define _STLP_NEW_PLATFORM_SDK

#include <atlbase.h>
#include <atlapp.h>

extern CServerAppModule _Module;

// This is here only to tell VC7 Class Wizard this is an ATL project
#ifdef ___VC7_CLWIZ_ONLY___
CComModule
CExeModule
#endif

#include <atlcom.h>
#include <atlhost.h>
#include <atlwin.h>
#include <atlctl.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlmisc.h>
#include "DocView\WTLDocView.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__AB14C95B_0947_42A2_AA97_532B4ABEB099__INCLUDED_)
