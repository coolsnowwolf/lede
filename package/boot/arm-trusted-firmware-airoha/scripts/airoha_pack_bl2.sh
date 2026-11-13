#!/bin/sh

# Airoha implement a complex logic for BL2
# BL21 is a minimal BL2 that gets loaded to NPU SRAM
# and is just a LZMA decompressor
# BL21 use the specific struct with all the info
# and decompresso/load the images.
# BL22 is what execute RAM calibration and first init setup
# BL23 is what prepares and load BL31
#
# BL2 is a concatenation of all these middle files and
# is organized as follow:
# 1. BL21
# 2. f_header_t struct
# 3. BL22
# 4. BL23
# 5. Special flash table (LZMA packed with NAND info table)

BL21=$1
BL22=$2
BL23=$3
FLASH_TABLE=$4
BL2=$5

cat $BL21 > $BL2

bl22_length=$(stat -c %s $BL22)
bl23_length=$(stat -c %s $BL23)
flash_table_length=$(stat -c %s $FLASH_TABLE)
lzma_src=$((0x1e843c00 + 36))
lzma_des=0x08004000
lzma_length=$bl22_length
lzma_cmd=0x0
fw_ver=0x0
reserved=0x0

write_le32()
{
    local v=$1
    printf "\\$(printf '%03o' $(( v        & 0xff)))"
    printf "\\$(printf '%03o' $(((v >> 8)  & 0xff)))"
    printf "\\$(printf '%03o' $(((v >> 16) & 0xff)))"
    printf "\\$(printf '%03o' $(((v >> 24) & 0xff)))"
}

# Write struct as little-endian unsigned ints
{
    write_le32 "$bl22_length"
    write_le32 "$bl23_length"
    write_le32 "$flash_table_length"
    write_le32 "$lzma_src"
    write_le32 "$lzma_des"
    write_le32 "$lzma_length"
    write_le32 "$lzma_cmd"
    write_le32 "$fw_ver"
    write_le32 "$reserved"
} >> $BL2

cat $BL22 >> $BL2
cat $BL23 >> $BL2
cat $FLASH_TABLE >> $BL2

crc=$(python3 -c 'import binascii, sys; print(binascii.crc32(sys.stdin.buffer.read()))' < "$BL2")

# XOR with 0xffffffff
crc=$((0xffffffff ^ $crc))

# append little endian
write_le32 "$crc" >> "$BL2"
