#ifndef DOCR_H_
#define DOCR_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <memory.h>
#include "jpeglib.h"
#include "jerror.h"
#include "setjmp.h"

using std::vector;
using std::string;

using std::cout;
using std::endl;

class DOcr
{
public:
    enum OcrMode
    {
        JPEG_FILE = 20,
        JPEG_MEM
    };
protected:
    OcrMode m_mode;
    int m_img_width;
    int m_img_height;
    int m_img_components;
    vector<JSAMPROW> m_img_buf;
    vector<string> m_keys;
    vector<char> m_value;
    string m_code;
    int _readJpeg(const string & str);
    int _writeJpeg(string & str, int quality);
    void _setKeys();
    void _findCode(const string & s);
public:
    DOcr();
    ~DOcr();
    void setMode(OcrMode mode);
    bool readImage(const string & str, OcrMode mode = JPEG_FILE);
    bool writeImage(string & str, OcrMode mode = JPEG_FILE);
    bool run();
    void getCode(string & code);


};

#endif // DOCR_H_

/* 注意：如果libjpeg编译中出现
 * error: conflicting types for 'jpeg_suppress_tables'
 * jpeg_suppress_tables (j_compress_ptr cinfo, boolean suppress)
 * 这种错误
 * 那么 把#undef HAVE_PROTOTYPES 改成
 * #define HAVE_PROTOTYPES
 * 然后重新编译
 * 第二点，
 * 如果有libjpeg由于configure脚本不正确配置导致出现
 * conflicting declaration 'typedef short int UINT8'之类的配置，解决得办法如下：
 * 在jconfig.h中把
 * #undef HAVE_UNSIGNED_CHAR
 * #undef HAVE_UNSIGNED_SHORT
 * 改成
 * #define HAVE_UNSIGNED_CHAR
 * #define HAVE_UNSIGNED_SHORT
 * 然后重新编译（一定要做，而且编译完后，使用这个库的项目要rebuild（重新编译））
 * 本人用的是MinGW32 不清楚其他编译器
 * 另外呢 我还define了这几个 HAVE_STDDEF_H HAVE_STDLIB_H
 * 不知道会不会影响大局
 */
