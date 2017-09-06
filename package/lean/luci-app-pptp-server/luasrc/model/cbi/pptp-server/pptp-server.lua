
--require("luci.tools.webadmin")

mp = Map("pptpd", "PPTP Server","")

s = mp:section(NamedSection, "pptpd", "service", translate("PPTP Service"))
s.anonymouse = true
--s.addremove = true

enabled = s:option(Flag, "enabled", translate("Enable"))
enabled.default = 0
enabled.rmempty = false

localip = s:option(Value, "localip", translate("Local IP"))
localip.datatype = "ip4addr"

clientip = s:option(Value, "remoteip", translate("Client IP"))
clientip.datatype = "string"
clientip.description = translate("LAN DHCP reserved start-to-end IP addresses with the same subnet mask")

remotedns = s:option(Value, "remotedns", translate("Remote Client DNS"))
remotedns.datatype = "ip4addr"

logging = s:option(Flag, "logwtmp", translate("Debug Logging"))
logging.default = 0
logging.rmempty = false

logins = mp:section(NamedSection, "login", "login", translate("PPTP Logins"))
logins.anonymouse = true

username = logins:option(Value, "username", translate("User name"))
username.datatype = "string"

password = logins:option(Value, "password", translate("Password"))
password.password = true

function mp.on_save(self)
    require "luci.model.uci"
    require "luci.sys"

	local have_pptp_rule = false
	local have_gre_rule = false

    luci.model.uci.cursor():foreach('firewall', 'rule',
        function (section)
			if section.name == 'pptp' then
				have_pptp_rule = true
			end
			if section.name == 'gre' then
				have_gre_rule = true
			end
        end
    )

	if not have_pptp_rule then
		local cursor = luci.model.uci.cursor()
		local pptp_rulename = cursor:add('firewall','rule')
		cursor:tset('firewall', pptp_rulename, {
			['name'] = 'pptp',
			['target'] = 'ACCEPT',
			['src'] = 'wan',
			['proto'] = 'tcp',
			['dest_port'] = 1723
		})
		cursor:save('firewall')
		cursor:commit('firewall')
	end
	if not have_gre_rule then
		local cursor = luci.model.uci.cursor()
		local gre_rulename = cursor:add('firewall','rule')
		cursor:tset('firewall', gre_rulename, {
			['name'] = 'gre',
			['target'] = 'ACCEPT',
			['src'] = 'wan',
			['proto'] = 47
		})
		cursor:save('firewall')
		cursor:commit('firewall')
	end
	
	
end


local pid = luci.util.exec("/usr/bin/pgrep pptpd")

function pptpd_process_status()
  local status = "PPTPD is not running now "

  if pid ~= "" then
      status = "PPTPD is running with the PID " .. pid .. ""
  end

  local status = { status=status }
  local table = { pid=status }
  return table
end

t = mp:section(Table, pptpd_process_status())
t.anonymous = true

t:option(DummyValue, "status", translate("PPTPD status"))

if pid == "" then
  start = t:option(Button, "_start", translate("Start"))
  start.inputstyle = "apply"
  function start.write(self, section)
        luci.util.exec("uci set pptpd.pptpd.enabled='1' &&  uci commit pptpd")
        message = luci.util.exec("/etc/init.d/pptpd start 2>&1")
        luci.util.exec("sleep 2")
        luci.http.redirect(
                luci.dispatcher.build_url("admin", "vpn", "pptp-server") .. "?message=" .. message
        )
  end
else
  stop = t:option(Button, "_stop", translate("Stop"))
  stop.inputstyle = "reset"
  function stop.write(self, section)
        luci.util.exec("uci set pptpd.pptpd.enabled='0' &&  uci commit pptpd")
        luci.util.exec("/etc/init.d/pptpd stop")
        luci.util.exec("sleep 2")
        luci.http.redirect(
                luci.dispatcher.build_url("admin", "vpn", "pptp-server")
        )
  end
end

function mp.on_before_commit (self)
  os.execute("rm /var/etc/chap-secrets")
end

function mp.on_after_commit(self)
  os.execute("/etc/init.d/pptpd restart >/dev/null 2>&1 &")
end


return mp
