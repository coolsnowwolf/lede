local ucursor = require "luci.model.uci".cursor()
local json = require "luci.jsonc"
local server_section = arg[1]
local server = ucursor:get_all("shadowsocksr", server_section)

local v2ray = {
	log = {
		--error = "/var/log/v2ray.log",
		loglevel = "warning"
	},
	-- 传入连接
	inbound = {
		port = tonumber(server.server_port),
		protocol = "vmess",
		settings = {
			clients = {
				{
					id = server.vmess_id,
					alterId = tonumber(server.alter_id),
					level = tonumber(server.VMess_level)
				}
			}
		},
		-- 底层传输配置
		streamSettings = {
			network = server.transport,
			security = (server.tls == '1') and "tls" or "none",
			kcpSettings = (server.transport == "kcp") and {
				mtu = tonumber(server.mtu),
				tti = tonumber(server.tti),
				uplinkCapacity = tonumber(server.uplink_capacity),
				downlinkCapacity = tonumber(server.downlink_capacity),
				congestion = (server.congestion == "1") and true or false,
				readBufferSize = tonumber(server.read_buffer_size),
				writeBufferSize = tonumber(server.write_buffer_size),
				header = {
					type = server.kcp_guise
				}
			} or nil,
			httpSettings = (server.transport == "h2") and {
				path = server.h2_path,
				host = server.h2_host,
			} or nil,
			quicSettings = (server.transport == "quic") and {
				security = server.quic_security,
				key = server.quic_key,
				header = {
					type = server.quic_guise
				}
			} or nil
		}
	},
	-- 传出连接
	outbound = {
		protocol = "freedom"
	},
	-- 额外传出连接
	outboundDetour = {
		{
			protocol = "blackhole",
			tag = "blocked"
		}
	}
}
print(json.stringify(v2ray,1))