local function getfield (t, f)
  for w in string.gfind(f, "[%w_]+") do
    if not t then return nil end
    t = t[w]
  end
  return t
end

function test_lib (libname)
	local ok, err = pcall (require, libname)
	if not ok then
		cgilua.put ("Library <tt><b>"..libname.."</b></tt> not found<br>\n"..
			err)
	else
		cgilua.put ("Library <tt><b>"..libname.."</b></tt><br>\n")
		local t = getfield (_G, libname)
		if type(t) ~= "table" then
			cgilua.put (tostring(t))
		else
			for i, v in pairs (t) do
				cgilua.put ("&nbsp;&nbsp;"..tostring(i).." = "..tostring(v).."<br>\n")
			end
		end
	end
	cgilua.put ("\n<p>\n")
end

cgilua.htmlheader ()
for _, lib in ipairs { "lfs", "socket", "luasql.postgres", "luasql", "lxp", "lxp.lom", "lualdap", "htk", "xmlrpc", "xmlrpc.http" } do
	test_lib (lib)
end
