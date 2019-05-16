#ifndef ANTILEECH_WX_H
#define ANTILEECH_WX_H

#include <wx/defs.h>
#include <string.h>
#include <wctype.h>   // for towlower()

#define LPCTSTR		const wxChar* 
#define BOOL		bool
//#define _T(var)		wxT(var)	//defined in wxWidgets
#define DWORD		wxUint32
#define UINT		wxUint16
#define WINAPI
#define HINSTANCE
#define LPVOID		void*
#define PBYTE		unsigned char*
#define TCHAR		wxChar
#define _TINT		wxInt32
#define SSIZE_T		ptrdiff_t

#define StrCmpI _tcsicmp
#define _tcsicmp _wcsicmp

#define _istdigit(var)		iswdigit(var)
#define _istcntrl(var)		iswcntrl(var)
#define _istpunct(var)		iswpunct(var)
#define _istspace(var)		iswspace(var)
#define _istxdigit(var)		iswxdigit(var)
inline float _tstof(const wchar_t* str){
	wchar_t** ptail = NULL;
	return wcstof(str, ptail);
}
//This function is not used. by Orzogc Lee
//But I think there is no need to removing, linker will remove it.
/*
inline void tolowers(wxChar* str){
	int i = 0;
	do{
		str[i] = towlower(str[i]);
	}while(str[++i]);
}
*/
#define _tcsstr(haystack, needle)	wcsstr(haystack, needle)
#define _tcslen(var)		wcslen(var)
#define StrStr(a, b)		wcsstr(a, b)
#define StrStrIW(a, b)		StrStrI(a, b)

LPCTSTR StrStrI(LPCTSTR haystack, LPCTSTR needle);
//Bill Lee: I think inlining this function make no senses, because it is a very large operation.

#define _wcsicmp(a, b)		wcscasecmp(a, b)
#define StrCmpIW(a, b)		wcscasecmp(a, b)

#endif
