
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local fs = require "luci.openclash"
local uci = require("luci.model.uci").cursor()
local CHIF = "0"

font_green = [[<b style=color:green>]]
font_off = [[</b>]]
bold_on  = [[<strong>]]
bold_off = [[</strong>]]
align_mid = [[<p align="center">]]
align_mid_off = [[</p>]]

function IsYamlFile(e)
   e=e or""
   local e=string.lower(string.sub(e,-5,-1))
   return e == ".yaml"
end
function IsYmlFile(e)
   e=e or""
   local e=string.lower(string.sub(e,-4,-1))
   return e == ".yml"
end

function default_config_set(f)
	local cf = uci:get("openclash", "config", "config_path")
	if cf == "/etc/openclash/config/"..f or not cf or cf == "" or not fs.isfile(cf) then
		if CHIF == "1" and cf == "/etc/openclash/config/"..f then
			return
		end
		local fis = fs.glob("/etc/openclash/config/*")[1]
		if fis ~= nil then
			fcf = fs.basename(fis)
			if fcf then
				uci:set("openclash", "config", "config_path", "/etc/openclash/config/"..fcf)
				uci:commit("openclash")
			end
		else
			uci:set("openclash", "config", "config_path", "/etc/openclash/config/config.yaml")
			uci:commit("openclash")
		end
	end
end

function config_check(CONFIG_FILE)
	local yaml = fs.isfile(CONFIG_FILE)
	if yaml then
		yaml = SYS.exec(string.format('ruby -ryaml -rYAML -I "/usr/share/openclash" -E UTF-8 -e "puts YAML.load_file(\'%s\')" 2>/dev/null',CONFIG_FILE))
		if yaml ~= "false\n" and yaml ~= "" then
			return "Config Normal"
		else
			return "Config Abnormal"
		end
	elseif (yaml ~= 0) then
	   return "File Not Exist"
	end
end

ful = SimpleForm("upload", translate("Config Manage"), nil)
ful.reset = false
ful.submit = false

sul =ful:section(SimpleSection, "")
o = sul:option(FileUpload, "")
o.template = "openclash/upload"
um = sul:option(DummyValue, "", nil)
um.template = "openclash/dvalue"

local dir, fd, clash
clash = "/etc/openclash/clash"
dir = "/etc/openclash/config/"
bakck_dir="/etc/openclash/backup"
proxy_pro_dir="/etc/openclash/proxy_provider/"
rule_pro_dir="/etc/openclash/rule_provider/"
core_dir="/etc/openclash/core/core/"
backup_dir="/tmp/"
create_bakck_dir=fs.mkdir(bakck_dir)
create_proxy_pro_dir=fs.mkdir(proxy_pro_dir)
create_rule_pro_dir=fs.mkdir(rule_pro_dir)

HTTP.setfilehandler(
	function(meta, chunk, eof)
		local fp = HTTP.formvalue("file_type")
		if not fd then
			if not meta then return end

			if fp == "config" then
				if meta and chunk then fd = nixio.open(dir .. meta.file, "w") end
			elseif fp == "proxy-provider" then
				if meta and chunk then fd = nixio.open(proxy_pro_dir .. meta.file, "w") end
			elseif fp == "rule-provider" then
				if meta and chunk then fd = nixio.open(rule_pro_dir .. meta.file, "w") end
			elseif fp == "clash" or fp == "clash_tun" or fp == "clash_meta" then
				create_core_dir=fs.mkdir(core_dir)
				if meta and chunk then fd = nixio.open(core_dir .. meta.file, "w") end
			elseif fp == "backup-file" then
				if meta and chunk then fd = nixio.open(backup_dir .. meta.file, "w") end
			end

			if not fd then
				um.value = translate("upload file error.")
				return
			end
		end
		if chunk and fd then
			fd:write(chunk)
		end
		if eof and fd then
			fd:close()
			fd = nil
			if fp == "config" then
				CHIF = "1"
				if IsYamlFile(meta.file) then
					local yamlbackup="/etc/openclash/backup/" .. meta.file
					local c=fs.copy(dir .. meta.file,yamlbackup)
					default_config_set(meta.file)
				end
				if IsYmlFile(meta.file) then
					local ymlname=string.lower(string.sub(meta.file,0,-5))
					local ymlbackup="/etc/openclash/backup/".. ymlname .. ".yaml"
					local c=fs.rename(dir .. meta.file,"/etc/openclash/config/".. ymlname .. ".yaml")
					local c=fs.copy("/etc/openclash/config/".. ymlname .. ".yaml",ymlbackup)
					local yamlname=ymlname .. ".yaml"
					default_config_set(yamlname)
				end
				um.value = translate("File saved to") .. ' "/etc/openclash/config/"'
			elseif fp == "proxy-provider" then
				um.value = translate("File saved to") .. ' "/etc/openclash/proxy_provider/"'
			elseif fp == "rule-provider" then
				um.value = translate("File saved to") .. ' "/etc/openclash/rule_provider/"'
			elseif fp == "clash" or fp == "clash_tun" or fp == "clash_meta" then
				if string.lower(string.sub(meta.file, -7, -1)) == ".tar.gz" then
					os.execute(string.format("tar -C '/etc/openclash/core/core' -xzf %s >/dev/null 2>&1", (core_dir .. meta.file)))
					fs.unlink(core_dir .. meta.file)
					os.execute(string.format("mv $(echo \"/etc/openclash/core/core/$(ls /etc/openclash/core/core/)\") '/etc/openclash/core/%s' >/dev/null 2>&1", fp))
				elseif string.lower(string.sub(meta.file, -3, -1)) == ".gz" then
					os.execute(string.format("mv %s '/etc/openclash/core/%s.gz' >/dev/null 2>&1", (core_dir .. meta.file), fp))
					os.execute("gzip -fd '/etc/openclash/core/%s.gz' >/dev/null 2>&1" %fp)
					fs.unlink("/etc/openclash/core/%s.gz" %fp)
				else
					os.execute(string.format("mv $(echo \"/etc/openclash/core/core/$(ls /etc/openclash/core/core/)\") '/etc/openclash/core/%s' >/dev/null 2>&1", fp))
				end
				os.execute("chmod 4755 /etc/openclash/core/%s >/dev/null 2>&1" %fp)
				os.execute("rm -rf %s >/dev/null 2>&1" %core_dir)
				um.value = translate("File saved to") .. ' "/etc/openclash/core/"'
			elseif fp == "backup-file" then
				os.execute("tar -C '/etc/openclash/' -xzf %s >/dev/null 2>&1" % (backup_dir .. meta.file))
				os.execute("mv /etc/openclash/openclash /etc/config/openclash >/dev/null 2>&1")
				fs.unlink(backup_dir .. meta.file)
				um.value = translate("Backup File Restore Successful!")
			end
			fs.unlink("/tmp/Proxy_Group")
		end
	end
)

if HTTP.formvalue("upload") then
	local f = HTTP.formvalue("ulfile")
	if #f <= 0 then
		um.value = translate("No Specify Upload File")
	end
end

local e,a={}
for t,o in ipairs(fs.glob("/etc/openclash/config/*"))do
a=fs.stat(o)
if a then
e[t]={}
e[t].name=fs.basename(o)
BACKUP_FILE="/etc/openclash/backup/".. e[t].name
if fs.mtime(BACKUP_FILE) then
   e[t].mtime=os.date("%Y-%m-%d %H:%M:%S",fs.mtime(BACKUP_FILE))
else
   e[t].mtime=os.date("%Y-%m-%d %H:%M:%S",a.mtime)
end
if uci:get("openclash", "config", "config_path") and string.sub(uci:get("openclash", "config", "config_path"), 23, -1) == e[t].name then
   e[t].state=translate("Enable")
else
   e[t].state=translate("Disable")
end
e[t].size=fs.filesize(a.size)
e[t].check=translate(config_check(o))
e[t].remove=0
end
end

form=SimpleForm("config_file_list",translate("Config File List"))
form.reset=false
form.submit=false
tb=form:section(Table,e)
st=tb:option(DummyValue,"state",translate("State"))
nm=tb:option(DummyValue,"name",translate("Config Alias"))
sb=tb:option(DummyValue,"name",translate("Subscription Info"))
mt=tb:option(DummyValue,"mtime",translate("Update Time"))
sz=tb:option(DummyValue,"size",translate("Size"))
ck=tb:option(DummyValue,"check",translate("Grammar Check"))
st.template="openclash/cfg_check"
ck.template="openclash/cfg_check"
sb.template="openclash/sub_info_show"

btnis=tb:option(Button,"switch",translate("Switch Config"))
btnis.template="openclash/other_button"
btnis.render=function(o,t,a)
if not e[t] then return false end
if IsYamlFile(e[t].name) or IsYmlFile(e[t].name) then
a.display=""
else
a.display="none"
end
o.inputstyle="apply"
Button.render(o,t,a)
end
btnis.write=function(a,t)
fs.unlink("/tmp/Proxy_Group")
uci:set("openclash", "config", "config_path", "/etc/openclash/config/"..e[t].name)
uci:commit("openclash")
HTTP.redirect(luci.dispatcher.build_url("admin", "services", "openclash", "config"))
end

btned=tb:option(Button,"edit",translate("Edit"))
btned.render=function(o,t,a)
o.inputstyle="apply"
Button.render(o,t,a)
end
btned.write=function(a,t)
	local file_path = "etc/openclash/config/" .. fs.basename(e[t].name)
	HTTP.redirect(DISP.build_url("admin", "services", "openclash", "other-file-edit", "config", "%s") %file_path)
end

btncp=tb:option(Button,"copy",translate("Copy Config"))
btncp.template="openclash/other_button"
btncp.render=function(o,t,a)
if not e[t] then return false end
if IsYamlFile(e[t].name) or IsYmlFile(e[t].name) then
a.display=""
else
a.display="none"
end
o.inputstyle="apply"
Button.render(o,t,a)
end
btncp.write=function(a,t)
	local num = 1
	while true do
		num = num + 1
		if not fs.isfile("/etc/openclash/config/"..fs.filename(e[t].name).."("..num..")"..".yaml") then
			fs.copy("/etc/openclash/config/"..e[t].name, "/etc/openclash/config/"..fs.filename(e[t].name).."("..num..")"..".yaml")
			break
		end
	end
	HTTP.redirect(luci.dispatcher.build_url("admin", "services", "openclash", "config"))
end

btnrn=tb:option(DummyValue,"/etc/openclash/config/",translate("Rename"))
btnrn.template="openclash/input_rename"
btnrn.rawhtml = true
btnrn.render=function(c,t,a)
c.value = e[t].name
Button.render(c,t,a)
end

btndl = tb:option(Button,"download",translate("Download Config"))
btndl.template="openclash/other_button"
btndl.render=function(e,t,a)
e.inputstyle="remove"
Button.render(e,t,a)
end
btndl.write = function (a,t)
	local sPath, sFile, fd, block
	sPath = "/etc/openclash/config/"..e[t].name
	sFile = NXFS.basename(sPath)
	if fs.isdirectory(sPath) then
		fd = io.popen('tar -C "%s" -cz .' % {sPath}, "r")
		sFile = sFile .. ".tar.gz"
	else
		fd = nixio.open(sPath, "r")
	end
	if not fd then
		return
	end
	HTTP.header('Content-Disposition', 'attachment; filename="%s"' % {sFile})
	HTTP.prepare_content("application/octet-stream")
	while true do
		block = fd:read(nixio.const.buffersize)
		if (not block) or (#block ==0) then
			break
		else
			HTTP.write(block)
		end
	end
	fd:close()
	HTTP.close()
end

btndlr = tb:option(Button,"download_run",translate("Download Running Config"))
btndlr.template="openclash/other_button"
btndlr.render=function(c,t,a)
	if nixio.fs.access("/etc/openclash/"..e[t].name)  then
		a.display=""
	else
		a.display="none"
	end
c.inputstyle="remove"
Button.render(c,t,a)
end
btndlr.write = function (a,t)
	local sPath, sFile, fd, block
	sPath = "/etc/openclash/"..e[t].name
	sFile = NXFS.basename(sPath)
	if fs.isdirectory(sPath) then
		fd = io.popen('tar -C "%s" -cz .' % {sPath}, "r")
		sFile = sFile .. ".tar.gz"
	else
		fd = nixio.open(sPath, "r")
	end
	if not fd then
		return
	end
	HTTP.header('Content-Disposition', 'attachment; filename="%s"' % {sFile})
	HTTP.prepare_content("application/octet-stream")
	while true do
		block = fd:read(nixio.const.buffersize)
		if (not block) or (#block ==0) then
			break
		else
			HTTP.write(block)
		end
	end
	fd:close()
	HTTP.close()
end

btnrm=tb:option(Button,"remove",translate("Remove"))
btnrm.render=function(e,t,a)
e.inputstyle="reset"
Button.render(e,t,a)
end
btnrm.write=function(a,t)
	fs.unlink("/tmp/Proxy_Group")
	fs.unlink("/etc/openclash/backup/"..fs.basename(e[t].name))
	fs.unlink("/etc/openclash/history/"..fs.filename(e[t].name))
	fs.unlink("/etc/openclash/history/"..fs.filename(e[t].name)..".db")
	fs.unlink("/etc/openclash/"..fs.basename(e[t].name))
	local a=fs.unlink("/etc/openclash/config/"..fs.basename(e[t].name))
	default_config_set(fs.basename(e[t].name))
	if a then table.remove(e,t)end
	HTTP.redirect(DISP.build_url("admin", "services", "openclash","config"))
end

p = SimpleForm("provider_file_manage",translate("Provider File Manage"))
p.reset = false
p.submit = false

local provider_manage = {
    {proxy_mg, rule_mg, game_mg}
}

promg = p:section(Table, provider_manage)

o = promg:option(Button, "proxy_mg", " ")
o.inputtitle = translate("Proxy Provider File List")
o.inputstyle = "reload"
o.write = function()
  HTTP.redirect(DISP.build_url("admin", "services", "openclash", "proxy-provider-file-manage"))
end

o = promg:option(Button, "rule_mg", " ")
o.inputtitle = translate("Rule Providers File List")
o.inputstyle = "reload"
o.write = function()
  HTTP.redirect(DISP.build_url("admin", "services", "openclash", "rule-providers-file-manage"))
end

o = promg:option(Button, "game_mg", " ")
o.inputtitle = translate("Game Rules File List")
o.inputstyle = "reload"
o.write = function()
  HTTP.redirect(DISP.build_url("admin", "services", "openclash", "game-rules-file-manage"))
end

m = SimpleForm("openclash",translate("Config File Edit"))
m.reset = false
m.submit = false

local tab = {
 {user, default}
}

s = m:section(Table, tab)
s.description = align_mid..translate("Support syntax check, press").." "..font_green..bold_on.."F10"..bold_off..font_off.." "..translate("to control diff option, press").." "..font_green..bold_on.."F11"..bold_off..font_off.." "..translate("to enter full screen editing mode")..align_mid_off
s.anonymous = true
s.addremove = false

local conf = uci:get("openclash", "config", "config_path")
local dconf = "/usr/share/openclash/res/default.yaml"
if not conf then conf = "/etc/openclash/config/config.yaml" end
local conf_name = fs.basename(conf)
if not conf_name then conf_name = "config.yaml"  end
local sconf = "/etc/openclash/"..conf_name

sev = s:option(TextValue, "user")
---sev.description = align_mid..translate("Modify Your Config file:").." "..font_green..bold_on..conf_name..bold_off..font_off.." "..translate("Here, Except The Settings That Were Taken Over")..align_mid_off
sev.rows = 40
sev.wrap = "off"
sev.cfgvalue = function(self, section)
	return NXFS.readfile(conf) or NXFS.readfile(dconf) or ""
end
sev.write = function(self, section, value)
if (CHIF == "0") then
    value = value:gsub("\r\n?", "\n")
    local old_value = NXFS.readfile(conf)
	  if value ~= old_value then
       NXFS.writefile(conf, value)
    end
end
end

def = s:option(TextValue, "default")
if fs.isfile(sconf) then
	---def.description = align_mid..translate("Config File Edited By OpenClash For Running")..align_mid_off
else
	---def.description = align_mid..translate("Default Config File With Correct Template")..align_mid_off
end
def.rows = 40
def.wrap = "off"
def.readonly = true
def.cfgvalue = function(self, section)
	return NXFS.readfile(sconf) or NXFS.readfile(dconf) or ""
end
def.write = function(self, section, value)
end

local t = {
    {Commit, Create, Apply}
}

a = m:section(Table, t)

o = a:option(Button, "Commit", " ")
o.inputtitle = translate("Commit Settings")
o.inputstyle = "apply"
o.write = function()
	fs.unlink("/tmp/Proxy_Group")
	uci:commit("openclash")
end

o = a:option(DummyValue, "Create", " ")
o.rawhtml = true
o.template = "openclash/input_file_name"
o.value = "/etc/openclash/config/"

o = a:option(Button, "Apply", " ")
o.inputtitle = translate("Apply Settings")
o.inputstyle = "apply"
o.write = function()
	fs.unlink("/tmp/Proxy_Group")
	uci:set("openclash", "config", "enable", 1)
	uci:commit("openclash")
	SYS.call("/etc/init.d/openclash restart >/dev/null 2>&1 &")
	HTTP.redirect(DISP.build_url("admin", "services", "openclash"))
end

m:append(Template("openclash/config_editor"))

return ful , form , p , m
