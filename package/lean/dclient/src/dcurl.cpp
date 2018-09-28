#include "dcurl.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
using namespace std;

DCurl::DCurl()
{
    // 获取用户临时文件夹
    GetTmpDir(m_tmpDir);

    // 设置cookie临时保存文件
    m_cookieFile = m_tmpDir + "dcurl.tmp";

    //初始化Url
    m_strUrl = "";

    // 初始化超时
    m_connTimeout = 3L;
    m_timeout = 30L;

    // 初始化POST字符串
    m_strPost = "";

    // 初始化CA证书路径
    m_pCaPath = "";

    // 初始化文件指针
    m_pFile = NULL;

    // 初始化输出变量
    m_strResponse = "";

    m_sType = W_NONE;

    m_rType = GET;
}

DCurl::~DCurl()
{

}

size_t DCurl::_handleWrite(void *buffer, size_t size, size_t nmemb, void *pfn)
{
    DCurl *pDCurl = dynamic_cast<DCurl *>((DCurl *)pfn);
    if (pDCurl)
    {
        return pDCurl->onWrite(buffer, size, nmemb);
    }
    return 0;
}

void DCurl::setOption()
{
    if(NULL == m_pCurl) return;

    // 远程URL，支持 http, https, ftp
    curl_easy_setopt(m_pCurl, CURLOPT_URL, m_strUrl.c_str());

    // 设置User-Agent
    std::string useragent = ("Mozilla/5.0 (Windows NT 6.1; WOW64; rv:13.0) Gecko/20100101 Firefox/13.0.1");
    curl_easy_setopt(m_pCurl, CURLOPT_USERAGENT, useragent.c_str());

    // 设置重定向的最大次数
    curl_easy_setopt(m_pCurl, CURLOPT_MAXREDIRS, 5);

    // 设置301、302跳转跟随location
    curl_easy_setopt(m_pCurl, CURLOPT_FOLLOWLOCATION, 1L);

    // 设置写入回调函数
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, _handleWrite);
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, this);

    // 备注：调试的时候打开
    //curl_easy_setopt(m_pCurl, CURLOPT_VERBOSE, 1);
    //curl_easy_setopt(m_pCurl, CURLOPT_PROXY, "127.0.0.1:8888");

    // 超时设置
    curl_easy_setopt(m_pCurl, CURLOPT_CONNECTTIMEOUT, m_connTimeout);
    curl_easy_setopt(m_pCurl, CURLOPT_TIMEOUT, m_timeout);


    // 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
    // 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
    curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1L);

    // Cookie设置
    if(!m_cookieFile.empty())    // 没有设定文件名就不使用Cookie
    {
        curl_easy_setopt(m_pCurl, CURLOPT_COOKIEFILE, m_cookieFile.c_str()); // 读取本地存储的cookie
        curl_easy_setopt(m_pCurl, CURLOPT_COOKIEJAR, m_cookieFile.c_str());  // 在cleanup之后保存cookie到文件
    }

    // 设置POST数据
    if(m_rType == POST)
    {
        curl_easy_setopt(m_pCurl, CURLOPT_POST, 1L);
        curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, m_strPost.c_str());
    }

    // HTTPS相关设置
    if(!m_pCaPath.empty())
    {
        curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYHOST, 0L);
    }
    else
    {
        //缺省情况就是PEM，所以无需设置，另外支持DER
        //curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
        curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(m_pCurl, CURLOPT_CAINFO, m_pCaPath.c_str());
    }
}

size_t DCurl::_run()
{
    CURLcode res;
    m_pCurl = curl_easy_init();
    if(NULL == m_pCurl)
        return CURLE_FAILED_INIT;

    setOption();

    res = curl_easy_perform(m_pCurl);
    curl_easy_cleanup(m_pCurl);
    return res;
}

size_t DCurl::onWrite(void *buffer, size_t size, size_t nmemb )
{
    size_t return_size = 0;
    if(m_sType == W_NONE) return 0;

    if(m_sType == W_FILE || m_sType == W_ALL)
    {
        if(m_pFile)
        {
            return_size = fwrite(buffer, size, nmemb, m_pFile);
        }
        else
            return_size = -1;
        //std::cout << (char*) buffer << std::endl;
    }

    if(m_sType == W_VAR || m_sType == W_ALL)
    {
        m_strResponse.append((char *)buffer, size * nmemb);
    }
    else
        m_strResponse.append((char *)buffer, size * nmemb);
    return return_size;
}

bool DCurl::_openFile()
{
    //  创建文件
    m_pFile = fopen(m_fileName.c_str(), "wb");
    if (m_pFile == NULL)
        return false;
    return true;
}

bool DCurl::_closeFile()
{
    if(m_pFile)
    {
        fclose(m_pFile);
        m_pFile = NULL;
    }
    return true;
}

int DCurl::httpDownload(const string & strUrl, const string & fileName, const RequestType rType)
{
    SaveType pre_sType = m_sType;
    string preFileName = m_fileName;
    string preStrUrl = m_strUrl;
    RequestType pre_rType = m_rType;
    m_rType = rType;
    m_sType = W_FILE; // 设置输出类型
    if(!strUrl.empty()) m_strUrl = strUrl;
    if(!fileName.empty()) m_fileName = fileName;
    if(!_openFile()) { m_sType = pre_sType; return -1; }

    _run();

    _closeFile();
    m_sType = pre_sType;
    m_fileName = preFileName;
    m_strUrl = preStrUrl;
    m_rType = pre_rType;
    return 1;
}


int DCurl::httpRequest(const string & strUrl, const RequestType rType, bool toVar)
{
    SaveType pre_sType = m_sType;
    RequestType pre_rType = m_rType;
    string preStrUrl = m_strUrl;
    m_rType = rType;

    if(!strUrl.empty())
        m_strUrl = strUrl;
    else
        if(m_strUrl.empty()) return 0;

    m_strResponse.clear();
    if(toVar)
        m_sType = W_VAR;
    else
        m_sType = W_NONE;

    _run();

    m_sType = pre_sType;
    m_rType = pre_rType;
    m_strUrl = preStrUrl;
    return true;
}

string DCurl::httpSuperRequest(const string & strUrl, const RequestType rType,
                             const SaveType sType, const string & fileName)
{
    SaveType pre_sType = m_sType, tmpType = sType;
    string preFileName = m_fileName;
    RequestType pre_rType = m_rType;
    string preStrUrl = m_strUrl;
    m_rType = rType;
    m_strResponse.clear();
    bool f_ext = !fileName.empty();
    if(m_strUrl.empty())
    {
        // 如果本身没有设置地址，并且没有传参，直接返回
        if(strUrl.empty()) return 0;
        else m_strUrl = strUrl;
    }

    // 处理参数为空或者是成员函数m_fileName为空的情况
    if((sType == W_FILE || sType == W_ALL) && !f_ext)
    {
        // 如果sType是W_FILE和W_ALL中的一种，而且没有传参，也没有预先设置m_fileName，执行W_NONE
        if(m_fileName.empty()) tmpType = (sType == W_FILE) ? W_NONE : W_VAR;
    }
    else
        if(f_ext) m_fileName = fileName;

    // 处理打开文件
    if((tmpType == W_FILE || tmpType == W_ALL) && !_openFile())
    {
        if(tmpType == W_ALL) tmpType = W_VAR;
        else tmpType = W_NONE;
    }

    if(sType == W_VAR)
        tmpType = W_VAR;

    m_sType = tmpType;

    _run();

    if(m_pFile) _closeFile();


    m_sType = pre_sType;
    m_rType = pre_rType;
    m_fileName = preFileName;
    m_strUrl = preStrUrl;
    if(tmpType == W_VAR || tmpType == W_ALL) return m_strResponse;
    else return "";
}

void DCurl::getResponse(string & strResponse)
{
    strResponse = m_strResponse;
    m_strResponse.clear();
}

void DCurl::setUrl(const string & strUrl)
{
    m_strUrl = strUrl;
}

void DCurl::setRequestType(RequestType rType)
{
    m_rType = rType;
}

void DCurl::setFileName(const string & fileName)
{
    m_fileName = fileName;
}

void DCurl::setCaPath(const string & pCaPath)
{
    m_pCaPath = pCaPath;
}

void DCurl::setTimeout(const long connTimeout, const long timeout)
{
    m_connTimeout = connTimeout;
    m_timeout = timeout;
}

void DCurl::setPostData(const string & postData)
{
    m_strPost = postData;
}

bool DCurl::cleanCookie()
{
    using std::ofstream;
    using std::ios_base;
    ofstream fout(m_cookieFile, ios_base::out | ios_base::trunc);
    if(!fout.is_open()) return false;
    fout.close();
    return true;
}


