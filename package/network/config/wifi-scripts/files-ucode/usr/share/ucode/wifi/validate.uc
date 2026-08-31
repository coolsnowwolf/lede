'use strict';

import { log } from 'wifi.common';
import * as fs from 'fs';

const schemas = {
	device: json(fs.readfile('/usr/share/schema/wireless.wifi-device.json')).properties,
	iface: json(fs.readfile('/usr/share/schema/wireless.wifi-iface.json')).properties,
	vlan: json(fs.readfile('/usr/share/schema/wireless.wifi-vlan.json')).properties,
	station: json(fs.readfile('/usr/share/schema/wireless.wifi-station.json')).properties,
};

const types = {
	"array": 1,
	"string": 3,
	"number": 5,
	"boolean": 7,
};

function dump_option(schema, key) {
	let _key = (schema[key].type == 'alias') ? schema[key].default : key;

	return [
		key,
		types[schema[_key].type]
	];
}

export function dump_options() {
	let dump = {
		"name": "mac80211",
	};

	for (let k, v in schemas) {
		dump[k] = [];
		for (let option in v)
			push(dump[k], dump_option(v, option));
	};

	printf('%J\n', dump);

	return 0;
};

function abort(msg) {
	log(msg);
	die();
}

function validate_value(schema, key, value) {
	switch(schema.type) {
	case 'number':
		value = +value;
		if (schema.minimum && value < schema.minimum)
			abort(`${key}: ${value} is lower than the minimum value`);
		if (schema.maximum && value > schema.maximum)
			abort(`${key}: ${value} is larger than the maximum value`);
		if (schema.enum && !(value in schema.enum))
			abort(`${key}: ${value} has to be one of ${schema.enum}`);
		break;

	case 'boolean':
		value = !!+value;
		break;

	case 'string':
		if (schema.enum && !(value in schema.enum))
			abort(`${key}: ${value} has to be one of ${schema.enum}`);
		break;

	case 'array':
		if (type(value) != 'array')
			value = [ value ];
		if (schema.items?.type)
			for (let k, v in value)
				value[k] = validate_value(schema.items, key, v);
		break;
	}

	return value;
}

export function validate(schema, dict) {
	schema = schemas[schema];

	/* complain about anything that is not in the schema */
	for (let k, v in dict) {
		if (substr(k, 0, 1) == '.')
			continue;
		if (schema[k])
			continue;
		log(`${k} is not present in the schema`);
	}

	/* convert all aliases */
	for (let k, v in dict) {
		if (schema[k]?.type != 'alias')
			continue;
		if (schema[k].default == null)
			abort(`${k} alias does not have a default value`);

		dict[schema[k].default] = v;
		delete dict[k];
	}

	/* set defaults */
	for (let k, v in schema) {
		if (schema[k]?.type == 'alias')
			continue;
		if (dict[k] != null || schema[k].default == null)
			continue;
		dict[k] = schema[k].default;
	}

	/* validate value constraints */
	for (let k, v in dict) {
		if (!schema[k])
			continue;
		dict[k] = validate_value(schema[k], k, v);
	}
};
