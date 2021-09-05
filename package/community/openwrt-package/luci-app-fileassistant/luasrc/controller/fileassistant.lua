module("luci.controller.fileassistant", package.seeall)

function index()

    entry({"admin", "nas"}, firstchild(), "NAS", 44).dependent = false

    local page
    page = entry({"admin", "nas", "fileassistant"}, template("fileassistant"), _("文件助手"), 1)
    page.i18n = "base"
    page.dependent = true

    page = entry({"admin", "nas", "fileassistant", "list"}, call("fileassistant_list"), nil)     
    page.leaf = true

    page = entry({"admin", "nas", "fileassistant", "open"}, call("fileassistant_open"), nil)
    page.leaf = true

    page = entry({"admin", "nas", "fileassistant", "delete"}, call("fileassistant_delete"), nil)
    page.leaf = true

    page = entry({"admin", "nas", "fileassistant", "rename"}, call("fileassistant_rename"), nil)
    page.leaf = true

    page = entry({"admin", "nas", "fileassistant", "upload"}, call("fileassistant_upload"), nil)
    page.leaf = true

    page = entry({"admin", "nas", "fileassistant", "install"}, call("fileassistant_install"), nil)
    page.leaf = true

end

function list_response(path, success)
    luci.http.prepare_content("application/json")
    local result
    if success then
        local rv = scandir(path)
        result = {
            ec = 0,
            data = rv
        }
    else
        result = {
            ec = 1
        }
    end
    luci.http.write_json(result)
end

function fileassistant_list()
    local path = luci.http.formvalue("path")
    list_response(path, true)
end

function fileassistant_open()
    local path = luci.http.formvalue("path")
    local filename = luci.http.formvalue("filename")
    local io = require "io"
    local mime = to_mime(filename)

    file = path..filename

    local download_fpi = io.open(file, "r")
    luci.http.header('Content-Disposition', 'inline; filename="'..filename..'"' )
    luci.http.prepare_content(mime)
    luci.ltn12.pump.all(luci.ltn12.source.file(download_fpi), luci.http.write)
end

function fileassistant_delete()
    local path = luci.http.formvalue("path")
    local isdir = luci.http.formvalue("isdir")
    path = path:gsub("<>", "/")
    path = path:gsub(" ", "\ ")
    local success
    if isdir then
        success = os.execute('rm -r "'..path..'"')
    else
        success = os.remove(path)
    end
    list_response(nixio.fs.dirname(path), success)
end

function fileassistant_rename()
    local filepath = luci.http.formvalue("filepath")
    local newpath = luci.http.formvalue("newpath")
    local success = os.execute('mv "'..filepath..'" "'..newpath..'"')
    list_response(nixio.fs.dirname(filepath), success)
end

function fileassistant_install()
    local filepath = luci.http.formvalue("filepath")
    local isdir = luci.http.formvalue("isdir")
    local ext = filepath:match(".+%.(%w+)$")
    filepath = filepath:gsub("<>", "/")
    filepath = filepath:gsub(" ", "\ ")
    local success
    if isdir == "1" then
        success = false  
    elseif ext == "ipk" then
        success = installIPK(filepath)
    else
        success = false
    end
    list_response(nixio.fs.dirname(filepath), success)
end

function installIPK(filepath)
    luci.sys.exec('opkg --force-depends install "'..filepath..'"')
    luci.sys.exec('rm -rf /tmp/luci-*')
    return true;
end

function fileassistant_upload()
    local filecontent = luci.http.formvalue("upload-file")
    local filename = luci.http.formvalue("upload-filename")
    local uploaddir = luci.http.formvalue("upload-dir")
    local filepath = uploaddir..filename

    local fp
    luci.http.setfilehandler(
        function(meta, chunk, eof)
            if not fp and meta and meta.name == "upload-file" then
                fp = io.open(filepath, "w")
            end
            if fp and chunk then
                fp:write(chunk)
            end
            if fp and eof then
                fp:close()
            end
      end
    )

    list_response(uploaddir, true)
end

function scandir(directory)
    local i, t, popen = 0, {}, io.popen

    local pfile = popen("ls -lh \""..directory.."\" | egrep '^d' ; ls -lh \""..directory.."\" | egrep -v '^d|^l'")
    for fileinfo in pfile:lines() do
        i = i + 1
        t[i] = fileinfo
    end
    pfile:close()
    pfile = popen("ls -lh \""..directory.."\" | egrep '^l' ;")
    for fileinfo in pfile:lines() do
        i = i + 1
        linkindex, _, linkpath = string.find(fileinfo, "->%s+(.+)$")
        local finalpath;
        if string.sub(linkpath, 1, 1) == "/" then
            finalpath = linkpath
        else
            finalpath = nixio.fs.realpath(directory..linkpath)
        end
        local linktype;
        if not finalpath then
            finalpath = linkpath;
            linktype = 'x'
        elseif nixio.fs.stat(finalpath, "type") == "dir" then
            linktype = 'z'
        else
            linktype = 'l'
        end
        fileinfo = string.sub(fileinfo, 2, linkindex - 1)
        fileinfo = linktype..fileinfo.."-> "..finalpath
        t[i] = fileinfo
    end
    pfile:close()
    return t
end

MIME_TYPES = {
    ["txt"]   = "text/plain";
    ["conf"]   = "text/plain";
    ["ovpn"]   = "text/plain";
    ["log"]   = "text/plain";
    ["js"]    = "text/javascript";
    ["json"]    = "application/json";
    ["css"]   = "text/css";
    ["htm"]   = "text/html";
    ["html"]  = "text/html";
    ["patch"] = "text/x-patch";
    ["c"]     = "text/x-csrc";
    ["h"]     = "text/x-chdr";
    ["o"]     = "text/x-object";
    ["ko"]    = "text/x-object";

    ["bmp"]   = "image/bmp";
    ["gif"]   = "image/gif";
    ["png"]   = "image/png";
    ["jpg"]   = "image/jpeg";
    ["jpeg"]  = "image/jpeg";
    ["svg"]   = "image/svg+xml";

    ["zip"]   = "application/zip";
    ["pdf"]   = "application/pdf";
    ["xml"]   = "application/xml";
    ["xsl"]   = "application/xml";
    ["doc"]   = "application/msword";
    ["ppt"]   = "application/vnd.ms-powerpoint";
    ["xls"]   = "application/vnd.ms-excel";
    ["odt"]   = "application/vnd.oasis.opendocument.text";
    ["odp"]   = "application/vnd.oasis.opendocument.presentation";
    ["pl"]    = "application/x-perl";
    ["sh"]    = "application/x-shellscript";
    ["php"]   = "application/x-php";
    ["deb"]   = "application/x-deb";
    ["iso"]   = "application/x-cd-image";
    ["tgz"]   = "application/x-compressed-tar";

    ["mp3"]   = "audio/mpeg";
    ["ogg"]   = "audio/x-vorbis+ogg";
    ["wav"]   = "audio/x-wav";

    ["mpg"]   = "video/mpeg";
    ["mpeg"]  = "video/mpeg";
    ["avi"]   = "video/x-msvideo";
}

function to_mime(filename)
    if type(filename) == "string" then
        local ext = filename:match("[^%.]+$")

        if ext and MIME_TYPES[ext:lower()] then
            return MIME_TYPES[ext:lower()]
        end
    end

    return "application/octet-stream"
end