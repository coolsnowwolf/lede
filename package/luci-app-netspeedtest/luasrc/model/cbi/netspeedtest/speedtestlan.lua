-- Copyright (C) 2020-2025  sirpdboy  <herboy2008@gmail.com> https://github.com/sirpdboy/netspeedtest
local m, s ,o

m = Map("netspeedtest", "<font color='green'>" .. translate("Net Speedtest") .."</font>",translate( "Network speed diagnosis test (including intranet and extranet)<br/>For specific usage, see:") ..translate("<a href=\'https://github.com/sirpdboy/netspeedtest.git' target=\'_blank\'>GitHub @sirpdboy/netspeedtest</a>") )

s = m:section(NamedSection, "netspeedtest", "netspeedtest", translate('Lan Speedtest Web'))

o=s:option(Flag,"enabled",translate("Enable Homebox service"))
o.default=0

o = s:option(DummyValue, '', '')
o.rawhtml = true
o.template ='netspeedtest/speedtestlan'

m.apply_on_parse = true
m.on_after_apply = function(self,map)
  io.popen("/etc/init.d/netspeedtest restart")
  luci.http.redirect(luci.dispatcher.build_url("admin","network","netspeedtest","speedtestlan"))
end
return m
