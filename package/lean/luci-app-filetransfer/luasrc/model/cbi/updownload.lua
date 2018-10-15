local fs = require "luci.fs"
local http = luci.http

ful = SimpleForm("upload", translate("Upload"), nil)
ful.reset = false
ful.submit = false

sul = ful:section(SimpleSection, "", translate("Upload file to '/tmp/upload/'"))
fu = sul:option(FileUpload, "")
fu.template = "cbi/other_upload"
um = sul:option(DummyValue, "", nil)
um.template = "cbi/other_dvalue"

fdl = SimpleForm("download", translate("Download"), nil)
fdl.reset = false
fdl.submit = false
sdl = fdl:section(SimpleSection, "", translate("Download file"))
fd = sdl:option(FileUpload, "")
fd.template = "cbi/other_download"
dm = sdl:option(DummyValue, "", nil)
dm.template = "cbi/other_dvalue"

function Download()
	local sPath, sFile, fd, block
	sPath = http.formvalue("dlfile")
	sFile = nixio.fs.basename(sPath)
	if luci.fs.isdirectory(sPath) then
		fd = io.popen('tar -C "%s" -cz .' % {sPath}, "r")
		sFile = sFile .. ".tar.gz"
	else
		fd = nixio.open(sPath, "r")
	end
	if not fd then
		dm.value = translate("Couldn't open file: ") .. sPath
		return
	end
	dm.value = nil
	http.header('Content-Disposition', 'attachment; filename="%s"' % {sFile})
	http.prepare_content("application/octet-stream")
	while true do
		block = fd:read(nixio.const.buffersize)
		if (not block) or (#block ==0) then
			break
		else
			http.write(block)
		end
	end
	fd:close()
	http.close()
end

local dir, fd
dir = "/tmp/upload/"
nixio.fs.mkdir(dir)
http.setfilehandler(
	function(meta, chunk, eof)
		if not fd then
			if not meta then return end

			if	meta and chunk then fd = nixio.open(dir .. meta.file, "w") end

			if not fd then
				um.value = translate("Create upload file error.")
				return
			end
		end
		if chunk and fd then
			fd:write(chunk)
		end
		if eof and fd then
			fd:close()
			fd = nil
			um.value = translate("File saved to") .. ' "/tmp/upload/' .. meta.file .. '"'
		end
	end
)

if luci.http.formvalue("upload") then


	local f = luci.http.formvalue("ulfile")
	if #f <= 0 then
		um.value = translate("No specify upload file.")
	end
elseif luci.http.formvalue("download") then
	Download()
end

local inits, attr = {}
for i, f in ipairs(fs.glob("/tmp/upload/*")) do
	attr = fs.stat(f)
	if attr then
		inits[i] = {}
		inits[i].name = fs.basename(f)
		inits[i].mtime = os.date("%Y-%m-%d %H:%M:%S", attr.mtime)
		inits[i].modestr = attr.modestr
		inits[i].size = tostring(attr.size)
		inits[i].remove = 0
		inits[i].install = false
	end
end

form = SimpleForm("filelist", translate("Upload file list"), nil)
form.reset = false
form.submit = false

tb = form:section(Table, inits)
nm = tb:option(DummyValue, "name", translate("File name"))
mt = tb:option(DummyValue, "mtime", translate("Modify time"))
ms = tb:option(DummyValue, "modestr", translate("Mode string"))
sz = tb:option(DummyValue, "size", translate("Size"))
btnrm = tb:option(Button, "remove", translate("Remove"))
btnrm.render = function(self, section, scope)
	self.inputstyle = "remove"
	Button.render(self, section, scope)
end

btnrm.write = function(self, section)
	local v = luci.fs.unlink("/tmp/upload/" .. luci.fs.basename(inits[section].name))
	if v then table.remove(inits, section) end
	return v
end

function IsIpkFile(name)
	name = name or ""
	local ext = string.lower(string.sub(name, -4, -1))
	return ext == ".ipk"
end

btnis = tb:option(Button, "install", translate("Install"))
btnis.template = "cbi/other_button"
btnis.render = function(self, section, scope)
	if not inits[section] then return false end
	if IsIpkFile(inits[section].name) then
		scope.display = ""
	else
		scope.display = "none"
	end
	self.inputstyle = "apply"
	Button.render(self, section, scope)
end

btnis.write = function(self, section)
	local r = luci.sys.exec(string.format('opkg --force-depends install "/tmp/upload/%s"', inits[section].name))
	form.description = string.format('<span style="color: red">%s</span>', r)
end

return ful, fdl, form
