'use strict';

import { glob, basename, realpath, chdir, mkstemp } from "fs";

export const TYPE_ARRAY = 1;
export const TYPE_STRING = 3;
export const TYPE_INT = 5;
export const TYPE_BOOL = 7;

export function parse_bool(val)
{
	switch (val) {
	case "1":
	case "true":
		return true;
	case "0":
	case "false":
		return false;
	}
};

export function parse_array(val)
{
	if (type(val) != "array")
		val = split(val, /\s+/);
	return val;
};

function __type_parsers()
{
	let ret = [];

	ret[TYPE_ARRAY] = parse_array;
	ret[TYPE_STRING] = function(val) {
		return val;
	};
	ret[TYPE_INT] = function(val) {
		return +val;
	};
	ret[TYPE_BOOL] = parse_bool;

	return ret;
}
export const type_parser = __type_parsers();

export function handler_load(path, cb)
{
	for (let script in glob(path + "/*.sh")) {
		script = basename(script);

		let f = mkstemp();
		let prev_dir = realpath(".");
		chdir(path);
		system(`./${script} "" "dump" >&${f.fileno()}`);
		chdir(prev_dir);
		f.seek();
		while (!f.error()) {
			let data = trim(f.read("line"));
			try {
				data = json(data);
			} catch (e) {
				continue;
			}

			if (type(data) != "object")
				continue;

			cb(script, data);
		}
		f.close();
	}
};

export function handler_attributes(data, extra, validate)
{
	let ret = { ...extra };
	for (let cur in data) {
		let name_data = split(cur[0], ":", 2);
		let name = name_data[0];
		ret[name] = cur[1];
		if (validate && name_data[1])
			validate[name] = name_data[1];
	}
	return ret;
};

export function parse_attribute_list(data, spec)
{
	let ret = {};

	for (let name, type_id in spec) {
		if (!(name in data))
			continue;

		let val = data[name];
		let parser = type_parser[type_id];
		if (parser)
			val = parser(val);
		ret[name] = val;
	}

	return ret;
};

export function sorted_json(value) {
	let t = type(value);

	if (t == "object") {
		let parts = [];
		for (let key in sort(keys(value)))
			push(parts, sprintf("%J", key) + ":" + sorted_json(value[key]));
		return "{" + join(",", parts) + "}";
	}

	if (t == "array") {
		let parts = [];
		for (let item in value)
			push(parts, sorted_json(item));
		return "[" + join(",", parts) + "]";
	}

	return sprintf("%J", value);
};

export function is_equal(val1, val2) {
	let t1 = type(val1);

	if (t1 != type(val2))
		return false;

	if (t1 == "array") {
		if (length(val1) != length(val2))
			return false;

		for (let i = 0; i < length(val1); i++)
			if (!is_equal(val1[i], val2[i]))
				return false;

		return true;
	} else if (t1 == "object") {
		for (let key in val1)
			if (!is_equal(val1[key], val2[key]))
				return false;
		for (let key in val2)
			if (val1[key] == null)
				return false;
		return true;
	} else {
		return val1 == val2;
	}
};
