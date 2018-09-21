#ifndef DCURL_H_
#define DCURL_H_
#include <curl/curl.h>
#include <string>
#include "dir.h"

using std::string;

class DCurl
{
public:
    enum SaveType   //输出类型
    {
        W_NONE = 0, //丢弃输出
        W_VAR,      //保存到变量
        W_FILE,     //保持到文件
        W_ALL       //保存到变量和文件
    };

    enum RequestType
    {
        GET = 10,
        POST
    };
protected:
    string m_cookieFile;
    string m_tmpDir;
    string m_strUrl;
    long m_connTimeout;
    long m_timeout;
    string m_strPost;
    string m_pCaPath;
    CURL* m_pCurl;
    FILE* m_pFile;
    string m_fileName;
    string m_strResponse;
    SaveType m_sType;
    RequestType m_rType;

protected:
    /// 写入回调函数（静态）
    static size_t _handleWrite(void *buffer, size_t size, size_t nmemb, void *pfn);

    /// 执行curl
    size_t _run();

    /// 写入回调函数
    size_t onWrite(void *buffer, size_t size, size_t nmemb);

    /// 打开文件
    bool _openFile();

    /// 关闭文件
    bool _closeFile();

public:
    /// 构造函数
    DCurl();

    ///析构函数
    ~DCurl();

    /// 设置libcurl选项
    void setOption();

    /// 下载文件
    int httpDownload(const string & strUrl, const string & fileName, const RequestType rType = GET);

    /// 普通请求
    int httpRequest(const string & strUrl, const RequestType rType = GET, const bool toVar = false);

    /// 超级请求
    string httpSuperRequest(const string & strUrl, const RequestType rType = GET,
                          const SaveType sType = W_NONE, const string & fileName = "");

    /// 获取执行结果
    void getResponse(string & strResponse);

    /// 设置Url地址
    void setUrl(const string & strUrl = "");

    /// 设置请求的类型, GET/POST
    void setRequestType(RequestType rType = GET);

    /// 设置要保存的文件名称（含路径）
    void setFileName(const string & fileName = "");

    /// 设置CA证书路径
    void setCaPath(const string & pCaPath = "");

    /// 设置超时时间
    void setTimeout(const long connTimeout, const long timeout);

    /// 设置Post数据
    void setPostData(const string & postData);

    /// 清空Cookie
    bool cleanCookie();
};
#endif // DCURL_H_
