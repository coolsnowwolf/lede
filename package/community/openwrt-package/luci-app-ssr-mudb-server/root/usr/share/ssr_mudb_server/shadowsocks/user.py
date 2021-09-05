#coding:utf-8

import base64
import re

class miss:
    def ssr4(self,ssrurl):
        missing_padding = 4 - len(ssrurl) % 4
        if missing_padding:
            ssrurl += b'='* missing_padding
            return ssrurl

def checkip(ip):
    p = re.compile('^((25[0-5]|2[0-4]\d|[01]?\d\d?)\.){3}(25[0-5]|2[0-4]\d|[01]?\d\d?)$')
    if p.match(ip):
        return False
    else:
        print "输入错误请重新输入"
        return True

def checkport(port):
    try:
        if int(port)<0 or int(port)>65535:
            print "输入错误请重新输入"
            return True
        return False
    except:
        print "输入错误请重新输入"
        return True

print '''
请输入SSR地址码
（就是ssr://XX,复制粘贴不嫌长您了就手打）
手动配置服务器信息请输入1'''

error = True
while error:
    ssrhead = raw_input()
    if ssrhead == "1":
        break
    try:
        ssrhead = re.split('[:/]',ssrhead)
        ssrurl = miss()
        ssrurl = ssrurl.ssr4(ssrhead[3])		
        ssrurl = base64.urlsafe_b64decode(ssrurl)
        ssrurl = re.split('[:/?&]',ssrurl)
        serverip = ssrurl[0]
        serverport = ssrurl[1]
        password = ssrurl[5]
        password = miss()
        password = password.ssr4(ssrurl[5])
        password = base64.urlsafe_b64decode(password)
        method = ssrurl[3]
        protocol = ssrurl[2]
        obfs = ssrurl[4]
        error = False
    except:
        print "导入失败，请输入正确的SSR地址或输入1手动配置"
        error = True

if ssrhead == "1":
    error = True
    while error:
        serverip = raw_input("请输入服务器IP地址:\n")
        error = checkip(serverip)
    error = True
    while error:
        serverport = raw_input("请输入服务器端口:\n")
        error = checkport(serverport)
    error = True
    while error:
        print "请输入本机代理地址，默认127.0.0.1，使用默认请回车"
        localaddress = raw_input()
        if localaddress == "":
            localaddress = "127.0.0.1"
        error = checkip(localaddress)
    error = True
    while error:
        print "请输入本机代理端口，默认1080，使用默认请回车"
        localport = raw_input()
        if localport == "":
            localport = "1080"
        error = checkport(localport)
    password = raw_input("请输入密码:\n")

    print '''
0="NONE不加密"
1="table"
2="rc4"
3="rc4-md5"
4="rc4-md5-6"
5="aes-128-cfb"
6="aes-192-cfb"
7="aes-256-cfb"
8="aes-128-ctr"
9="aes-192-ctr"
10="aes-256-ctr"
11="bf-cfb"
12="camellia-128-cfb"
13="camellia-192-cfb"
14="camellia-256-cfb"
15="cast5-cfb"
16="des-cfb"
17="idea-cfb"
18="rc2-cfb"
19="seed-cfb"
20="salsa20"
21="chacha20"
22="chacha20-ietf"

请输入对应的加密方式数字'''
    method = ["","table","rc4","rc4-md5","rc4-md5-6","aes-128-cfb"\
,"aes-192-cfb","aes-256-cfb","aes-128-ctr","aes-192-ctr","aes-256-ctr"\
,"bf-cfb","camellia-128-cfb","camellia-192-cfb","camellia-256-cfb"\
,"cast5-cfb","des-cfb","idea-cfb","rc2-cfb","seed-cfb","salsa20"\
,"chacha20","chacha20-ietf"]
    error = True
    while error:
        try:
            num = input()
            if num < 0:
                num = "错误"
            method = method[num]
            error = False
        except:
            print "输入错误，请输入正确的数字"
            error = True

    print '''
1="origin"
2="verify_simple"
3="verify_sha1"
4="auth_sha1"
5="auth_sha1_v2"
6="auth_sha1_v4"
7="auth_aes128_sha1"
8="auth_aes128_md5"
9="auth_chain_a"
10="auth_chain_b"
11="auth_chain_c"
12="auth_chain_d"
    
请输入对应的协议插件数字'''
    protocol = ["origin","verify_simple","verify_sha1","auth_sha1","auth_sha1_v2","auth_sha1_v4"\
,"auth_aes128_sha1","auth_aes128_md5","auth_chain_a","auth_chain_b","auth_chain_c","auth_chain_d"]
    error = True
    while error:
        try:
            num = input()
            if num < 1:
                num = "错误"
            protocol = protocol[num-1]
            error = False
        except:
            print "请输入正确的数字"
            error = True

    protocolparam = raw_input("请输入协议参数，不使用参数请回车:\n")

    print '''
1="plain"
2="http_simple"
3="http_post"
4="tls_simple"
5="tls1.2_ticket_auth"
6="tls1.2_ticket_fastauth"
    
请输入对应的混淆参数的数字'''
    obfs = ["plain","http_simple","http_post","tls_simple"\
,"tls1.2_ticket_auth","tls1.2_ticket_fastauth"]
    error = True
    while error:
        try:
            num = input()
            if num < 1:
                num = "错误"
            obfs = obfs[num-1]
            error = False
        except:
            print "请输入正确的数字"
            error = True

    print '''
请输入混淆参数
示例:baidu.com (不需要加http)
不使用参数请回车'''
    obfsparam = raw_input()

else:
    error = True
    while error:
        print "请输入本机代理地址，默认127.0.0.1，使用默认请回车"
        localaddress = raw_input()
        if localaddress == "":
            localaddress = "127.0.0.1"
        error = checkip(localaddress)
    error = True
    while error:
        print "请输入本机代理端口，默认1080，使用默认请回车"
        localport = raw_input()
        if localport == "":
            localport = "1080"
        error = checkport(localport)
    protocolparam = raw_input("请输入协议参数，不使用参数请回车:\n")
    print '''
请输入混淆参数
示例:baidu.com (不需要加http)
不使用参数请回车'''
    obfsparam = raw_input()

user='''{
    "server": "%s",
    "server_ipv6": "::",
    "server_port": %s,
    "local_address": "%s",
    "local_port": %s,

    "password": "%s",
    "method": "%s",
    "protocol": "%s",
    "protocol_param": "%s",
    "obfs": "%s",
    "obfs_param": "%s",
    "speed_limit_per_con": 0,
    "speed_limit_per_user": 0,

    "additional_ports" : {},
    "timeout": 120,
    "udp_timeout": 60,
    "dns_ipv6": false,
    "connect_verbose_info": 0,
    "redirect": "",
    "fast_open": false
}
'''%(serverip,serverport,localaddress,localport,\
password,method,protocol,protocolparam,obfs,obfsparam)
with open('user-config.json','w') as f:
	f.write(user.encode("utf-8"))
print user+"\n"+"请检查输入是否有误，若需要修改请重新执行程序。\
\n启动ssr请在终端切换至shadowsocksr/shadowsocks目录执行python local.py -d start"
