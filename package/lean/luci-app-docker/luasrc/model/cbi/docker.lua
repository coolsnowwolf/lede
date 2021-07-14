local running = (luci.sys.call("pidof portainer >/dev/null") == 0)
local button = ""

if running then
	button = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<br /><br /><input type=\"button\" value=\" " .. translate("Open Portainer Docker Admin") .. " \" onclick=\"window.open('http://'+window.location.hostname+':" .. 9999 .. "')\"/><br />"
end

m = Map("dockerd", "Docker CE", translate("Docker is a set of platform-as-a-service (PaaS) products that use OS-level virtualization to deliver software in packages called containers.") .. button)


m:section(SimpleSection).template  = "docker/docker_status"

s = m:section(TypedSection, "docker")
s.anonymous = true

wan_mode = s:option(Flag, "wan_mode", translate("Enable WAN access Docker"), translate("Enable WAN access docker mapped ports"))
wan_mode.default = 0
wan_mode.rmempty = false

o=s:option(DummyValue,"readme",translate(" "))
o.description=translate("<a href=\"../../../../DockerReadme.pdf\" target=\"_blank\" />"..translate("Download DockerReadme.pdf").."</a>")

return m
