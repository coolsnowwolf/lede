Userspace utilties for jboot based devices config partition read

## Building

```
mkdir build
cd build
cmake /path/to/jboot-tools
make
```

## Usage

All command line parameters are documented:
```
jboot_config_read -h
```

Show all stored MACs:
```
jboot_config_read -m -i PATH_TO_CONFIG_PARTITIO
```

Extract wifi eeprom data:
```
jboot_config_read  -i PATH_TO_CONFIG_PARTITION -e OUTPUT_PATH
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
