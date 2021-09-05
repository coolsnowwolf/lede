<dt><a name="<%=tab.name%>"></a><strong><%=tab.name%></strong></dt>
<dd><%=tab.description%>

<%if type(tab.field) == "table" and #tab.field > 0 then%>
<em>Fields</em>
<ul>
	<%for p = 1, #tab.field do%>
	<li>
	  <%=tab.field[p]%>: <%=tab.field[tab.field[p]] or ""%>
	</li>
	<%end%>
</ul>
<%end%>

</dd>
