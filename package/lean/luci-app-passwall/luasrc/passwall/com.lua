local _M = {}

local function gh_release_url(self)
	return "https://api.github.com/repos/" .. self.repo .. "/releases/latest"
end

local function gh_pre_release_url(self)
	return "https://api.github.com/repos/" .. self.repo .. "/releases?per_page=1"
end

_M.brook = {
	name = "Brook",
	repo = "txthinking/brook",
	get_url = gh_release_url,
	cmd_version = "-v | awk '{print $3}'",
	zipped = false,
	default_path = "/usr/bin/brook",
	match_fmt_str = "linux_%s$",
	file_tree = {}
}

_M.hysteria = {
	name = "Hysteria",
	repo = "HyNetwork/hysteria",
	get_url = gh_release_url,
	cmd_version = "version | awk '/^Version:/ {print $2}'",
	remote_version_str_replace = "app/",
	zipped = false,
	default_path = "/usr/bin/hysteria",
	match_fmt_str = "linux%%-%s$",
	file_tree = {
		armv6 = "arm",
		armv7 = "arm"
	}
}

_M["trojan-go"] = {
	name = "Trojan-Go",
	repo = "p4gefau1t/trojan-go",
	get_url = gh_release_url,
	cmd_version = "-version | awk '{print $2}' | sed -n 1P",
	zipped = true,
	default_path = "/usr/bin/trojan-go",
	match_fmt_str = "linux%%-%s%%.zip",
	file_tree = {
		aarch64 = "armv8",
		armv8   = "armv8",
		mips    = "mips%-hardfloat",
		mipsel  = "mipsle%-hardfloat"
	}
}

_M.singbox = {
	name = "Sing-Box",
	repo = "SagerNet/sing-box",
	get_url = gh_pre_release_url,
	cmd_version = "version | awk '{print $3}' | sed -n 1P",
	zipped = true,
	zipped_suffix = "tar.gz",
	default_path = "/usr/bin/sing-box",
	match_fmt_str = "linux%%-%s",
	file_tree = {
		x86_64 = "amd64"
	}
}

_M.xray = {
	name = "Xray",
	repo = "XTLS/Xray-core",
	get_url = gh_pre_release_url,
	cmd_version = "version | awk '{print $2}' | sed -n 1P",
	zipped = true,
	default_path = "/usr/bin/xray",
	match_fmt_str = "linux%%-%s",
	file_tree = {
		x86_64 = "64",
		x86    = "32",
		mips   = "mips32",
		mipsel = "mips32le"
	}
}

_M["chinadns-ng"] = {
	name = "ChinaDNS-NG",
	repo = "zfl9/chinadns-ng",
	get_url = gh_release_url,
	cmd_version = "-V | awk '{print $2}'",
	zipped = false,
	default_path = "/usr/bin/chinadns-ng",
	match_fmt_str = "%s$",
	file_tree = {
		x86_64  = "x86_64",
		x86     = "i686",
		mipsel  = "mipsel",
		aarch64 = "aarch64",
		armv5   = "arm%-eabi",
		armv6   = "armv6%-eabihf",
		armv7   = "armv7l%-eabihf",
		armv8   = "aarch64"
	}
}

return _M
