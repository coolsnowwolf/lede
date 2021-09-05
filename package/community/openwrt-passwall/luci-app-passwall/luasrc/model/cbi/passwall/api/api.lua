module("luci.model.cbi.passwall.api.api", package.seeall)
fs = require "nixio.fs"
sys = require "luci.sys"
uci = require"luci.model.uci".cursor()
util = require "luci.util"
datatypes = require "luci.cbi.datatypes"
jsonc = require "luci.jsonc"
i18n = require "luci.i18n"

appname = "passwall"
curl = "/usr/bin/curl"
curl_args = {"-skfL", "--connect-timeout 3", "--retry 3", "-m 60"}
command_timeout = 300
LEDE_BOARD = nil
DISTRIB_TARGET = nil

function url(...)
    local url = string.format("admin/services/%s", appname)
    local args = { ... }
    for i, v in pairs(args) do
        if v ~= "" then
            url = url .. "/" .. v
        end
    end
    return require "luci.dispatcher".build_url(url)
end

function trim(s)
    return (s:gsub("^%s*(.-)%s*$", "%1"))
end

function is_exist(table, value)
    for index, k in ipairs(table) do
        if k == value then
            return true
        end
    end
    return false
end

function repeat_exist(table, value)
    local count = 0
    for index, k in ipairs(table) do
        if k:find("-") and k == value then
            count = count + 1
        end
    end
    if count > 1 then
        return true
    end
    return false
end

function get_args(arg)
    local var = {}
    for i, arg_k in pairs(arg) do
        if i > 0 then
            local v = arg[i + 1]
            if v then
                if repeat_exist(arg, v) == false then
                    var[arg_k] = v
                end
            end
        end
    end
    return var
end

function strToTable(str)
    if str == nil or type(str) ~= "string" then
        return {}
    end
    
    return loadstring("return " .. str)()
end

function is_normal_node(e)
    if e and e.type and e.protocol and (e.protocol == "_balancing" or e.protocol == "_shunt") then
        return false
    end
    return true
end

function is_special_node(e)
    return is_normal_node(e) == false
end

function is_ip(val)
    return datatypes.ipaddr(val)
end

function is_ipv6(val)
    local str = val
    local address = val:match('%[(.*)%]')
    if address then
        str = address
    end
    if datatypes.ip6addr(str) then
        return true
    end
    return false
end

function is_ipv6addrport(val)
    if is_ipv6(val) then
        local address, port = val:match('%[(.*)%]:([^:]+)$')
        if port then
            return datatypes.port(port)
        end
    end
    return false
end

function get_ip_type(val)
    if is_ipv6(val) then
        return "6"
    elseif datatypes.ip4addr(val) then
        return "4"
    end
    return ""
end

function is_mac(val)
    return datatypes.macaddr(val)
end

function ip_or_mac(val)
    if val then
        if get_ip_type(val) == "4" then
            return "ip"
        end
        if is_mac(val) then
            return "mac"
        end
    end
    return ""
end

function get_valid_nodes()
    local nodes_ping = uci_get_type("global_other", "nodes_ping") or ""
    local nodes = {}
    uci:foreach(appname, "nodes", function(e)
        e.id = e[".name"]
        if e.type and e.remarks then
            if e.protocol and (e.protocol == "_balancing" or e.protocol == "_shunt") then
                e["remark"] = "%s：[%s] " % {i18n.translatef(e.type .. e.protocol), e.remarks}
                e["node_type"] = "special"
                nodes[#nodes + 1] = e
            end
            if e.port and e.address then
                local address = e.address
                if datatypes.ipaddr(address) or datatypes.hostname(address) then
                    local type2 = e.type
                    local address2 = address
                    if (type2 == "V2ray" or type2 == "Xray") and e.protocol then
                        local protocol = e.protocol
                        if protocol == "vmess" then
                            protocol = "VMess"
                        elseif protocol == "vless" then
                            protocol = "VLESS"
                        else
                            protocol = protocol:gsub("^%l",string.upper)
                        end
                        type2 = type2 .. " " .. protocol
                    end
                    if datatypes.ip6addr(address) then address2 = "[" .. address .. "]" end
                    e["remark"] = "%s：[%s]" % {type2, e.remarks}
                    if nodes_ping:find("info") then
                        e["remark"] = "%s：[%s] %s:%s" % {type2, e.remarks, address2, e.port}
                    end
                    if e.use_kcp and e.use_kcp == "1" then
                        e["remark"] = "%s+%s：[%s]" % {type2, "Kcptun", e.remarks}
                        if nodes_ping:find("info") then
                            e["remark"] = "%s+%s：[%s] %s" % {type2, "Kcptun", e.remarks, address2}
                        end
                    end
                    e.node_type = "normal"
                    nodes[#nodes + 1] = e
                end
            end
        end
    end)
    return nodes
end

function get_full_node_remarks(n)
    local remarks = ""
    if n then
        if n.protocol and (n.protocol == "_balancing" or n.protocol == "_shunt") then
            remarks = "%s：[%s] " % {i18n.translatef(n.type .. n.protocol), n.remarks}
        else
            local type2 = n.type
            if (n.type == "V2ray" or n.type == "Xray") and n.protocol then
                local protocol = n.protocol
                if protocol == "vmess" then
                    protocol = "VMess"
                elseif protocol == "vless" then
                    protocol = "VLESS"
                else
                    protocol = protocol:gsub("^%l",string.upper)
                end
                type2 = type2 .. " " .. protocol
            end
            if n.use_kcp and n.use_kcp == "1" then
                remarks = "%s+%s：[%s] %s" % {type2, "Kcptun", n.remarks, n.address}
            else
                remarks = "%s：[%s] %s:%s" % {type2, n.remarks, n.address, n.port}
            end
        end
    end
    return remarks
end

function gen_uuid(format)
    local uuid = sys.exec("echo -n $(cat /proc/sys/kernel/random/uuid)")
    if format == nil then
        uuid = string.gsub(uuid, "-", "")
    end
    return uuid
end

function uci_get_type(type, config, default)
    local value = uci:get_first(appname, type, config, default) or sys.exec("echo -n $(uci -q get " .. appname .. ".@" .. type .."[0]." .. config .. ")")
    if (value == nil or value == "") and (default and default ~= "") then
        value = default
    end
    return value
end

function uci_get_type_id(id, config, default)
    local value = uci:get(appname, id, config, default) or sys.exec("echo -n $(uci -q get " .. appname .. "." .. id .. "." .. config .. ")")
    if (value == nil or value == "") and (default and default ~= "") then
        value = default
    end
    return value
end

function chmod_755(file)
    if file and file ~= "" then
        if not fs.access(file, "rwx", "rx", "rx") then
            fs.chmod(file, 755)
        end
    end
end

function get_customed_path(e)
    return uci_get_type("global_app", e .. "_file")
end

function is_finded(e)
    return luci.sys.exec('type -t -p "/bin/%s" -p "%s" "%s"' % {e, get_customed_path(e), e}) ~= "" and true or false
end


function clone(org)
    local function copy(org, res)
        for k,v in pairs(org) do
            if type(v) ~= "table" then
                res[k] = v;
            else
                res[k] = {};
                copy(v, res[k])
            end
        end
    end
 
    local res = {}
    copy(org, res)
    return res
end

function get_v2ray_path()
    local path = uci_get_type("global_app", "v2ray_file")
    return path
end

function get_v2ray_version(file)
    if file == nil then file = get_v2ray_path() end
    chmod_755(file)
    if fs.access(file) then
        if file == get_v2ray_path() then
            local md5 = sys.exec("echo -n $(md5sum " .. file .. " | awk '{print $1}')")
            if fs.access("/tmp/psw_" .. md5) then
                return sys.exec("echo -n $(cat /tmp/psw_%s)" % md5)
            else
                local version = sys.exec("echo -n $(%s -version | awk '{print $2}' | sed -n 1P)" % file)
                sys.call("echo '" .. version .. "' > " .. "/tmp/psw_" .. md5)
                return version
            end
        else
            return sys.exec("echo -n $(%s -version | awk '{print $2}' | sed -n 1P)" % file)
        end
    end
    return ""
end

function get_xray_path()
    local path = uci_get_type("global_app", "xray_file")
    return path
end

function get_xray_version(file)
    if file == nil then file = get_xray_path() end
    chmod_755(file)
    if fs.access(file) then
        if file == get_xray_path() then
            local md5 = sys.exec("echo -n $(md5sum " .. file .. " | awk '{print $1}')")
            if fs.access("/tmp/psw_" .. md5) then
                return sys.exec("echo -n $(cat /tmp/psw_%s)" % md5)
            else
                local version = sys.exec("echo -n $(%s -version | awk '{print $2}' | sed -n 1P)" % file)
                sys.call("echo '" .. version .. "' > " .. "/tmp/psw_" .. md5)
                return version
            end
        else
            return sys.exec("echo -n $(%s -version | awk '{print $2}' | sed -n 1P)" % file)
        end
    end
    return ""
end

function get_trojan_go_path()
    local path = uci_get_type("global_app", "trojan_go_file")
    return path
end

function get_trojan_go_version(file)
    if file == nil then file = get_trojan_go_path() end
    chmod_755(file)
    if fs.access(file) then
        if file == get_trojan_go_path() then
            local md5 = sys.exec("echo -n $(md5sum " .. file .. " | awk '{print $1}')")
            if fs.access("/tmp/psw_" .. md5) then
                return sys.exec("echo -n $(cat /tmp/psw_%s)" % md5)
            else
                local version = sys.exec("echo -n $(%s -version | awk '{print $2}' | sed -n 1P)" % file)
                sys.call("echo '" .. version .. "' > " .. "/tmp/psw_" .. md5)
                return version
            end
        else
            return sys.exec("echo -n $(%s -version | awk '{print $2}' | sed -n 1P)" % file)
        end
    end
    return ""
end

function get_kcptun_path()
    local path = uci_get_type("global_app", "kcptun_client_file")
    return path
end

function get_kcptun_version(file)
    if file == nil then file = get_kcptun_path() end
    chmod_755(file)
    if fs.access(file) then
        if file == get_kcptun_path() then
            local md5 = sys.exec("echo -n $(md5sum " .. file .. " | awk '{print $1}')")
            if fs.access("/tmp/psw_" .. md5) then
                return sys.exec("echo -n $(cat /tmp/psw_%s)" % md5)
            else
                local version = sys.exec("echo -n $(%s -v | awk '{print $3}')" % file)
                sys.call("echo '" .. version .. "' > " .. "/tmp/psw_" .. md5)
                return version
            end
        else
            return sys.exec("echo -n $(%s -v | awk '{print $3}')" % file)
        end
    end
    return ""
end

function get_brook_path()
    local path = uci_get_type("global_app", "brook_file")
    return path
end

function get_brook_version(file)
    if file == nil then file = get_brook_path() end
    chmod_755(file)
    if fs.access(file) then
        if file == get_brook_path() then
            local md5 = sys.exec("echo -n $(md5sum " .. file .. " | awk '{print $1}')")
            if fs.access("/tmp/psw_" .. md5) then
                return sys.exec("echo -n $(cat /tmp/psw_%s)" % md5)
            else
                local version = sys.exec("echo -n $(%s -v | awk '{print $3}')" % file)
                sys.call("echo '" .. version .. "' > " .. "/tmp/psw_" .. md5)
                return version
            end
        else
            return sys.exec("echo -n $(%s -v | awk '{print $3}')" % file)
        end
    end
    return ""
end

function get_hysteria_path()
    local path = uci_get_type("global_app", "hysteria_file")
    return path
end

function get_hysteria_version(file)
    if file == nil then file = get_hysteria_path() end
    chmod_755(file)
    if fs.access(file) then
        if file == get_hysteria_path() then
            local md5 = sys.exec("echo -n $(md5sum " .. file .. " | awk '{print $1}')")
            if fs.access("/tmp/psw_" .. md5) then
                return sys.exec("echo -n $(cat /tmp/psw_%s)" % md5)
            else
                local version = sys.exec("echo -n $(%s -v | awk '{print $3}')" % file)
                sys.call("echo '" .. version .. "' > " .. "/tmp/psw_" .. md5)
                return version
            end
        else
            return sys.exec("echo -n $(%s -v | awk '{print $3}')" % file)
        end
    end
    return ""
end

function get_free_space(dir)
    if dir == nil then dir = "/" end
    if sys.call("df -k " .. dir .. " >/dev/null") == 0 then
        return tonumber(sys.exec("echo -n $(df -k " .. dir .. " | awk 'NR>1' | awk '{print $4}')"))
    end
    return 0
end

function get_file_space(file)
    if file == nil then return 0 end
    if fs.access(file) then
        return tonumber(sys.exec("echo -n $(du -k " .. file .. " | awk '{print $1}')"))
    end
    return 0
end

function _unpack(t, i)
    i = i or 1
    if t[i] ~= nil then return t[i], _unpack(t, i + 1) end
end

function exec(cmd, args, writer, timeout)
    local os = require "os"
    local nixio = require "nixio"

    local fdi, fdo = nixio.pipe()
    local pid = nixio.fork()

    if pid > 0 then
        fdo:close()

        if writer or timeout then
            local starttime = os.time()
            while true do
                if timeout and os.difftime(os.time(), starttime) >= timeout then
                    nixio.kill(pid, nixio.const.SIGTERM)
                    return 1
                end

                if writer then
                    local buffer = fdi:read(2048)
                    if buffer and #buffer > 0 then
                        writer(buffer)
                    end
                end

                local wpid, stat, code = nixio.waitpid(pid, "nohang")

                if wpid and stat == "exited" then return code end

                if not writer and timeout then nixio.nanosleep(1) end
            end
        else
            local wpid, stat, code = nixio.waitpid(pid)
            return wpid and stat == "exited" and code
        end
    elseif pid == 0 then
        nixio.dup(fdo, nixio.stdout)
        fdi:close()
        fdo:close()
        nixio.exece(cmd, args, nil)
        nixio.stdout:close()
        os.exit(1)
    end
end

function compare_versions(ver1, comp, ver2)
    local table = table

    if not ver1 then ver1 = "" end
    if not ver2 then ver2 = "" end

    local av1 = util.split(ver1, "[%.%-]", nil, true)
    local av2 = util.split(ver2, "[%.%-]", nil, true)

    local max = table.getn(av1)
    local n2 = table.getn(av2)
    if (max < n2) then max = n2 end

    for i = 1, max, 1 do
        local s1 = tonumber(av1[i] or 0) or 0
        local s2 = tonumber(av2[i] or 0) or 0

        if comp == "~=" and (s1 ~= s2) then return true end
        if (comp == "<" or comp == "<=") and (s1 < s2) then return true end
        if (comp == ">" or comp == ">=") and (s1 > s2) then return true end
        if (s1 ~= s2) then return false end
    end

    return not (comp == "<" or comp == ">")
end

function auto_get_arch()
    local arch = nixio.uname().machine or ""
    if fs.access("/usr/lib/os-release") then
        LEDE_BOARD = sys.exec("echo -n $(grep 'LEDE_BOARD' /usr/lib/os-release | awk -F '[\\042\\047]' '{print $2}')")
    end
    if fs.access("/etc/openwrt_release") then
        DISTRIB_TARGET = sys.exec("echo -n $(grep 'DISTRIB_TARGET' /etc/openwrt_release | awk -F '[\\042\\047]' '{print $2}')")
    end

    if arch == "mips" then
        if LEDE_BOARD and LEDE_BOARD ~= "" then
            if string.match(LEDE_BOARD, "ramips") == "ramips" then
                arch = "ramips"
            else
                arch = sys.exec("echo '" .. LEDE_BOARD .. "' | grep -oE 'ramips|ar71xx'")
            end
        elseif DISTRIB_TARGET and DISTRIB_TARGET ~= "" then
            if string.match(DISTRIB_TARGET, "ramips") == "ramips" then
                arch = "ramips"
            else
                arch = sys.exec("echo '" .. DISTRIB_TARGET .. "' | grep -oE 'ramips|ar71xx'")
            end
        end
    end

    return util.trim(arch)
end

function get_file_info(arch)
    local file_tree = ""
    local sub_version = ""

    if arch == "x86_64" then
        file_tree = "amd64"
    elseif arch == "aarch64" then
        file_tree = "arm64"
    elseif arch == "ramips" then
        file_tree = "mipsle"
    elseif arch == "ar71xx" then
        file_tree = "mips"
    elseif arch:match("^i[%d]86$") then
        file_tree = "386"
    elseif arch:match("^armv[5-8]") then
        file_tree = "arm"
        sub_version = arch:match("[5-8]")
        if LEDE_BOARD and string.match(LEDE_BOARD, "bcm53xx") == "bcm53xx" then
            sub_version = "5"
        elseif DISTRIB_TARGET and string.match(DISTRIB_TARGET, "bcm53xx") ==
            "bcm53xx" then
            sub_version = "5"
        end
        sub_version = "5"
    end

    return file_tree, sub_version
end

function get_api_json(url)
    local jsonc = require "luci.jsonc"
    local json_content = luci.sys.exec(curl .. " " .. _unpack(curl_args) .. " " .. url)
    if json_content == "" then return {} end
    return jsonc.parse(json_content) or {}
end
