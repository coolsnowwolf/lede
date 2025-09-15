-- Copyright (C) 2020-2025  sirpdboy  <herboy2008@gmail.com> https://github.com/sirpdboy/netspeedtest
require("luci.util")
local o,t,e

o = Map("netspeedtest", "<font color='green'>" .. translate("Net Speedtest") .."</font>",translate( "Network speed diagnosis test (including intranet and extranet)<br/>For specific usage, see:") ..translate("<a href=\'https://github.com/sirpdboy/netspeedtest.git' target=\'_blank\'>GitHub @sirpdboy/netspeedtest</a>") )

t = o:section(TypedSection, "speedtestport", translate('Server Port Latency Test'))
t.addremove=false
t.anonymous=true

e = t:option(Value, 'sdomain', translate('Test server address'))
e.default = "www.baidu.com"
e.description = translate('Enter the domain name or IP address of the server that needs to be tested')

e = t:option(Value, 'sport', translate('Test server port'))
e.default = "443"

e = t:option(Value, 'tcpspeed', translate('Server Port Delay Value'))

e = t:option(DummyValue, '', '')
e.rawhtml = true
e.template ='netspeedtest/speedtestport'


return o
