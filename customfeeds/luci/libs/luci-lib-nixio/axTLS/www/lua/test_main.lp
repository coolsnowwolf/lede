<html>
<head><title>Embeded Lua Test</title></head>

<body>
cgi = {
<?lua
for i,v in pairs (cgi) do
	if type(v) == "table" then
		local vv = "{"
		for a,b in pairs(v) do
			vv = string.format ("%s%s = %s<br>\n", vv, a, tostring(b))
		end
		v = vv.."}"
	end
?>
<%= i %> = <%= tostring(v) %> <br>
<%
end
%>
}
<br>
Remote address: <%= cgilua.servervariable"REMOTE_ADDR" %>
<br>
Is persistent = <%= tostring (SAPI.Info.ispersistent) %>
<br>
ap = <?lua= tostring(ap) ?> <br>
lfcgi = <% = tostring(lfcgi) %> <br>
<%= (ap and ap.handler()) or "" %> <br>

</body>
</html>
