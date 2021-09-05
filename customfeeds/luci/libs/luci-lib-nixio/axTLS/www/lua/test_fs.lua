function link_dir (dir, base)
	local path = base.."/"..dir
	local mode = lfs.attributes (path).mode
	if mode == "directory" then
		return string.format ('<a href="%s">%s</a>',
			cgilua.mkurlpath ("test_fs.lua", { dir = path }),
			dir)
	else
		return dir
	end
end

cgilua.htmlheader ()
cgilua.put ("<h1>Testing Filesystem library</h1>\n")
cgilua.put ("<table>\n")
cgilua.put ("<tr><td colspan=2>Testing <b>dir</b></td></tr>\n")
local i = 0
local dir = cgi.dir or "."
for file in lfs.dir (dir) do
	i = i+1
	cgilua.put ("<tr><td>"..i.."</td><td>"..link_dir(file, dir).."</td></tr>\n")
end
cgilua.put ("</table>\n")
