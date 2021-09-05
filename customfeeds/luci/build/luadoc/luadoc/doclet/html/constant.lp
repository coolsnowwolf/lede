<%
if module_doc then
	from = "modules/"..module_doc.name
elseif file_doc then
	from = "files/.."..file_doc.name
else
	from = ""
end
%>

<dt><%=const.private and "local " or ""%><a name="<%=const.name%>"></a><strong><%=const.name:gsub(".+%.","")%></strong></dt>
<dd>
<%=const.description or ""%>

<%if type(const.see) == "string" then %>
<h3>See also:</h3>
	<a href="<%=const.see%>"><%=const.see%></a>
<%elseif type(const.see) == "table" and #const.see > 0 then %>
<h3>See also:</h3>
<ul>
	<%for i = 1, #const.see do%>
	<li><a href="<%=luadoc.doclet.html.symbol_link(const.see[i], doc, module_doc, file_doc, from)%>">
		<%=const.see[i]:gsub(".+%.","")%>
	</a>
	<%end%>
</ul
<%end%>
</dd>
