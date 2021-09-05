local e = {}
local o = require "luci.dispatcher"
local a = luci.util.execi("/bin/busybox top -bn1 | grep '/usr/sbin/pppd'")
for t in a do
    local a, n, h, s, o, i = t:match("^ *(%d+) +(%d+) +.+options%.pptpd +(%d+) +(%S.-%S)%:(%S.-%S) +.+ +(.+)")
    local t = tonumber(a)
    if t then
        e["%02i.%s" % {t, "online"}] = {
            ['PID'] = a,
            ['PPID'] = n,
            ['SPEED'] = h,
            ['GATEWAY'] = s,
            ['VIP'] = o,
            ['CIP'] = i,
            ['BLACKLIST'] = 0
        }
    end
end
local a = luci.util.execi("sed = /etc/firewall.user | sed 'N;s/\\n/:/'")
for t in a do
    local t, a = t:match("^ *(%d+)%:.+%#%# pptpd%-blacklist%-(.+)")
    local t = tonumber(t)
    if t then
        e["%02i.%s" % {t, "blacklist"}] =
            {
                ['PID'] = "-1",
                ['PPID'] = "-1",
                ['SPEED'] = "-1",
                ['GATEWAY'] = "-",
                ['VIP'] = "-",
                ['CIP'] = a,
                ['BLACKLIST'] = 1
            }
    end
end
f = SimpleForm("processes", translate("PPTP VPN Server"))
f.reset = false
f.submit = false
f.description = translate("Simple, quick and convenient PPTP VPN, universal across the platform")
t = f:section(Table, e, translate("Online Users"))
t:option(DummyValue, "GATEWAY", translate("Server IP"))
t:option(DummyValue, "VIP", translate("Client IP"))
t:option(DummyValue, "CIP", translate("IP address"))
blacklist = t:option(Button, "_blacklist", translate("Blacklist"))
function blacklist.render(e, t, a)
    if e.map:get(t, "BLACKLIST") == 0 then
        e.title = translate("Add to Blacklist")
        e.inputstyle = "remove"
    else
        e.title = translate("Remove from Blacklist")
        e.inputstyle = "apply"
    end
    Button.render(e, t, a)
end
function blacklist.write(t, a)
    local e = t.map:get(a, "CIP")
    if t.map:get(a, "BLACKLIST") == 0 then
        luci.util.execi(
            "echo 'iptables -A input_rule -s %s -p tcp --dport 1723 -j DROP ## pptpd-blacklist-%s' >> /etc/firewall.user" %
                {e, e})
        luci.util.execi(
            "iptables -A input_rule -s %s -p tcp --dport 1723 -j DROP" % {e})
        null, t.tag_error[a] = luci.sys.process.signal(t.map:get(a, "PID"), 9)
    else
        luci.util.execi(
            "sed -i -e '/## pptpd-blacklist-%s/d' /etc/firewall.user" % {e})
        luci.util.execi(
            "iptables -D input_rule -s %s -p tcp --dport 1723 -j DROP" % {e})
    end
    luci.http.redirect(o.build_url("admin/vpn/pptpd/online"))
end
kill = t:option(Button, "_kill", translate("Forced offline"))
kill.inputstyle = "reset"
function kill.write(e, t)
    null, e.tag_error[t] = luci.sys.process.signal(e.map:get(t, "PID"), 9)
    luci.http.redirect(o.build_url("admin/vpn/pptpd/online"))
end
return f
