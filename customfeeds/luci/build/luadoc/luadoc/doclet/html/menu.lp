<%
if module_doc then
	from = "modules/"..module_doc.name
elseif file_doc then
	from = "files/.."..file_doc.name
else
	from = ""
end
%>

<h1>LuaDoc</h1>
<ul>
	<%if not module_doc and not file_doc then%>
	<li><strong>Index</strong></li>
	<%else%>
	<li><a href="<%=luadoc.doclet.html.link("index.html", from)%>">Index</a></li>
	<%end%>
</ul>


<!-- Module list -->
<%if not options.nomodules and #doc.modules > 0 then%>
<h1>Modules</h1>
<ul>
<%for _, modulename in ipairs(doc.modules) do
	if module_doc and module_doc.name == modulename then%>
	<li><strong><%=modulename%></strong></li>
	<%else%>
	<li>
		<a href="<%=luadoc.doclet.html.module_link(modulename, doc, from)%>"><%=modulename%></a>
	</li>
<%	end
end%>
</ul>
<%end%>


<!-- File list -->
<%if not options.nofiles and #doc.files > 0 then%>
<h1>Files</h1>
<ul>
<%for _, filepath in ipairs(doc.files) do
	if file_doc and file_doc.name == filepath then%>
	<li><strong><%=filepath%></strong></li>
	<%else%>
	<li>
		<a href="<%=luadoc.doclet.html.file_link(filepath, from)%>"><%=filepath%></a>
	</li>
<%	end
end%>
</ul>
<%end%>



