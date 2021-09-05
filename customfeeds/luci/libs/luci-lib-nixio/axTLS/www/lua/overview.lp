<?lua
-- Tries to load known libraries
for _, t in ipairs { "lxp", "luasql.postgres", "luasql.mysql", "luasql.oci8", "luasql.sqlite", "luasql.odbc", "socket", "xmlrpc", "soap", "lualdap", "logging", "md5", "zip", "stable", "copas", } do
	pcall (require, t)
end

libraries = { "lxp", "luasql", "socket", "xmlrpc", "soap", "lualdap", "logging", "md5", "zip", "stable", "copas", }

local colors = { "#999999", "#CCCCCC", "#FFFFFF", }
local i = 0
function color () i = math.mod (i + 1, 3) return colors[i + 1] end

function pack_name (p) return string.gsub (p, "^([^.]+).-", "%1") end

function idx (t, f) return _G[t][f] or _G[t]["_"..f] or "" end
?>
<html>
<head><title>CGILua installation overview</title></head>

<body bgcolor="#FFFFFF">
<h1>CGILua installation overview</h1>

<table>
  <tr>
    <th bgcolor="#999999">Version
    <th bgcolor="#999999">Copyright
    <th bgcolor="#999999">Description
  </tr>
<?lua
local support = {
	{ "Lua", "_VERSION" },
	{ "compat-5.1", "_COMPAT51" },
}
for _, l in ipairs (support) do bg = color()
?>
  <tr>
    <td bgcolor = "<%= bg %>"><%= tostring(_G[l[2]]) %>
    <td bgcolor = "<%= bg %>">
    <td bgcolor = "<%= bg %>">
  </tr>
<? end ?>
  <tr><td colspan="4"></tr>
<?lua
local pack = {}
for i, p in ipairs (libraries) do
	local s = _G[p]
	local n = pack_name(p)
	if type(_G[n]) == "table" and _G[n]._VERSION then
		pack[n] = true
		table.insert (pack, n)
	end
end
table.sort (pack)
for _, p in ipairs (pack) do bg = color() ?>
  <tr>
    <td bgcolor = "<%= bg %>"><%= idx(p,"VERSION") %>
    <td bgcolor = "<%= bg %>"><small><%= idx(p,"COPYRIGHT") %></small>
    <td bgcolor = "<%= bg %>"><small><%= idx(p,"DESCRIPTION") %></small>
  </tr>
<?lua end ?>
</table>

</body>
</html>
