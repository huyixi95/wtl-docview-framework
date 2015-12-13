# WTL Docview Framework
Toolbox control similar to the one used in Visual Studio .NET 2003.

The main target was to achieve maximum similarity with the VS.NET Toolbox object model and GUI behaviour. It is written as an ATL ActiveX control using WTL 7.5 and STL and ATL collections.

##Introduction
This article presents a Toolbox control similar to the one used in Visual Studio .NET 2003. The main target was to achieve maximum similarity with the VS.NET Toolbox object model and GUI behaviour. It is written as an ATL ActiveX control using WTL 7.5 and STL and ATL collections.

###Details
Because I couldn't find such a control on the Internet, I decided write it myself. In the beginning, I had to fight with the GUI, but when I found a great article MS Access Bar written by Bjarke Viksoe, the fight was done.

I do not have much time to completely describe each and every method of all interfaces, but I'll give you a short overview. This version of the Toolbox contains some bugs that I have not fixed (moving tabs by selecting Move up/down from popup menu... try and you will find them) and the work is not finished yet, because of lack of time - but I am working on it.

###Implementation
The Toolbox is a classic full ATL control. For my requirements I had to extend Bjarke Viksoe's MS Access Bar control (I have changed the code for drawing tabs and have added some helper methods).

###The Toolbox control consists of six interfaces:

* IToolbox
* IToolBoxTab
* ITooBoxItem
* IToolBoxTabs
* IToolBoxItems
* IToolboxEvents
* Interfaces related to the GUI

###IToolbox
Represents the main control.

###IToolBoxTab
Represents a Toolbox tab object. You can rearrange tab positions by dragging them with mouse or by choosing the appropriate command from the popup menu (this does not work correctly).

###IToolBoxItem
Represents a Toolbox tab item object.

There are four kinds of tab items (the names are borrowed from VS.NET 2003 Toolbox, and they are self describing I think :-)

* ToolBoxItemFormatText? - As a data contains text value, and displays "text" icon.
* ToolBoxItemFormatHTML - As a data contains text value, and displays "html" icon.
* ToolBoxItemFormatGUID - An ActiveX control, as a data contains the GUID of a control and displays the icon from a control DLL.
* ToolBoxItemFormatPointer? - displays "arrow" icon, data property is empty.


The functionalities of the described interfaces are nearly the same as that of VS.NET 2003 toolbox interfaces, with some extra methods and properties. Interface IToolboxEvents is a dispatch event interface.

###Collection and enumerator interfaces
There are two collections and enumerator interfaces (more on this kind of interfaces is described in this article):

###IToolBoxTabs
Collection of Toolbox tab objects.

###IToolBoxItems
Collection of Toolbox tab item objects.

These interfaces allow you to manipulate an object from Visual Basic (or VB script) using For..Next and For...Each syntax:
```vb
Private Sub Form_Load()
Dim tab As ToolBoxTab
  Set tab = Toolbox1.ToolBoxTabs.Add("Tb 1")
  For Each tab In Toolbox1.ToolBoxTabs
    tb.Name = "Tab"
  Next
End Sub
```
###Events
There is an event dispatch interface IToolboxEvents, and it is related to the Toolbox object.

###Property pages
In this version, the Toolbox has its own property page for enabling a popup menu and its "customize" menu item. It has a stock font property page as well.



###Popup menu
There are two different popup menus for the Tab and the Tab Item. For the Tab Item, you can cut, copy and paste the Tab Item or paste text from the clipboard to the Toolbox as the Tab Item of type ToolBoxItemFormatText?.


## References
* MS Access Bar - by Bjarke Viksoe.
* DataObject and other stuf - by Bjarke Viksoe.
* CComBool - by Chris Sells.
