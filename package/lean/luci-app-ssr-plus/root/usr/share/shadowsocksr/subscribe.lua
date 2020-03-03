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
local luci = luci
local tinsert = table.insert
local ssub, slen, schar, sbyte, sformat, sgsub = string.sub, string.len, string.char, string.byte, string.format, string.gsub
local jsonParse, jsonStringify = luci.jsonc.parse, luci.jsonc.stringify
local b64decode = nixio.bin.b64decode
local cache = {}
local nodeResult = setmetatable({}, { __index = cache })  -- update result
local name = 'shadowsocksr'
local uciType = 'servers'
local ucic = luci.model.uci.cursor()
local proxy = ucic:get_first(name, 'server_subscribe', 'proxy', '0')
local switch = ucic:get_first(name, 'server_subscribe', 'switch', '1')
local subscribe_url = ucic:get_first(name, 'server_subscribe', 'subscribe_url', {})

local log = function(...)
	print(os.date("%Y-%m-%d %H:%M:%S ") .. table.concat({ ... }, " "))
end
-- 分割字符串
local function split(full, sep)
	full = full:gsub("%z", "")  -- 这里不是很清楚 有时候结尾带个\0
	local off, result = 1, {}
	while true do
		local nStart, nEnd = full:find(sep, off)
		if not nEnd then
			local res = ssub(full, off, slen(full))
			if #res > 0 then -- 过滤掉 \0
				tinsert(result, res)
			end
			break
		else
			tinsert(result, ssub(full, off, nStart - 1))
			off = nEnd + 1
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
local function base64Decode(text)
	local raw = text
	if not text then return '' end
	text = text:gsub("%z", "")
	text = text:gsub("_", "/")
	text = text:gsub("-", "+")
	local mod4 = #text % 4
	text = text .. string.sub('====', mod4 + 1)
	local result = b64decode(text)
	if result then
		return result:gsub("%z", "")
	else
		return raw
	end
end
-- 处理数据
local function processData(szType, content)
	local result = {
	type = szType,
	local_port = 1234,
	kcp_param = '--nocomp'
	}
	if szType == 'ssr' then
		local dat = split(content, "/%?")
		local hostInfo = split(dat[1], ':')
		result.server = hostInfo[1]
		result.server_port = hostInfo[2]
		result.protocol = hostInfo[3]
		result.encrypt_method = hostInfo[4]
		result.obfs = hostInfo[5]
		result.password = base64Decode(hostInfo[6])
		local params = {}
		for _, v in pairs(split(dat[2], '&')) do
			local t = split(v, '=')
			params[t[1]] = t[2]
		end
		result.obfs_param = base64Decode(params.obfsparam)
		result.protocol_param = base64Decode(params.protoparam)
		local group = base64Decode(params.group)
		if group then
			result.alias = "["  .. group .. "] "
		end
		result.alias = result.alias .. base64Decode(params.remarks)
	elseif szType == 'vmess' then
		local info = jsonParse(content)
		result.type = 'v2ray'
		result.server = info.add
		result.server_port = info.port
		result.transport = info.net
		result.alter_id = info.aid
		result.vmess_id = info.id
		result.alias = info.ps
		result.insecure = 1
		-- result.mux = 1
		-- result.concurrency = 8
		if info.net == 'ws' then
			result.ws_host = info.host
			result.ws_path = info.path
		end
		if info.net == 'h2' then
			result.h2_host = info.host
			result.h2_path = info.path
		end
		if info.net == 'tcp' then
			result.tcp_guise = info.type
			result.http_host = info.host
			result.http_path = info.path
		end
		if info.net == 'kcp' then
			result.kcp_guise = info.type
			result.mtu = 1350
			result.tti = 50
			result.uplink_capacity = 5
			result.downlink_capacity = 20
			result.read_buffer_size = 2
			result.write_buffer_size = 2
		end
		if info.net == 'quic' then
			result.quic_guise = info.type
			result.quic_key = info.key
			result.quic_security = info.securty
		end
		if info.security then
			result.security = info.security
		end
		if info.tls == "tls" or info.tls == "1" then
			result.tls = "1"
			result.tls_host = info.host
		else
			result.tls = "0"
		end
	elseif szType == "ss" then
		local idx_sp = 0
		local alias = ""
		if content:find("#") then
			idx_sp = content:find("#")
			alias = content:sub(idx_sp + 1, -1)
		end
		local info = content:sub(1, idx_sp - 1)
		local hostInfo = split(base64Decode(info), "@")
		local host = split(hostInfo[2], ":")
		local userinfo = base64Decode(hostInfo[1])
		local method = userinfo:sub(1, userinfo:find(":") - 1)
		local password = userinfo:sub(userinfo:find(":") + 1, #userinfo)
		result.alias = UrlDecode(alias)
		result.type = "ss"
		result.server = host[1]
		if host[2]:find("/%?") then
			local query = split(host[2], "/%?")
			result.server_port = query[1]
			local params = {}
			for _, v in pairs(split(query[2], '&')) do
				local t = split(v, '=')
				params[t[1]] = t[2]
			end
			if params.plugin then
				local plugin_info = UrlDecode(params.plugin)
				local idx_pn = plugin_info:find(";")
				if idx_pn then
					result.plugin = plugin_info:sub(1, idx_pn - 1)
					result.plugin_opts = plugin_info:sub(idx_pn + 1, #plugin_info)
				else
					result.plugin = plugin_info
				end
			end
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
		result.plugin = content.plugin
		result.plugin_opts = content.plugin_options
		result.alias = "[" .. content.airport .. "] " .. content.remarks
	elseif szType == "trojan" then
		local idx_sp = 0
		local alias = ""
		if content:find("#") then
			idx_sp = content:find("#")
			alias = content:sub(idx_sp + 1, -1)
		end
		local info = content:sub(1, idx_sp - 1)
		local hostInfo = split(info, "@")
		local host = split(hostInfo[2], ":")
		local userinfo = hostInfo[1]
		local password = userinfo
		result.alias = UrlDecode(alias)
		result.type = "trojan"
		result.server = host[1]
		-- 按照官方的建议 默认验证ssl证书
		result.insecure = "0"
		result.tls = "1"
		if host[2]:find("?") then
			local query = split(host[2], "?")
			result.server_port = query[1]
			local params = {}
			for _, v in pairs(split(query[2], '&')) do
				local t = split(v, '=')
				params[t[1]] = t[2]
			end
			
			if params.peer then
				-- 未指定peer（sni）默认使用remote addr
				result.tls_host = params.peer
			end
			
			if params.allowInsecure == "1" then
				result.insecure = "1"
			else
				result.insecure = "0"
			end
		else
			result.server_port = host[2]
		end
		result.password = password
	end
	if not result.alias then
		result.alias = result.server .. ':' .. result.server_port
	end
	-- alias 不参与 hashkey 计算
	local alias = result.alias
	result.alias = nil
	local switch_enable = result.switch_enable
	result.switch_enable = nil
	result.hashkey = md5(jsonStringify(result))
	result.alias = alias
	result.switch_enable = switch_enable
	return result
end
-- wget
local function wget(url)
	local stdout = luci.sys.exec('wget-ssl --user-agent="Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/77.0.3865.90 Safari/537.36" --no-check-certificate -t 3 -T 10 -O- "' .. url .. '"')
	return trim(stdout)
end

local execute = function()
	-- exec
	do
		if proxy == '0' then -- 不使用代理更新的话先暂停
			log('服务正在暂停')
			luci.sys.init.stop(name)
		end
		for k, url in ipairs(subscribe_url) do
			local raw = wget(url)
			if #raw > 0 then
				local nodes, szType
				local groupHash = md5(url)
				cache[groupHash] = {}
				tinsert(nodeResult, {})
				local index = #nodeResult
				-- SSD 似乎是这种格式 ssd:// 开头的
				if raw:find('ssd://') then
					szType = 'ssd'
					local nEnd = select(2, raw:find('ssd://'))
					nodes = base64Decode(raw:sub(nEnd + 1, #raw))
					nodes = jsonParse(nodes)
					local extra = {
						airport = nodes.airport,
						port = nodes.port,
						encryption = nodes.encryption,
						password = nodes.password
					}
					local servers = {}
					-- SS里面包着 干脆直接这样
					for _, server in ipairs(nodes.servers) do
						tinsert(servers, setmetatable(server, { __index = extra }))
					end
					nodes = servers
				else
					-- ssd 外的格式
					nodes = split(base64Decode(raw):gsub(" ", "\n"), "\n")
				end
				for _, v in ipairs(nodes) do
					if v then
						local result
						if szType == 'ssd' then
							result = processData(szType, v)
						elseif not szType then
							local node = trim(v)
							local dat = split(node, "://")
							if dat and dat[1] and dat[2] then
								if dat[1] == 'ss' or dat[1] == 'trojan' then
									result = processData(dat[1], dat[2])
								else
									result = processData(dat[1], base64Decode(dat[2]))
								end
							end
						else
							log('跳过未知类型: ' .. szType)
						end
						-- log(result)
						if result then
							if result.alias:find("过期时间") or
								result.alias:find("剩余流量") or
								result.alias:find("QQ群") or
								result.alias:find("官网") or
								result.alias:find("防失联地址") or
								not result.server or
								result.server:match("[^0-9a-zA-Z%-%.%s]") -- 中文做地址的 也没有人拿中文域名搞，就算中文域也有Puny Code SB 机场
							then
								log('丢弃无效节点: ' .. result.type ..' 节点, ' .. result.alias)
							else
								log('成功解析: ' .. result.type ..' 节点, ' .. result.alias)
								result.grouphashkey = groupHash
								tinsert(nodeResult[index], result)
								cache[groupHash][result.hashkey] = nodeResult[index][#nodeResult[index]]
							end
						end
					end
				end
				log('成功解析节点数量: ' ..#nodes)
			else
				log(url .. ': 获取内容为空')
			end
		end
	end
	-- diff
	do
		if next(nodeResult) == nil then
			log("更新失败，没有可用的节点信息")
			return
		end
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
				if not old.alias then
					old.alias = old.server .. ':' .. old.server_port
				end
				log('忽略手动添加的节点: ' .. old.alias)
			end

		end)
		for k, v in ipairs(nodeResult) do
			for kk, vv in ipairs(v) do
				if not vv._ignore then
					local section = ucic:add(name, uciType)
					ucic:tset(name, section, vv)
					ucic:set(name, section, "switch_enable", switch)
					add = add + 1
				end
			end
		end
		ucic:commit(name)
		-- 如果原有服务器节点已经不见了就尝试换为第一个节点
		local globalServer = ucic:get_first(name, 'global', 'global_server', '')
		local firstServer = ucic:get_first(name, uciType)
		if firstServer then
			if not ucic:get(name, globalServer) then
				luci.sys.call("/etc/init.d/" .. name .. " stop > /dev/null 2>&1 &")
				ucic:commit(name)
				ucic:set(name, ucic:get_first(name, 'global'), 'global_server', ucic:get_first(name, uciType))
				ucic:commit(name)
				log('当前主服务器节点已被删除，正在自动更换为第一个节点。')
				luci.sys.call("/etc/init.d/" .. name .. " start > /dev/null 2>&1 &")
			else
				log('维持当前主服务器节点。')
				luci.sys.call("/etc/init.d/" .. name .." restart > /dev/null 2>&1 &")
			end
		else
			log('没有服务器节点了，停止服务')
			luci.sys.call("/etc/init.d/" .. name .. " stop > /dev/null 2>&1 &")
		end
		log('新增节点数量: ' ..add, '删除节点数量: ' .. del)
		log('订阅更新成功')
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
			log('重启服务成功')
		else
			luci.sys.call("/etc/init.d/" .. name .." stop > /dev/null 2>&1 &") -- 不加&的话日志会出现的更早
			log('停止服务成功')
		end
	end)
end
