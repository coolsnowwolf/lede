#!/usr/bin/lua
------------------------------------------------
-- This file is part of the luci-app-ssr-plus subscribe.lua
-- @author William Chan <root@williamchan.me>
------------------------------------------------
require 'nixio'
require 'luci.util'
require 'luci.jsonc'
require 'luci.sys'

-- these global functions are accessed all the time by the event handler
-- so caching them is worth the effort
local tinsert = table.insert
local ssub, slen, schar, srep, sbyte, sformat, sgsub =
      string.sub, string.len, string.char, string.rep, string.byte, string.format, string.gsub
local cache = {}
local nodeResult = setmetatable({}, { __index = cache })  -- update result
local name = 'shadowsocksr'
local uciType = 'servers'
local ucic = luci.model.uci.cursor()
local proxy = ucic:get_first(name, 'server_subscribe', 'proxy', '0')
local subscribe_url = ucic:get_first(name, 'server_subscribe', 'subscribe_url', {})

local log = function(...)
    print(os.date("%Y-%m-%d %H:%M:%S ") .. table.concat({ ... }, " "))
end
-- 分割字符串
local function split(full, sep)
    full = full:gsub("%z", "")  -- 这里不是很清楚 有时候结尾带个\0
    local off, result = 1, {}
    while true do
        local nEnd = full:find(sep, off)
        if not nEnd then
            local res = ssub(full, off, slen(full))
            if #res > 0 then -- 过滤掉 \0
                tinsert(result, res)
            end
            break
        else
            tinsert(result, ssub(full, off, nEnd - 1))
            off = nEnd + slen(sep)
        end
    end
    return result
end
-- urlencode
local function get_urlencode(c)
    return sformat("%%%02X", sbyte(c))
end

local function urlEncode(szText)
    local str = szText:gsub("([^0-9a-zA-Z ])", get_urlencode)
    str = str:gsub(" ", "+")
    return str
end

local function get_urldecode(h)
	return schar(tonumber(h, 16))
end
local function UrlDecode(szText)
	return szText:gsub("+", " "):gsub("%%(%x%x)", get_urldecode)
end

-- trim
local function trim(text)
    if not text or text == "" then
        return ""
    end
    return (sgsub(text, "^%s*(.-)%s*$", "%1"))
end
-- md5
local function md5(content)
    local stdout = luci.sys.exec('echo \"' .. urlEncode(content) .. '\" | md5sum | cut -d \" \"  -f1')
    -- assert(nixio.errno() == 0)
    return trim(stdout)
end
-- base64
local function base64Decode(text, safe)
    local raw = text
    if not text then return '' end
    text = text:gsub("%z", "")
    if safe then
        text = text:gsub("_", "/")
        text = text:gsub("-", "+")
        local mod4 = #text % 4
        text = text .. string.sub('====', mod4 + 1)
    end
    local result = nixio.bin.b64decode(text)
    if result then
        return result:gsub("%z", "")
    else
        return raw
    end
end
-- 处理数据
local function processData(szType, content)
    local result = {
        auth_enable = '0',
        switch_enable = '1',
        type = szType,
        local_port = 1234,
        timeout = 60, -- 不太确定 好像是死的
        fast_open = 0,
        kcp_enable = 0,
        kcp_port = 0,
        kcp_param = '--nocomp'
    }
    local hash
    if type(content) == 'string' then
        hash = md5(content)
    else
        hash = md5(luci.jsonc.stringify(content))
    end
    result.hashkey = hash
    if szType == 'ssr' then
        local dat = split(content, "/\\?")
        local hostInfo = split(dat[1], ':')
        result.server = hostInfo[1]
        result.server_port = hostInfo[2]
        result.protocol = hostInfo[3]
        result.encrypt_method = hostInfo[4]
        result.obfs = hostInfo[5]
        result.password = base64Decode(hostInfo[6], true)
        local params = {}
        for k, v in pairs(split(dat[2], '&')) do
            local t = split(v, '=')
            params[t[1]] = t[2]
        end
        result.obfs_param = base64Decode(params.bfsparam, true)
        result.protocol_param = base64Decode(params.protoparam, true)
        local group = base64Decode(params.group, true)
        if group then
            result.alias = "["  .. group .. "] "
        end
        result.alias = result.alias .. base64Decode(params.remarks, true)
    elseif szType == 'vmess' then
        local info = luci.jsonc.parse(content)
        result.type = 'v2ray'
        result.server = info.add
        result.server_port = info.port
        result.tcp_guise = "none"
        result.transport = info.net
        result.alter_id = info.aid
        result.vmess_id = info.id
        result.alias = info.ps
        result.ws_host = info.host
        result.ws_path = info.path
        result.h2_host = info.host
        result.h2_path = info.path
        if not info.security then
            result.security = "auto"
        end
        if info.tls == "tls" or info.tls == "1" then
            result.tls = "1"
        else
            result.tls = "0"
        end
    elseif szType == "ss" then
        local info = content:sub(1, content:find("#") - 1)
        local alias = content:sub(content:find("#") + 1, #content)
        local hostInfo = split(base64Decode(info, true), "@")
        local host = split(hostInfo[2], ":")
        local userinfo = base64Decode(hostInfo[1], true)
        local method = userinfo:sub(1, userinfo:find(":") - 1)
        local password = userinfo:sub(userinfo:find(":") + 1, #userinfo)
        result.alias = UrlDecode(alias)
        result.type = "ss"
        result.server = host[1]
        if host[2]:find("/\\?") then
            local query = split(host[2], "/\\?")
            result.server_port = query[1]
            -- local params = {}
            -- for k, v in pairs(split(query[2], '&')) do
            --     local t = split(v, '=')
            --     params[t[1]] = t[2]
            -- end
            -- 这里似乎没什么用 我看数据结构没有写插件的支持 先抛弃
        else
            result.server_port = host[2]
        end
        result.encrypt_method_ss = method
        result.password = password
    elseif szType == "ssd" then
        result.type = "ss"
        result.server = content.server
        result.server_port = content.port
        result.password = content.password
        result.encrypt_method_ss = content.encryption
        result.alias = "[" .. content.airport .. "] " .. content.remarks
    end
    return result, hash
end
-- wget
local function wget(url)
    local stdout = luci.sys.exec('wget-ssl --user-agent="Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/77.0.3865.90 Safari/537.36" --no-check-certificate -t 3 -T 10 -O- "' .. url .. '"')
    return trim(stdout)
end

local execute = function()
    -- exec
    do
        -- subscribe_url = {'https://www.google.comc'}
        if proxy == '0' then -- 不使用代理更新的话先暂停
            log('服务正在暂停')
            luci.sys.init.stop(name)
        end
        for k, url in ipairs(subscribe_url) do
            local raw = wget(url)
            if #raw > 0 then
                local node, szType
                local groupHash = md5(url)
                cache[groupHash] = {}
                tinsert(nodeResult, {})
                local index = #nodeResult
                -- SSD 似乎是这种格式 ssd:// 开头的
                if raw:find('ssd://') then
                    szType = 'ssd'
                    local nEnd = select(2, raw:find('ssd://'))
                    node = base64Decode(raw:sub(nEnd + 1, #raw), true)
                    node = luci.jsonc.parse(node)
                    local extra = {
                        airport = node.airport,
                        port = node.port,
                        encryption = node.encryption,
                        password = node.password
                    }
                    local servers = {}
                    -- SS里面包着 干脆直接这样
                    for _, server in ipairs(node.servers) do
                        tinsert(servers, setmetatable(server, { __index = extra }))
                    end
                    node = servers
                else
                    -- ssd 外的格式
                    node = split(base64Decode(raw, true), "\n")
                end
                for _, v in ipairs(node) do
                    if v then
                        v = trim(v)
                        local result, hash
                        if szType == 'ssd' then
                            result, hash = processData(szType, v)
                        elseif not szType then
                            local dat = split(v, "://")
                            if dat and dat[1] and dat[2] then
                                if dat[1] == 'ss' then
                                    result, hash = processData(dat[1], dat[2])
                                else
                                    result, hash = processData(dat[1], base64Decode(dat[2], true))
                                end
                            end
                        else
                            log('跳过未知类型: ' .. szType)
                        end
                        -- log(hash, result)
                        if hash and result then
                            if result.alias:find("过期时间") or
                                result.alias:find("剩余流量") or
                                result.alias:find("QQ群") or
                                result.alias:find("官网") or
                                result.server == ''
                            then
                                log('丢弃无效节点: ' .. result.type ..' 节点, ' .. result.alias)
                            else
                                log('成功解析: ' .. result.type ..' 节点, ' .. result.alias)
                                result.grouphashkey = groupHash
                                tinsert(nodeResult[index], result)
                                cache[groupHash][hash] = nodeResult[index][#nodeResult[index]]
                            end
                        end
                    end
                end
                log('成功解析节点数量: ' ..#node)
            end
        end
    end
    -- diff
    do
        assert(next(nodeResult), "node result is empty")
        local add, del = 0, 0
        ucic:foreach(name, uciType, function(old)
            if old.grouphashkey or old.hashkey then -- 没有 hash 的不参与删除
                if not nodeResult[old.grouphashkey] or not nodeResult[old.grouphashkey][old.hashkey] then
                    ucic:delete(name, old['.name'])
                    del = del + 1
                else
                    local dat = nodeResult[old.grouphashkey][old.hashkey]
                    ucic:tset(name, old['.name'], dat)
                    -- 标记一下
                    setmetatable(nodeResult[old.grouphashkey][old.hashkey], { __index =  { _ignore = true } })
                end
            else
                log('忽略手动添加的节点: ' .. old.alias)
            end
        end)
        for k, v in ipairs(nodeResult) do
            for kk, vv in ipairs(v) do
                if not vv._ignore then
                    local section = ucic:add(name, uciType)
                    ucic:tset(name, section, vv)
                    add = add + 1
                end

            end
        end
        ucic:commit(name)
        -- 如果服务器已经不见了把帮换一个
        local globalServer = ucic:get_first(name, 'global', 'global_server', '')
        local firstServer = ucic:get_first(name, uciType)
        if not ucic:get(name, globalServer) then
            if firstServer then
                ucic:set(name, ucic:get_first(name, 'global'), 'global_server', firstServer)
                ucic:commit(name)
                log('当前主服务器已更新，正在自动更换。')
            end
        end
        if firstServer then
            luci.sys.call("/etc/init.d/" .. name .." restart > /dev/null 2>&1 &") -- 不加&的话日志会出现的更早
        else
            luci.sys.call("/etc/init.d/" .. name .." stop > /dev/null 2>&1 &") -- 不加&的话日志会出现的更早
        end
        log('新增节点数量: ' ..add, '删除节点数量: ' .. del)
        log('更新成功服务正在启动')
    end
end

if subscribe_url and #subscribe_url > 0 then
    xpcall(execute, function(e)
        log(e)
        log(debug.traceback())
        log('发生错误, 正在恢复服务')
        local firstServer = ucic:get_first(name, uciType)
        if firstServer then
            luci.sys.call("/etc/init.d/" .. name .." restart > /dev/null 2>&1 &") -- 不加&的话日志会出现的更早
        else
            luci.sys.call("/etc/init.d/" .. name .." stop > /dev/null 2>&1 &") -- 不加&的话日志会出现的更早
        end
    end)
end