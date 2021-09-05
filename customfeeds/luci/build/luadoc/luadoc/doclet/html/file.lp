<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
    <title>Reference</title>
    <link rel="stylesheet" href="<%=luadoc.doclet.html.link('luadoc.css', 'files/'..file_doc.name)%>" type="text/css" />
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
<%=luadoc.doclet.html.include("menu.lp", { doc=doc, file_doc=file_doc })%>

</div> <!-- id="navigation" -->

<div id="content">

<h1>File <code><%=file_doc.name%></code></h1>

<%if file_doc.description then%>
<p><%=file_doc.description%></p>
<%end%>
<%if file_doc.author then%>
<p><b><%= #file_doc.author>1 and "Authors" or "Author" %>:</b>
<table class="authors_list">
<%for _, author in ipairs(file_doc.author) do%>
	<tr><td class="name"><%= author %></td></tr>
<%end%>
</table>
</p>
<%end%>
<%if file_doc.copyright then%>
<p>Copyright &copy;<%=file_doc.copyright%></p>
<%end%>
<%if file_doc.release then%>
<p><small><b>Release:</b> <%=file_doc.release%></small></p>
<%end%>

<%if #file_doc.functions > 0 then%>
<h2>Functions</h2>
<table class="function_list">
<%for _, func_name in ipairs(file_doc.functions) do
  local func_data = file_doc.functions[func_name]%>
	<tr>
	<td class="name" nowrap><%=func_data.private and "local " or ""%><a href="#<%=func_name%>"><%=func_name%></a>&nbsp;(<%=table.concat(func_data.param or {}, ", ")%>)</td>
	<td class="summary"><%=func_data.summary%></td>
	</tr>
<%end%>
</table>
<%end%>


<%if #file_doc.tables > 0 then%>
<h2>Tables</h2>
<table class="table_list">
<%for _, tab_name in ipairs(file_doc.tables) do%>
	<tr>
	<td class="name" nowrap><a href="#<%=tab_name%>"><%=tab_name%></a></td>
	<td class="summary"><%=file_doc.tables[tab_name].summary%></td>
	</tr>
<%end%>
</table>
<%end%>


<br/>
<br/>



<%if #file_doc.functions > 0 then%>
<h2><a name="functions"></a>Functions</h2>
<dl class="function">
<%for _, func_name in ipairs(file_doc.functions) do%>
<%=luadoc.doclet.html.include("function.lp", { doc=doc, file_doc=file_doc, func=file_doc.functions[func_name] })%>
<%end%>
</dl>
<%end%>


<%if #file_doc.tables > 0 then%>
<h2><a name="tables"></a>Tables</h2>
<dl class="table">
<%for _, tab_name in ipairs(file_doc.tables) do%>
<%=luadoc.doclet.html.include("table.lp", { doc=doc, file_doc=file_doc, tab=file_doc.tables[tab_name] })%>
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
