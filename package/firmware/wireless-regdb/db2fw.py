#!/usr/bin/env python

from io import BytesIO, open
import struct
import hashlib
from dbparse import DBParser
import sys
from math import log

MAGIC = 0x52474442
VERSION = 20

if len(sys.argv) < 3:
    print('Usage: %s output-file input-file' % sys.argv[0])
    sys.exit(2)

def create_rules(countries):
    result = {}
    for c in countries.values():
        for rule in c.permissions:
            result[rule] = 1
    return list(result)

def create_collections(countries):
    result = {}
    for c in countries.values():
        result[(c.permissions, c.dfs_region)] = 1
    return list(result)

def create_wmms(countries):
    result = {}
    for c in countries.values():
        for rule in c.permissions:
            if rule.wmmrule is not None:
                result[rule.wmmrule] = 1
    return list(result)

def be32(output, val):
    output.write(struct.pack('>I', val))
def be16(output, val):
    output.write(struct.pack('>H', val))

class PTR(object):
    def __init__(self, output):
        self._output = output
        self._pos = output.tell()
        be16(output, 0)
        self._written = False

    def set(self, val=None):
        if val is None:
            val = self._output.tell()
        assert val & 3 == 0
        self._offset = val
        pos = self._output.tell()
        self._output.seek(self._pos)
        be16(self._output, val >> 2)
        self._output.seek(pos)
        self._written = True

    def get(self):
        return self._offset

    @property
    def written(self):
        return self._written

p = DBParser()
countries = p.parse(open(sys.argv[2], 'r', encoding='utf-8'))
rules = create_rules(countries)
rules.sort()
collections = create_collections(countries)
collections.sort()
wmms = create_wmms(countries)
wmms.sort()

output = BytesIO()

# struct regdb_file_header
be32(output, MAGIC)
be32(output, VERSION)

country_ptrs = {}
countrynames = list(countries)
countrynames.sort()
for alpha2 in countrynames:
    coll = countries[alpha2]
    output.write(struct.pack('>2s', alpha2))
    country_ptrs[alpha2] = PTR(output)
output.write(b'\x00' * 4)

wmmdb = {}
for w in wmms:
    assert output.tell() & 3 == 0
    wmmdb[w] = output.tell() >> 2
    for r in w._as_tuple():
        ecw = int(log(r[0] + 1, 2)) << 4 | int(log(r[1] + 1, 2))
        ac = (ecw, r[2],r[3])
        output.write(struct.pack('>BBH', *ac))

reg_rules = {}
flags = 0
for reg_rule in rules:
    freq_range, power_rule, wmm_rule = reg_rule.freqband, reg_rule.power, reg_rule.wmmrule
    reg_rules[reg_rule] = output.tell()
    assert power_rule.max_ant_gain == 0
    flags = 0
    # convert to new rule flags
    assert reg_rule.flags & ~0x899 == 0
    if reg_rule.flags & 1<<0:
        flags |= 1<<0
    if reg_rule.flags & 1<<3:
        flags |= 1<<1
    if reg_rule.flags & 1<<4:
        flags |= 1<<2
    if reg_rule.flags & 1<<7:
        flags |= 1<<3
    if reg_rule.flags & 1<<11:
        flags |= 1<<4
    rule_len = 16
    cac_timeout = 0 # TODO
    if not (flags & 1<<2):
        cac_timeout = 0
    if cac_timeout or wmm_rule:
        rule_len += 2
    if wmm_rule is not None:
        rule_len += 2
    output.write(struct.pack('>BBHIII', rule_len, flags, int(power_rule.max_eirp * 100),
                             int(freq_range.start * 1000), int(freq_range.end * 1000), int(freq_range.maxbw * 1000),
                             ))
    if rule_len > 16:
        output.write(struct.pack('>H', cac_timeout))

    if rule_len > 18:
        be16(output, wmmdb[wmm_rule])

    while rule_len % 4:
        output.write('\0')
        rule_len += 1

for coll in collections:
    for alpha2 in countrynames:
        if (countries[alpha2].permissions, countries[alpha2].dfs_region) == coll:
            assert not country_ptrs[alpha2].written
            country_ptrs[alpha2].set()
    slen = 3
    output.write(struct.pack('>BBBx', slen, len(list(coll[0])), coll[1]))
    coll = list(coll[0])
    for regrule in coll:
        be16(output, reg_rules[regrule] >> 2)
    if len(coll) % 2:
        be16(output, 0)

for alpha2 in countrynames:
    assert country_ptrs[alpha2].written

outfile = open(sys.argv[1], 'wb')
outfile.write(output.getvalue())
