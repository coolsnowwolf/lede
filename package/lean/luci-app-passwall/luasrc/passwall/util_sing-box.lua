module("luci.passwall.util_sing-box", package.seeall)
local api = require "luci.passwall.api"
local uci = api.uci
local sys = api.sys
local jsonc = api.jsonc
local appname = api.appname
local fs = api.fs

local new_port

local function get_new_port()
	if new_port then
		new_port = tonumber(sys.exec(string.format("echo -n $(/usr/share/%s/app.sh get_new_port %s tcp)", appname, new_port + 1)))
	else
		new_port = tonumber(sys.exec(string.format("echo -n $(/usr/share/%s/app.sh get_new_port auto tcp)", appname)))
	end
	return new_port
end

function gen_outbound(flag, node, tag, proxy_table)
	local result = nil
	if node and node ~= "nil" then
		local node_id = node[".name"]
		if tag == nil then
			tag = node_id
		end

		local proxy = 0
		local proxy_tag = "nil"
		if proxy_table ~= nil and type(proxy_table) == "table" then
			proxy = proxy_table.proxy or 0
			proxy_tag = proxy_table.tag or "nil"
		end

		if node.type == "sing-box" then
			proxy = 0
			if proxy_tag ~= "nil" then
				node.detour = proxy_tag
			end
		end

		if node.type ~= "sing-box" then
			local relay_port = node.port
			new_port = get_new_port()
			local config_file = string.format("%s_%s_%s.json", flag, tag, new_port)
			if tag and node_id and tag ~= node_id then
				config_file = string.format("%s_%s_%s_%s.json", flag, tag, node_id, new_port)
			end
			sys.call(string.format('/usr/share/%s/app.sh run_socks "%s"> /dev/null',
				appname,
				string.format("flag=%s node=%s bind=%s socks_port=%s config_file=%s relay_port=%s",
					new_port, --flag
					node_id, --node
					"127.0.0.1", --bind
					new_port, --socks port
					config_file, --config file
					(proxy == 1 and relay_port) and tostring(relay_port) or "" --relay port
					)
				)
			)
			node = {
				protocol = "socks",
				address = "127.0.0.1",
				port = new_port
			}
		end

		result = {
			_flag_tag = node_id,
			_flag_proxy = proxy,
			_flag_proxy_tag = proxy_tag,
			tag = tag,
			type = node.protocol,
			server = node.address,
			server_port = tonumber(node.port),
			detour = node.detour,
		}

		local tls = nil
		if node.tls == "1" then
			local alpn = nil
			if node.alpn and node.alpn ~= "default" then
				alpn = {}
				string.gsub(node.alpn, '[^' .. "," .. ']+', function(w)
					table.insert(alpn, w)
				end)
			end
			tls = {
				enabled = true,
				disable_sni = false, --不要在 ClientHello 中发送服务器名称.
				server_name = node.tls_serverName, --用于验证返回证书上的主机名，除非设置不安全。它还包含在 ClientHello 中以支持虚拟主机，除非它是 IP 地址。
				insecure = (node.tls_allowInsecure == "1") and true or false, --接受任何服务器证书。
				alpn = alpn, --支持的应用层协议协商列表，按优先顺序排列。如果两个对等点都支持 ALPN，则选择的协议将是此列表中的一个，如果没有相互支持的协议则连接将失败。
				--min_version = "1.2",
				--max_version = "1.3",
				ech = {
					enabled = (node.ech == "1") and true or false,
					config = (node.ech_config and node.ech_config:gsub("\\n","\n")) and node.ech_config:gsub("\\n","\n") or nil,
					pq_signature_schemes_enabled = node.pq_signature_schemes_enabled and true or false,
					dynamic_record_sizing_disabled = node.dynamic_record_sizing_disabled and true or false
				},
				utls = {
					enabled = (node.utls == "1" or node.reality == "1") and true or false,
					fingerprint = node.fingerprint or "chrome"
				},
				reality = {
					enabled = (node.reality == "1") and true or false,
					public_key = node.reality_publicKey,
					short_id = node.reality_shortId
				}
			}
		end

		local mux = nil
		if node.mux == "1" then
			mux = {
				enabled = true,
				protocol = node.mux_type or "h2mux",
				max_connections = tonumber(node.mux_concurrency) or 4,
				padding = (node.mux_padding == "1") and true or false,
				--min_streams = 4,
				--max_streams = 0,
			}
		end

		local v2ray_transport = nil

		if node.transport == "http" then
			v2ray_transport = {
				type = "http",
				host = { node.http_host },
				path = node.http_path or "/",
				idle_timeout = (node.http_h2_health_check == "1") and node.http_h2_read_idle_timeout or nil,
				ping_timeout = (node.http_h2_health_check == "1") and node.http_h2_health_check_timeout or nil,
			}
			--不强制执行 TLS。如果未配置 TLS，将使用纯 HTTP 1.1。
		end

		if node.transport == "ws" then
			v2ray_transport = {
				type = "ws",
				path = node.ws_path or "/",
				headers = (node.ws_host ~= nil) and { Host = node.ws_host } or nil,
				max_early_data = tonumber(node.ws_maxEarlyData) or nil,
				early_data_header_name = (node.ws_earlyDataHeaderName) and node.ws_earlyDataHeaderName or nil --要与 Xray-core 兼容，请将其设置为 Sec-WebSocket-Protocol。它需要与服务器保持一致。
			}
		end

		if node.transport == "quic" then
			v2ray_transport = {
				type = "quic"
			}
			--没有额外的加密支持： 它基本上是重复加密。 并且 Xray-core 在这里与 v2ray-core 不兼容。
		end

		if node.transport == "grpc" then
			v2ray_transport = {
				type = "grpc",
				service_name = node.grpc_serviceName,
				idle_timeout = tonumber(node.grpc_idle_timeout) or nil,
				ping_timeout = tonumber(node.grpc_health_check_timeout) or nil,
				permit_without_stream = (node.grpc_permit_without_stream == "1") and true or nil,
			}
		end

		local protocol_table = nil

		if node.protocol == "socks" then
			protocol_table = {
				version = "5",
				username = (node.username and node.password) and node.username or nil,
				password = (node.username and node.password) and node.password or nil,
				udp_over_tcp = false,
			}
		end

		if node.protocol == "http" then
			protocol_table = {
				username = (node.username and node.password) and node.username or nil,
				password = (node.username and node.password) and node.password or nil,
				path = nil,
				headers = nil,
				tls = tls
			}
		end

		if node.protocol == "shadowsocks" then
			protocol_table = {
				method = node.method or nil,
				password = node.password or "",
				plugin = node.plugin and nil,
				plugin_opts = node.plugin_opts and nil,
				udp_over_tcp = node.uot == "1" and {
					enabled = true,
					version = 2
				} or nil,
				multiplex = mux,
			}
		end

		if node.protocol == "shadowsocksr" then
			protocol_table = {
				method = node.method or nil,
				password = node.password or "",
				obfs = node.ssr_obfs,
				obfs_param = node.ssr_obfs_param,
				protocol = node.ssr_protocol,
				protocol_param = node.ssr_protocol_param,
			}
		end

		if node.protocol == "trojan" then
			protocol_table = {
				password = node.password,
				tls = tls,
				multiplex = mux,
				transport = v2ray_transport
			}
		end

		if node.protocol == "vmess" then
			protocol_table = {
				uuid = node.uuid,
				security = node.security,
				alter_id = (node.alter_id) and tonumber(node.alter_id) or 0,
				global_padding = (node.global_padding == "1") and true or false,
				authenticated_length = (node.authenticated_length == "1") and true or false,
				tls = tls,
				packet_encoding = "", --UDP 包编码。(空)：禁用	packetaddr：由 v2ray 5+ 支持	xudp：由 xray 支持
				multiplex = mux,
				transport = v2ray_transport,
			}
		end

		if node.protocol == "vless" then
			protocol_table = {
				uuid = node.uuid,
				flow = (node.tls == '1' and node.flow) and node.flow or nil,
				tls = tls,
				packet_encoding = "xudp", --UDP 包编码。(空)：禁用	packetaddr：由 v2ray 5+ 支持	xudp：由 xray 支持
				multiplex = mux,
				transport = v2ray_transport,
			}
		end

		if node.protocol == "wireguard" then
			if node.wireguard_reserved then
				local bytes = {}
				if not node.wireguard_reserved:match("[^%d,]+") then
					node.wireguard_reserved:gsub("%d+", function(b)
						bytes[#bytes + 1] = tonumber(b)
					end)
				else
					local result = api.bin.b64decode(node.wireguard_reserved)
					for i = 1, #result do
						bytes[i] = result:byte(i)
					end
				end
				node.wireguard_reserved = #bytes > 0 and bytes or nil
			end
			protocol_table = {
				system_interface = nil,
				interface_name = nil,
				local_address = node.wireguard_local_address,
				private_key = node.wireguard_secret_key,
				peer_public_key = node.wireguard_public_key,
				pre_shared_key = node.wireguard_preSharedKey,
				reserved = node.wireguard_reserved,
				mtu = tonumber(node.wireguard_mtu),
			}
		end

		if node.protocol == "hysteria" then
			protocol_table = {
				up = node.hysteria_up_mbps .. " Mbps",
				down = node.hysteria_down_mbps .. " Mbps",
				up_mbps = tonumber(node.hysteria_up_mbps),
				down_mbps = tonumber(node.hysteria_down_mbps),
				obfs = node.hysteria_obfs,
				auth = (node.hysteria_auth_type == "base64") and node.hysteria_auth_password or nil,
				auth_str = (node.hysteria_auth_type == "string") and node.hysteria_auth_password or nil,
				recv_window_conn = tonumber(node.hysteria_recv_window_conn),
				recv_window = tonumber(node.hysteria_recv_window),
				disable_mtu_discovery = (node.hysteria_disable_mtu_discovery == "1") and true or false,
				tls = {
					enabled = true,
					server_name = node.tls_serverName,
					insecure = (node.tls_allowInsecure == "1") and true or false,
					alpn = (node.hysteria_alpn and node.hysteria_alpn ~= "") and {
						node.hysteria_alpn
					} or nil,
					ech = {
						enabled = (node.ech == "1") and true or false,
						config = (node.ech_config and node.ech_config:gsub("\\n","\n")) and node.ech_config:gsub("\\n","\n") or nil,
						pq_signature_schemes_enabled = node.pq_signature_schemes_enabled and true or false,
						dynamic_record_sizing_disabled = node.dynamic_record_sizing_disabled and true or false
					}
				}
			}
		end

		if node.protocol == "shadowtls" then
			protocol_table = {
				version = tonumber(node.shadowtls_version),
				password = (node.shadowtls_version == "2" or node.shadowtls_version == "3") and node.password or nil,
				tls = tls,
			}
		end

		if node.protocol == "tuic" then
			protocol_table = {
				uuid = node.uuid,
				password = node.password,
				congestion_control = node.tuic_congestion_control or "cubic",
				udp_relay_mode = node.tuic_udp_relay_mode or "native",
				udp_over_stream = false,
				zero_rtt_handshake = (node.tuic_zero_rtt_handshake == "1") and true or false,
				heartbeat = node.tuic_heartbeat .. "s",
				tls = {
					enabled = true,
					server_name = node.tls_serverName,
					insecure = (node.tls_allowInsecure == "1") and true or false,
					alpn = (node.tuic_alpn and node.tuic_alpn ~= "") and {
						node.tuic_alpn
					} or nil,
					ech = {
						enabled = (node.ech == "1") and true or false,
						config = (node.ech_config and node.ech_config:gsub("\\n","\n")) and node.ech_config:gsub("\\n","\n") or nil,
						pq_signature_schemes_enabled = node.pq_signature_schemes_enabled and true or false,
						dynamic_record_sizing_disabled = node.dynamic_record_sizing_disabled and true or false
					}
				}
			}
		end

		if node.protocol == "hysteria2" then
			protocol_table = {
				up_mbps = (node.hysteria2_up_mbps and tonumber(node.hysteria2_up_mbps)) and tonumber(node.hysteria2_up_mbps) or nil,
				down_mbps = (node.hysteria2_down_mbps and tonumber(node.hysteria2_down_mbps)) and tonumber(node.hysteria2_down_mbps) or nil,
				obfs = {
					type = node.hysteria2_obfs_type,
					password = node.hysteria2_obfs_password
				},
				password = node.hysteria2_auth_password or nil,
				tls = {
					enabled = true,
					server_name = node.tls_serverName,
					insecure = (node.tls_allowInsecure == "1") and true or false,
					ech = {
						enabled = (node.ech == "1") and true or false,
						config = (node.ech_config and node.ech_config:gsub("\\n","\n")) and node.ech_config:gsub("\\n","\n") or nil,
						pq_signature_schemes_enabled = node.pq_signature_schemes_enabled and true or false,
						dynamic_record_sizing_disabled = node.dynamic_record_sizing_disabled and true or false
					}
				}
			}
		end

		if protocol_table then
			for key, value in pairs(protocol_table) do
				result[key] = value
			end
		end
	end
	return result
end

function gen_config_server(node)
	local outbounds = {
		{ type = "direct", tag = "direct" },
		{ type = "block", tag = "block" }
	}

	local tls = {
		enabled = true,
		certificate_path = node.tls_certificateFile,
		key_path = node.tls_keyFile,
	}

	if node.tls == "1" and node.reality == "1" then
		tls.certificate_path = nil
		tls.key_path = nil
		tls.reality = {
			enabled = true,
			private_key = node.reality_private_key,
			short_id = {
				node.reality_shortId
			},
			handshake = {
				server = node.reality_handshake_server,
				server_port = tonumber(node.reality_handshake_server_port)
			}
		}
	end

	if node.tls == "1" and node.ech == "1" then
		tls.ech = {
			enabled = true,
			key = (node.ech_key and node.ech_key:gsub("\\n","\n")) and node.ech_key:gsub("\\n","\n") or nil,
			pq_signature_schemes_enabled = (node.pq_signature_schemes_enabled == "1") and true or false,
			dynamic_record_sizing_disabled = (node.dynamic_record_sizing_disabled == "1") and true or false,
		}
	end

	local v2ray_transport = nil

	if node.transport == "http" then
		v2ray_transport = {
			type = "http",
			host = node.http_host,
			path = node.http_path or "/",
		}
	end

	if node.transport == "ws" then
		v2ray_transport = {
			type = "ws",
			path = node.ws_path or "/",
			headers = (node.ws_host ~= nil) and { Host = node.ws_host } or nil,
			early_data_header_name = (node.ws_earlyDataHeaderName) and node.ws_earlyDataHeaderName or nil --要与 Xray-core 兼容，请将其设置为 Sec-WebSocket-Protocol。它需要与服务器保持一致。
		}
	end

	if node.transport == "quic" then
		v2ray_transport = {
			type = "quic"
		}
		--没有额外的加密支持： 它基本上是重复加密。 并且 Xray-core 在这里与 v2ray-core 不兼容。
	end

	if node.transport == "grpc" then
		v2ray_transport = {
			type = "grpc",
			service_name = node.grpc_serviceName,
		}
	end

	local inbound = {
		type = node.protocol,
		tag = "inbound",
		listen = (node.bind_local == "1") and "127.0.0.1" or "::",
		listen_port = tonumber(node.port),
	}

	local protocol_table = nil

	if node.protocol == "mixed" then
		protocol_table = {
			users = (node.auth == "1") and {
				{
					username = node.username,
					password = node.password
				}
			} or nil,
			set_system_proxy = false
		}
	end

	if node.protocol == "socks" then
		protocol_table = {
			users = (node.auth == "1") and {
				{
					username = node.username,
					password = node.password
				}
			} or nil
		}
	end

	if node.protocol == "http" then
		protocol_table = {
			users = (node.auth == "1") and {
				{
					username = node.username,
					password = node.password
				}
			} or nil,
			tls = (node.tls == "1") and tls or nil,
		}
	end

	if node.protocol == "shadowsocks" then
		protocol_table = {
			method = node.method,
			password = node.password,
		}
	end

	if node.protocol == "vmess" then
		if node.uuid then
			local users = {}
			for i = 1, #node.uuid do
				users[i] = {
					name = node.uuid[i],
					uuid = node.uuid[i],
					alterId = 0,
				}
			end
			protocol_table = {
				users = users,
				tls = (node.tls == "1") and tls or nil,
				transport = v2ray_transport,
			}
		end
	end

	if node.protocol == "vless" then
		if node.uuid then
			local users = {}
			for i = 1, #node.uuid do
				users[i] = {
					name = node.uuid[i],
					uuid = node.uuid[i],
					flow = node.flow,
				}
			end
			protocol_table = {
				users = users,
				tls = (node.tls == "1") and tls or nil,
				transport = v2ray_transport,
			}
		end
	end

	if node.protocol == "trojan" then
		if node.uuid then
			local users = {}
			for i = 1, #node.uuid do
				users[i] = {
					name = node.uuid[i],
					password = node.uuid[i],
				}
			end
			protocol_table = {
				users = users,
				tls = (node.tls == "1") and tls or nil,
				fallback = nil,
				fallback_for_alpn = nil,
				transport = v2ray_transport,
			}
		end
	end

	if node.protocol == "naive" then
		protocol_table = {
			users = {
				{
					username = node.username,
					password = node.password
				}
			},
			tls = tls,
		}
	end

	if node.protocol == "hysteria" then
		tls.alpn = (node.hysteria_alpn and node.hysteria_alpn ~= "") and {
			node.hysteria_alpn
		} or nil
		protocol_table = {
			up = node.hysteria_up_mbps .. " Mbps",
			down = node.hysteria_down_mbps .. " Mbps",
			up_mbps = tonumber(node.hysteria_up_mbps),
			down_mbps = tonumber(node.hysteria_down_mbps),
			obfs = node.hysteria_obfs,
			users = {
				{
					name = "user1",
					auth = (node.hysteria_auth_type == "base64") and node.hysteria_auth_password or nil,
					auth_str = (node.hysteria_auth_type == "string") and node.hysteria_auth_password or nil,
				}
			},
			recv_window_conn = node.hysteria_recv_window_conn and tonumber(node.hysteria_recv_window_conn) or nil,
			recv_window_client = node.hysteria_recv_window_client and tonumber(node.hysteria_recv_window_client) or nil,
			max_conn_client = node.hysteria_max_conn_client and tonumber(node.hysteria_max_conn_client) or nil,
			disable_mtu_discovery = (node.hysteria_disable_mtu_discovery == "1") and true or false,
			tls = tls
		}
	end

	if node.protocol == "tuic" then
		tls.alpn = (node.tuic_alpn and node.tuic_alpn ~= "") and {
			node.tuic_alpn
		} or nil
		protocol_table = {
			users = {
				{
					name = "user1",
					uuid = node.uuid,
					password = node.password
				}
			},
			congestion_control = node.tuic_congestion_control or "cubic",
			zero_rtt_handshake = (node.tuic_zero_rtt_handshake == "1") and true or false,
			heartbeat = node.tuic_heartbeat .. "s",
			tls = tls
		}
	end

	if node.protocol == "hysteria2" then
		protocol_table = {
			up_mbps = (node.hysteria2_ignore_client_bandwidth ~= "1" and node.hysteria2_up_mbps and tonumber(node.hysteria2_up_mbps)) and tonumber(node.hysteria2_up_mbps) or nil,
			down_mbps = (node.hysteria2_ignore_client_bandwidth ~= "1" and node.hysteria2_down_mbps and tonumber(node.hysteria2_down_mbps)) and tonumber(node.hysteria2_down_mbps) or nil,
			obfs = {
				type = node.hysteria2_obfs_type,
				password = node.hysteria2_obfs_password
			},
			users = {
				{
					name = "user1",
					password = node.hysteria2_auth_password or nil,
				}
			},
			ignore_client_bandwidth = (node.hysteria2_ignore_client_bandwidth == "1") and true or false,
			tls = tls
		}
	end

	if node.protocol == "direct" then
		protocol_table = {
			network = (node.d_protocol ~= "TCP,UDP") and node.d_protocol or nil,
			override_address = node.d_address,
			override_port = tonumber(node.d_port)
		}
	end

	if protocol_table then
		for key, value in pairs(protocol_table) do
			inbound[key] = value
		end
	end

	local route = {
		rules = {
			{
				ip_cidr = { "10.0.0.0/8", "172.16.0.0/12", "192.168.0.0/16" },
				outbound = (node.accept_lan == nil or node.accept_lan == "0") and "block" or "direct"
			}
		}
	}

	if node.outbound_node and node.outbound_node ~= "nil" then
		local outbound = nil
		if node.outbound_node == "_iface" and node.outbound_node_iface then
			outbound = {
				type = "direct",
				tag = "outbound",
				bind_interface = node.outbound_node_iface,
				routing_mark = 255,
			}
			sys.call("mkdir -p /tmp/etc/passwall/iface && touch /tmp/etc/passwall/iface/" .. node.outbound_node_iface)
		else
			local outbound_node_t = uci:get_all("passwall", node.outbound_node)
			if node.outbound_node == "_socks" or node.outbound_node == "_http" then
				outbound_node_t = {
					type = node.type,
					protocol = node.outbound_node:gsub("_", ""),
					address = node.outbound_node_address,
					port = tonumber(node.outbound_node_port),
					username = (node.outbound_node_username and node.outbound_node_username ~= "") and node.outbound_node_username or nil,
					password = (node.outbound_node_password and node.outbound_node_password ~= "") and node.outbound_node_password or nil,
				}
			end
			outbound = require("luci.passwall.util_sing-box").gen_outbound(nil, outbound_node_t, "outbound")
		end
		if outbound then
			route.final = "outbound"
			table.insert(outbounds, 1, outbound)
		end
	end

	local config = {
		log = {
			disabled = (not node or node.log == "0") and true or false,
			level = node.loglevel or "info",
			timestamp = true,
			--output = logfile,
		},
		inbounds = { inbound },
		outbounds = outbounds,
		route = route
	}

	for index, value in ipairs(config.outbounds) do
		for k, v in pairs(config.outbounds[index]) do
			if k:find("_") == 1 then
				config.outbounds[index][k] = nil
			end
		end
	end

	return config
end

function gen_config(var)
	local flag = var["-flag"]
	local log = var["-log"] or "0"
	local loglevel = var["-loglevel"] or "warn"
	local logfile = var["-logfile"] or "/dev/null"
	local node_id = var["-node"]
	local tcp_proxy_way = var["-tcp_proxy_way"]
	local tcp_redir_port = var["-tcp_redir_port"]
	local udp_redir_port = var["-udp_redir_port"]
	local local_socks_address = var["-local_socks_address"] or "0.0.0.0"
	local local_socks_port = var["-local_socks_port"]
	local local_socks_username = var["-local_socks_username"]
	local local_socks_password = var["-local_socks_password"]
	local local_http_address = var["-local_http_address"] or "0.0.0.0"
	local local_http_port = var["-local_http_port"]
	local local_http_username = var["-local_http_username"]
	local local_http_password = var["-local_http_password"]
	local dns_listen_port = var["-dns_listen_port"]
	local direct_dns_port = var["-direct_dns_port"]
	local direct_dns_udp_server = var["-direct_dns_udp_server"]
	local direct_dns_query_strategy = var["-direct_dns_query_strategy"]
	local remote_dns_port = var["-remote_dns_port"]
	local remote_dns_udp_server = var["-remote_dns_udp_server"]
	local remote_dns_tcp_server = var["-remote_dns_tcp_server"]
	local remote_dns_doh_url = var["-remote_dns_doh_url"]
	local remote_dns_doh_host = var["-remote_dns_doh_host"]
	local remote_dns_query_strategy = var["-remote_dns_query_strategy"]
	local remote_dns_fake = var["-remote_dns_fake"]
	local dns_cache = var["-dns_cache"]
	local dns_socks_address = var["-dns_socks_address"]
	local dns_socks_port = var["-dns_socks_port"]
	local tags = var["-tags"]

	local dns_domain_rules = {}
	local dns = nil
	local inbounds = {}
	local outbounds = {}

	local singbox_settings = uci:get_all(appname, "@global_singbox[0]") or {}

	local route = {
		rules = {},
		geoip = {
			path = singbox_settings.geoip_path or "/usr/share/singbox/geoip.db",
			download_url = singbox_settings.geoip_url or nil,
			download_detour = nil,
		},
		geosite = {
			path = singbox_settings.geosite_path or "/usr/share/singbox/geosite.db",
			download_url = singbox_settings.geosite_url or nil,
			download_detour = nil,
		},
	}

	local experimental = nil

	local default_outTag = nil
	if node_id then
		local node = uci:get_all(appname, node_id)

		if local_socks_port then
			local inbound = {
				type = "socks",
				tag = "socks-in",
				listen = local_socks_address,
				listen_port = tonumber(local_socks_port),
				sniff = true
			}
			if local_socks_username and local_socks_password and local_socks_username ~= "" and local_socks_password ~= "" then
				inbound.users = {
					{
						username = local_socks_username,
						password = local_socks_password
					}
				}
			end
			table.insert(inbounds, inbound)
		end

		if local_http_port then
			local inbound = {
				type = "http",
				tag = "http-in",
				listen = local_http_address,
				listen_port = tonumber(local_http_port)
			}
			if local_http_username and local_http_password and local_http_username ~= "" and local_http_password ~= "" then
				inbound.users = {
					{
						username = local_http_username,
						password = local_http_password
					}
				}
			end
			table.insert(inbounds, inbound)
		end

		if tcp_redir_port then
			if tcp_proxy_way ~= "tproxy" then
				local inbound = {
					type = "redirect",
					tag = "redirect_tcp",
					listen = "::",
					listen_port = tonumber(tcp_redir_port),
					sniff = true,
					sniff_override_destination = (singbox_settings.sniff_override_destination == "1") and true or false,
				}
				table.insert(inbounds, inbound)
			else
				local inbound = {
					type = "tproxy",
					tag = "tproxy_tcp",
					network = "tcp",
					listen = "::",
					listen_port = tonumber(tcp_redir_port),
					sniff = true,
					sniff_override_destination = (singbox_settings.sniff_override_destination == "1") and true or false,
				}
				table.insert(inbounds, inbound)
			end
		end

		if udp_redir_port then
			local inbound = {
				type = "tproxy",
				tag = "tproxy_udp",
				network = "udp",
				listen = "::",
				listen_port = tonumber(udp_redir_port),
				sniff = true,
				sniff_override_destination = (singbox_settings.sniff_override_destination == "1") and true or false,
			}
			table.insert(inbounds, inbound)
		end

		if node.protocol == "_shunt" then
			local rules = {}

			local preproxy_enabled = node.preproxy_enabled == "1"
			local preproxy_tag = "main"
			local preproxy_node_id = node["main_node"]
			local preproxy_node = preproxy_enabled and preproxy_node_id and uci:get_all(appname, preproxy_node_id) or nil

			if not preproxy_node and preproxy_node_id and api.parseURL(preproxy_node_id) then
				local parsed1 = api.parseURL(preproxy_node_id)
				local _node = {
					type = "sing-box",
					protocol = parsed1.protocol,
					username = parsed1.username,
					password = parsed1.password,
					address = parsed1.host,
					port = parsed1.port,
				}
				local preproxy_outbound = gen_outbound(flag, _node, preproxy_tag)
				if preproxy_outbound then
					table.insert(outbounds, preproxy_outbound)
				else
					preproxy_enabled = false
				end
			elseif preproxy_node and api.is_normal_node(preproxy_node) then
				local preproxy_outbound = gen_outbound(flag, preproxy_node, preproxy_tag)
				if preproxy_outbound then
					if preproxy_node.shadowtls == "1" then
						local _node = {
							type = "sing-box",
							protocol = "shadowtls",
							shadowtls_version = preproxy_node.shadowtls_version,
							password = (preproxy_node.shadowtls_version == "2" or preproxy_node.shadowtls_version == "3") and preproxy_node.shadowtls_password or nil,
							address = preproxy_node.address,
							port = preproxy_node.port,
							tls = "1",
							tls_serverName = preproxy_node.shadowtls_serverName,
							utls = preproxy_node.shadowtls_utls,
							fingerprint = preproxy_node.shadowtls_fingerprint
						}
						local shadowtls_outbound = gen_outbound(flag, _node, preproxy_tag .. "_shadowtls")
						if shadowtls_outbound then
							table.insert(outbounds, shadowtls_outbound)
							preproxy_outbound.detour = preproxy_outbound.tag .. "_shadowtls"
							preproxy_outbound.server = nil
							preproxy_outbound.server_port = nil
						end
					end
					table.insert(outbounds, preproxy_outbound)
				else
					preproxy_enabled = false
				end
			end

			local function gen_shunt_node(rule_name, _node_id, as_proxy)
				if not rule_name then return nil, nil end
				if not _node_id then _node_id = node[rule_name] or "nil" end
				local rule_outboundTag
				if _node_id == "_direct" then
					rule_outboundTag = "direct"
				elseif _node_id == "_blackhole" then
					rule_outboundTag = "block"
				elseif _node_id == "_default" and rule_name ~= "default" then
					rule_outboundTag = "default"
				elseif api.parseURL(_node_id) then
					local parsed1 = api.parseURL(_node_id)
					local _node = {
						type = "sing-box",
						protocol = parsed1.protocol,
						username = parsed1.username,
						password = parsed1.password,
						address = parsed1.host,
						port = parsed1.port,
					}
					local _outbound = gen_outbound(flag, _node, rule_name)
					if _outbound then
						table.insert(outbounds, _outbound)
						rule_outboundTag = rule_name
					end
				elseif _node_id ~= "nil" then
					local _node = uci:get_all(appname, _node_id)
					if not _node then return nil, nil end

					if api.is_normal_node(_node) then
						local proxy = preproxy_enabled and node[rule_name .. "_proxy_tag"] == preproxy_tag and _node_id ~= preproxy_node_id
						local copied_outbound
						for index, value in ipairs(outbounds) do
							if value["_flag_tag"] == _node_id and value["_flag_proxy_tag"] == preproxy_tag then
								copied_outbound = api.clone(value)
								break
							end
						end
						if copied_outbound then
							copied_outbound.tag = rule_name
							table.insert(outbounds, copied_outbound)
							rule_outboundTag = rule_name
						else
							if proxy then
								local pre_proxy = nil
								if _node.type ~= "sing-box" then
									pre_proxy = true
								else
									if _node.flow == "xtls-rprx-vision" then
										pre_proxy = true
									end
								end
								if pre_proxy then
									new_port = get_new_port()
									table.insert(inbounds, {
										type = "direct",
										tag = "proxy_" .. rule_name,
										listen = "127.0.0.1",
										listen_port = new_port,
										override_address = _node.address,
										override_port = tonumber(_node.port),
									})
									if _node.tls_serverName == nil then
										_node.tls_serverName = _node.address
									end
									_node.address = "127.0.0.1"
									_node.port = new_port
									table.insert(rules, 1, {
										inbound = {"proxy_" .. rule_name},
										outbound = preproxy_tag,
									})
								end
							end
							local _outbound = gen_outbound(flag, _node, rule_name, { proxy = proxy and 1 or 0, tag = proxy and preproxy_tag or nil })
							if _outbound then
								if _node.shadowtls == "1" then
									local shadowtls_node = {
										type = "sing-box",
										protocol = "shadowtls",
										shadowtls_version = _node.shadowtls_version,
										password = (_node.shadowtls_version == "2" or _node.shadowtls_version == "3") and _node.shadowtls_password or nil,
										address = _node.address,
										port = _node.port,
										tls = "1",
										tls_serverName = _node.shadowtls_serverName,
										utls = _node.shadowtls_utls,
										fingerprint = _node.shadowtls_fingerprint
									}
									local shadowtls_outbound = gen_outbound(flag, shadowtls_node, rule_name .. "_shadowtls", { proxy = proxy and 1 or 0, tag = proxy and preproxy_tag or nil })
									if shadowtls_outbound then
										table.insert(outbounds, shadowtls_outbound)
										_outbound.detour = _outbound.tag .. "_shadowtls"
										_outbound.server = nil
										_outbound.server_port = nil
									end
								end
								table.insert(outbounds, _outbound)
								rule_outboundTag = rule_name
							end
						end
					elseif _node.protocol == "_iface" then
						if _node.iface then
							local _outbound = {
								type = "direct",
								tag = rule_name,
								bind_interface = _node.iface,
								routing_mark = 255,
							}
							table.insert(outbounds, _outbound)
							rule_outboundTag = rule_name
							sys.call("touch /tmp/etc/passwall/iface/" .. _node.iface)
						end
					end
				end
				return rule_outboundTag
			end
			--default_node
			local default_node_id = node.default_node or "_direct"
			local default_outboundTag = gen_shunt_node("default", default_node_id)
			--shunt rule
			uci:foreach(appname, "shunt_rules", function(e)
				local outboundTag = gen_shunt_node(e[".name"])
				if outboundTag and e.remarks then
					if outboundTag == "default" then
						outboundTag = default_outboundTag
					end
					local protocols = nil
					if e["protocol"] and e["protocol"] ~= "" then
						protocols = {}
						string.gsub(e["protocol"], '[^' .. " " .. ']+', function(w)
							table.insert(protocols, w)
						end)
					end
					
					local rule = {
						outbound = outboundTag,
						invert = false, --匹配反选
						protocol = protocols
					}

					if e.network then
						local network = {}
						string.gsub(e.network, '[^' .. "," .. ']+', function(w)
							table.insert(network, w)
						end)
						rule.network = network
					end

					if e.source then
						local source_geoip = {}
						local source_ip_cidr = {}
						string.gsub(e.source, '[^' .. " " .. ']+', function(w)
							if w:find("geoip") == 1 then
								table.insert(source_geoip, w)
							else
								table.insert(source_ip_cidr, w)
							end
						end)
						rule.source_geoip = #source_geoip > 0 and source_geoip or nil
						rule.source_ip_cidr = #source_ip_cidr > 0 and source_ip_cidr or nil
					end

					if e.sourcePort then
						local source_port = {}
						local source_port_range = {}
						string.gsub(e.sourcePort, '[^' .. "," .. ']+', function(w)
							if tonumber(w) and tonumber(w) >= 1 and tonumber(w) <= 65535 then
								table.insert(source_port, tonumber(w))
							else
								table.insert(source_port_range, w)
							end
						end)
						rule.source_port = #source_port > 0 and source_port or nil
						rule.source_port_range = #source_port_range > 0 and source_port_range or nil
					end

					if e.port then
						local port = {}
						local port_range = {}
						string.gsub(e.port, '[^' .. "," .. ']+', function(w)
							if tonumber(w) and tonumber(w) >= 1 and tonumber(w) <= 65535 then
								table.insert(port, tonumber(w))
							else
								table.insert(port_range, w)
							end
						end)
						rule.port = #port > 0 and port or nil
						rule.port_range = #port_range > 0 and port_range or nil
					end

					if e.domain_list then
						local domain_table = {
							outboundTag = outboundTag,
							domain = {},
							domain_suffix = {},
							domain_keyword = {},
							domain_regex = {},
							geosite = {},
						}
						string.gsub(e.domain_list, '[^' .. "\r\n" .. ']+', function(w)
							if w:find("geosite:") == 1 then
								table.insert(domain_table.geosite, w:sub(1 + #"geosite:"))
							elseif w:find("regexp:") == 1 then
								table.insert(domain_table.domain_regex, w:sub(1 + #"regexp:"))
							elseif w:find("full:") == 1 then
								table.insert(domain_table.domain, w:sub(1 + #"full:"))
							elseif w:find("domain:") == 1 then
								table.insert(domain_table.domain, w:sub(1 + #"domain:"))
								table.insert(domain_table.domain_suffix, "." .. w:sub(1 + #"domain:"))
							else
								table.insert(domain_table.domain_keyword, w)
							end
						end)
						rule.domain = #domain_table.domain > 0 and domain_table.domain or nil
						rule.domain_suffix = #domain_table.domain_suffix > 0 and domain_table.domain_suffix or nil
						rule.domain_keyword = #domain_table.domain_keyword > 0 and domain_table.domain_keyword or nil
						rule.domain_regex = #domain_table.domain_regex > 0 and domain_table.domain_regex or nil
						rule.geosite = #domain_table.geosite > 0 and domain_table.geosite or nil

						if outboundTag and outboundTag ~= "nil" then
							table.insert(dns_domain_rules, api.clone(domain_table))
						end
					end

					if e.ip_list then
						local ip_cidr = {}
						local geoip = {}
						string.gsub(e.ip_list, '[^' .. "\r\n" .. ']+', function(w)
							if w:find("geoip:") == 1 then
								table.insert(geoip, w:sub(1 + #"geoip:"))
							else
								table.insert(ip_cidr, w)
							end
						end)

						rule.ip_cidr = #ip_cidr > 0 and ip_cidr or nil
						rule.geoip = #geoip > 0 and geoip or nil
					end

					table.insert(rules, rule)
				end
			end)

			if default_outboundTag then
				route.final = default_outboundTag
				default_outTag = default_outboundTag
			end

			for index, value in ipairs(rules) do
				table.insert(route.rules, rules[index])
			end
		else
			local outbound = nil
			if node.protocol == "_iface" then
				if node.iface then
					outbound = {
						type = "direct",
						tag = "outbound",
						bind_interface = node.iface,
						routing_mark = 255,
					}
					sys.call("touch /tmp/etc/passwall/iface/" .. node.iface)
				end
			else
				outbound = gen_outbound(flag, node)
				if outbound then
					if node.shadowtls == "1" then
						local shadowtls_node = {
							type = "sing-box",
							protocol = "shadowtls",
							shadowtls_version = node.shadowtls_version,
							password = (node.shadowtls_version == "2" or node.shadowtls_version == "3") and node.shadowtls_password or nil,
							address = node.address,
							port = node.port,
							tls = "1",
							tls_serverName = node.shadowtls_serverName,
							utls = node.shadowtls_utls,
							fingerprint = node.shadowtls_fingerprint
						}
						local shadowtls_outbound = gen_outbound(flag, shadowtls_node, outbound.tag .. "_shadowtls")
						if shadowtls_outbound then
							table.insert(outbounds, shadowtls_outbound)
							outbound.detour = outbound.tag .. "_shadowtls"
							outbound.server = nil
							outbound.server_port = nil
						end
					end
				end
			end
			if outbound then
				default_outTag = outbound.tag
				table.insert(outbounds, outbound)
			end

			route.final = node_id
		end
	end

	if dns_listen_port then
		dns = {
			servers = {},
			rules = {},
			disable_cache = (dns_cache and dns_cache == "0") and true or false,
			disable_expire = false, --禁用 DNS 缓存过期。
			independent_cache = false, --使每个 DNS 服务器的缓存独立，以满足特殊目的。如果启用，将轻微降低性能。
			reverse_mapping = true, --在响应 DNS 查询后存储 IP 地址的反向映射以为路由目的提供域名。
			fakeip = nil,
		}

		table.insert(dns.servers, {
			tag = "block",
			address = "rcode://success",
		})

		if dns_socks_address and dns_socks_port then
			default_outTag = "dns_socks_out"
			table.insert(outbounds, 1, {
				type = "socks",
				tag = default_outTag,
				server = dns_socks_address,
				server_port = tonumber(dns_socks_port)
			})
		end

		local remote_strategy = "prefer_ipv6"
		if remote_dns_query_strategy == "UseIPv4" then
			remote_strategy = "ipv4_only"
		elseif remote_dns_query_strategy == "UseIPv6" then
			remote_strategy = "ipv6_only"
		end

		local remote_server = {
			tag = "remote",
			address_strategy = "prefer_ipv4",
			strategy = remote_strategy,
			address_resolver = "direct",
			detour = default_outTag,
		}

		if remote_dns_udp_server then
			local server_port = tonumber(remote_dns_port) or 53
			remote_server.address = "udp://" .. remote_dns_udp_server .. ":" .. server_port
		end

		if remote_dns_tcp_server then
			remote_server.address = remote_dns_tcp_server
		end

		if remote_dns_doh_url and remote_dns_doh_host then
			remote_server.address = remote_dns_doh_url
		end

		if remote_server.address then
			table.insert(dns.servers, remote_server)
		end

		local fakedns_tag = "remote_fakeip"
		if remote_dns_fake then
			dns.fakeip = {
				enabled = true,
				inet4_range = "198.18.0.0/16",
				inet6_range = "fc00::/18",
			}
			
			table.insert(dns.servers, {
				tag = fakedns_tag,
				address = "fakeip",
				strategy = remote_strategy,
			})

			if tags and tags:find("with_clash_api") then
				if not experimental then
					experimental = {}
				end
				experimental.clash_api = {
					store_fakeip = true,
					cache_file = "/tmp/singbox_passwall_" .. flag .. ".db"
				}
			end
		end
	
		if direct_dns_udp_server then
			local domain = {}
			local nodes_domain_text = sys.exec('uci show passwall | grep ".address=" | cut -d "\'" -f 2 | grep "[a-zA-Z]$" | sort -u')
			string.gsub(nodes_domain_text, '[^' .. "\r\n" .. ']+', function(w)
				table.insert(domain, w)
			end)
			if #domain > 0 then
				table.insert(dns_domain_rules, 1, {
					outboundTag = "direct",
					domain = domain
				})
			end
	
			local direct_strategy = "prefer_ipv6"
			if direct_dns_query_strategy == "UseIPv4" then
				direct_strategy = "ipv4_only"
			elseif direct_dns_query_strategy == "UseIPv6" then
				direct_strategy = "ipv6_only"
			end
	
			local port = tonumber(direct_dns_port) or 53
	
			table.insert(dns.servers, {
				tag = "direct",
				address = "udp://" .. direct_dns_udp_server .. ":" .. port,
				address_strategy = "prefer_ipv6",
				strategy = direct_strategy,
				detour = "direct",
			})
		end

		local default_dns_flag = "remote"
		if dns_socks_address and dns_socks_port then
		else
			if node_id and (tcp_redir_port or udp_redir_port) then
				local node = uci:get_all(appname, node_id)
				if node.protocol == "_shunt" then
					if node.default_node == "_direct" then
						default_dns_flag = "direct"
					end
				end
			else default_dns_flag = "direct"
			end
		end
		if default_dns_flag == "remote" then
			if remote_dns_fake then
				table.insert(dns.rules, {
					query_type = { "A", "AAAA" },
					server = fakedns_tag
				})
			end
		end
		dns.final = default_dns_flag

		--按分流顺序DNS
		if dns_domain_rules and #dns_domain_rules > 0 then
			for index, value in ipairs(dns_domain_rules) do
				if value.outboundTag and (value.domain or value.domain_suffix or value.domain_keyword or value.domain_regex or value.geosite) then
					local dns_rule = {
						server = value.outboundTag,
						domain = (value.domain and #value.domain > 0) and value.domain or nil,
						domain_suffix = (value.domain_suffix and #value.domain_suffix > 0) and value.domain_suffix or nil,
						domain_keyword = (value.domain_keyword and #value.domain_keyword > 0) and value.domain_keyword or nil,
						domain_regex = (value.domain_regex and #value.domain_regex > 0) and value.domain_regex or nil,
						geosite = (value.geosite and #value.geosite > 0) and value.geosite or nil,
						disable_cache = false,
					}
					if value.outboundTag ~= "block" and value.outboundTag ~= "direct" then
						dns_rule.server = "remote"
						if value.outboundTag ~= "default" and remote_server.address then
							local remote_dns_server = api.clone(remote_server)
							remote_dns_server.tag = value.outboundTag
							remote_dns_server.detour = value.outboundTag
							table.insert(dns.servers, remote_dns_server)
							dns_rule.server = remote_dns_server.tag
						end
						if remote_dns_fake then
							local fakedns_dns_rule = api.clone(dns_rule)
							fakedns_dns_rule.query_type = {
								"A", "AAAA"
							}
							fakedns_dns_rule.server = fakedns_tag
							fakedns_dns_rule.disable_cache = true
							table.insert(dns.rules, fakedns_dns_rule)
						end
					end
					table.insert(dns.rules, dns_rule)
				end
			end
		end
	
		table.insert(inbounds, {
			type = "direct",
			tag = "dns-in",
			listen = "127.0.0.1",
			listen_port = tonumber(dns_listen_port),
			sniff = true,
		})
		table.insert(outbounds, {
			type = "dns",
			tag = "dns-out",
		})
		table.insert(route.rules, 1, {
			protocol = "dns",
			inbound = {
				"dns-in"
			},
			outbound = "dns-out"
		})
	end
	
	if inbounds or outbounds then
		local config = {
			log = {
				disabled = log == "0" and true or false,
				level = loglevel,
				timestamp = true,
				output = logfile,
			},
			-- DNS
			dns = dns,
			-- 传入连接
			inbounds = inbounds,
			-- 传出连接
			outbounds = outbounds,
			-- 路由
			route = route,
			--实验性
			experimental = experimental,
		}
		table.insert(outbounds, {
			type = "direct",
			tag = "direct",
			routing_mark = 255,
			domain_strategy = "prefer_ipv6",
		})
		table.insert(outbounds, {
			type = "block",
			tag = "block"
		})
		for index, value in ipairs(config.outbounds) do
			for k, v in pairs(config.outbounds[index]) do
				if k:find("_") == 1 then
					config.outbounds[index][k] = nil
				end
			end
		end
		return jsonc.stringify(config, 1)
	end
end

function gen_proto_config(var)
	local local_socks_address = var["-local_socks_address"] or "0.0.0.0"
	local local_socks_port = var["-local_socks_port"]
	local local_socks_username = var["-local_socks_username"]
	local local_socks_password = var["-local_socks_password"]
	local local_http_address = var["-local_http_address"] or "0.0.0.0"
	local local_http_port = var["-local_http_port"]
	local local_http_username = var["-local_http_username"]
	local local_http_password = var["-local_http_password"]
	local server_proto = var["-server_proto"]
	local server_address = var["-server_address"]
	local server_port = var["-server_port"]
	local server_username = var["-server_username"]
	local server_password = var["-server_password"]

	local inbounds = {}
	local outbounds = {}

	if local_socks_address and local_socks_port then
		local inbound = {
			type = "socks",
			tag = "socks-in",
			listen = local_socks_address,
			listen_port = tonumber(local_socks_port),
		}
		if local_socks_username and local_socks_password and local_socks_username ~= "" and local_socks_password ~= "" then
			inbound.users = {
				username = local_socks_username,
				password = local_socks_password
			}
		end
		table.insert(inbounds, inbound)
	end

	if local_http_address and local_http_port then
		local inbound = {
			type = "http",
			tag = "http-in",
			tls = nil,
			listen = local_http_address,
			listen_port = tonumber(local_http_port),
		}
		if local_http_username and local_http_password and local_http_username ~= "" and local_http_password ~= "" then
			inbound.users = {
				{
					username = local_http_username,
					password = local_http_password
				}
			}
		end
		table.insert(inbounds, inbound)
	end

	if server_proto ~= "nil" and server_address ~= "nil" and server_port ~= "nil" then
		local outbound = {
			type = server_proto,
			tag = "out",
			server = server_address,
			server_port = tonumber(server_port),
			username = (server_username and server_password) and server_username or nil,
			password = (server_username and server_password) and server_password or nil,
		}
		if outbound then table.insert(outbounds, outbound) end
	end
	
	local config = {
		log = {
			disabled = true,
			level = "warn",
			timestamp = true,
		},
		-- 传入连接
		inbounds = inbounds,
		-- 传出连接
		outbounds = outbounds,
	}
	return jsonc.stringify(config, 1)
end

_G.gen_config = gen_config
_G.gen_proto_config = gen_proto_config

if arg[1] then
	local func =_G[arg[1]]
	if func then
		print(func(api.get_function_args(arg)))
	end
end
