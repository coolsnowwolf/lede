#!/usr/bin/lua
------------------------------------------------
-- This file is part of the luci-app-ssr-plus subscribe.lua
-- @author William Chan <root@williamchan.me>
------------------------------------------------
require 'nixio'
require 'luci.util'
require 'luci.jsonc'
require 'luci.sys'
require 'uci'
-- these global functions are accessed all the time by the event handler
-- so caching them is worth the effort
local tinsert = table.insert
local ssub, slen, schar, sbyte, sformat, sgsub = string.sub, string.len, string.char, string.byte, string.format, string.gsub
local jsonParse, jsonStringify = luci.jsonc.parse, luci.jsonc.stringify
local b64decode = nixio.bin.b64decode
local nodeResult = {} -- update result
local application = 'shadowsocksr'
local uciType = 'servers'
local ucic2 = uci.cursor()
local proxy = ucic2:get(application, '@server_subscribe[0]', 'proxy') or '0'
local switch = ucic2:get(application, '@server_subscribe[0]', 'switch') or '1'
local subscribe_url = ucic2:get(application, '@server_subscribe[0]', 'subscribe_url') or {}
local filter_words = ucic2:get(application, '@server_subscribe[0]', 'filter_words') or '过期时间/剩余流量'
ucic2:revert(application)

local log = function(...)
	print(os.date("%Y-%m-%d %H:%M:%S ") .. table.concat({ ... }, " "))
end

-- 获取各项动态配置的当前服务器，可以用 get 和 set， get必须要获取到节点表
local CONFIG = {
	GLOBAL_SERVER = {
	remarks = '主节点',
	type = "global", option = "global_server",
	set = function(server)
		ucic2:set(application, '@global[0]', "global_server", server)
	end
	}
}
do
	for k, v in pairs(CONFIG) do
		local currentNode
		if v.get then
			currentNode = v.get()
		else
			local cfgid = ucic2:get(application, '@' .. v.type .. '[0]', v.option)
			if cfgid then
				currentNode = ucic2:get_all(application, cfgid)
			end
		end
		if currentNode then
			CONFIG[k].currentNode = currentNode
		else
			CONFIG[k] = nil
		end
	end
end

-- 分割字符串
local function split(full, sep)
	full = full:gsub("%z", "") -- 这里不是很清楚 有时候结尾带个\0
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
-- local function get_urlencode(c)
-- return sformat("%%%02X", sbyte(c))
-- end

-- local function urlEncode(szText)
-- local str = szText:gsub("([^0-9a-zA-Z ])", get_urlencode)
-- str = str:gsub(" ", "+")
-- return str
-- end

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
	kcp_param = '--nocomp',
	isSubscribe = 1,
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
			result.alias = "[" .. group .. "] "
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
	return result
end
-- wget
local function wget(url)
	local stdout = luci.sys.exec('wget-ssl -q --user-agent="Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.157 Safari/537.36" --no-check-certificate -t 3 -T 10 -O- "' .. url .. '"')
	return trim(stdout)
end

local function check_filer(result)
	do
		local filter_word = split(filter_words, "/")
		for i, v in pairs(filter_word) do
			if result.alias:find(v) then
				log('订阅节点关键字过滤:“' .. v ..'” ，该节点被丢弃')
				return true
			end
		end
	end
end

local function select_node(nodes, config)
	local server
	-- 第一优先级 IP + 端口
	for id, node in pairs(nodes) do
		if node.server .. ':' .. node.server_port == config.currentNode.server .. ':' .. config.currentNode.server_port then
			log('选择【' .. config.remarks .. '】第一匹配节点：' .. node.alias)
			server = id
			break
		end
	end
	-- 第二优先级 IP
	if not server then
		for id, node in pairs(nodes) do
			if node.server == config.currentNode.server then
				log('选择【' .. config.remarks .. '】第二匹配节点：' .. node.alias)
				server = id
				break
			end
		end
	end
	-- 第三优先级备注
	if not server then
		for id, node in pairs(nodes) do
			if node.alias == config.currentNode.alias then
				log('选择【' .. config.remarks .. '】第三匹配节点：' .. node.alias)
				server = id
				break
			end
		end
	end
	-- 第四 cfgid
	if not server then
		for id, node in pairs(nodes) do
			if id == config.currentNode['.name'] then
				log('选择【' .. config.remarks .. '】第四匹配节点：' .. node.alias)
				server = id
				break
			end
		end
	end
	-- 还不行 随便找一个
	if not server then
		server = ucic2:get(application, '@'.. uciType .. '[0]')
		if server then
			log('无法找到最匹配的节点，当前已更换为' .. ucic2:get_all(application, server).alias)
		end
	end
	if server then
		config.set(server)
	end
end

local execute = function()
	-- exec
	do
		if proxy == '0' then -- 不使用代理更新的话先暂停
			log('服务正在暂停')
			luci.sys.init.stop(application)
		end
		for k, url in ipairs(subscribe_url) do
			local raw = wget(url)
			if #raw > 0 then
				local nodes, szType
				local all_odes = {}
				tinsert(nodeResult, all_odes)
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
							if
								not result.server or
								not result.server_port or
								check_filer(result) or
								result.server:match("[^0-9a-zA-Z%-%.%s]") -- 中文做地址的 也没有人拿中文域名搞，就算中文域也有Puny Code SB 机场
								then
								log('丢弃无效节点: ' .. result.type ..' 节点, ' .. result.alias)
							else
								log('成功解析: ' .. result.type ..' 节点, ' .. result.alias)
								tinsert(all_odes, result)
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
		assert(next(nodeResult), '更新失败，没有可用的节点信息')
		-- delete all for subscribe nodes
		ucic2:foreach(application, uciType, function(node)
			if node.isSubscribe or node.hashkey then -- 兼容之前的hashkey
				ucic2:delete(application, node['.name'])
			end
		end)
		for _, v in ipairs(nodeResult) do
			for _, vv in ipairs(v) do
				vv.switch_enable = switch
				local cfgid = ucic2:add(application, uciType)
				for kkk, vvv in pairs(vv) do
					ucic2:set(application, cfgid, kkk, vvv)
				end
			end
		end
		ucic2:commit(application)
		local ucic3 = uci.cursor()
		-- repair configuration
		if next(CONFIG) then
			local nodes = {}
			ucic3:foreach(application, uciType, function(node)
				if node.server and node.server_port and node.alias then
					nodes[node['.name']] = node
				end
			end)
			for _, config in pairs(CONFIG) do
				select_node(nodes, config)
			end
			ucic3:commit(application)
		end
		-- select first server
		local globalServer = ucic3:get(application, '@global[0]', 'global_server') or ''
		if not globalServer or not ucic3:get_all(application, globalServer) then
			ucic3:set(application, '@global[0]', 'global_server', select(2, ucic3:get(application, '@' .. uciType .. '[0]')))
			ucic3:commit(application)
			log('当前没有主节点，自动选择第一个节点开启服务。')
		end
		luci.sys.call("/etc/init.d/" .. application .." restart > /dev/null 2>&1 &") -- 不加&的话日志会出现的更早
		log('订阅更新成功')
	end
end

if subscribe_url and #subscribe_url > 0 then
	xpcall(execute, function(e)
		log(e)
		log(debug.traceback())
		log('发生错误, 正在恢复服务')
		if CONFIG.GLOBAL_SERVER and CONFIG.GLOBAL_SERVER.currentNode then
			luci.sys.call("/etc/init.d/" .. application .." restart > /dev/null 2>&1 &") -- 不加&的话日志会出现的更早
			log('重启服务成功')
		else
			luci.sys.call("/etc/init.d/" .. application .." stop > /dev/null 2>&1 &") -- 不加&的话日志会出现的更早
			log('停止服务成功')
		end
	end)
end
