/*

The MIT License

Copyright (c) 2008-2009 deepxw

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/*

FileName:	ThemeFileInfo.h

Project:	Universal Theme Patcher (Sample Code)
Author:		deepxw
E-mail:		deepxw#gmail.com
Blog:		http://deepxw.blogspot.com

Comment:	If you using this code, you must add my name to your final software, and link to my blog.


Last Updated:  2008.10.28


*//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __ThemeFileInfo_H__
#define __ThemeFileInfo_H__
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ThemeCommon.h"


class CThemeFileInfo
{
public:
	CThemeFileInfo(void);
public:
	virtual ~CThemeFileInfo(void);
public:

	PatchInfo m_patchInfo;

	// Get patch information from file
	BOOL GetPatchInfo(LPTSTR pszTargetFile, PPatchInfo pInfo);

protected:


	BOOL GetPatchInfoXp32(LPTSTR pszTargetFile, PPatchInfo pInfo);
	BOOL GetPatchInfoXp64(LPTSTR pszTargetFile, PPatchInfo pInfo);

	BOOL GetPatchInfoVista32(LPTSTR pszTargetFile, PPatchInfo pInfo);
	BOOL GetPatchInfoVista64(LPTSTR pszTargetFile, PPatchInfo pInfo);

	// Read file to buffer, return buffer size, and buffer pointer
	PVOID ReadFileToBuffer(
					   LPTSTR	pszFile,			// in,  File name
					   PDWORD	pdwFileReadStart,	// I/O,  Start offset to read
					   PDWORD	pdwFileReadEnd,		// I/O,  End offset
					   PDWORD	pdwBufferSize		// out, Buffer size
					   );
	
};




/////////////////////////////////////////////////////////////////////////////
#endif