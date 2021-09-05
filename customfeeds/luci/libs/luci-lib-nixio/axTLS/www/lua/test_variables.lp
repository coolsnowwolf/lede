<table>
<% for _, var in pairs { "SERVER_SOFTWARE", "SERVER_NAME", "GATEWAY_INTERFACE", "SERVER_PROTOCOL", "SERVER_PORT", "REQUEST_METHOD", "PATH_INFO", "PATH_TRANSLATED", "SCRIPT_NAME", "QUERY_STRING", "REMOTE_HOST", "REMOTE_ADDR", "AUTH_TYPE", "REMOTE_USER", "REMOTE_IDENT", "CONTENT_TYPE", "CONTENT_LENGTH", "HTTP_REFERER", "HTTP_COOKIE", "SCRIPT_FILENAME", "DOCUMENT_ROOT", } do %>
  <tr><td><%= var %><td>=<td>"<%= cgilua.servervariable(var) or "<i>not defined</i>"%>"</tr>
<% end %>
</table>

<p>
<table>
<% for _, var in ipairs { "script_file", "script_path", "script_pdir", "script_vdir", "script_vpath", "urlpath", } do %>
  <tr><td><%= var %><td>=<td>"<%= cgilua[var] %>"</tr>
<% end %>
</table>

<p>
