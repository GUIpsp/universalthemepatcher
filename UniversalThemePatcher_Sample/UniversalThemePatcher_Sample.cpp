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
FileName:	UniversalThemePatcher_Sample.cpp

			This project is demo how to use the class "CThemeFileInfo".

Project:	Universal Theme Patcher (Sample Code)
Author:		deepxw
E-mail:		deepxw#gmail.com
Blog:		http://deepxw.blogspot.com (English)
			http://deepxw.lingd.net (ÖÐÎÄ)

Comment:	If you using this code, you must add my name to your final software, and link to my blog.

Updated:
2009.05.08, First create.


*//////////////////////////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "UniversalThemePatcher_Sample.h"
#include <shlwapi.h>
#include "ThemeFileInfo.h"
#include "ThemeCommon.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;


/////////////////////////////////////////////////////////////////////////////////////////////////

// Get information from uxtheme.dll / themeui.dll ...
void GetThemeFileInfo(LPTSTR pszFile);

// Patch file
void PatchThemeFile(LPTSTR pszFile);

/////////////////////////////////////////////////////////////////////////////////////////////////

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{
		// TODO: code your application's behavior here.

		BOOL	bErrorArgs	= FALSE;

		if (argc == 2)
		{
			if (PathFileExists(argv[1]))
			{
				// try to get info
				GetThemeFileInfo(argv[1]);
			}
			else
			{
				_tprintf(_T("\nFile not found:\t%s\n"), argv[1]);
				return 1;
			}
		}
		else if (argc == 3)
		{
			if (lstrcmp(argv[2], _T("-patch")) == 0)
			{
				if (!PathFileExists(argv[1]))
				{
					_tprintf(_T("\nFile not found:\t%s\n"), argv[1]);
					return 1;
				}

				// try to patch file
				PatchThemeFile(argv[1]);
			}
			else
			{
				bErrorArgs	= TRUE;
			}
		}
		else
		{
			bErrorArgs	= TRUE;
		}


		if (bErrorArgs)
		{
			_tprintf(_T("\nError args.\n\nUsage:\n"));
			_tprintf(_T("  UniversalThemePatcher_Sample.exe  FileName\n"));
			_tprintf(_T("  UniversalThemePatcher_Sample.exe  FileName  -patch\n"));
		}
		
	}

	return nRetCode;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Get information from uxtheme.dll / themeui.dll ...
//
void GetThemeFileInfo(LPTSTR pszFile)
{
	CThemeFileInfo		themeInfo;
	PatchInfo			info;
	TCHAR				szTmp[MAX_PATH] = {0};

	if (!PathFileExists(pszFile))
	{
		_tprintf(_T("\n\nFile not found:\t%s\n"), pszFile);
		return;
	}

	_tprintf(_T("\nFile name:\t%s\n"), pszFile);

	info.dwSize = sizeof(PatchInfo);

	if (themeInfo.GetPatchInfo(pszFile, &info))
	{
		_tprintf(_T("File Version:\t%s\n"), info.szFileVersion);
		_tprintf(_T("File platform:\t%s\n"), info.szPlatform);

		PatchStatusToString(&info, szTmp);
		_tprintf(_T("Patched Status:\t%s\n"), szTmp);

		_tprintf(_T("Patched offset:\t0x%x\n"), info.PatchOffset);


		// more...
	}
	else
	{
		_tprintf(_T("Fail to get limited information.\n"));
	}

	return;

} // GetThemeFileInfo()


/////////////////////////////////////////////////////////////////////////////////////////////////
//
// Patch file
//
// Step:
// 1) Get the info of the file. we need the file offset/patch char, and more.
// 2) Write the file by the patch info.
//
void PatchThemeFile(LPTSTR pszFile)
{
	CThemeFileInfo		themeInfo;
	PatchInfo			info;
	TCHAR				szTmp[MAX_PATH] = {0};

	if (!PathFileExists(pszFile))
	{
		_tprintf(_T("\n\nFile not found:\t%s\n"), pszFile);
		return;
	}

	_tprintf(_T("\n\nTry to get info from:\t%s\n"), pszFile);

	info.dwSize = sizeof(PatchInfo);

	if (themeInfo.GetPatchInfo(pszFile, &info))
	{
		_tprintf(_T("File Version:\t%s\n"), info.szFileVersion);
		_tprintf(_T("File platform:\t%s\n"), info.szPlatform);

		PatchStatusToString(&info, szTmp);
		_tprintf(_T("Patched Status:\t%s\n"), szTmp);

		_tprintf(_T("Patched offset:\t0x%x\n"), info.PatchOffset);


		//
		// try to patch file
		//

		// make sure it's a original file, and we have got the correct offset
		if ( (info.PatchStatus == PATCH_NO) &&  (info.PatchOffset > 0) )
		{
			// now, we patch it
			SetPatchInfo(pszFile, &info);
		}
		else if (info.PatchStatus == PATCH_YES)
		{
			_tprintf(_T("This file already has been patched, no need to patch again.\n"));
		}
		else
		{
			_tprintf(_T("This file is un-support.\n"));
		}

	}
	else
	{
		_tprintf(_T("Fail to get limited information.\n"));
	}

	return;

} //  PatchThemeFile()