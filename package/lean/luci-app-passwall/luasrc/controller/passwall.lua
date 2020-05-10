-- Copyright 2018-2020 Lienol <lawlienol@gmail.com>
module("luci.controller.passwall", package.seeall)
local appname = "passwall"
local http = require "luci.http"
local passwall = require "luci.model.cbi.passwall.api.passwall"
local kcptun = require "luci.model.cbi.passwall.api.kcptun"
local brook = require "luci.model.cbi.passwall.api.brook"
local v2ray = require "luci.model.cbi.passwall.api.v2ray"

function index()
    if not nixio.fs.access("/etc/config/passwall") then return end
    entry({"admin", "vpn"}, firstchild(), "VPN", 45).dependent = false
    entry({"admin", "vpn", "passwall", "reset_config"}, call("reset_config")).leaf =
        true
    entry({"admin", "vpn", "passwall", "show"}, call("show_menu")).leaf = true
    entry({"admin", "vpn", "passwall", "hide"}, call("hide_menu")).leaf = true
    if nixio.fs.access("/etc/config/passwall") and
        nixio.fs.access("/etc/config/passwall_show") then
        entry({"admin", "vpn", "passwall"},
              alias("admin", "vpn", "passwall", "settings"), _("Pass Wall"), 1).dependent =
            true
    end
    entry({"admin", "vpn", "passwall", "settings"}, cbi("passwall/global"),
          _("Basic Settings"), 1).dependent = true
    entry({"admin", "vpn", "passwall", "node_list"}, cbi("passwall/node_list"),
          _("Node List"), 2).dependent = true
    entry({"admin", "vpn", "passwall", "auto_switch"},
          cbi("passwall/auto_switch"), _("Auto Switch"), 3).leaf = true
    entry({"admin", "vpn", "passwall", "other"},
          cbi("passwall/other", {autoapply = true}), _("Other Settings"), 93).leaf =
        true
    if nixio.fs.access("/usr/sbin/haproxy") then
        entry({"admin", "vpn", "passwall", "haproxy"},
              cbi("passwall/haproxy"), _("Load Balancing"), 94).leaf = true
    end
    entry({"admin", "vpn", "passwall", "node_subscribe"},
          cbi("passwall/node_subscribe"), _("Node Subscribe"), 95).dependent =
        true
    entry({"admin", "vpn", "passwall", "rule"}, cbi("passwall/rule"),
          _("Rule Update"), 96).leaf = true
    entry({"admin", "vpn", "passwall", "acl"}, cbi("passwall/acl"),
          _("Access control"), 97).leaf = true
    entry({"admin", "vpn", "passwall", "log"}, form("passwall/log"),
          _("Watch Logs"), 99).leaf = true
    entry({"admin", "vpn", "passwall", "node_config"},
          cbi("passwall/node_config")).leaf = true

    entry({"admin", "vpn", "passwall", "link_add_node"}, call("link_add_node")).leaf =
        true
    entry({"admin", "vpn", "passwall", "get_log"}, call("get_log")).leaf = true
    entry({"admin", "vpn", "passwall", "clear_log"}, call("clear_log")).leaf =
        true
    entry({"admin", "vpn", "passwall", "status"}, call("status")).leaf = true
    entry({"admin", "vpn", "passwall", "connect_status"}, call("connect_status")).leaf =
        true
    entry({"admin", "vpn", "passwall", "check_port"}, call("check_port")).leaf =
        true
    entry({"admin", "vpn", "passwall", "ping_node"}, call("ping_node")).leaf =
        true
    entry({"admin", "vpn", "passwall", "set_node"}, call("set_node")).leaf =
        true
    entry({"admin", "vpn", "passwall", "copy_node"}, call("copy_node")).leaf =
        true
    entry({"admin", "vpn", "passwall", "update_rules"}, call("update_rules")).leaf =
        true
    entry({"admin", "vpn", "passwall", "luci_check"}, call("luci_check")).leaf =
        true
    entry({"admin", "vpn", "passwall", "luci_update"}, call("luci_update")).leaf =
        true
    entry({"admin", "vpn", "passwall", "kcptun_check"}, call("kcptun_check")).leaf =
        true
    entry({"admin", "vpn", "passwall", "kcptun_update"}, call("kcptun_update")).leaf =
        true
    entry({"admin", "vpn", "passwall", "brook_check"}, call("brook_check")).leaf =
        true
    entry({"admin", "vpn", "passwall", "brook_update"}, call("brook_update")).leaf =
        true
    entry({"admin", "vpn", "passwall", "v2ray_check"}, call("v2ray_check")).leaf =
        true
    entry({"admin", "vpn", "passwall", "v2ray_update"}, call("v2ray_update")).leaf =
        true
end

local function http_write_json(content)
    http.prepare_content("application/json")
    http.write_json(content or {code = 1})
end

function reset_config()
    luci.sys.call(
        '[ -f "/usr/share/passwall/config.default" ] && cp -f /usr/share/passwall/config.default /etc/config/passwall && /etc/init.d/passwall reload')
    luci.http.redirect(luci.dispatcher.build_url("admin", "vpn", "passwall"))
end

function show_menu()
    luci.sys.call("touch /etc/config/passwall_show")
    luci.http.redirect(luci.dispatcher.build_url("admin", "vpn", "passwall"))
end

function hide_menu()
    luci.sys.call("rm -rf /etc/config/passwall_show")
    luci.http.redirect(luci.dispatcher.build_url("admin", "status", "overview"))
end

function link_add_node()
    local link = luci.http.formvalue("link")
    luci.sys.call('rm -f /tmp/links.conf && echo "' .. link ..
                      '" >> /tmp/links.conf')
    luci.sys.call("lua /usr/share/passwall/subscribe.lua add log")
end

function get_log()
    -- luci.sys.exec("[ -f /var/log/passwall.log ] && sed '1!G;h;$!d' /var/log/passwall.log > /var/log/passwall_show.log")
    luci.http.write(luci.sys.exec(
                        "[ -f '/var/log/passwall.log' ] && cat /var/log/passwall.log"))
end

function clear_log() luci.sys.call("echo '' > /var/log/passwall.log") end

function status()
    -- local dns_mode = luci.sys.exec("echo -n $(uci -q get " .. appname .. ".@global[0].dns_mode)")
    local e = {}
    e.dns_mode_status = luci.sys.call("netstat -apn | grep 7913 >/dev/null") ==
                            0
    e.haproxy_status = luci.sys.call(string.format(
                                         "ps -w | grep -v grep | grep '%s/bin/' | grep haproxy >/dev/null",
                                         appname)) == 0
    local tcp_node_num = luci.sys.exec(
                             "echo -n $(uci -q get %s.@global_other[0].tcp_node_num)" %
                                 appname)
    for i = 1, tcp_node_num, 1 do
        e["kcptun_tcp_node%s_status" % i] =
            luci.sys.call(string.format(
                              "ps -w | grep -v grep | grep '%s/bin/' | grep 'kcptun_tcp_%s' >/dev/null",
                              appname, i)) == 0
        e["tcp_node%s_status" % i] = luci.sys.call(
                                         string.format(
                                             "ps -w | grep -v grep | grep -v kcptun | grep '%s/bin/' | grep -i -E 'TCP_%s' >/dev/null",
                                             appname, i)) == 0
    end

    local udp_node_num = luci.sys.exec(
                             "echo -n $(uci -q get %s.@global_other[0].udp_node_num)" %
                                 appname)
    for i = 1, udp_node_num, 1 do
        e["udp_node%s_status" % i] = luci.sys.call(
                                         string.format(
                                             "ps -w | grep -v grep | grep '%s/bin/' | grep -i -E 'UDP_%s' >/dev/null",
                                             appname, i)) == 0
    end

    local socks_node_num = luci.sys.exec(
                                "echo -n $(uci -q get %s.@global_other[0].socks_node_num)" %
                                    appname)
    for i = 1, socks_node_num, 1 do
        e["kcptun_socks_node%s_status" % i] =
            luci.sys.call(string.format(
                              "ps -w | grep -v grep | grep '%s/bin/' | grep 'kcptun_socks_%s' >/dev/null",
                              appname, i)) == 0
        e["socks_node%s_status" % i] = luci.sys.call(
                                            string.format(
                                                "ps -w | grep -v grep | grep -v kcptun | grep '%s/bin/' | grep -i 'SOCKS_%s' >/dev/null",
                                                appname, i, i)) == 0
    end
    luci.http.prepare_content("application/json")
    luci.http.write_json(e)
end

function connect_status()
    local os = require "os"
    local e = {}
    local start_time = os.time()
    if luci.http.formvalue("type") == "google" then
        e.status = luci.sys.call(
                       "echo $(/usr/share/passwall/test.sh test_url 'www.google.com/generate_204') | grep 200 >/dev/null") ==
                       0
    else
        e.status = luci.sys.call(
                       "echo $(/usr/share/passwall/test.sh test_url 'www.baidu.com') | grep 200 >/dev/null") ==
                       0
    end
    local use_time = os.difftime(os.time(), start_time)
    e.use_time = use_time
    luci.http.prepare_content("application/json")
    luci.http.write_json(e)
end

function ping_node()
    local index = luci.http.formvalue("index")
    local address = luci.http.formvalue("address")
    local port = luci.http.formvalue("port")
    local e = {}
    e.index = index
    if luci.sys.exec("echo -n $(uci -q get %s.@global_other[0].use_tcping)" % appname) == "1" and luci.sys.exec("echo -n $(command -v tcping)") ~= "" then
        e.ping = luci.sys.exec(string.format("echo -n $(tcping -q -c 1 -i 1 -p %s %s 2>&1 | grep -o 'time=[0-9]*' | awk -F '=' '{print$2}') 2>/dev/null", port, address))
        luci.sys.call(string.format("ps -w | grep 'tcping -q -c 1 -i 1 -p %s %s' | grep -v grep | awk '{print $1}' | xargs kill -9 2>/dev/null", port, address))
    end
    if e.ping == nil or tonumber(e.ping) == 0 then
        e.ping = luci.sys.exec("echo -n $(ping -c 1 -W 1 %q 2>&1 | grep -o 'time=[0-9]*' | awk -F '=' '{print$2}') 2>/dev/null" % address)
    end
    luci.http.prepare_content("application/json")
    luci.http.write_json(e)
end

function set_node()
    local protocol = luci.http.formvalue("protocol")
    local number = luci.http.formvalue("number")
    local section = luci.http.formvalue("section")
    luci.sys.call("uci set passwall.@global[0]." .. protocol .. "_node" ..
                      number .. "=" .. section ..
                      " && uci commit passwall && /etc/init.d/passwall restart > /dev/null 2>&1 &")
    luci.http.redirect(luci.dispatcher.build_url("admin", "vpn", "passwall",
                                                 "log"))
end

function copy_node()
    local e = {}
    local section = luci.http.formvalue("section")
    luci.http.prepare_content("application/json")
    luci.http.write_json(e)
end

function check_port()
    local s
    local node_name = ""
    local uci = luci.model.uci.cursor()

    local retstring = "<br />"
    -- retstring = retstring .. "<font color='red'>暂时不支持UDP检测</font><br />"

    retstring = retstring ..
                    "<font color='green'>检测端口可用性</font><br />"
    uci:foreach("passwall", "nodes", function(s)
        local ret = ""
        local tcp_socket
        if (s.use_kcp and s.use_kcp == "1" and s.kcp_port) or
            (s.v2ray_transport and s.v2ray_transport == "mkcp" and s.port) then
        else
            local type = s.type
            if type and type ~= "V2ray_balancing" and type ~= "V2ray_shunt" and
                s.address and s.port and s.remarks then
                node_name = "%s：[%s] %s:%s" %
                                {s.type, s.remarks, s.address, s.port}
                tcp_socket = nixio.socket("inet", "stream")
                tcp_socket:setopt("socket", "rcvtimeo", 3)
                tcp_socket:setopt("socket", "sndtimeo", 3)
                ret = tcp_socket:connect(s.address, s.port)
                if tostring(ret) == "true" then
                    retstring =
                        retstring .. "<font color='green'>" .. node_name ..
                            "   OK.</font><br />"
                else
                    retstring =
                        retstring .. "<font color='red'>" .. node_name ..
                            "   Error.</font><br />"
                end
                ret = ""
            end
        end
        if tcp_socket then tcp_socket:close() end
    end)
    luci.http.prepare_content("application/json")
    luci.http.write_json({ret = retstring})
end

function update_rules()
    local update = luci.http.formvalue("update")
    luci.sys.call("lua /usr/share/passwall/rule_update.lua log '" .. update ..
                      "' > /dev/null 2>&1 &")
end

function luci_check()
    local json = passwall.to_check("")
    http_write_json(json)
end

function luci_update()
    local json = passwall.update_luci(http.formvalue("url"),
                                      http.formvalue("save"))
    http_write_json(json)
end

function kcptun_check()
    local json = kcptun.to_check("")
    http_write_json(json)
end

function kcptun_update()
    local json = nil
    local task = http.formvalue("task")
    if task == "extract" then
        json = kcptun.to_extract(http.formvalue("file"),
                                 http.formvalue("subfix"))
    elseif task == "move" then
        json = kcptun.to_move(http.formvalue("file"))
    else
        json = kcptun.to_download(http.formvalue("url"))
    end

    http_write_json(json)
end

function brook_check()
    local json = brook.to_check("")
    http_write_json(json)
end

function brook_update()
    local json = nil
    local task = http.formvalue("task")
    if task == "move" then
        json = brook.to_move(http.formvalue("file"))
    else
        json = brook.to_download(http.formvalue("url"))
    end

    http_write_json(json)
end

function v2ray_check()
    local json = v2ray.to_check("")
    http_write_json(json)
end

function v2ray_update()
    local json = nil
    local task = http.formvalue("task")
    if task == "extract" then
        json =
            v2ray.to_extract(http.formvalue("file"), http.formvalue("subfix"))
    elseif task == "move" then
        json = v2ray.to_move(http.formvalue("file"))
    else
        json = v2ray.to_download(http.formvalue("url"))
    end

    http_write_json(json)
end
