-- Copyright (C) 2020-2025  sirpdboy  <herboy2008@gmail.com> https://github.com/sirpdboy/netspeedtest
require("luci.util")
local o,s,e


o = Map("netspeedtest", "<font color='green'>" .. translate("Net Speedtest") .."</font>",translate( "Network speed diagnosis test (including intranet and extranet)<br/>For specific usage, see:") ..translate("<a href=\'https://github.com/sirpdboy/netspeedtest.git' target=\'_blank\'>GitHub @sirpdboy/netspeedtest</a>") )
o:section(SimpleSection).template  = "netspeedtest/speedtestiperf3_status"

s = o:section(TypedSection, "speedtestiperf3", translate('Lan Speedtest Iperf3'))
s.addremove=false
s.anonymous=true

e = s:option(DummyValue, '', '')
e.rawhtml = true
e.template ='netspeedtest/speedtestiperf3'


return o
