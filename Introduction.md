# Introduction #

This article explains the implementation of a simple Doc View framework in a WTL (version 7.5) MDI application. The sample project included with this article is a wizard-generated MDI application enhanced with my framework classes.

I enhanced code from **Gabriel Kniznik** with a little bit closer to the MFC DocView approach, and there exists only one kind of document template - MDI Document template. This framework was written in a week, so I apologize for mistakes and unfinished solutions :).

In this library is also participating **Rodrigo Pinho Pereira de Souza**, who made great enhancements to this framework (see the History section).

# Implementation #

In the demo project, you can see how to integrate the framework to your own project. You have to make changes to _CMainFrame_, _CChildFrame_, _CYourView_, and create a new class _CYourDoc_. Do not forget to change the _IDR\_CHILDFRAME_ string to the form for MFC -> \nDocument\nDVF\n\n\nDVFSimple.Document\nDVF Document.

Below is a simplified class diagram of the framework:

http://www.codeproject.com/KB/wtl/DocView_framework/fw_class_diagram_simple.JPG

# History #

  * 1 Jan 2006
    * Demo application split to VC++ 6.0 and VS.NET 2003.
> > Changes made by **Rodrigo Pinho Pereira de Souza**:
      * Automatic handling of _ID\_FILE\_NEW_
      * Automatic handling of _ID\_FILE\_SAVE_
      * Automatic handling of _ID\_FILE\_SAVEAS_
      * Implemented Serialization mechanism
      * Changed all declarations of std::vector to the ATL _CSimpleArray_
      * Created a _CFormView_ class

  * Aug 2005 - Initial version.