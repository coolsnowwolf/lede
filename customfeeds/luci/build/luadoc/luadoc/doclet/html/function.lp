<%
if module_doc then
	from = "modules/"..module_doc.name
elseif file_doc then
	from = "files/.."..file_doc.name
else
	from = ""
end
%>

<dt><%=func.private and "local " or ""%><a name="<%=func.name%>"></a><strong><%=func.printname%></strong>&nbsp;(<%=table.concat(func.param or {}, ", ")%>)</dt>
<dd>
<%=func.description or ""%>

<%if type(func.param) == "table" and #func.param > 0 then%>
<h3>Parameters</h3>
<ul>
	<%for p = 1, #func.param do%>
	<li>
	  <%=func.param[p]%>: <%=func.param[func.param[p]] or ""%>
	</li>
	<%end%>
</ul>
<%end%>


<%if type(func.usage) == "string" then%>
<h3>Usage:</h3>
<%=func.usage%>
<%elseif type(func.usage) == "table" then%>
<h3>Usage</h3>
<ul>
	<%for _, usage in ipairs(func.usage) do%>
	<li><%= usage %>
	<%end%>
</ul>
<%end%>

<%if type(func.ret) == "string" then%>
<h3>Return value:</h3>
<%=func.ret%>
<%elseif type(func.ret) == "table" then%>
<h3>Return values:</h3>
<ol>
	<%for _, ret in ipairs(func.ret) do%>
	<li><%= ret %>
	<%end%>
</ol>
<%end%>

<%if type(func.see) == "string" then %>
<h3>See also:</h3>
	<a href="<%=func.see%>"><%=func.see%></a>
<%elseif type(func.see) == "table" and #func.see > 0 then %>
<h3>See also:</h3>
<ul>
	<%for i = 1, #func.see do%>
	<li><a href="<%=luadoc.doclet.html.symbol_link(func.see[i], doc, module_doc, file_doc, from)%>">
		<%=(oop and func.see[i]:gsub("%.",":") or func.see[i]:gsub(".+%.",""))%>
	</a>
	<%end%>
</ul>
<%end%>
</dd>
