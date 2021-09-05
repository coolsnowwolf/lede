local e = {}
local o = require "luci.dispatcher"
local a = luci.util.execi(
              "/bin/busybox top -bn1 | grep 'pppd plugin rp-pppoe.so' | grep -v 'grep'")
for t in a do
    local a, n, h, s, o = t:match(
                              "^ *(%d+) +(%d+) +.+rp_pppoe_sess 1:+([A-Fa-f0-9]+:[A-Fa-f0-9]+:[A-Fa-f0-9]+:[A-Fa-f0-9]+:[A-Fa-f0-9]+:[A-Fa-f0-9]+[A-Fa-f0-9]) +.+options +(%S.-%S)%:(%S.-%S) ")
    local t = tonumber(a)
    if t then
        e["%02i.%s" % {t, "online"}] = {
            ['PID'] = a,
            ['PPID'] = n,
            ['MAC'] = h,
            ['GATEWAY'] = s,
            ['CIP'] = o,
            ['BLACKLIST'] = 0
        }
    end
end
f = SimpleForm("processes", translate("PPPoE Server"))
f.reset = false
f.submit = false
f.description = translate(
                    "The PPPoE server is a broadband access authentication server that prevents ARP spoofing.")
t = f:section(Table, e, translate("Online Users"))
t:option(DummyValue, "GATEWAY", translate("Server IP"))
t:option(DummyValue, "CIP", translate("IP address"))
t:option(DummyValue, "MAC", translate("MAC"))

kill = t:option(Button, "_kill", translate("Forced offline"))
kill.inputstyle = "reset"
function kill.write(e, t)
    null, e.tag_error[t] = luci.sys.process.signal(e.map:get(t, "PID"), 9)
    luci.http.redirect(o.build_url("admin/services/pppoe-server/online"))
end
return f
