import { sorted_json } from "./utils.uc";
import { dirname, glob } from "fs";

let ctx;

function proto_config_load(config_fn, section_name)
{
	if (!ctx)
		return null;

	let section_data = ctx.get_all("network", section_name);
	if (!section_data)
		return null;

	let config_obj = {
		iface: section_name,
		section: section_name,
		data: section_data,
		uci: ctx,
	};

	let result;
	if (config_fn)
		result = config_fn(config_obj);
	else
		result = section_data;

	return sorted_json(result);
}

netifd.cb.proto_config_load = proto_config_load;

let base = dirname(sourcepath());
for (let script in glob(base + "/proto/*.uc")) {
	try {
		loadfile(script)();
	} catch (e) {
		netifd.log(netifd.L_WARNING,
			`Error loading proto handler ${script}: ${e}\n${e.stacktrace[0].context}\n`);
	}
}

function config_init(uci)
{
	ctx = uci;
	if (!ctx.load("network"))
		netifd.log(netifd.L_WARNING, `Failed to load network config\n`);
}

return {
	config_init,
};
