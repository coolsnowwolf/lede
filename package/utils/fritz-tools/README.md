Userspace utilties for accessing TFFS (a name-value storage usually found in AVM Fritz!Box based devices)

## Building

```
mkdir build
cd build
cmake /path/to/fritz_tffs_tools
make
```

## Usage

All command line parameters are documented:
```
fritz_tffs_read -h
```

Show all entries from a TFFS partition dump  (in the format: name=value):
```
fritz_tffs_read -i /path/to/tffs.dump -a
```

Read a TFFS partition and show all entries (in the format: name=value):
```
fritz_tffs_read -i /dev/mtdX -a
```

Output only the value of a specific key (this will only show the value):
```
fritz_tffs_read -i /dev/mtdX -n my_ipaddress
```

## LICENSE

See `LICENSE`:

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
