-- Copyright (C) 2020-2025  sirpdboy  <herboy2008@gmail.com> https://github.com/sirpdboy/netspeedtest

require("luci.util")
local o,t,e

luci.sys.exec("echo '-' >/tmp/netspeedtest.log&&echo 1 > /tmp/netspeedtestpos" )
o = Map("netspeedtest", "<font color='green'>" .. translate("Net Speedtest") .."</font>",translate( "Network speed diagnosis test (including intranet and extranet)<br/>For specific usage, see:") ..translate("<a href=\'https://github.com/sirpdboy/netspeedtest.git\' target=\'_blank\'>GitHub @sirpdboy/netspeedtest</a>") )

t=o:section(TypedSection,"speedtestwan",translate("Broadband speedtest"))
t.anonymous=true

e = t:option(ListValue, 'speedtest_cli', translate('client version selection'))
e:value("0",translate("ookla-speedtest-cli"))
e:value("1",translate("python3-speedtest-cli"))
e.default = "1"

-- optional: specify server id for speedtest
e = t:option(Value, 'server_id', translate('Server ID (optional)'))
e.datatype = "uinteger"
e.placeholder = "e.g. 12345"

-- optional: run single connection mode
e = t:option(Flag, 'single', translate('Single connection mode'))
e.default = 0

e=t:option(Button, "restart", translate("speedtest.net Broadband speed test"))
e.inputtitle=translate("Click to execute")
e.template ='netspeedtest/speedtestwan'

-- display ookla version
e = t:option(DummyValue, '_ookla', translate('Ookla Speedtest-CLI'))
e.rawhtml = true
e.cfgvalue = function()
    return "<span id='ookla_ver' style='margin-left:10px;color:gray'>"..translate('Checking...').."</span>"
end

return o
