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

FileName:	ThemeCommon.cpp

Project:	Universal Theme Patcher (Sample Code)
Author:		deepxw
E-mail:		deepxw#gmail.com
Blog:		http://deepxw.blogspot.com (English)
			http://deepxw.lingd.net (ÖÐÎÄ)

Comment:	If you using this code, you must add my name to your final software, and link to my blog.

  Updated:
  2008.10.29:  (LONG_PTR)pDOS);	// DWORD->LONG_PTR, For x64.
  2008.12.30:  Update to Unicode version  


*//////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "ThemeCommon.h"
//#include "Util.h"


#include <shlwapi.h>
#include <Imagehlp.h>
#pragma comment( lib, "ImageHlp.lib" )
#pragma comment(lib,  "Version.lib")

//
// Using the Strsafe.h Functions
//
#define STRSAFE_LIB					// use the functions in library form
#include <strsafe.h>
//


#define		MALLOC(x)	HeapAlloc(GetProcessHeap(), 0, (x))
#define		FREE(x)		HeapFree(GetProcessHeap(), 0, (x))


// Convert patch status from number to string
BOOL PatchStatusToString(PPatchInfo pInfo, LPTSTR szType)
{
	switch (pInfo->PatchStatus)
	{
	case PATCH_YES:
		StringCbCopy(szType, MAX_PATH, _T("Yes"));
		break;

	case PATCH_NO:
		StringCbCopy(szType, MAX_PATH, _T("No"));
		break;

	case PATCH_UNSURE:
		StringCbCopy(szType, MAX_PATH, _T("Unsure"));
		break;

	default:
		StringCbCopy(szType, MAX_PATH, _T("Unknown"));
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Patch the file with special offset and sign
//
BOOL SetPatchInfo(LPTSTR pszFile, PPatchInfo pInfo)
{
	//TRACEF(_T("Begin to patch: %s\n"), pszFile);

	char		szMySignal[]	= "deepxw";
	DWORD		dwMySignalSize	= 6;

	// Check the size of struct
	if (sizeof(PatchInfo) != pInfo->dwSize)
	{
		pInfo->dwSize	= sizeof(PatchInfo);
		return FALSE;
	}

	// Check file is exists
	if(!PathFileExists(pszFile))
		return FALSE;

	if ( (pInfo->PatchCharLength == 0) || (pInfo->PatchOffset == 0) )
		return FALSE;

	HANDLE		hFile;
	DWORD		dwFileSize		= 0;
	TCHAR		sztmp[20]		= {0};	
	DWORD		dwBytesRead		= 0;
	DWORD		dwBytesWrite	= 0;
	BOOL		bRet			= FALSE;

	TCHAR		szFileBackup[MAX_PATH]	= {0};
	TCHAR		szFileTemp[MAX_PATH]	= {0};
	TCHAR		szCmd[500]				= {0};

	if (isNT6())
	{
		wsprintf(szCmd, _T("takeown /f  %s"), pszFile); 
		RunHiddenConsole(szCmd, TRUE);
		wsprintf(szCmd, _T("icacls %s /grant "), pszFile); 
		lstrcat(szCmd, _T("%username%:F"));
		RunHiddenConsole(szCmd, TRUE);
		wsprintf(szCmd, _T("icacls %s /grant *S-1-1-0:(F)"), pszFile); 
		RunHiddenConsole(szCmd, TRUE);
	}

	if (isNT5())
	{
		DisableWFP(pszFile);
	}

	Sleep(200);

	// build backup file name
	StringCbCopy(szFileBackup, sizeof(szFileBackup), pszFile);
	StringCbCat(szFileBackup, sizeof(szFileBackup), _T(".backup"));

	// Make a backup..
	if ( (CopyFile(pszFile, szFileBackup, FALSE)) == FALSE)
	{
		AfxMessageBox(_T("Fail to backup file!"));
	}

	// build temp file name
	StringCbCopy(szFileTemp, sizeof(szFileTemp), pszFile);
	StringCbCat(szFileTemp, sizeof(szFileTemp), _T(".tmp"));
	// Make a temp file
	if ( (CopyFile(pszFile, szFileTemp, FALSE)) == FALSE )
	{
		AfxMessageBox(_T("Fail to create temp file!"));
		return FALSE;
	}

	hFile = CreateFile(szFileTemp, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox(_T("Fail to open file!"));
		return FALSE;
	}

	dwFileSize = GetFileSize(hFile,NULL);

	if (pInfo->PatchOffset >= dwFileSize)
	{
		CloseHandle(hFile);
		AfxMessageBox(_T("File pointer is too big!"));
	}
	else
	{
		SetFilePointer(hFile, dwFileSize - dwMySignalSize -1, NULL, FILE_BEGIN);	// Write deepxw signal
		WriteFile(hFile, szMySignal, dwMySignalSize, &dwBytesWrite, 0);				// Added 2008.10.29

		SetFilePointer(hFile, pInfo->PatchOffset, NULL, FILE_BEGIN);				// write patch char
		WriteFile(hFile, pInfo->PatchChar, pInfo->PatchCharLength, &dwBytesWrite, 0);
		CloseHandle(hFile);

		if (dwBytesWrite == pInfo->PatchCharLength)
		{
			//TRACEF(_T("Re-checksum the file... \n"));

			if ( SetCorrectChecksum(szFileTemp) )
			{
				TCHAR		szFileName[MAX_PATH]		= {0};
				TCHAR		szFileDllcache[MAX_PATH]	= {0};
				LPTSTR		pszSplit;

				//
				// Split path, combo c:\windows\system32\dllcache\xxx.dll
				//
				StringCbCopy(szFileDllcache, sizeof(szFileDllcache), pszFile);
				pszSplit	= _tcsrchr(szFileDllcache, _T('\\'));
				StringCbCopy(szFileName, sizeof(szFileName), pszSplit + 1);
				pszSplit[0]	= 0;
				StringCbCat(szFileDllcache, sizeof(szFileDllcache), _T("\\dllcache\\"));
				StringCbCat(szFileDllcache, sizeof(szFileDllcache), szFileName);

				if (PathFileExists(szFileDllcache))
				{
					//TRACEF(_T("Try to overwite the file in dllcache... \n"));

					if (isNT5())
					{
						DisableWFP(szFileDllcache);		// Disable SFC/WFP
					}

					CopyFile(szFileTemp, szFileDllcache, FALSE);
				}

				//printf(_T("\nTry to overwrite old file..."));
				if (SuperCopyFile(szFileTemp, pszFile))
				{
					bRet = TRUE;
						AfxMessageBox(_T("Patch success."));
				}
				else
					AfxMessageBox(_T("Fail to to overwrite target file."));
			}
			else
			{
				AfxMessageBox(_T("Fail to to re-checksum."));
			}

		}
		else
		{
			AfxMessageBox(_T("Fail to to modify file."));
		}

	} // pInfo>PatchOffset < dwFileSize
	

	return bRet;

} // SetPatchInfo()


/////////////////////////////////////////////////////////////////////////////////////////////////////
/*
Function:	Get Machine Type of PE File
Parameter:
IN:  pszFile
OUT: pszFileType, dwFileType
Created by deepxw.
***********************/
DWORD GetPEFileMachineType(LPCTSTR pszFile, LPTSTR pszFileType)
{
	DWORD				dwFileType		= 0;		// PE file type, platform

	HANDLE				hFile			= NULL;
	HANDLE				hMapping		= NULL;
	PUCHAR				pView			= NULL;
	PIMAGE_NT_HEADERS	pNTHeader		= NULL;
	PIMAGE_DOS_HEADER	pDOS			= NULL;


	__try
	{
		// Open file
		hFile	= CreateFile(pszFile, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			__leave;
		}

		// Create mapping
		hMapping	= CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 0, NULL);
		if (!hMapping)
		{
			__leave;
		}

		// MapView of the PE file
		pView	= (PUCHAR)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0x400);
		if (!pView)
		{
			__leave;
		}

		// Make sure it's a valid PE file
		pDOS		= (PIMAGE_DOS_HEADER)pView;
		pNTHeader	= (PIMAGE_NT_HEADERS)(pDOS->e_lfanew + (ULONG_PTR)pDOS);		
		if(pDOS->e_magic != IMAGE_DOS_SIGNATURE \
			|| IsBadReadPtr((PVOID)pNTHeader, sizeof(_IMAGE_OPTIONAL_HEADER)) \
			|| pNTHeader->Signature != IMAGE_NT_SIGNATURE)
		{
			__leave;
		}

		// Get file type
		dwFileType = (DWORD)pNTHeader->FileHeader.Machine;

	} // end try
	__finally
	{
		//
		// Clean up
		//

		if (pView)
			UnmapViewOfFile((LPCVOID)pView);

		if (hMapping)
			CloseHandle(hMapping);

		if (hFile)
			CloseHandle(hFile);

	} //__finally


	// Now to phase file type
	switch (dwFileType)
	{
	case IMAGE_FILE_MACHINE_I386:			// 0x014c  // Intel 386.
		StringCbCopy(pszFileType, 32, _T("x86"));
		break;

	case IMAGE_FILE_MACHINE_IA64:			//0x0200  // Intel 64
		StringCbCopy(pszFileType, 32, _T("ia64"));
		break;

	case IMAGE_FILE_MACHINE_AMD64:			//0x8664  // AMD64 (K8)
		StringCbCopy(pszFileType, 32, _T("x64"));
		break;

	default:
		StringCbCopy(pszFileType, 32, _T("UnKnown"));
	}

	return dwFileType;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Correct the checksum of PE file
//
BOOL SetCorrectChecksum(LPCTSTR pszFile)
{
	BOOL				bRet			= FALSE;

	HANDLE				hFile			= NULL;
	HANDLE				hMapping		= NULL;
	PUCHAR				pView			= NULL;
	PIMAGE_NT_HEADERS	pNTHeader		= NULL;
	PIMAGE_DOS_HEADER	pDOS			= NULL;
	DWORD				dwHeaderSum		= 0;
	DWORD				dwCorrectSum	= 0;
	DWORD				dwFileSize		= 0;

	__try
	{
		// Open file
		hFile	= CreateFile(pszFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if (INVALID_HANDLE_VALUE == hFile)
			__leave;

		dwFileSize	= GetFileSize(hFile, NULL);
		
		// Create mapping
		hMapping	= CreateFileMapping(hFile, 0, PAGE_READWRITE, 0, 0, NULL);
		if (!hMapping)
			__leave;


		// MapView of the PE file
		pView	= (PUCHAR)MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (!pView)
			__leave;

		// Make sure it's a valid PE file
		pDOS		= (PIMAGE_DOS_HEADER)pView;
		pNTHeader	= (PIMAGE_NT_HEADERS)(pDOS->e_lfanew + (ULONG_PTR)pDOS);		
		if(pDOS->e_magic != IMAGE_DOS_SIGNATURE \
			|| IsBadReadPtr((PVOID)pNTHeader, sizeof(_IMAGE_OPTIONAL_HEADER)) \
			|| pNTHeader->Signature != IMAGE_NT_SIGNATURE)
		{
			__leave;
		}

		// Get correct checksum of the PE file
		pNTHeader = CheckSumMappedFile((LPVOID)pView, dwFileSize, &dwHeaderSum, &dwCorrectSum);
		//TRACEF(_T("Header checksum: %08X \tCorrect checksum: %0X\n"), dwHeaderSum, dwCorrectSum);
		if (!pNTHeader)
			__leave;

		// Update the correct checksum to the file header
		pNTHeader->OptionalHeader.CheckSum = dwCorrectSum;

		//
		// All done, OK!
		//
		bRet	= TRUE;

	} // end try
	__finally
	{
		//
		// Clean up
		//

		if (pView)
			UnmapViewOfFile((LPCVOID)pView);

		if (hMapping)
		{
			if (!CloseHandle(hMapping))
				bRet	= FALSE;
		}

		if (hFile)
		{
			if (!CloseHandle(hFile))
				bRet	= FALSE;
		}

	} //__finally

	return bRet;

} // SetCorrectChecksum()



//////////////////////////////////////////////////////////////////////////////////////////////////
// Get the PE file version info
// Last modified, 
// 2008.12.24, added bOK
//
BOOL GetDllFileVersion(LPCTSTR pszFileName, PMYVERSIONINFO pVersionInfo) 
{
	// Check struct size
	if (sizeof(MYVERSIONINFO) != pVersionInfo->dwSize)
		return FALSE;

	BOOL	bOK			= FALSE;
	DWORD   dwHandle	= NULL;   
	DWORD   dwVerSize;
	
	// Get the file version info size
	dwVerSize = GetFileVersionInfoSize(pszFileName, &dwHandle);
	if(dwVerSize == 0)
		return FALSE;

	LPVOID				pbuf		= NULL; 
	UINT				uLen		= 0;   
	VS_FIXEDFILEINFO	*pFileInfo;   

	pbuf = MALLOC(dwVerSize);
	if(!pbuf)
		return FALSE;
  
	__try
	{
		bOK = GetFileVersionInfo(pszFileName, dwHandle, dwVerSize, pbuf);
		if (!bOK)
			__leave;

		bOK	= VerQueryValue(pbuf, (LPTSTR)("\\"), (LPVOID*)&pFileInfo, &uLen);   
		if (!bOK)
			__leave;

		// get data
		pVersionInfo->wMajorVersion		= HIWORD(pFileInfo->dwProductVersionMS);     
		pVersionInfo->wMinorVersion		= LOWORD(pFileInfo->dwProductVersionMS);   
		pVersionInfo->wBuildNumber		= HIWORD(pFileInfo->dwProductVersionLS);   
		pVersionInfo->wRevisionNumber	= LOWORD(pFileInfo->dwProductVersionLS);

		StringCbPrintf(pVersionInfo->szShortVersion, 
						sizeof(pVersionInfo->szShortVersion), _T("%u.%u.%u.%u"), \
						pVersionInfo->wMajorVersion, pVersionInfo->wMinorVersion, \
						pVersionInfo->wBuildNumber, pVersionInfo->wRevisionNumber
						);


		bOK		= TRUE;


	}
	__finally
	{
		// clean up
		
		if (pbuf)
			FREE(pbuf);

	}

	return   bOK;

} // GetDllFileVersion()


// Chect OS version if is NT5
__inline BOOL isNT5(void)
{
	DWORD		dwVersion	= GetVersion();

	if (((DWORD)LOBYTE(LOWORD(dwVersion))) == 5) 
	{
		if (((DWORD)HIBYTE(LOWORD(dwVersion))) > 0) // Windows XP/2003
		{
			return TRUE;
		}
	}

	return FALSE;
}

// Chect OS version if is NT6
__inline BOOL isNT6(void)
{
	DWORD		dwVersion	= GetVersion();

	if (((DWORD)LOBYTE(LOWORD(dwVersion))) == 6) 
	{
		return TRUE;
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Replace the file in using by API MoveFileEx()
//
BOOL SuperCopyFile(LPTSTR pszSrcFile, LPTSTR pszTargetFile)
{
	TCHAR	szTempFileName[MAX_PATH];
	TCHAR	szTempPathName[MAX_PATH];
	TCHAR	szFileName[MAX_PATH];


	//DWORD	dwRetVal;
	//DWORD	dwBufSize	= MAX_PATH;

	//// Get the temp path.
	//dwRetVal = GetTempPath(dwBufSize, szTempPathName);
	//if (dwRetVal > dwBufSize || (dwRetVal == 0))
	//	return FALSE;

	// Get the temp path name, use driver root
	if (pszTargetFile[1] ==  _T(':'))
	{
		wsprintf(szTempPathName, _T("%c:\\"), pszTargetFile[0]);
	}
	else
	{
		GetModuleFileName(NULL, szFileName, sizeof(szFileName));
		wsprintf(szTempPathName,  _T("%c:\\"), szFileName[0]);
	}

	// Try get temp file name
	if (GetTempFileName(szTempPathName,  _T("_@"), 0, szTempFileName) == 0)
		return FALSE;

	// move target to temp file
	if (MoveFileEx(pszTargetFile, szTempFileName, MOVEFILE_REPLACE_EXISTING) == 0)
		return FALSE;

	// move temp file to NULL 
	if (MoveFileEx(szTempFileName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT ) == 0)
		return FALSE;

	// Let me have a try, copy source file to target, althoug it's not necessary.
	CopyFile(pszSrcFile, pszTargetFile, FALSE );

	// Real work, copy source file to target, try again 
	if (MoveFileEx(pszSrcFile, pszTargetFile, MOVEFILE_REPLACE_EXISTING ) == 0)
		return FALSE;

	// try del temp file, in mostly, it's not works.
	DeleteFile(szTempFileName);

	return TRUE;

} // SuperCopyFile()


//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Run console program in Hidden Console
//
BOOL RunHiddenConsole(LPTSTR pszCmdLine, BOOL bWait)
{ 	
	BOOL bRet			= FALSE;

	PROCESS_INFORMATION ProcInfo		= {0};
	STARTUPINFOW		StartupInfo		= {0};
	DWORD				dwCreationFlag	= 0;

	//********************* This Section only for run hidden console (Start) ***********************
	ZeroMemory(&StartupInfo, sizeof(StartupInfo));
	ZeroMemory(&ProcInfo, sizeof(ProcInfo));
	StartupInfo.cb			= sizeof(StartupInfo);			// The size of the structure, in bytes.
	StartupInfo.dwFlags		= STARTF_USESHOWWINDOW;			// The wShowWindow member is valid.
	StartupInfo.wShowWindow = SW_HIDE;

	dwCreationFlag			= CREATE_UNICODE_ENVIRONMENT;	// Using unicode
	
	dwCreationFlag |= CREATE_NEW_CONSOLE;					// run cmdline in console

	//********************* This Section only for run hidden console (End) *************************

	bRet = CreateProcess(NULL, pszCmdLine, 
			NULL, NULL, FALSE, dwCreationFlag, NULL, NULL, &StartupInfo, &ProcInfo);


	// if bWait is true, then wait for process to close.
	if (bRet)
	{
		if (bWait)
			WaitForSingleObject(ProcInfo.hProcess, INFINITE);

		CloseHandle(UlongToHandle(ProcInfo.dwProcessId));
		CloseHandle(UlongToHandle(ProcInfo.dwThreadId));
		
	}

	return bRet;

} // RunHiddenConsole()


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// For disable SFC/WFP in NT5. Using No. 5 function in sfc_os.dll.
//
// Return value:
// 0, S_OK: success
// 1:       error occurred (usually that the file is not protected by WFP)
//
typedef HRESULT (WINAPI *DllfnSetSfcFileException) (
													DWORD dwUnknown0,	// Unknown. Set to 0 
													PWCHAR pwszFile,	// File name 
													DWORD dwUnknown1	// 	Unknown. Set to -1
													);
// 
HRESULT DisableWFP(LPTSTR pszFileName) 
{
	HMODULE	hmod = LoadLibrary( _T("\sfc_os.dll"));

	if (hmod == NULL)	// Fail to load sfc_os.dll
		return 1;

	DllfnSetSfcFileException	fnSetSfcFileException;

	// No. 5 function in sfc_os.dll
	fnSetSfcFileException=(DllfnSetSfcFileException)GetProcAddress(hmod,  MAKEINTRESOURCEA(5));

	HRESULT ret = fnSetSfcFileException(0, pszFileName, -1);

	FreeLibrary(hmod);

	return ret;

} // DisableWFP()