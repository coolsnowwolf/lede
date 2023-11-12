module("luci.passwall.util_tuic", package.seeall)
local api = require "luci.passwall.api"
local uci = api.uci
local json = api.jsonc

function gen_config(var)
	local node_id = var["-node"]
	if not node_id then
		print("-node 不能为空")
		return
	end
	local node = uci:get_all("passwall", node_id)
	local local_addr = var["-local_addr"]
	local local_port = var["-local_port"]
	local server_host = var["-server_host"] or node.address
	local server_port = var["-server_port"] or node.port
	local loglevel = var["-loglevel"] or "warn"

	local tuic= {
			relay = {
				server = server_host .. ":" .. server_port,
				ip = node.tuic_ip,
				uuid = node.uuid,
				password = node.tuic_password,
				-- certificates = node.tuic_certificate and { node.tuic_certpath } or nil,
				udp_relay_mode = node.tuic_udp_relay_mode,
				congestion_control = node.tuic_congestion_control,
				heartbeat = node.tuic_heartbeat .. "s",
				timeout = node.tuic_timeout .. "s",
				gc_interval = node.tuic_gc_interval .. "s",
				gc_lifetime = node.tuic_gc_lifetime .. "s",
				alpn = node.tuic_tls_alpn,
				disable_sni = (node.tuic_disable_sni == "1"),
				zero_rtt_handshake = (node.tuic_zero_rtt_handshake == "1"),
				send_window = tonumber(node.tuic_send_window),
				receive_window = tonumber(node.tuic_receive_window)
			},
			["local"] = {
				server = "[::]:" .. local_port,
				username = node.tuic_socks_username,
				password = node.tuic_socks_password,
				dual_stack = (node.tuic_dual_stack == "1") and true or false,
				max_packet_size = tonumber(node.tuic_max_package_size)
			},
			log_level = loglevel
	}
	return json.stringify(tuic, 1)
end

_G.gen_config = gen_config

if arg[1] then
	local func =_G[arg[1]]
	if func then
		print(func(api.get_function_args(arg)))
	end
end
