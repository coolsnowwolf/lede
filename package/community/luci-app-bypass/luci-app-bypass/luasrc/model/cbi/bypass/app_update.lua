m = Map("bypass")

-- [[ App Settings ]]--
s = m:section(TypedSection, "global", translate("App Update"),
              "<font color='red'>" ..
                  translate("Please confirm that your firmware supports FPU.") ..
                  "</font>")
s.anonymous = true
s:append(Template("bypass/xray_version"))
s:append(Template("bypass/trojan_go_version"))

return m
