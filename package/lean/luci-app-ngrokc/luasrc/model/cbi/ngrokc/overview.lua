-- Mantainer : maz-1 < ohmygod19993 at gmail dot com >


m = Map("ngrokc", translate("Ngrok"),translate("Secure tunnels to localhost."))

local apply = luci.http.formvalue("cbi.apply")               
if apply then                                                
        os.execute("/etc/init.d/ngrokc reload &")       -- reload configuration
end 

local DISP = require "luci.dispatcher"
local CTRL = require "luci.controller.ngrokc"
local HTTP = require "luci.http"
local UCI  = (require "luci.model.uci").cursor()

servers=m:section(TypedSection, "servers", translate("Servers"))
servers.template = "cbi/tblsection"
servers.anonymous = false
servers.addremove = true


nhost=servers:option(Value, "host", translate("Ngrok Host"))
nhost.rmempty = false
nhost.datatype = "host"

hport=servers:option(Value, "port", translate("Ngrok Port"))
hport.rmempty = false
hport.datatype = "port"

servers:option(Value, "atoken", translate("Auth Token")).rmempty = true


tunnel=m:section(TypedSection, "tunnel", translate("Tunnels"))
tunnel.template = "cbi/tblsection"
tunnel.anonymous = false
tunnel.addremove = true

tunnel.extedit = DISP.build_url("admin", "services", "ngrokc", "detail", "%s")
function tunnel.create(self, name)
	AbstractSection.create(self, name)
	HTTP.redirect( self.extedit:format(name) )
end

ena=tunnel:option(Flag, "enabled", translate("Enabled"))
ena.template = "ngrokc/overview_enabled"
ena.rmempty = false

lport=tunnel:option(DummyValue, "_lport", translate("Local Port"))
lport.template = "ngrokc/overview_value"
lport.rmempty = false
function lport.set_one(self, section)
	local localport = self.map:get(section, "lport") or ""
	if localport ~= "" then
		return localport
	else
		return [[<em>]] .. translate("config error") .. [[</em>]]
	end
end


server=tunnel:option(DummyValue, "_server", translate("Server"))
server.template = "ngrokc/overview_value"
server.rmempty = false
function server.set_one(self, section)
	local servername = self.map:get(section, "server") or ""
	local host = UCI.get("ngrokc", servername, "host") or ""
	local port = UCI.get("ngrokc", servername, "port") or ""
	if servername ~= "" or host ~= "" or port ~= "" then
		return host .. ":" .. port
	else
		return [[<em>]] .. translate("config error") .. [[</em>]]
	end
end

type=tunnel:option(DummyValue, "_type", translate("Type"))
type.template = "ngrokc/overview_value"
type.rmempty = false
function type.set_one(self, section)
	local tunneltype = self.map:get(section, "type") or ""
	if tunneltype ~= "" then
		return string.upper(tunneltype)
	else
		return [[<em>]] .. translate("config error") .. [[</em>]]
	end
end

url=tunnel:option(DummyValue, "_url", translate("URL"))
url.template = "ngrokc/overview_value"
url.rmempty = false
function url.set_one(self, section)
	local servername = self.map:get(section, "server") or ""
	local host = UCI.get("ngrokc", servername, "host") or ""
	local tunneltype = self.map:get(section, "type") or ""
	local dname = self.map:get(section, "dname") or ""
	local cusdom = self.map:get(section, "custom_domain") or ""
	local rport = self.map:get(section, "rport") or ""
	if tunneltype == "tcp" and rport ~= "" then
		return "tcp://" .. host .. ":" .. rport 
	elseif cusdom == "1" and dname ~= "" then
		return tunneltype .. "://" .. dname
	elseif dname ~= "" then
		return tunneltype .. "://" .. dname .. "." .. string.gsub(host, "www", "")
	else
		return [[<em>]] .. translate("config error") .. [[</em>]]
	end
end

return m
