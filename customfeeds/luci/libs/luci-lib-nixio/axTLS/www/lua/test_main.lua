cgilua.htmlheader()
cgilua.put[[
<html>
<head><title>Script Lua Test</title></head>

<body>
cgi = {
]]

for i,v in pairs (cgi) do
	if type(v) == "table" then
		local vv = "{"
		for a,b in pairs(v) do
			vv = string.format ("%s%s = %s<br>\n", vv, a, tostring(b))
		end
		v = vv.."}"
	end
	cgilua.put (string.format ("%s = %s<br>\n", i, tostring(v)))
end
cgilua.put "}<br>\n"
cgilua.put ("Remote address: "..cgilua.servervariable"REMOTE_ADDR")
cgilua.put "<br>\n"
cgilua.put ("Is persistent = "..tostring (SAPI.Info.ispersistent).."<br>\n")
cgilua.put ("ap="..tostring(ap).."<br>\n")
cgilua.put ("lfcgi="..tostring(lfcgi).."<br>\n")

-- Checking Virtual Environment
local my_output = cgilua.put
cgilua.put = nil
local status, err = pcall (function ()
	assert (cgilua.put == nil, "cannot change cgilua.put value")
end)
cgilua.put = my_output
assert (status == true, err)

-- Checking require
local status, err = pcall (function () require"unknown_module" end)
assert (status == false, "<tt>unknown_module</tt> loaded!")
-- assert (package == nil, "Access to <tt>package</tt> table allowed!")

cgilua.put[[
<p>
</body>
</html>
]]
cgilua = nil
