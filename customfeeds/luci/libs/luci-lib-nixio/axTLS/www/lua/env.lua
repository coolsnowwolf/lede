-- This file should be executed before any script in this directory
-- according to the configuration (cgilua/conf.lua).

pcall (cgilua.enablesession)

local put, mkurlpath = cgilua.put, cgilua.mkurlpath

cgilua.addclosefunction (function ()
	put [[
<p>
<small>
<a href="test_main.html">Main</a>]]
	for _, test in {
			{ "Get", "test_main.lua", {ab = "cd", ef = "gh"} },
			{ "Cookies", "test_cookies.lua", },
			{ "FileSystem", "test_fs.lua", },
			{ "Libraries", "test_lib.lua", },
			{ "Session", "test_session.lua", },
			{ "Variables", "test_variables.lp", },
		} do
		put (string.format (' &middot; <a href="%s">%s</a>',
			mkurlpath (test[2], test[3]), test[1]))
	end
	put [[
</small>]]
end)
