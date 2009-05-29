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

FileName:	ThemeFileInfo.cpp

Project:	Universal Theme Patcher (Sample Code)
Author:		deepxw
E-mail:		deepxw#gmail.com
Blog:		http://deepxw.blogspot.com (English)
			http://deepxw.lingd.net (中文)

Comment:	If you using this code, you must add my name to your final software, and link to my blog.
Updated:

2008.10.29: Add support xp64/vista64
2009.03.30: Modified, support windows 7 x64 7068
2009.04.07: Modified PATCH_YES compare method, remove && (*(PUCHAR)(chbuf+i+9)== 0xDB)，
            add to read the file sign of other pather

*//////////////////////////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ThemeFileInfo.h"

#include <shlwapi.h>
#pragma comment (lib, "shlwapi.lib")


//
// Using the Strsafe.h Functions
//
#define STRSAFE_LIB					// use the functions in library form
#include <strsafe.h>
//

#define		MALLOC(x)	HeapAlloc(GetProcessHeap(), 0, (x))
#define		FREE(x)		HeapFree(GetProcessHeap(), 0, (x))


CThemeFileInfo::CThemeFileInfo(void)
{
	ZeroMemory(&m_patchInfo, sizeof(PatchInfo));
}

CThemeFileInfo::~CThemeFileInfo(void)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/*
Function:	Get patch information from file
Parameters:
IN:  pszTargetFile
OUT: pInfo
***********************/
BOOL CThemeFileInfo::GetPatchInfo(
										LPTSTR pszTargetFile,	// In,  file name
										PPatchInfo pInfo		// Out, Patch info
										)
{
	//TRACEF(_T("File name:\t%s\n"), pszTargetFile);

	// Check the size of struct
	if (sizeof(PatchInfo) != pInfo->dwSize)
	{
		pInfo->dwSize	= sizeof(PatchInfo);
		return FALSE;
	}

	// Zero the buffer
	ZeroMemory(pInfo, sizeof(PatchInfo));
	pInfo->dwSize	= sizeof(PatchInfo);

	// Check the file if exists
	if(!PathFileExists(pszTargetFile))
		return FALSE;


	//
	// Collect information of the file
	//

	BOOL			ret		= FALSE;
	PatchInfo		info;
	MYVERSIONINFO	vi;

	// Zero the buffer
	ZeroMemory(&info, sizeof(PatchInfo));
	info.dwSize	= sizeof(PatchInfo);

	ZeroMemory(&vi, sizeof(MYVERSIONINFO));
	vi.dwSize	= sizeof(MYVERSIONINFO);
	if (GetDllFileVersion(pszTargetFile, &vi))
		lstrcpyn(info.szFileVersion, vi.szShortVersion, 16);
	else
		return FALSE;


	TCHAR	szFileType[64]	= {0};
	info.dwPlatform = GetPEFileMachineType(pszTargetFile, szFileType);
	lstrcpyn(info.szPlatform, szFileType, 16);


	//TRACEF(_T("File version:\t%s\n"), vi.szFileVersion);
	//TRACEF(_T("File platform:\t%s\n"), szFileType);
		
	// 6.0.2xxx.xxx: XP,  6.0.3xxx.xxx: 2003
	if ( (vi.wMajorVersion == 6) && (vi.wMinorVersion == 0) && (vi.wBuildNumber < 6000) )	// xp/2003
	{
		if ( IMAGE_FILE_MACHINE_AMD64 == info.dwPlatform )		// x64
		{
			ret = GetPatchInfoXp64(pszTargetFile, &info);
		}
		else
		{
			// x86
			if (PATCH_YES != info.PatchStatus)
				ret = GetPatchInfoXp32(pszTargetFile, &info);
		}

	} // xp, 2003, end
	else if ( (vi.wMajorVersion == 6) && (vi.wMinorVersion == 0) )  // vista/2008
	{
		if ( IMAGE_FILE_MACHINE_AMD64 == info.dwPlatform )		// x64
		{
			ret = GetPatchInfoVista64(pszTargetFile, &info);
		}
		else
		{
			// x86
			ret = GetPatchInfoVista32(pszTargetFile, &info);
		}

	} // vista, end
	else if( (vi.wMajorVersion == 6) && (vi.wMinorVersion == 1) )  // Windows 7
	{
		if ( IMAGE_FILE_MACHINE_AMD64 == info.dwPlatform )		// x64
		{
			ret = GetPatchInfoVista64(pszTargetFile, &info);
		}
		else
		{
			// x86
			ret = GetPatchInfoVista32(pszTargetFile, &info);
		}

	} // windows 7, end

	// copy data
	CopyMemory(&m_patchInfo, &info, sizeof(PatchInfo));

	// copy data for return
	CopyMemory(pInfo, &info, sizeof(info));

	return ret;

}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// For XP/2003, 32bit
// Last mod: 2008.10.28
//
BOOL CThemeFileInfo::GetPatchInfoXp32(
										LPTSTR pszTargetFile,	// In,  file name
										PPatchInfo pInfo		// Out, Patch info
										)
{
	// The magic sign
	UCHAR		szSignChar[]	= {0x8B, 0xF6, 0x8B, 0xFF, 0x55, 0x8B, 0xEC, 0x56};
	UCHAR		szSignIndex[]	= {0,    37,   19,   1,     2,    3,   4,    22};
	ULONG		ulSignMagicIndex= 5;
	ULONG		ulLenSignChar	= sizeof(szSignChar);

	// The Cracked char, we need to patch file to this
	UCHAR		szPatchChar[]	= {0x33, 0xF6, 0x8B, 0xC6, 0xC9, 0xC2, 0x08, 0x00};
	ULONG		ulLenPatchChar	= sizeof(szPatchChar);

	// define the range to search
	ULONG		ulFileReadStart	= 0x10000;
	ULONG		ulFileReadEnd	= 0x50000;


	//
	// Begin of stable Code
	//
	BOOL		bRet			= FALSE;
	DWORD		dwMatch			= 0;		// The count of matched sign
	DWORD		dwPatchOffset	= 0;		// Sign offset in the file
	PATCH_STATUS_CLASS PatchStatus	= PATCH_UNKNOWN;

	PVOID		pBuffer			= NULL;		// Pointer to File buffer
	DWORD		dwBufferSize	= 0;		// File buffer size
	PUCHAR		pData			= 0;		// pointer to the data
	ULONG		i				= 0;
	ULONG		k				= 0;


	// Try to open file
	pBuffer = ReadFileToBuffer(pszTargetFile, &ulFileReadStart, &ulFileReadEnd, &dwBufferSize);

	if (NULL == pBuffer)
	{
		// Fail to open file
		return FALSE;
	}

	pData = (PUCHAR)pBuffer;
	for (i = 0; i < (dwBufferSize - 64); i++)
	{

		if ( (*(PUCHAR)(pData + i) == szSignChar[0])
			&& (*(PUCHAR)(pData + i + szSignIndex[1]) == szSignChar[1]) )
		{
			for (k = 2; k < ulLenSignChar; k++)
			{
				// if sign not match, then break
				if ( *(PUCHAR)(pData + i + szSignIndex[k]) != szSignChar[k] )
					break;
			}

			if (k == ulLenSignChar)
			{
				// patched version
				if ( (*(PUCHAR)(pData + i + ulSignMagicIndex) == 0x33) 
					&& (*(PUCHAR)(pData + i + ulSignMagicIndex + 1)== 0xF6) )
				{
					PatchStatus = PATCH_YES;

					// get the real patch offset in the file
					dwPatchOffset = ulFileReadStart + ( i + ulSignMagicIndex);
					dwMatch ++;
				}
				else if ( (*(PUCHAR)(pData + i + ulSignMagicIndex) == 0x81) 
					&& (*(PUCHAR)(pData + i + ulSignMagicIndex + 1) == 0xEC) )	// not patched
				{
					// not patched
					PatchStatus = PATCH_NO;

					// get the real patch offset in the file
					dwPatchOffset = ulFileReadStart + ( i + ulSignMagicIndex);
					dwMatch ++;
				}			
				else if (*(PUCHAR)(pData + i + ulSignMagicIndex) == 0x33)		// not sure
				{
					// 2009.04.06, not sure status
					PatchStatus = PATCH_UNSURE;

					// get the real patch offset in the file
					dwPatchOffset = ulFileReadStart + ( i + ulSignMagicIndex);
					//LimitedValue  = *(PUCHAR)(chbuf+i+8);
					dwMatch ++;
				}

				//TRACEF(_T("Get the right magic offset: 0x%08x\n"), dwPatchOffset);

			} // all char match

		} // first 2 char match

	} // for i

	// Important! free memory
	if (NULL != pBuffer)
		FREE(pBuffer);

	// if match count is 1, we are sure it's the correct offset.
	if (dwMatch == 1)
	{
		pInfo->PatchOffset		= dwPatchOffset;
		pInfo->PatchStatus		= PatchStatus;
		pInfo->PatchCharLength	= ulLenPatchChar;
		CopyMemory(pInfo->PatchChar, szPatchChar, ulLenPatchChar);
	}
	else
	{
		// match count is 0, or more than 1, so it's unknown

		pInfo->PatchOffset		= 0;
		pInfo->PatchStatus		= PATCH_UNKNOWN;
		pInfo->PatchCharLength	= 0;
	}

	pInfo->MatchingCount	= dwMatch;

	return  TRUE;

} // GetPatchInfoXp32



/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// For XP/2003, 64bit
// Last mod: 2008.10.29
//
BOOL CThemeFileInfo::GetPatchInfoXp64(
										LPTSTR pszTargetFile,	// In,  file name
										PPatchInfo pInfo		// Out, Patch info
										)
{
	// The magic sign
	UCHAR		szSignChar[]	= {0x4C, 0x48, 0x45, 0x8B, 0xDC, 0x48, 0x8B, 0x89, 0x4C, 0x8B, 0x11};
	UCHAR		szSignIndex[]	= {0,    17,   40,   1,    2,    10,   11,   18,   25,   26,   27};
	ULONG		ulSignMagicIndex= 3;
	ULONG		ulLenSignChar	= sizeof(szSignChar);

	// The Cracked char, we need to patch file to this
	UCHAR		szPatchChar[]	= {0x48, 0x31, 0xC0, 0xC3, 0x00, 0x00, 0x00};
	ULONG		ulLenPatchChar	= sizeof(szPatchChar);

	// define the range to search
	ULONG		ulFileReadStart	= 0x2000;
	ULONG		ulFileReadEnd	= 0x50000;


	//
	// Begin of stable Code
	//
	BOOL		bRet			= FALSE;
	DWORD		dwMatch			= 0;		// The count of matched sign
	DWORD		dwPatchOffset	= 0;		// Sign offset in the file
	PATCH_STATUS_CLASS PatchStatus	= PATCH_UNKNOWN;

	PVOID		pBuffer			= NULL;		// Pointer to File buffer
	DWORD		dwBufferSize	= 0;		// File buffer size
	PUCHAR		pData			= 0;		// pointer to the data
	ULONG		i				= 0;
	ULONG		k				= 0;


	// Try to open file
	pBuffer = ReadFileToBuffer(pszTargetFile, &ulFileReadStart, &ulFileReadEnd, &dwBufferSize);

	if (NULL == pBuffer)
	{
		// Fail to open file
		return FALSE;
	}

	pData = (PUCHAR)pBuffer;
	for (i = 0; i < (dwBufferSize - 64); i++)
	{

		if ( (*(PUCHAR)(pData + i) == szSignChar[0])
			&& (*(PUCHAR)(pData + i + szSignIndex[1]) == szSignChar[1]) )
		{
			for (k = 2; k < ulLenSignChar; k++)
			{
				// if sign not match, then break
				if ( *(PUCHAR)(pData + i + szSignIndex[k]) != szSignChar[k] )
					break;
			}

			if (k == ulLenSignChar)
			{
				// patched version
				if ( (*(PUCHAR)(pData + i + ulSignMagicIndex) == 0x48) 
					&& (*(PUCHAR)(pData + i + ulSignMagicIndex + 1)== 0x31) )
				{
					PatchStatus = PATCH_YES;

					// get the real patch offset in the file
					dwPatchOffset = ulFileReadStart + ( i + ulSignMagicIndex);
					dwMatch ++;
				}
				else if ( (*(PUCHAR)(pData + i + ulSignMagicIndex) == 0x48) 
					&& (*(PUCHAR)(pData + i + ulSignMagicIndex + 1) == 0x81) )	// not patched
				{
					// not patched
					PatchStatus = PATCH_NO;

					// get the real patch offset in the file
					dwPatchOffset = ulFileReadStart + ( i + ulSignMagicIndex);
					dwMatch ++;
				}			

				//TRACEF(_T("Get the right magic offset: 0x%08x\n"), dwPatchOffset);

			} // all char match

		} // first 2 char match

	} // for i

	// Important! free memory
	if (NULL != pBuffer)
		FREE(pBuffer);

	// if match count is 1, we are sure it's the correct offset.
	if (dwMatch == 1)
	{
		pInfo->PatchOffset		= dwPatchOffset;
		pInfo->PatchStatus		= PatchStatus;
		pInfo->PatchCharLength	= ulLenPatchChar;
		CopyMemory(pInfo->PatchChar, szPatchChar, ulLenPatchChar);
	}
	else
	{
		// match count is 0, or more than 1, so it's unknown

		pInfo->PatchOffset		= 0;
		pInfo->PatchStatus		= PATCH_UNKNOWN;
		pInfo->PatchCharLength	= 0;
	}

	pInfo->MatchingCount	= dwMatch;

	return  TRUE;

} // GetPatchInfoXp64


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// For Vista/2008, 32bit
// Last mod: 2008.10.30
//
BOOL CThemeFileInfo::GetPatchInfoVista32(
										LPTSTR pszTargetFile,	// In,  file name
										PPatchInfo pInfo		// Out, Patch info
										)
{
	// The magic sign
	UCHAR		szSignChar[]	= {0x8B, 0x33, 0x8B, 0xFF, 0x55, 0x8B, 0xEC};
	UCHAR		szSignIndex[]	= {0,    16,   23,   1,    2,    3 ,   4};
	ULONG		ulSignMagicIndex= 5;
	ULONG		ulLenSignChar	= sizeof(szSignChar);

	// The Cracked char, we need to patch file to this
	UCHAR		szPatchChar[]	= {0x33, 0xC0, 0xC9, 0xC2, 0x04, 0x00};
	ULONG		ulLenPatchChar	= sizeof(szPatchChar);

	// define the range to search
	ULONG		ulFileReadStart	= 0x2000;
	ULONG		ulFileReadEnd	= 0x50000;


	//
	// Begin of stable Code
	//
	BOOL		bRet			= FALSE;
	DWORD		dwMatch			= 0;		// The count of matched sign
	DWORD		dwPatchOffset	= 0;		// Sign offset in the file
	PATCH_STATUS_CLASS PatchStatus	= PATCH_UNKNOWN;

	PVOID		pBuffer			= NULL;		// Pointer to File buffer
	DWORD		dwBufferSize	= 0;		// File buffer size
	PUCHAR		pData			= 0;		// pointer to the data
	ULONG		i				= 0;
	ULONG		k				= 0;


	// Try to open file
	pBuffer = ReadFileToBuffer(pszTargetFile, &ulFileReadStart, &ulFileReadEnd, &dwBufferSize);

	if (NULL == pBuffer)
	{
		// Fail to open file
		return FALSE;
	}

	pData = (PUCHAR)pBuffer;
	for (i = 0; i < (dwBufferSize - 64); i++)
	{

		if ( (*(PUCHAR)(pData + i) == szSignChar[0])
			&& (*(PUCHAR)(pData + i + szSignIndex[1]) == szSignChar[1]) )
		{
			for (k = 2; k < ulLenSignChar; k++)
			{
				// if sign not match, then break
				if ( *(PUCHAR)(pData + i + szSignIndex[k]) != szSignChar[k] )
					break;
			}

			// before win7 M1
			if ( (k == ulLenSignChar)
				&& (*(PUCHAR)(pData + i + 33) == 0xF6)
				&& (*(PUCHAR)(pData + i + 34) == 0xD8) )
			{
				//TRACEF(_T("Windows 7 M1, and before\n"), dwPatchOffset);

				// patched version
				if ( (*(PUCHAR)(pData + i + ulSignMagicIndex) == 0x33) 
					&& (*(PUCHAR)(pData + i + ulSignMagicIndex + 1)== 0xC0) )
				{
					PatchStatus = PATCH_YES;

					// get the real patch offset in the file
					dwPatchOffset = ulFileReadStart + ( i + ulSignMagicIndex);
					dwMatch ++;
				}
				else if ( (*(PUCHAR)(pData + i + ulSignMagicIndex) == 0x81) 
					&& (*(PUCHAR)(pData + i + ulSignMagicIndex + 1) == 0xEC) )	// not patched
				{
					// not patched
					PatchStatus = PATCH_NO;

					// get the real patch offset in the file
					dwPatchOffset = ulFileReadStart + ( i + ulSignMagicIndex);
					dwMatch ++;
				}			

				//TRACEF(_T("Get the right magic offset: 0x%08x\n"), dwPatchOffset);

			} // all char match, before win7 M1
			else if ( (k == ulLenSignChar)
				&& (*(PUCHAR)(pData + i + 33) == 0x0F)
				&& (*(PUCHAR)(pData + i + 36) == 0xF7) )
			{
				// win 7 M3
				//TRACEF(_T("Windows 7 M3, and later\n"), dwPatchOffset);

				// patched version
				if ( (*(PUCHAR)(pData + i + ulSignMagicIndex) == 0x33) 
					&& (*(PUCHAR)(pData + i + ulSignMagicIndex + 1)== 0xC0) )
				{
					PatchStatus = PATCH_YES;

					// get the real patch offset in the file
					dwPatchOffset = ulFileReadStart + ( i + ulSignMagicIndex);
					dwMatch ++;
				}
				else if ( (*(PUCHAR)(pData + i + ulSignMagicIndex) == 0x81) 
					&& (*(PUCHAR)(pData + i + ulSignMagicIndex + 1) == 0xEC) )	// not patched
				{
					// not patched
					PatchStatus = PATCH_NO;

					// get the real patch offset in the file
					dwPatchOffset = ulFileReadStart + ( i + ulSignMagicIndex);
					dwMatch ++;
				}			

				//TRACEF(_T("Get the right magic offset: 0x%08x\n"), dwPatchOffset);

			} // all char match, after win7 M3

		} // first 2 char match

	} // for i

	// Important! free memory
	if (NULL != pBuffer)
		FREE(pBuffer);

	// if match count is 1, we are sure it's the correct offset.
	if (dwMatch == 1)
	{
		pInfo->PatchOffset		= dwPatchOffset;
		pInfo->PatchStatus		= PatchStatus;
		pInfo->PatchCharLength	= ulLenPatchChar;
		CopyMemory(pInfo->PatchChar, szPatchChar, ulLenPatchChar);
	}
	else
	{
		// match count is 0, or more than 1, so it's unknown

		pInfo->PatchOffset		= 0;
		pInfo->PatchStatus		= PATCH_UNKNOWN;
		pInfo->PatchCharLength	= 0;
	}

	pInfo->MatchingCount	= dwMatch;

	return  TRUE;

} // GetPatchInfoVista32


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// For Vista/2008, 64bit
// Last mod: 2009.04.07
// 2009.04.07: Modified PATCH_YES比较条件, 去除&& (*(PUCHAR)(chbuf+i+9)== 0xDB)，兼容其它补丁
// 2009.03.30: Modified ulPointEnd 0x50000 -> 0x70000, support windows 7 x64 7068
//
BOOL CThemeFileInfo::GetPatchInfoVista64(
										LPTSTR pszTargetFile,	// In,  file name
										PPatchInfo pInfo		// Out, Patch info
										)
{
	// The magic sign
	UCHAR		szSignChar[]	= {0x48, 0x8B, 0x48, 0x89, 0x55, 0x56, 0x57, 0x48, 0x08, 0x8B, 0xF9};
	UCHAR		szSignIndex[]	= {0,    16,   33,   1,    5,    6 ,   7,    22,   39,   34 ,   35};
	ULONG		ulSignMagicIndex= 8;
	ULONG		ulLenSignChar	= sizeof(szSignChar);

	// The Cracked char, we need to patch file to this
	UCHAR		szPatchChar[]	= {0x33, 0xDB, 0x8B, 0xC3, 0x5F, 0x5E, 0x5D, 0xC3};
	ULONG		ulLenPatchChar	= sizeof(szPatchChar);

	// define the range to search
	ULONG		ulFileReadStart	= 0x2000;
	ULONG		ulFileReadEnd	= 0x80000;


	//
	// Begin of stable Code
	//
	BOOL		bRet			= FALSE;
	DWORD		dwMatch			= 0;		// The count of matched sign
	DWORD		dwPatchOffset	= 0;		// Sign offset in the file
	PATCH_STATUS_CLASS PatchStatus	= PATCH_UNKNOWN;

	PVOID		pBuffer			= NULL;		// Pointer to File buffer
	DWORD		dwBufferSize	= 0;		// File buffer size
	PUCHAR		pData			= 0;		// pointer to the data
	ULONG		i				= 0;
	ULONG		k				= 0;


	// Try to open file
	pBuffer = ReadFileToBuffer(pszTargetFile, &ulFileReadStart, &ulFileReadEnd, &dwBufferSize);

	if (NULL == pBuffer)
	{
		// Fail to open file
		return FALSE;
	}

	pData = (PUCHAR)pBuffer;
	for (i = 0; i < (dwBufferSize - 64); i++)
	{

		if ( (*(PUCHAR)(pData + i) == szSignChar[0])
			&& (*(PUCHAR)(pData + i + szSignIndex[1]) == szSignChar[1]) )
		{
			for (k = 2; k < ulLenSignChar; k++)
			{
				// if sign not match, then break
				if ( *(PUCHAR)(pData + i + szSignIndex[k]) != szSignChar[k] )
					break;
			}

			if (k == ulLenSignChar)
			{
				// patched version
				if ( (*(PUCHAR)(pData + i + ulSignMagicIndex) == 0x33) 
					&& (*(PUCHAR)(pData + i + 15) == 0xC3) )
				{
					PatchStatus = PATCH_YES;

					// get the real patch offset in the file
					dwPatchOffset = ulFileReadStart + ( i + ulSignMagicIndex);
					dwMatch ++;
				}
				else if ( (*(PUCHAR)(pData + i + ulSignMagicIndex) == 0x48) 
					&& (*(PUCHAR)(pData + i + ulSignMagicIndex + 1) == 0x81) 
					&& (*(PUCHAR)(pData + i + 15) == 0x48))						// not patched
				{
					// not patched
					PatchStatus = PATCH_NO;

					// get the real patch offset in the file
					dwPatchOffset = ulFileReadStart + ( i + ulSignMagicIndex);
					dwMatch ++;
				}			
				else if (*(PUCHAR)(pData + i + ulSignMagicIndex) == 0x33)		// not sure
				{
					// 2009.04.06, not sure status
					PatchStatus = PATCH_UNSURE;

					// get the real patch offset in the file
					dwPatchOffset = ulFileReadStart + ( i + ulSignMagicIndex);
					//LimitedValue  = *(PUCHAR)(chbuf+i+8);
					dwMatch ++;
				}

				//TRACEF(_T("Get the right magic offset: 0x%08x\n"), dwPatchOffset);

			} // all char match

		} // first 2 char match

	} // for i

	// Important! free memory
	if (NULL != pBuffer)
		FREE(pBuffer);

	// if match count is 1, we are sure it's the correct offset.
	if (dwMatch == 1)
	{
		pInfo->PatchOffset		= dwPatchOffset;
		pInfo->PatchStatus		= PatchStatus;
		pInfo->PatchCharLength	= ulLenPatchChar;
		CopyMemory(pInfo->PatchChar, szPatchChar, ulLenPatchChar);
	}
	else
	{
		// match count is 0, or more than 1, so it's unknown

		pInfo->PatchOffset		= 0;
		pInfo->PatchStatus		= PATCH_UNKNOWN;
		pInfo->PatchCharLength	= 0;
	}

	pInfo->MatchingCount	= dwMatch;

	return  TRUE;

} // GetPatchInfoVista64



/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Read file to buffer, return buffer size, and buffer pointer
//
PVOID CThemeFileInfo::ReadFileToBuffer(
					   LPTSTR	pszFile,			// in,  File name
					   PDWORD	pdwFileReadStart,	// I/O,  Start offset to read
					   PDWORD	pdwFileReadEnd,		// I/O,  End offset
					   PDWORD	pdwBufferSize		// out, Buffer size
					   )		
{
	HANDLE		hFile;						// Handle of the File
	DWORD		dwFileSize		= 0;
	DWORD		dwBytesRead		= 0;
	PVOID		pBuffer			= NULL;		// Pointer to File buffer

	*pdwBufferSize	= 0;

	// Try to open file
	hFile = CreateFile(pszFile, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		// Fail to open file
		return NULL;
	}

	dwFileSize = GetFileSize(hFile, NULL);

	// Check the range is valid
	if (*pdwFileReadEnd > dwFileSize)
		*pdwFileReadEnd = dwFileSize;

	if (*pdwFileReadEnd < *pdwFileReadStart)
	{
		CloseHandle(hFile);
		return NULL;
	}


	// Allocate memory
	*pdwBufferSize = *pdwFileReadEnd - *pdwFileReadStart;
	pBuffer = MALLOC(*pdwBufferSize);

	// ZeroMemory(pBuffer, *pdwBufferSize);

	// seek to start of range
	SetFilePointer(hFile, *pdwFileReadStart, NULL, FILE_BEGIN);

	// Read data
	BOOL bRet = ReadFile(hFile, pBuffer, *pdwBufferSize, &dwBytesRead, 0);
	CloseHandle(hFile);

	// If fail to read file, then exit.
	if (!bRet)
	{
		FREE(pBuffer);
		return NULL;
	}

	return pBuffer;
}