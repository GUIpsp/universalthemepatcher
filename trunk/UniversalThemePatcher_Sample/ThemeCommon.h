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
FileName:	ThemeCommon.h

Project:	Universal Theme Patcher (Sample Code)
Author:		deepxw
E-mail:		deepxw#gmail.com
Blog:		http://deepxw.blogspot.com

Comment:	If you using this code, you must add my name to your final software, and link to my blog.

Last Updated:  2008.10.28


*//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __ThemeCommon_H__
#define __ThemeCommon_H__
/////////////////////////////////////////////////////////////////////////////

#pragma once


// patch status of file
typedef enum _PATCH_STATUS_CLASS
{
	PATCH_UNKNOWN		= 0,
	PATCH_YES			= 1,
	PATCH_NO			= 2,
	PATCH_UNSURE		= 3
} PATCH_STATUS_CLASS;

// Structure to define patch file information
typedef struct _PatchInfo
{
	DWORD				dwSize;						// Struct size

	PATCH_STATUS_CLASS	PatchStatus;				// Status of file, patched: Yes/No/Unknown/Unsure
	DWORD 				PatchOffset;				// Offset of magic jump
	DWORD				PatchCharLength;			// Length of patch char
	UCHAR 				PatchChar[10];				// We need to mod file to this char
	DWORD				MatchingCount;				// Count of Matching Sign

	TCHAR 				szFileVersion[64];			// File version
	TCHAR 				szPlatform[64];				// File platform
	TCHAR 				szCheckSum[64];				// File checksum

	DWORD				dwPlatform;					// File platform
	DWORD				dwSignatureType;			// File signature type
} PatchInfo, *PPatchInfo;


// Struct for PE file version info
typedef struct _MYVERSIONINFO
{
	DWORD	dwSize;							// struct size

	WORD	wMajorVersion;					// Major version number
	WORD	wMinorVersion;					// Minor version number
	WORD	wBuildNumber;					// Build number
	WORD	wRevisionNumber;				// Revision number

	WORD	wLangID;						// Language ID;

	TCHAR	szShortVersion[32];				// File version string, likes "1.0.2.2000"
	TCHAR	szFileVersion[MAX_PATH];		// File version string, likes "1.0.2.2000, rc_061220"
	TCHAR	szOriginalFilename[64];			// Original file name
}MYVERSIONINFO, *PMYVERSIONINFO;


// For PE file Machine type <Start>

#ifndef IMAGE_FILE_MACHINE_AMD64
#define IMAGE_FILE_MACHINE_AMD64             0x8664  // AMD64 (K8)
#endif

// For PE file Machine type <End>


// Convert patch status from number to string
extern BOOL PatchStatusToString(PPatchInfo pInfo, LPTSTR szType);

// Patch the file with special offset and sign
extern BOOL SetPatchInfo(LPTSTR pszFile, PPatchInfo pInfo);


// Get Machine Type of PE File
extern DWORD GetPEFileMachineType(LPCTSTR pszFile, LPTSTR pszFileType);

// Correct the checksum of PE file
extern BOOL SetCorrectChecksum(LPCTSTR pszFile);


// Get the PE file version info
extern BOOL GetDllFileVersion(LPCTSTR pszFileName, PMYVERSIONINFO pVersionInfo);


// Chect OS version
extern __inline BOOL isNT5(void);
extern __inline BOOL isNT6(void);

// Replace the file in using by API MoveFileEx()
extern BOOL SuperCopyFile(LPTSTR pszSrcFile, LPTSTR pszTargetFile);

// Run console program in Hidden Console
extern BOOL RunHiddenConsole(LPTSTR pszCmdLine, BOOL bWait);

// For disable SFC/WFP in NT5. Using No. 5 function in sfc_os.dll.
HRESULT DisableWFP(LPTSTR pszFileName);

/////////////////////////////////////////////////////////////////////////////
#endif