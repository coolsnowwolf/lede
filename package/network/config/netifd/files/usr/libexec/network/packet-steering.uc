#!/usr/bin/env ucode
'use strict';
import { glob, basename, dirname, readlink, readfile, realpath, writefile, error, open } from "fs";

let napi_weight = 1.0;
let cpu_thread_weight = 0.75;
let rx_weight = 0.75;
let eth_bias = 2.0;
let debug = 0, do_nothing = 0;
let disable;
let cpus;
let all_cpus;
let local_flows = 0;

while (length(ARGV) > 0) {
	let arg = shift(ARGV);
	switch (arg) {
	case "-d":
		debug++;
		break;
	case "-n":
		do_nothing++;
		break;
	case '0':
		disable = true;
		break;
	case '2':
		all_cpus = true;
		break;
	case '-l':
		local_flows = +shift(ARGV);
		break;
	}
}

function task_name(pid)
{
	let stat = open(`/proc/${pid}/status`, "r");
	if (!stat)
		return;
	let line = stat.read("line");
	stat.close();
	return trim(split(line, "\t", 2)[1]);
}

function set_task_cpu(pid, cpu) {
	if (disable)
		cpu = join(",", map(cpus, (cpu) => cpu.id));
	let name = task_name(pid);
	if (!name)
		return;
	if (debug || do_nothing)
		warn(`taskset -p -c ${cpu} ${name}\n`);
	if (!do_nothing)
		system(`taskset -p -c ${cpu} ${pid}`);
}

function cpu_mask(cpu)
{
	let mask;
	if (cpu < 0)
		mask = (1 << length(cpus)) - 1;
	else
		mask = (1 << int(cpu));
	return sprintf("%x", mask);
}

function set_netdev_cpu(dev, cpu, rx_queue) {
	rx_queue ??= "rx-*";
	let queues = glob(`/sys/class/net/${dev}/queues/${rx_queue}/rps_cpus`);
	let val = cpu_mask(cpu);
	if (disable)
		val = 0;
	for (let queue in queues) {
		if (debug || do_nothing)
			warn(`echo ${val} > ${queue}\n`);
		if (!do_nothing)
			writefile(queue, `${val}`);
	}
	queues = glob(`/sys/class/net/${dev}/queues/${rx_queue}/rps_flow_cnt`);
	for (let queue in queues) {
		if (debug || do_nothing)
			warn(`echo ${local_flows} > ${queue}\n`);
		if (!do_nothing)
			writefile(queue, `${local_flows}`);
	}
}

function task_device_match(name, device)
{
	let napi_match = match(name, /napi\/([^-]*)-\d+/);
	if (!napi_match)
		napi_match = match(name, /mt76-tx (phy\d+)/);
	if (napi_match &&
	    (index(device.phy, napi_match[1]) >= 0 ||
	     index(device.netdev, napi_match[1]) >= 0))
		return true;

	if (device.driver == "mtk_soc_eth" && match(name, /napi\/mtk_eth-/))
		return true;

	return false;
}

cpus = map(glob("/sys/bus/cpu/devices/*"), (path) => {
	return {
		id: int(match(path, /.*cpu(\d+)/)[1]),
		core: int(trim(readfile(`${path}/topology/core_id`))),
		load: 0.0,
	};
});

cpus = slice(cpus, 0, 64);
if (length(cpus) < 2)
	exit(0);

function cpu_add_weight(cpu_id, weight)
{
	let cpu = cpus[cpu_id];
	cpu.load += weight;
	for (let sibling in cpus) {
		if (sibling == cpu || sibling.core != cpu.core)
			continue;
		sibling.load += weight * cpu_thread_weight;
	}
}

function get_next_cpu(weight, prev_cpu)
{
	if (disable)
		return 0;

	let sort_cpus = sort(slice(cpus), (a, b) => a.load - b.load);
	let idx = 0;

	if (prev_cpu != null && sort_cpus[idx].id == prev_cpu)
		idx++;

	let cpu = sort_cpus[idx].id;
	cpu_add_weight(cpu, weight);
	return cpu;
}

let phys_devs = {};
let netdev_phys = {};
let netdevs = map(glob("/sys/class/net/*"), (dev) => basename(dev));

for (let dev in netdevs) {
	let pdev_path = realpath(`/sys/class/net/${dev}/device`);
	if (!pdev_path)
		continue;

	if (length(glob(`/sys/class/net/${dev}/lower_*`)) > 0)
		continue;

	let pdev = phys_devs[pdev_path];
	if (!pdev) {
		pdev = phys_devs[pdev_path] = {
			path: pdev_path,
			driver: basename(readlink(`${pdev_path}/driver`)),
			netdev: [],
			phy: [],
			tasks: [],
			rx_tasks: [],
			rx_queues: map(glob(`/sys/class/net/${dev}/queues/rx-*/rps_cpus`),
			               (v) => basename(dirname(v))),
		};
	}

	let phyidx = trim(readfile(`/sys/class/net/${dev}/phy80211/index`));
	if (phyidx != null) {
		let phy = `phy${phyidx}`;
		if (index(pdev.phy, phy) < 0)
			push(pdev.phy, phy);
	}

	push(pdev.netdev, dev);
	netdev_phys[dev] = pdev;
}

for (let path in glob("/proc/*/exe")) {
	readlink(path);
	if (error() != "No such file or directory")
		continue;

	let pid = basename(dirname(path));
	let name = task_name(pid);
	for (let devname in phys_devs) {
		let dev = phys_devs[devname];
		if (!task_device_match(name, dev))
			continue;

		push(dev.tasks, pid);

		let napi_match = match(name, /napi\/([^-]*)-(\d+)/);
		if (napi_match && napi_match[2] > 0)
			push(dev.rx_tasks, pid);
		break;
	}
}

function assign_dev_queues_cpu(dev) {
	let num = length(dev.rx_queues);
	if (num < length(dev.rx_tasks))
		num = length(dev.rx_tasks);

	for (let i = 0; i < num; i++) {
		let cpu;

		let task = dev.rx_tasks[i];
		if (num >= length(cpus))
			cpu = i % length(cpus);
		else if (task)
			cpu = get_next_cpu(napi_weight);
		else
			cpu = -1;
		set_task_cpu(task, cpu);

		let rxq = dev.rx_queues[i];
		if (!rxq)
			continue;

		if (num >= length(cpus))
			cpu = (i + 1) % length(cpus);
		else if (all_cpus)
			cpu = -1;
		else
			cpu = get_next_cpu(napi_weight, cpu);
		for (let netdev in dev.netdev)
			set_netdev_cpu(netdev, cpu, rxq);
	}
}

function assign_dev_cpu(dev) {
	if (length(dev.rx_queues) > 1 &&
		length(dev.rx_tasks) > 1)
		return assign_dev_queues_cpu(dev);

	if (length(dev.tasks) > 0) {
		let cpu = dev.napi_cpu = get_next_cpu(napi_weight);
		for (let task in dev.tasks)
			set_task_cpu(task, cpu);
	}

	if (length(dev.netdev) > 0) {
		let cpu;
		if (all_cpus)
			cpu = -1;
		else
			cpu = get_next_cpu(rx_weight, dev.napi_cpu);
		for (let netdev in dev.netdev)
			set_netdev_cpu(netdev, cpu);
	}
}

// Assign ethernet devices first
for (let devname in phys_devs) {
	let dev = phys_devs[devname];
	if (!length(dev.phy))
		assign_dev_cpu(dev);
}

// Add bias to avoid assigning other tasks to CPUs with ethernet NAPI
for (let devname in phys_devs) {
	let dev = phys_devs[devname];
	if (!length(dev.tasks) || dev.napi_cpu == null)
		continue;
	cpu_add_weight(dev.napi_cpu, eth_bias);
}

// Assign WLAN devices
for (let devname in phys_devs) {
	let dev = phys_devs[devname];
	if (length(dev.phy) > 0)
		assign_dev_cpu(dev);
}

if (debug > 1)
	warn(sprintf("devices: %.J\ncpus: %.J\n", phys_devs, cpus));
