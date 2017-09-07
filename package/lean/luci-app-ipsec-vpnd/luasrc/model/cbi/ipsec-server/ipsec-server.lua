
--require("luci.tools.webadmin")

mp = Map("ipsec", translate("IPSec VPN Server"))
mp.description = translate("IPSec VPN connectivity using the native built-in VPN Client on iOS or Andriod (IKEv1 with PSK and Xauth)")

s = mp:section(NamedSection, "ipsec", "service")
s.anonymouse = true
--s.addremove = true

enabled = s:option(Flag, "enabled", translate("Enable"))
enabled.default = 0
enabled.rmempty = false

clientip = s:option(Value, "clientip", translate("VPN Client IP"))
clientip.datatype = "ip4addr"
clientip.description = translate("LAN DHCP reserved started IP addresses with the same subnet mask")
clientip.optional = false
clientip.rmempty = false

clientdns = s:option(Value, "clientdns", translate("VPN Client DNS"))
clientdns.datatype = "ip4addr"
clientdns.description = translate("DNS using in VPN tunnel.Set to the router's LAN IP is recommended")
clientdns.optional = false
clientdns.rmempty = false

account = s:option(Value, "account", translate("Account"))
account.datatype = "string"

password = s:option(Value, "password", translate("Password"))
password.password = true

secret = s:option(Value, "secret", translate("Secret Pre-Shared Key"))
secret.password = true


function mp.on_save(self)
    require "luci.model.uci"
    require "luci.sys"

	local have_ike_rule = false
	local have_ipsec_rule = false
	local have_ah_rule = false
	local have_esp_rule = false

    luci.model.uci.cursor():foreach('firewall', 'rule',
        function (section)
			if section.name == 'ike' then
				have_ike_rule = true
			end
			if section.name == 'ipsec' then
				have_ipsec_rule = true
			end
			if section.name == 'ah' then
				have_ah_rule = true
			end
			if section.name == 'esp' then
				have_esp_rule = true
			end
        end
    )

	if not have_ike_rule then
		local cursor = luci.model.uci.cursor()
		local ike_rulename = cursor:add('firewall','rule')
		cursor:tset('firewall', ike_rulename, {
			['name'] = 'ike',
			['target'] = 'ACCEPT',
			['src'] = 'wan',
			['proto'] = 'udp',
			['dest_port'] = 500
		})
		cursor:save('firewall')
		cursor:commit('firewall')
	end
  if not have_ipsec_rule then
		local cursor = luci.model.uci.cursor()
		local ipsec_rulename = cursor:add('firewall','rule')
		cursor:tset('firewall', ipsec_rulename, {
			['name'] = 'ipsec',
			['target'] = 'ACCEPT',
			['src'] = 'wan',
			['proto'] = 'udp',
			['dest_port'] = 4500
		})
		cursor:save('firewall')
		cursor:commit('firewall')
	end
	if not have_ah_rule then
		local cursor = luci.model.uci.cursor()
		local ah_rulename = cursor:add('firewall','rule')
		cursor:tset('firewall', ah_rulename, {
			['name'] = 'ah',
			['target'] = 'ACCEPT',
			['src'] = 'wan',
			['proto'] = 'ah'
		})
		cursor:save('firewall')
		cursor:commit('firewall')
	end
  if not have_esp_rule then
		local cursor = luci.model.uci.cursor()
		local esp_rulename = cursor:add('firewall','rule')
		cursor:tset('firewall', esp_rulename, {
			['name'] = 'esp',
			['target'] = 'ACCEPT',
			['src'] = 'wan',
			['proto'] = 'esp'
		})
		cursor:save('firewall')
		cursor:commit('firewall')
	end
	
		
end


local pid = luci.util.exec("/usr/bin/pgrep ipsec")

function ipsec_process_status()
  local status = "IPSec VPN Server is not running now "

  if pid ~= "" then
      status = "IPSec VPN Server is running with the PID " .. pid .. ""
  end

  local status = { status=status }
  local table = { pid=status }
  return table
end

t = mp:section(Table, ipsec_process_status())
t.anonymous = true

t:option(DummyValue, "status", translate("IPSec VPN Server status"))

if pid == "" then
  start = t:option(Button, "_start", translate("Start"))
  start.inputstyle = "apply"
  function start.write(self, section)
        luci.util.exec("uci set ipsec.ipsec.enabled='1' &&  uci commit ipsec")
        message = luci.util.exec("/etc/init.d/ipsec start 2>&1")
        luci.util.exec("sleep 2")
        luci.http.redirect(
                luci.dispatcher.build_url("admin", "vpn", "ipsec-server")
        )
  end
else
  stop = t:option(Button, "_stop", translate("Stop"))
  stop.inputstyle = "reset"
  function stop.write(self, section)
        luci.util.exec("uci set ipsec.ipsec.enabled='0' &&  uci commit ipsec")
        luci.util.exec("/etc/init.d/ipsec stop")
        luci.util.exec("sleep 2")
        luci.http.redirect(
                luci.dispatcher.build_url("admin", "vpn", "ipsec-server")
        )
  end
end

function mp.on_after_commit(self)
  os.execute("/etc/ipsecvpn restart >/dev/null 2>&1 &")
  os.execute("/etc/init.d/firewall restart >/dev/null 2>&1 &")
end


return mp
