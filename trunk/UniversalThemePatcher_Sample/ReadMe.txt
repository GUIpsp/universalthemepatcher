========================================================================
    CONSOLE APPLICATION : UniversalThemePatcher_Sample Project Overview
========================================================================

Project:	Universal Theme Patcher (Sample Code)
Support:	Windows XP SP2 SP3/2003/2008/Vista SP1 SP2/Windows 7,  32bit(x86)/64bit(x64)
E-mail:		deepxw#gmail.com
Blog:		http://deepxw.blogspot.com (English)
Blog:		http://deepxw.lingd.net (ÖÐÎÄ)
Comment:	If you using this code, you must add my name to your final software, and link to my blog.

This is a simple demo for introduce how to remove the restriction in the file "uxtheme.dll".

Crack uxtheme, free your Windows supports 3rd party desktop msstyle themes.

It is a universal theme patcher. Without language limited, Supports all language of windows!


How to use?

Get info:
UniversalThemePatcher_Sample.exe  FileName

Patch file:
UniversalThemePatcher_Sample.exe  FileName  -patch

/////////////////////////////////////////////////////////////////////////////

Source file lists:

/////////////////////////////////////////////////////////////////////////////
Patcher implement files:

ThemeCommon.h, ThemeCommon.cpp
  Define the necessary struct, common function.
  
ThemeFileInfo.h, ThemeFileInfo.cpp
  Get the patch information from uxtheme.dll / themeui.dll / shsvc.dll / themeservice.dll.
  

/////////////////////////////////////////////////////////////////////////////
Project basic files:

UniversalThemePatcher_Sample.vcproj
UniversalThemePatcher_Sample.cpp
UniversalThemePatcher_Sample.rc   
Resource.h
StdAfx.h, StdAfx.cpp

/////////////////////////////////////////////////////////////////////////////
