local e=require"luci.model.uci".cursor()
local a=require"luci.jsonc"
local t=arg[1]
local e=e:get_all("v2ray_server",t)

local e={
	log = {
			loglevel = "warning"
		},
		inbound = {
			port = tonumber(e.port),
			protocol = e.protocol,
			(e.protocol == 'vmess') and {
			settings = {
				clients = {
					{
						id = e.VMess_id,
						alterId = tonumber(e.VMess_alterId),
						level = tonumber(e.VMess_level)
					}
				}
			}
		},
		(e.protocol == 'socks') and {
			settings = {
        auth= "password",
				accounts = {
					{
						user = e.Socks_user,
						pass = e.Socks_pass
					}
				}
			}
		},
			streamSettings = {
				network = e.transport,
				security = (e.tls == '1') and "tls" or "none",
				kcpSettings = (e.transport == "mkcp") and {
					mtu = tonumber(e.mkcp_mtu),
						tti = tonumber(e.mkcp_tti),
						uplinkCapacity = tonumber(e.mkcp_uplinkCapacity),
						downlinkCapacity = tonumber(e.mkcp_downlinkCapacity),
						congestion = (e.mkcp_congestion == "1") and true or false,
						readBufferSize = tonumber(e.mkcp_readBufferSize),
						writeBufferSize = tonumber(e.mkcp_writeBufferSize),
						header = {
							type = e.mkcp_guise
						}
				}
				or nil,
				httpSettings = (e.transport == "h2") and {
					path = e.h2_path,
						host = e.h2_host,
				}
				or nil,
				quicSettings = (e.transport == "quic") and {
					security = e.quic_security,
						key = e.quic_key,
						header = {
							type = e.quic_guise
						}
				}
				or nil
			}
		},
		outbound = {
			protocol = "freedom"
		},
		outboundDetour = {
			{
				protocol = "blackhole",
					tag = "blocked"
			}
		}
}

print(a.stringify(e,1))