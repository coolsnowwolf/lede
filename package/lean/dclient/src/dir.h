#ifndef DIR_H_INCLUDED
#define DIR_H_INCLUDED

#include <string>
using std::string;

#include "global.h"

#ifdef WIN32
    #include <windows.h>
#endif

bool GetTmpDir(string & tmpDir);

#endif // DIR_H_INCLUDED
