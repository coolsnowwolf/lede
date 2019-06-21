#!/usr/bin/lua
-- Author: Anton Chen <contact@antonchen.com>
-- Create Date: 2019-05-06 10:32:52
-- Last Modified: 2019-05-30 21:04:33
-- Description:
local uci = require("uci")
local sys = require "luci.sys"
local json = require "luci.json"

local uci_instance = uci.cursor()

-- HTTP GET
function httpget (url)
    local source = stream and io.popen or luci.util.exec
    return source("curl -s -k --connect-timeout 5 -m 5 -A \"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/63.0.3239.132 Safari/537.36\" %s" % {url})
end

-- Base64 解码
function base64decode(data)
    local b='ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'
    data = string.gsub(data, '[^'..b..'=]', '')
    return (data:gsub('.', function(x)
        if (x == '=') then return '' end
        local r,f='',(b:find(x)-1)
        for i=6,1,-1 do r=r..(f%2^i-f%2^(i-1)>0 and '1' or '0') end
        return r;
    end):gsub('%d%d%d?%d?%d?%d?%d?%d?', function(x)
    if (#x ~= 8) then return '' end
    local c=0
    for i=1,8 do c=c+(x:sub(i,i)=='1' and 2^(8-i) or 0) end
    return string.char(c)
end))
end

-- URL 解码
local function urlDecode(str)
    return string.gsub(str, '%%(%x%x)', function(h) return string.char(tonumber(h, 16)) end)
end

-- 判定 table 是否存在某个 key
function has_key(table, key)
    return table[key] ~= nil
end

-- table 转 clash 配置并排序，类 json 格式
function table_to_string(tbl)
    sort_tpl = {"name", "type", "server", "port", "uuid", "alterId", "cipher", "network", "tls", "skip-cert-verify", "plugin", "plugin-opts", "ws-path", "ws-headers", "mode", "tls", "host", "path", "headers", "custom"}

    local result = "- { "
    for _, k in pairs(sort_tpl) do
        if has_key(tbl, k) then
            if k == "name" then
                result = result..string.format("%s: \"%s\"", k, tostring(tbl[k]))
            else
                result = result..string.format("%s: %s", k, tostring(tbl[k]))
            end
            result = result..", "
            tbl[k] = nil
        end
    end

    for k, v in pairs(tbl) do
        if type(v) == "table" then
            result = result..table_to_string(v)
        else
            result = result..string.format("%s: %s", k, tostring(v))
        end
        result = result..", "
    end
    if result ~= "- { " then
        result = result:sub(1, result:len()-2)
    end
    return result.." }"
end

-- 判断节点名称是否包含关键字
function has_rules (name, rules)
    local rules = rules:split(",")
    for _, keyword in pairs(rules) do
        if string.find(name, keyword) ~= nil then
            return true
        end
    end
    return false
end

-- 获取订阅并解析
function subscription_get (subscription_url)
    local result = {}
    local base64_data = httpget(subscription_url)
    local all_link = base64decode(base64_data)
    for link in all_link:gmatch("([^\n]*)\n?") do
        if link ~= "" then
            local link_type = link:split("://")[1]
            local link_uri = link:split("://")[2]

            if link_type == "vmess" then
                local vmess_map = {
                    ps = "name",
                    add = "server",
                    port = "port",
                    id = "uuid",
                    security = "cipher",
                    net = "network",
                    tls = "tls",
                    path = "ws-path",
                    aid = "alterId"
                }

                local clash_vmess_tpl = {
                    name = "local",
                    type = "vmess",
                    server = "127.0.0.1",
                    port = 1080,
                    uuid = "",
                    alterId = 0,
                    cipher = "auto",
                    tls = false,
                    ["skip-cert-verify"] = true
                }

                local link_dict = json.decode(base64decode(link_uri))
                if link_dict["net"] == "tcp" or link_dict["net"] == "ws" then
                    for key, value in pairs(vmess_map) do
                        if link_dict[key] == "" then
                        elseif key == "net" then
                            if link_dict[key] ~= "tcp" then
                                clash_vmess_tpl[value] = link_dict[key]
                            end
                        elseif key == "ps" then
                            local link_name = link_dict[key]:gsub("%c","")
                            clash_vmess_tpl[value] = link_name
                        elseif key == "tls" then
                            clash_vmess_tpl[value] = true
                        elseif has_key(link_dict, key) then
                            if tonumber(link_dict[key]) == nil then
                                clash_vmess_tpl[value] = link_dict[key]
                            else
                                clash_vmess_tpl[value] = tonumber(link_dict[key])
                            end
                        end
                    end
                    table.insert(result, clash_vmess_tpl)
                end
            elseif link_type == "ss" then
                local link_info = base64decode(link_uri:split("#")[1])
                local link_name = urlDecode(link_uri:split("#")[2]):gsub("%c","")

                clash_ss_tpl = {}
                clash_ss_tpl["name"] = link_name
                clash_ss_tpl["type"] = "ss"
                ss_temp_userinfo = link_info:split("@")[1]
                ss_temp_host_port = link_info:split("@")[2]
                clash_ss_tpl["server"] = ss_temp_host_port:split(":")[1]
                clash_ss_tpl["port"] = ss_temp_host_port:split(":")[2]
                clash_ss_tpl["cipher"] = ss_temp_userinfo:split(":")[1]
                clash_ss_tpl["password"] = ss_temp_userinfo:split(":")[2]

                table.insert(result, clash_ss_tpl)
            end
        end
    end
    return result
end


local proxy_output = "/etc/clash/subscription-proxy"
local group_output = "/etc/clash/subscription-group"
if tonumber(uci_instance.get("clash", "@general[0]", "enabled")) == 1 and tonumber(uci_instance.get("clash", "@subscription[0]", "enabled")) == 1 then
    local group_list = {}

    local subscription_url = uci_instance.get("clash", "@subscription[0]", "url")
    local group_pattern = uci_instance.get("clash", "@subscription[0]", "pattern")
    local include_rules = uci_instance.get("clash", "@subscription[0]", "include")
    local exclude_rules = uci_instance.get("clash", "@subscription[0]", "exclude")
    local node_list = subscription_get(subscription_url)

    -- 为空时退出
    if table.getn(node_list) == 0 then
        return
    end

    local proxy_file = io.open(proxy_output, "w+")
    for _, link in pairs(node_list) do
        local name = link["name"]

        -- 包含
        local select_node = true
        if include_rules ~= nil then
            select_node = has_rules(name, include_rules)
        end

        -- 排除
        if exclude_rules ~= nil then
            if has_rules(name, exclude_rules) then
                select_node = false
            end
        end

        if name:find("流量") then
            print("Subscription: "..name)
            select_node = false
        end

        if select_node then
            -- 节点分组
            if group_pattern ~= nil then
                local region = (string.gsub(link["name"], group_pattern, "%1"))

                if has_key(group_list, region) then
                    table.insert(group_list[region], name)
                else
                    group_list[region] = {}
                    table.insert(group_list[region], name)
                end
            end

            -- 节点
            proxy_file:write(table_to_string(link).."\n")
        end
    end
    proxy_file:close()

    local group_file = io.open(group_output, "w+")
    if group_pattern ~= nil then
        table.sort(group_list)
        for key, value in pairs(group_list) do
            local group_one = "- { name: \""..key.."\", type: url-test, proxies: "..json.encode(value)..", url: \"http://www.gstatic.com/generate_204\", interval: 300 }\n"
            group_file:write(group_one)
        end
    end
    group_file:close()
end
