//Author:	greensea <gs@bbxy.net>
#include "antiLeech_wx.h" //Modified by Bill Lee.

//Bug fixed by Orzogc Lee
LPCTSTR StrStrI(LPCTSTR haystack, LPCTSTR needle){
	//Bill Lee: allocate wchar array on the stack
	wchar_t haystacki[512];
	wchar_t needlei[512];
	int i = 0;
	do{
		haystacki[i] = towlower(haystack[i]);
		if(i == 511)
			break;
	}while(haystack[i++]); //As haystacki is allocated on the stack, it wans't set 0. So the NULL needs to be copy.
	i = 0;
	do{
		needlei[i] = towlower(needle[i]);
		if(i == 511)
			break;
	}while(needle[i++]);
	const wchar_t* ret = wcsstr(haystacki, needlei);
	if(ret != NULL)
		ret = ret - haystacki + haystack;
	return ret;
}
