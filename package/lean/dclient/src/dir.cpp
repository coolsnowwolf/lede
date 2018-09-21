#include "dir.h"

bool GetTmpDir(string & tmpDir)
{
#ifndef WIN32
    tmpDir = "/tmp/";
    return true;
#else
    //#define MAX_PATH 260
    char OutPath[MAX_PATH];
    int StrLen = GetTempPath(MAX_PATH, OutPath);
    tmpDir = OutPath;
    return true;
#endif
}
