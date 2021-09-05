<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
    <title>Reference</title>
    <link rel="stylesheet" href="<%=luadoc.doclet.html.link('luadoc.css', 'modules/'..module_doc.name)%>" type="text/css" />
	<!--meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/-->
</head>

<body>
<div id="container">

<div id="product">
	<div id="product_logo"></div>
	<div id="product_name"><big><b></b></big></div>
	<div id="product_description"></div>
</div> <!-- id="product" -->

<div id="main">

<div id="navigation">
<%=luadoc.doclet.html.include("menu.lp", { doc=doc, module_doc=module_doc })%>
<% oop = not not ( module_doc.doc[1] and module_doc.doc[1].cstyle == "instance" ) %>

</div><!-- id="navigation" -->

<div id="content">

<h1><%=( oop and "Object Instance" or "Class" )%> <code><%=module_doc.name%></code></h1>

<p><%=module_doc.description%></p>
<%if module_doc.author then%>
<p><b><%= #module_doc.author>1 and "Authors" or "Author" %>:</b>
<table class="authors_list">
<%for _, author in ipairs(module_doc.author) do%>
	<tr><td class="name"><%= author %></td></tr>
<%end%>
</table>
</p>
<%end%>
<%if module_doc.copyright then%>
<p>Copyright&copy; <%=module_doc.copyright%></p>
<%end%>
<%if module_doc.release then%>
<p><small><b>Release:</b> <%=module_doc.release%></small></p>
<%end%>

<%if #module_doc.constants > 0 then %>
<h2>Constants</h2>
<table class="function_list">
<%for _, const_name in ipairs(module_doc.constants) do
  local const_data = module_doc.constants[const_name]%>
	<tr>
	<td class="name" nowrap><code><%=const_data.private and "local " or ""%><%=(const_name:gsub(".+%.",""))%></code></td>
	<td class="summary"><%=const_data.summary%></td>
	</tr>
<%end%>
</table>
<%end%>

<% local funcs = { }; if #module_doc.functions > 0 then %>
<h2>Functions</h2>
<table class="function_list">
<%
for _, func_name in ipairs(module_doc.functions) do
  funcs[#funcs+1] = func_name
end

table.sort(funcs, function(a, b)
  local func_data_a = module_doc.functions[a]
  local func_data_b = module_doc.functions[b]
  local x = func_data_a.sort or a
  local y = func_data_b.sort or b
  return x < y
end)

for _, func_name in ipairs(funcs) do
  local func_data = module_doc.functions[func_name]

  func_data.printname = func_name:gsub("^%d+#", "")
  if oop then
	func_data.printname = func_data.printname:gsub("%.", ":")
  else
    func_data.printname = func_data.printname:gsub("^.+%.", "")
  end
%>
	<tr>
	<td class="name" nowrap><%=func_data.private and "local " or ""%><a href="#<%=func_name%>"><%=func_data.printname%></a>&nbsp;(<%=table.concat(module_doc.functions[func_name].param or {}, ", ")%>)</td>
	<td class="summary"><%=module_doc.functions[func_name].summary%></td>
	</tr>
<%end%>
</table>
<%end%>


<% local tabs = { }; if #module_doc.tables > 0 then%>
<h2>Tables</h2>
<table class="table_list">
<%
for _, tab_name in ipairs(module_doc.tables) do
  tabs[#tabs+1] = tab_name
end

table.sort(tabs, function(a, b)
  local tab_data_a = module_doc.tables[a]
  local tab_data_b = module_doc.tables[b]
  local x = tab_data_a.sort or a
  local y = tab_data_b.sort or b
  return x < y
end)

for _, tab_name in ipairs(tabs) do
%>
	<tr>
	<td class="name" nowrap><a href="#<%=tab_name%>"><%=tab_name%></a></td>
	<td class="summary"><%=module_doc.tables[tab_name].summary%></td>
	</tr>
<%end%>
</table>
<%end%>


<br/>
<br/>

<%if #module_doc.functions > 0 then%>
<h2><a name="functions"></a>Functions</h2>
<dl class="function">
<%for _, func_name in ipairs(funcs) do%>
<%=luadoc.doclet.html.include("function.lp", { doc=doc, module_doc=module_doc, func=module_doc.functions[func_name], oop=oop })%>
<%end%>
</dl>
<%end%>

<%if #module_doc.tables > 0 then%>
<h2><a name="tables"></a>Tables</h2>
<dl class="table">
<%for _, tab_name in ipairs(tabs) do%>
<%=luadoc.doclet.html.include("table.lp", { doc=doc, module_doc=module_doc, tab=module_doc.tables[tab_name] })%>
<%end%>
</dl>
<%end%>


</div> <!-- id="content" -->

</div> <!-- id="main" -->

<div id="about">
	<p><a href="http://validator.w3.org/check?uri=referer"><img src="http://www.w3.org/Icons/valid-xhtml10" alt="Valid XHTML 1.0!" height="31" width="88" /></a></p>
</div> <!-- id="about" -->

</div> <!-- id="container" -->
</body>
</html>
