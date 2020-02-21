#!/usr/bin/env python

from functools import total_ordering
import sys, math
from math import ceil, log
from collections import defaultdict, OrderedDict

# must match <linux/nl80211.h> enum nl80211_reg_rule_flags

flag_definitions = {
    'NO-OFDM':		1<<0,
    'NO-CCK':		1<<1,
    'NO-INDOOR':	1<<2,
    'NO-OUTDOOR':	1<<3,
    'DFS':		1<<4,
    'PTP-ONLY':		1<<5,
    'PTMP-ONLY':	1<<6,
    'NO-IR':	        1<<7,
    # hole at bit 8
    # hole at bit 9. FIXME: Where is NO-HT40 defined?
    'NO-HT40':		1<<10,
    'AUTO-BW':		1<<11,
}

dfs_regions = {
    'DFS-FCC':		1,
    'DFS-ETSI':		2,
    'DFS-JP':		3,
}

@total_ordering

class WmmRule(object):

    def __init__(self, vo_c, vi_c, be_c, bk_c, vo_ap, vi_ap, be_ap, bk_ap):
        self.vo_c = vo_c
        self.vi_c = vi_c
        self.be_c = be_c
        self.bk_c = bk_c
        self.vo_ap = vo_ap
        self.vi_ap = vi_ap
        self.be_ap = be_ap
        self.bk_ap = bk_ap

    def _as_tuple(self):
        return (self.vo_c, self.vi_c, self.be_c, self.bk_c,
                self.vo_ap, self.vi_ap, self.be_ap, self.bk_ap)

    def __eq__(self, other):
        if other is None:
            return False
        return (self._as_tuple() == other._as_tuple())

    def __ne__(self, other):
        return not (self == other)

    def __lt__(self, other):
        if other is None:
            return False
        return (self._as_tuple() < other._as_tuple())

    def __hash__(self):
        return hash(self._as_tuple())

class FreqBand(object):
    def __init__(self, start, end, bw, comments=None):
        self.start = start
        self.end = end
        self.maxbw = bw
        self.comments = comments or []

    def _as_tuple(self):
        return (self.start, self.end, self.maxbw)

    def __eq__(self, other):
        return (self._as_tuple() == other._as_tuple())

    def __ne__(self, other):
        return not (self == other)

    def __lt__(self, other):
        return (self._as_tuple() < other._as_tuple())

    def __hash__(self):
        return hash(self._as_tuple())

    def __str__(self):
        return '<FreqBand %.3f - %.3f @ %.3f>' % (
                  self.start, self.end, self.maxbw)

@total_ordering
class PowerRestriction(object):
    def __init__(self, max_ant_gain, max_eirp, comments = None):
        self.max_ant_gain = max_ant_gain
        self.max_eirp = max_eirp
        self.comments = comments or []

    def _as_tuple(self):
        return (self.max_ant_gain, self.max_eirp)

    def __eq__(self, other):
        return (self._as_tuple() == other._as_tuple())

    def __ne__(self, other):
        return not (self == other)

    def __lt__(self, other):
        return (self._as_tuple() < other._as_tuple())

    def __hash__(self):
        return hash(self._as_tuple())

    def __str__(self):
        return '<PowerRestriction ...>'

class DFSRegionError(Exception):
    def __init__(self, dfs_region):
        self.dfs_region = dfs_region

class FlagError(Exception):
    def __init__(self, flag):
        self.flag = flag

@total_ordering
class Permission(object):
    def __init__(self, freqband, power, flags, wmmrule):
        assert isinstance(freqband, FreqBand)
        assert isinstance(power, PowerRestriction)
        assert isinstance(wmmrule, WmmRule) or wmmrule is None
        self.freqband = freqband
        self.power = power
        self.wmmrule = wmmrule
        self.flags = 0
        for flag in flags:
            if not flag in flag_definitions:
                raise FlagError(flag)
            self.flags |= flag_definitions[flag]
        self.textflags = flags

    def _as_tuple(self):
        return (self.freqband, self.power, self.flags, self.wmmrule)

    def __eq__(self, other):
        return (self._as_tuple() == other._as_tuple())

    def __ne__(self, other):
        return not (self == other)

    def __lt__(self, other):
        return (self._as_tuple() < other._as_tuple())

    def __hash__(self):
        return hash(self._as_tuple())

    def __str__(self):
        return str(self.freqband) + str(self.power) + str(self.wmmrule)

class Country(object):
    def __init__(self, dfs_region, permissions=None, comments=None):
        self._permissions = permissions or []
        self.comments = comments or []
        self.dfs_region = 0

        if dfs_region:
            if not dfs_region in dfs_regions:
                raise DFSRegionError(dfs_region)
            self.dfs_region = dfs_regions[dfs_region]

    def add(self, perm):
        assert isinstance(perm, Permission)
        self._permissions.append(perm)
        self._permissions.sort()

    def __contains__(self, perm):
        assert isinstance(perm, Permission)
        return perm in self._permissions

    def __str__(self):
        r = ['(%s, %s)' % (str(b), str(p)) for b, p in self._permissions]
        return '<Country (%s)>' % (', '.join(r))

    def _get_permissions_tuple(self):
        return tuple(self._permissions)
    permissions = property(_get_permissions_tuple)

class SyntaxError(Exception):
    pass

class DBParser(object):
    def __init__(self, warn=None):
        self._warn_callout = warn or sys.stderr.write

    def _syntax_error(self, txt=None):
        txt = txt and ' (%s)' % txt or ''
        raise SyntaxError("Syntax error in line %d%s" % (self._lineno, txt))

    def _warn(self, txt):
        self._warn_callout("Warning (line %d): %s\n" % (self._lineno, txt))

    def _parse_band_def(self, bname, banddef, dupwarn=True):
        try:
            freqs, bw = banddef.split('@')
            bw = float(bw)
        except ValueError:
            bw = 20.0

        try:
            start, end = freqs.split('-')
            start = float(start)
            end = float(end)
            # The kernel will reject these, so might as well reject this
            # upon building it.
            if start <= 0:
                self._syntax_error("Invalid start freq (%d)" % start)
            if end <= 0:
                self._syntax_error("Invalid end freq (%d)" % end)
            if start > end:
                self._syntax_error("Inverted freq range (%d - %d)" % (start, end))
            if start == end:
                self._syntax_error("Start and end freqs are equal (%d)" % start)
        except ValueError:
            self._syntax_error("band must have frequency range")

        b = FreqBand(start, end, bw, comments=self._comments)
        self._comments = []
        self._banddup[bname] = bname
        if b in self._bandrev:
            if dupwarn:
                self._warn('Duplicate band definition ("%s" and "%s")' % (
                              bname, self._bandrev[b]))
            self._banddup[bname] = self._bandrev[b]
        self._bands[bname] = b
        self._bandrev[b] = bname
        self._bandline[bname] = self._lineno

    def _parse_band(self, line):
        try:
            bname, line = line.split(':', 1)
            if not bname:
                self._syntax_error("'band' keyword must be followed by name")
        except ValueError:
            self._syntax_error("band name must be followed by colon")

        if bname in flag_definitions:
            self._syntax_error("Invalid band name")

        self._parse_band_def(bname, line)

    def _parse_power(self, line):
        try:
            pname, line = line.split(':', 1)
            if not pname:
                self._syntax_error("'power' keyword must be followed by name")
        except ValueError:
            self._syntax_error("power name must be followed by colon")

        if pname in flag_definitions:
            self._syntax_error("Invalid power name")

        self._parse_power_def(pname, line)

    def _parse_power_def(self, pname, line, dupwarn=True):
        try:
            max_eirp = line
            if max_eirp == 'N/A':
                max_eirp = '0'
            max_ant_gain = float(0)
            def conv_pwr(pwr):
                if pwr.endswith('mW'):
                    pwr = float(pwr[:-2])
                    return 10.0 * math.log10(pwr)
                else:
                    return float(pwr)
            max_eirp = conv_pwr(max_eirp)
        except ValueError:
            self._syntax_error("invalid power data")

        p = PowerRestriction(max_ant_gain, max_eirp,
                             comments=self._comments)
        self._comments = []
        self._powerdup[pname] = pname
        if p in self._powerrev:
            if dupwarn:
                self._warn('Duplicate power definition ("%s" and "%s")' % (
                              pname, self._powerrev[p]))
            self._powerdup[pname] = self._powerrev[p]
        self._power[pname] = p
        self._powerrev[p] = pname
        self._powerline[pname] = self._lineno

    def _parse_wmmrule(self, line):
        regions = line[:-1].strip()
        if not regions:
            self._syntax_error("'wmmrule' keyword must be followed by region")

        regions = regions.split(',')

        self._current_regions = {}
        for region in regions:
            if region in self._wmm_rules:
                self._warn("region %s was added already to wmm rules" % region)
            self._current_regions[region] = 1
        self._comments = []

    def _validate_input(self, cw_min, cw_max, aifsn, cot):
        if  cw_min < 1:
            self._syntax_error("Invalid cw_min value (%d)" % cw_min)
        if cw_max < 1:
            self._syntax_error("Invalid cw_max value (%d)" % cw_max)
        if cw_min > cw_max:
            self._syntax_error("Inverted contention window (%d - %d)" %
                    (cw_min, cw_max))
        if not (bin(cw_min + 1).count('1') == 1 and cw_min < 2**15):
            self._syntax_error("Invalid cw_min value should be power of 2 - 1 (%d)"
                    % cw_min)
        if not (bin(cw_max + 1).count('1') == 1 and cw_max < 2**15):
            self._syntax_error("Invalid cw_max value should be power of 2 - 1 (%d)"
                    % cw_max)
        if aifsn < 1:
            self._syntax_error("Invalid aifsn value (%d)" % aifsn)
        if cot < 0:
            self._syntax_error("Invalid cot value (%d)" % cot)


    def _validate_size(self, var, bytcnt):
        return bytcnt < ceil(len(bin(var)[2:]) / 8.0)

    def _parse_wmmrule_item(self, line):
        bytcnt = (2.0, 2.0, 1.0, 2.0)
        try:
            ac, cval = line.split(':')
            if not ac:
                self._syntax_error("wmm item must have ac prefix")
        except ValueError:
                self._syntax_error("access category must be followed by colon")
        p = tuple([int(v.split('=', 1)[1]) for v in cval.split(',')])
        self._validate_input(*p)
        for v, b in zip(p, bytcnt):
            if self._validate_size(v, b):
                self._syntax_error("unexpected input size expect %d got %d"
                        % (b, v))

            for r in self._current_regions:
                self._wmm_rules[r][ac] = p

    def _parse_country(self, line):
        try:
            cname, cvals= line.split(':', 1)
            dfs_region = cvals.strip()
            if not cname:
                self._syntax_error("'country' keyword must be followed by name")
        except ValueError:
            self._syntax_error("country name must be followed by colon")

        cnames = cname.split(',')

        self._current_countries = {}
        for cname in cnames:
            if len(cname) != 2:
                self._warn("country '%s' not alpha2" % cname)
            cname = cname.encode('ascii')
            if not cname in self._countries:
                self._countries[cname] = Country(dfs_region, comments=self._comments)
            self._current_countries[cname] = self._countries[cname]
        self._comments = []

    def _parse_country_item(self, line):
        if line[0] == '(':
            try:
                band, line = line[1:].split('),', 1)
                bname = 'UNNAMED %d' % self._lineno
                self._parse_band_def(bname, band, dupwarn=False)
            except:
                self._syntax_error("Badly parenthesised band definition")
        else:
            try:
                bname, line = line.split(',', 1)
                if not bname:
                    self._syntax_error("country definition must have band")
                if not line:
                    self._syntax_error("country definition must have power")
            except ValueError:
                self._syntax_error("country definition must have band and power")

        if line[0] == '(':
            items = line.split('),', 1)
            if len(items) == 1:
                pname = items[0]
                line = ''
                if not pname[-1] == ')':
                    self._syntax_error("Badly parenthesised power definition")
                pname = pname[:-1]
                flags = []
            else:
                pname = items[0]
                flags = items[1].split(',')
            power = pname[1:]
            pname = 'UNNAMED %d' % self._lineno
            self._parse_power_def(pname, power, dupwarn=False)
        else:
            line = line.split(',')
            pname = line[0]
            flags = line[1:]
        w = None
        if flags and 'wmmrule' in flags[-1]:
            try:
                region = flags.pop().split('=', 1)[1]
                if region not in self._wmm_rules.keys():
                    self._syntax_error("No wmm rule for %s" % region)
            except IndexError:
                self._syntax_error("flags is empty list or no region was found")
            w = WmmRule(*self._wmm_rules[region].values())

        if not bname in self._bands:
            self._syntax_error("band does not exist")
        if not pname in self._power:
            self._syntax_error("power does not exist")
        self._bands_used[bname] = True
        self._power_used[pname] = True
        # de-duplicate so binary database is more compact
        bname = self._banddup[bname]
        pname = self._powerdup[pname]
        b = self._bands[bname]
        p = self._power[pname]
        try:
            perm = Permission(b, p, flags, w)
        except FlagError as e:
            self._syntax_error("Invalid flag '%s'" % e.flag)
        for cname, c in self._current_countries.items():
            if perm in c:
                self._warn('Rule "%s, %s" added to "%s" twice' % (
                              bname, pname, cname))
            else:
                c.add(perm)

    def parse(self, f):
        self._current_countries = None
        self._current_regions = None
        self._bands = {}
        self._power = {}
        self._countries = {}
        self._bands_used = {}
        self._power_used = {}
        self._bandrev = {}
        self._powerrev = {}
        self._banddup = {}
        self._powerdup = {}
        self._bandline = {}
        self._powerline = {}
        self._wmm_rules = defaultdict(lambda: OrderedDict())

        self._comments = []

        self._lineno = 0
        for line in f:
            self._lineno += 1
            line = line.strip()
            if line[0:1] == '#':
                self._comments.append(line[1:].strip())
            line = line.replace(' ', '').replace('\t', '')
            if not line:
                self._current_regions = None
                self._comments = []
            line = line.split('#')[0]
            if not line:
                continue
            if line[0:4] == 'band':
                self._parse_band(line[4:])
                self._current_countries = None
                self._current_regions = None
                self._comments = []
            elif line[0:5] == 'power':
                self._parse_power(line[5:])
                self._current_countries = None
                self._current_regions = None
                self._comments = []
            elif line[0:7] == 'country':
                self._parse_country(line[7:])
                self._comments = []
                self._current_regions = None
            elif self._current_countries is not None:
                self._current_regions = None
                self._parse_country_item(line)
                self._comments = []
            elif line[0:7] == 'wmmrule':
                self._parse_wmmrule(line[7:])
                self._current_countries = None
                self._comments = []
            elif self._current_regions is not None:
                self._parse_wmmrule_item(line)
                self._current_countries = None
                self._comments = []
            else:
                self._syntax_error("Expected band, power or country definition")

        countries = self._countries
        bands = {}
        for k, v in self._bands.items():
            if k in self._bands_used:
                bands[self._banddup[k]] = v
                continue
            # we de-duplicated, but don't warn again about the dupes
            if self._banddup[k] == k:
                self._lineno = self._bandline[k]
                self._warn('Unused band definition "%s"' % k)
        power = {}
        for k, v in self._power.items():
            if k in self._power_used:
                power[self._powerdup[k]] = v
                continue
            # we de-duplicated, but don't warn again about the dupes
            if self._powerdup[k] == k:
                self._lineno = self._powerline[k]
                self._warn('Unused power definition "%s"' % k)
        return countries
