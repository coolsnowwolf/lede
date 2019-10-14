local running = (luci.sys.call("pidof portainer >/dev/null") == 0)
local button = ""

if running then
	button = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<br /><br /><input type=\"button\" value=\" " .. translate("Open Portainer Docker Admin") .. " \" onclick=\"window.open('http://'+window.location.hostname+':" .. 9999 .. "')\"/>"
end

m = Map("docker", "Docker CE", translate("Docker is a set of platform-as-a-service (PaaS) products that use OS-level virtualization to deliver software in packages called containers."))


m:section(SimpleSection).template  = "docker/docker_status"

s = m:section(TypedSection, "docker")
s.anonymous = true

wan_mode = s:option(Flag, "enabled", translate("Enable WAN access Dokcer"))
wan_mode.default = 0
wan_mode.rmempty = false
wan_mode.description = translate(("!") .. button)

o = s:option(Button,"certificate",translate("Docker Readme First"))
o.inputtitle = translate("Download DockerReadme.pdf")
o.description = translate("Please download DockerReadme.pdf to read when first-running")
o.inputstyle = "reload"
o.write = function()
	Download()
end

function Download()
	local t,e
	t=nixio.open("/www/DockerReadme.pdf","r")
	luci.http.header('Content-Disposition','attachment; filename="DockerReadme.pdf"')
	luci.http.prepare_content("application/octet-stream")
	while true do
		e=t:read(nixio.const.buffersize)
		if(not e)or(#e==0)then
			break
		else
			luci.http.write(e)
		end
	end
	t:close()
	luci.http.close()
end

return m