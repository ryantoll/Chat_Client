========================================================================
    WIN32 APPLICATION : Chat_Client Project Overview
========================================================================

Ryan Toll 2018

Contact me at:
[last name] (dot) [first name] (at) G M A I L (dot) COM

This program is the client side of an instant messenger application.
It is designed to be paired with the corresponding server application on a private network.
The pair is a portfolio piece meant to demonstrate competence with C++ and the Windows API.
Be sure to connect to the server before sending a message.
Later versions will change the UI to ensure users must connect to a server before starting.
Known bug: Using return key to send messages adds a newline character after clearing input window.

Other topics:
Socket communication with BSD/Winsocket
Multithreading
UI/UX
Class design & encapsulation
Template usage

========================================================================
    Default ReadMe Below
========================================================================

AppWizard has created this Chat_Client application for you.

This file contains a summary of what you will find in each of the files that
make up your Chat_Client application.


Chat_Client.vcxproj
    This is the main project file for VC++ projects generated using an Application Wizard.
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

Chat_Client.vcxproj.filters
    This is the filters file for VC++ projects generated using an Application Wizard. 
    It contains information about the association between the files in your project 
    and the filters. This association is used in the IDE to show grouping of files with
    similar extensions under a specific node (for e.g. ".cpp" files are associated with the
    "Source Files" filter).

Chat_Client.cpp
    This is the main application source file.

/////////////////////////////////////////////////////////////////////////////
AppWizard has created the following resources:

Chat_Client.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes the icons, bitmaps, and cursors that are stored
    in the RES subdirectory.  This file can be directly edited in Microsoft
    Visual C++.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Visual C++ reads and updates this file.

Chat_Client.ico
    This is an icon file, which is used as the application's icon (32x32).
    This icon is included by the main resource file Chat_Client.rc.

small.ico
    This is an icon file, which contains a smaller version (16x16)
    of the application's icon. This icon is included by the main resource
    file Chat_Client.rc.

/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named Chat_Client.pch and a precompiled types file named StdAfx.obj.

/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" comments to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////
