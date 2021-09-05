local e = {}
local log = luci.util.exec('cat /var/etc/xl2tpd/xl2tpd.log 2>/dev/null')
local a = luci.util.execi("/bin/busybox top -bn1 | grep -v 'grep' | grep '/usr/sbin/pppd' | grep '/var/etc/xl2tpd/'")
for t in a do
    local a, n, s, o = t:match("^ *(%d+) +(%d+) +.+nodetach +(%S.-%S)%:(%S.-%S) +.+")
    local t = tonumber(a)
    if t then
        e["%02i.%s" % {t, "online"}] = {
            ['PID'] = a,
            ['PPID'] = n,
            ['GATEWAY'] = s,
            ['VIP'] = o,
            ['CIP'] = log:match("Call established with (%S-), PID: " .. t)
        }
    end
end

f = SimpleForm("processes")
f.reset = false
f.submit = false

t = f:section(Table, e, "L2TP " .. translate("Online Users"))

o = t:option(DummyValue, "GATEWAY", translate("Server IP"))
o = t:option(DummyValue, "VIP", translate("Client IP"))
o = t:option(DummyValue, "CIP", translate("IP address"))

kill = t:option(Button, "_kill", translate("Forced offline"))
kill.inputstyle = "remove"
function kill.write(e, t)
    null, e.tag_error[t] = luci.sys.process.signal(e.map:get(t, "PID"), 9)
    luci.http.redirect(luci.dispatcher.build_url("admin/vpn/ipsec-server/online"))
end

return f
