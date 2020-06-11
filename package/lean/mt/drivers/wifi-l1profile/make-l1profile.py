#!/usr/bin/env python

# Hua Shao <hua.shao@mediatek.com>

import sys
import re
import random

l1conf = []

def parseconfig(conf):
	global l1conf

	l1conf.extend([[],[],[]])
	with open(conf, "r") as fp:
		for line in fp:
			if line.startswith("CONFIG_first_card_"):
				kv = line.split("=")
				l1conf[0].append((kv[0][len("CONFIG_first_card_"):], kv[1].strip("\"\'\r\n\t")))
			elif line.startswith("CONFIG_second_card_"):
				kv = line.split("=")
				l1conf[1].append((kv[0][len("CONFIG_second_card_"):], kv[1].strip("\"\'\r\n\t")))
			elif line.startswith("CONFIG_third_card_"):
				kv = line.split("=")
				l1conf[2].append((kv[0][len("CONFIG_third_card_"):], kv[1].strip("\"\'\r\n\t")))
			else:
				continue	

def validate():
	global l1conf

	d1 = dict(l1conf[0]) if len(l1conf) > 0 else {}
	d2 = dict(l1conf[1]) if len(l1conf) > 1 else {}
	d3 = dict(l1conf[2]) if len(l1conf) > 2 else {}

	# make sure no empty value
	for dx in [d1,d2,d3]:
		for k,v in dx.items():
			assert v

	# make sure these configs are unique
	for name in ["main_ifname", "ext_ifname", "wds_ifname",
				 "apcli_name", "mesh_ifname", "nvram_zone",
				 "profile_path"]:
		if1 = d1.get(name, random.random())
		if2 = d2.get(name, random.random())
		if3 = d3.get(name, random.random())
		assert len(set([if1, if2, if3])) == 3, "duplication found in "+name

	# main_ifname should end with "0"
	if1 = [ x.strip() for x in d1.get("main_ifname","").split(";") if x]
	if2 = [ x.strip() for x in d2.get("main_ifname","").split(";") if x]
	if3 = [ x.strip() for x in d3.get("main_ifname","").split(";") if x]
	for each in if1:
		assert not each or each.endswith("0"), "1st main_ifname {0} does not ends with 0".format(each)
	for each in if2:
		assert not each or each.endswith("0"), "2nd main_ifname {0} does not ends with 0".format(each)
	for each in if3:
		assert not each or each.endswith("0"), "3rd main_ifname {0} does not ends with 0".format(each)

	# main_ifname should start with ext_ifname
	if1ext = [ x.strip() for x in d1.get("ext_ifname","").split(";") if x]
	if2ext = [ x.strip() for x in d2.get("ext_ifname","").split(";") if x]
	if3ext = [ x.strip() for x in d3.get("ext_ifname","").split(";") if x]

	assert len(if1) == len(if1ext), "number of 1st main_ifname does not equal to 1st ext_ifname"
	assert len(if2) == len(if2ext), "number of 2nd main_ifname does not equal to 2nd ext_ifname"
	assert len(if3) == len(if3ext), "number of 3rd main_ifname does not equal to 3rd ext_ifname"

	for i,each in enumerate(if1ext):
		assert if1[i].startswith(each), "1st main_ifname {0} does not start with its ext_ifname {1}".format(if1[i], each)
	for i,each in enumerate(if2ext):
		assert if2[i].startswith(each), "2nd main_ifname {0} does not start with its ext_ifname {1}".format(if2[i], each)
	for i,each in enumerate(if3ext):
		assert if3[i].startswith(each), "3rd main_ifname {0} does not start with its ext_ifname {1}".format(if3[i], each)

	# assertion failure or returning any python non-true value will terminate the build procedure.
	# if you need more validations, feel free to add you code below.

	return True

def genfile(dest):
	global l1conf

	with open(dest, "w") as fp:
		print("Default")
		fp.write("Default\n")
		for i,lst in enumerate(l1conf):
			for (k,v) in lst:
				if k == "name":
					line = "INDEX{0}={1}".format(i, v)
				else:
					line = "INDEX{0}_{1}={2}".format(i, k, v)
				print(line)
				fp.write(line+"\n")
		fp.write("\n") # extra line-end to make drivers happy

if __name__ == "__main__":
	if len(sys.argv) < 3:
		print("arguments missing!")
		print("usage: make-l1profile.py <.config> <l1profile.dat>!")
		sys.exit(-1)
	conf = sys.argv[1]
	dest = sys.argv[2]
	parseconfig(conf)
	if validate():
		genfile(dest)
	else:
		print("something is wrong with your l1profile configurations!")
		sys.exit(-1)

