local ucursor = require"luci.model.uci".cursor()
local json = require "luci.jsonc"
local server_section = arg[1]
local proto = arg[2]
local local_port = arg[3] or "0"
local socks_port = arg[4] or "0"
local server = ucursor:get_all("shadowsocksr", server_section)
local outbound_settings = nil
function vmess_vless()
	outbound_settings = {
		vnext = {
			{
				address = server.server,
				port = tonumber(server.server_port),
				users = {
					{
						id = server.vmess_id,
						alterId = (server.v2ray_protocol == "vmess" or not server.v2ray_protocol) and tonumber(server.alter_id) or nil,
						security = (server.v2ray_protocol == "vmess" or not server.v2ray_protocol) and server.security or nil,
						encryption = (server.v2ray_protocol == "vless") and server.vless_encryption or nil,
						flow = (server.xtls == '1') and (server.vless_flow and server.vless_flow or "xtls-rprx-splice") or nil
					}
				}
			}
		}
	}
end
function trojan_shadowsocks()
	outbound_settings = {
		servers = {
			{
				address = server.server,
				port = tonumber(server.server_port),
				password = server.password,
				method = (server.v2ray_protocol == "shadowsocks") and server.encrypt_method_v2ray_ss or nil,
				flow = (server.v2ray_protocol == "trojan") and (server.xtls == '1') and (server.vless_flow and server.vless_flow or "xtls-rprx-splice") or nil
			}
		}
	}
end
function socks_http()
	outbound_settings = {
		servers = {
			{
				address = server.server,
				port = tonumber(server.server_port),
				users = (server.auth_enable == "1") and {
					{
						user = server.username,
						pass = server.password
					}
				} or nil
			}
		}
	}
end
local outbound = {}
function outbound:new(o)
	o = o or {}
	setmetatable(o, self)
	self.__index = self
	return o
end
function outbound:handleIndex(index)
	local switch = {
		vmess = function()
			vmess_vless()
		end,
		vless = function()
			vmess_vless()
		end,
		trojan = function()
			trojan_shadowsocks()
		end,
		shadowsocks = function()
			trojan_shadowsocks()
		end,
		socks = function()
			socks_http()
		end,
		http = function()
			socks_http()
		end
	}
	if switch[index] then
		switch[index]()
	end
end
local settings = outbound:new()
settings:handleIndex(server.v2ray_protocol)
local Xray = {
	log = {
		-- error = "/var/ssrplus.log",
		loglevel = "warning"
	},
	-- 传入连接
	inbound = (local_port ~= "0") and {
		-- listening
		port = tonumber(local_port),
		protocol = "dokodemo-door",
		settings = {network = proto, followRedirect = true},
		sniffing = {enabled = true, destOverride = {"http", "tls"}}
	} or nil,
	-- 开启 socks 代理
	inboundDetour = (proto:find("tcp") and socks_port ~= "0") and {
		{
			-- socks
			protocol = "socks",
			port = tonumber(socks_port),
			settings = {auth = "noauth", udp = true}
		}
	} or nil,
	-- 传出连接
	outbound = {
		protocol = server.v2ray_protocol,
		settings = outbound_settings,
		-- 底层传输配置
		streamSettings = {
			network = server.transport or "tcp",
			security = (server.xtls == '1') and "xtls" or (server.tls == '1'or server.transport == "grpc") and "tls" or nil,
			tlsSettings = (server.tls == '1' and (server.insecure == "1" or server.tls_host or server.fingerprint)) and {
				-- tls
				fingerprint = server.fingerprint,
				allowInsecure = (server.insecure == "1") and true or nil,
				serverName = server.tls_host
			} or nil,
			xtlsSettings = (server.xtls == '1' and (server.insecure == "1" or server.tls_host)) and {
				-- xtls
				allowInsecure = (server.insecure == "1") and true or nil,
				serverName = server.tls_host
			} or nil,
			tcpSettings = (server.transport == "tcp" and server.tcp_guise == "http") and {
				-- tcp
				header = {
					type = server.tcp_guise,
					request = {
						-- request
						path = {server.http_path} or {"/"},
						headers = {Host = {server.http_host} or {}}
					}
				}
			} or nil,
			kcpSettings = (server.transport == "kcp") and {
				mtu = tonumber(server.mtu),
				tti = tonumber(server.tti),
				uplinkCapacity = tonumber(server.uplink_capacity),
				downlinkCapacity = tonumber(server.downlink_capacity),
				congestion = (server.congestion == "1") and true or false,
				readBufferSize = tonumber(server.read_buffer_size),
				writeBufferSize = tonumber(server.write_buffer_size),
				header = {type = server.kcp_guise},
				seed = server.seed or nil
			} or nil,
			wsSettings = (server.transport == "ws") and (server.ws_path or server.ws_host or server.tls_host) and {
				-- ws
				path = server.ws_path,
				headers = (server.ws_host or server.tls_host) and {
					-- headers
					Host = server.ws_host or server.tls_host
				} or nil
			} or nil,
			httpSettings = (server.transport == "h2") and {
				-- h2
				path = server.h2_path or "",
				host = {server.h2_host} or nil
			} or nil,
			quicSettings = (server.transport == "quic") and {
				-- quic
				security = server.quic_security,
				key = server.quic_key,
				header = {type = server.quic_guise}
			} or nil,
			grpcSettings = (server.transport == "grpc") and {
				-- grpc
				serviceName = server.serviceName or "",
				multiMode = (server.mux == "1") and true or false
			} or nil
		},
		mux = (server.mux == "1" and server.xtls ~= "1" and server.transport ~= "grpc") and {
			-- mux
			enabled = true,
			concurrency = tonumber(server.concurrency)
		} or nil
	} or nil
}
local cipher = "ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES128-SHA:ECDHE-RSA-AES256-SHA:DHE-RSA-AES128-SHA:DHE-RSA-AES256-SHA:AES128-SHA:AES256-SHA:DES-CBC3-SHA"
local cipher13 = "TLS_AES_128_GCM_SHA256:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_256_GCM_SHA384"
local trojan = {
	log_level = 3,
	run_type = (proto == "nat" or proto == "tcp") and "nat" or "client",
	local_addr = "0.0.0.0",
	local_port = tonumber(local_port),
	remote_addr = server.server,
	remote_port = tonumber(server.server_port),
	udp_timeout = 60,
	-- 传入连接
	password = {server.password},
	-- 传出连接
	ssl = {
		verify = (server.insecure == "0") and true or false,
		verify_hostname = (server.tls == "1") and true or false,
		cert = (server.certificate) and server.certpath or nil,
		cipher = cipher,
		cipher_tls13 = cipher13,
		sni = server.tls_host,
		alpn = {"h2", "http/1.1"},
		curve = "",
		reuse_session = true,
		session_ticket = (server.tls_sessionTicket == "1") and true or false
	},
	udp_timeout = 60,
	tcp = {
		-- tcp
		no_delay = true,
		keep_alive = true,
		reuse_port = true,
		fast_open = (server.fast_open == "1") and true or false,
		fast_open_qlen = 20
	}
}
local naiveproxy = {
	proxy = (server.username and server.password and server.server and server.server_port) and "https://" .. server.username .. ":" .. server.password .. "@" .. server.server .. ":" .. server.server_port,
	listen = (proto == "redir") and "redir" .. "://0.0.0.0:" .. tonumber(local_port) or "socks" .. "://0.0.0.0:" .. tonumber(local_port),
	concurrency = (socks_port ~= "0") and tonumber(socks_port) or "1"
}
local ss = {
	server = (server.kcp_enable == "1") and "127.0.0.1" or server.server,
	server_port = tonumber(server.server_port),
	local_address = "0.0.0.0",
	local_port = tonumber(local_port),
	mode = (proto == "tcp,udp") and "tcp_and_udp" or proto .. "_only",
	password = server.password,
	method = server.encrypt_method_ss,
	timeout = tonumber(server.timeout),
	fast_open = (server.fast_open == "1") and true or false,
	reuse_port = true
}
local config = {}
function config:new(o)
	o = o or {}
	setmetatable(o, self)
	self.__index = self
	return o
end
function config:handleIndex(index)
	local switch = {
		ss = function()
			ss.protocol = socks_port
			if server.plugin and server.plugin ~= "none" then
				ss.plugin = server.plugin
				ss.plugin_opts = server.plugin_opts or nil
			end
			print(json.stringify(ss, 1))
		end,
		ssr = function()
			ss.protocol = server.protocol
			ss.protocol_param = server.protocol_param
			ss.method = server.encrypt_method
			ss.obfs = server.obfs
			ss.obfs_param = server.obfs_param
			print(json.stringify(ss, 1))
		end,
		v2ray = function()
			print(json.stringify(Xray, 1))
		end,
		trojan = function()
			print(json.stringify(trojan, 1))
		end,
		naiveproxy = function()
			print(json.stringify(naiveproxy, 1))
		end
	}
	if switch[index] then
		switch[index]()
	end
end
local f = config:new()
f:handleIndex(server.type)
