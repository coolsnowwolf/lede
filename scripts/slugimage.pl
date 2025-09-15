#!/usr/bin/env perl
# 
# SlugImage : Manipulate NSLU2 firmware images
#             Dwayne Fontenot (jacques)
#             Rod Whitby (rwhitby)
#	      www.nslu2-linux.org
#
# Copyright (c) 2004, 2006, Dwayne Fontenot & Rod Whitby
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
# Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
# Neither the name of the NSLU2-Linux Development Team nor the names
# of its contributors may be used to endorse or promote products
# derived from this software without specific prior written
# permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

use strict;
use warnings;

use Getopt::Long qw(:config no_ignore_case);
use File::Temp qw(tempfile);

my($debug) = 0;
my($quiet) = 0;
my($flash_start)    = 0x50000000;
my($flash_len)      = 0x00800000;
my($block_size)     = 0x00020000;
my($kernel_offset)  = 0x00060000;
my($kernel_size)    = 0x00100000;
my($ramdisk_offset) = 0x00160000;
my(@cleanup);

# The last 70 bytes of the SercommRedBootTrailer (i.e. excluding MAC
# address).  Needed to create an image with an empty RedBoot partition
# since the Sercomm upgrade tool checks for this trailer.
# http://www.nslu2-linux.org/wiki/Info/SercommRedBootTrailer
my @sercomm_redboot_trailer = (0x4573, 0x4372, 0x4d6f, 0x006d, 0x0001,
       0x0400, 0x3170, 0x5895, 0x0010, 0x0000, 0x0000, 0x0000, 0x0000,
       0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
       0x0000, 0x0000, 0x0001, 0x0000, 0x0000, 0x0000, 0x0003, 0x2300,
       0x0063, 0x0000, 0x7320, 0x7245, 0x6f43, 0x6d4d);

# There's a 16 byte Sercomm trailer at the end of the flash. It is used
# by RedBoot to detect a Sercomm flash layout and to configure the
# Sercomm upgrade system.
# http://www.nslu2-linux.org/wiki/Info/SercommFlashTrailer
my @sercomm_flash_trailer = (0x0100, 0x0000, 0x6323, 0xf790, 0x5265,
                             0x4f63, 0x4d6d, 0xb400);

# Take $data, and pad it out to $total_len bytes, appending 0xff's.
sub padBytes {
    my($data,$total_len) = @_;

    # 0xFF is used to pad, as it's the erase value of the flash.
    my($pad_char) = pack("C",0xff);
    my($pad_len) = $total_len - length($data);

    # A request for negative padding is indicative of a logic error ...
    if (length($data) > $total_len) {
	die sprintf("padBytes error: data (%d) is longer than total_len (%d)", length($data), $total_len);
    }

    return $data . ($pad_char x $pad_len);
}

# Return the next multiple of block_size larger than or equal to $data_len.
sub paddedSize {
    my($data_len) = @_;

    use integer;
    return (($data_len - 1) / $block_size) * $block_size + $block_size;
}

# Return the number of block_size blocks required to hold $data_len.
sub numBlocks {
    my($data_len) = @_;

    use integer;
    return (($data_len - 1) / $block_size) + 1;
}

# Pack the name, address, size and optional skip regions of a partition entry into binary form.
sub createPartitionEntry {
    my($name, $flash_base, $size, $skips) = @_;
    my $entry;

    my($zero_long) = 0x0000;

    # Pack the partition entry according to the format that RedBoot (and the MTD partition parsing code) requires.
    $entry = pack("a16N5x212N2",$name,$flash_base,$zero_long,$size,$zero_long,$zero_long,$zero_long,$zero_long);

    # Optionally put a skip header into the padding area.
    if (defined $skips) {
	my $i = scalar(@$skips);
	foreach my $region (@$skips) {
	    substr($entry, -8 - 12*$i, 12) =
		pack("a4N2", "skip", $region->{'offset'}, $region->{'size'});
	    $i--;
	}
    }

    return $entry;
}

# Parse partition entry and return anon array ref [$name, $offset, $size, $skip] or return 0 on partition terminator.
sub parsePartitionEntry {
    my($partition_entry) = @_;

    my($entry_len) = 0x100;
    length($partition_entry) eq $entry_len or die "parsePartitionEntry: partition entry length is not $entry_len!\n";

    # Unpack the partition table entry, saving those values in which we are interested.
    my($name, $flash_base, $size, $dummy_long, $padding, $skips);
    ($name, $flash_base, $dummy_long, $size, $dummy_long, $dummy_long, $padding, $dummy_long, $dummy_long) =
	unpack("a16N5a212N2",$partition_entry);

    # A partition entry starting with 0xFF terminates the table.
    if (unpack("C", $name) eq 0xff) {
	# %%% FIXME: This should only skip, not terminate. %%%
	$debug and print "Found terminator for <FIS directory>\n";
	return 0;
    }

    # Remove trailing nulls from the partition name.
    $name =~ s/\000+//;

    # Extract the skip regions out of the padding area.
    $padding =~ s/^\000+//;
    $padding =~ s/\000*skip(........)\000*/$1/g;
    $padding =~ s/\000+$//;

    # Store the skip regions in an array for later use.
    while (length($padding)) {
	my $region = {};
	($region->{'offset'}, $region->{'size'}) =
	    unpack("N2", $padding);
	$debug and printf("Found skip region at 0x%05X, size 0x%05X\n",
			  $region->{'offset'}, $region->{'size'});
	push(@$skips, $region);
	$padding = substr($padding,8);
    }

    return [$name, $flash_base - $flash_start, $size, $skips];
}

# Return partition table from data is one exists, otherwise return 0.
sub findPartitionTable {
    my($data_buf) = @_;

    unpack("a7", $data_buf) eq 'RedBoot' or return 0;
    return substr($data_buf, 0, 0x1000)
}

# Parse partition table and return array of anonymous array references ([$name, $offset, $size, $skips], ...).
sub parsePartitionTable {
    my($partition_table) = @_;

    my(@partitions, $fields_ref);
    my($entry_len) = 0x100;
    my($partition_count) = 0;

    # Loop through the fixed size partition table entries, and store the entries in @partitions.
    # %%% FIXME: This doesn't handle the case of a completely full partition table. %%%
    while ($fields_ref = parsePartitionEntry(substr($partition_table, $partition_count * $entry_len, $entry_len))) {
	$debug and printf("Found <%s> at 0x%08X (%s)%s\n", $fields_ref->[0], $fields_ref->[1],
			  ($fields_ref->[2] >= $block_size ?
			   sprintf("%d blocks", numBlocks($fields_ref->[2])) :
			   sprintf("0x%05X bytes", $fields_ref->[2])),
			  (defined $fields_ref->[3] ?
			   sprintf(" [%s]",
				   join(", ",
					map { sprintf("0x%05X/0x%05X", $_->{'offset'},$_->{'size'}) }
					@{$fields_ref->[3]})) :
			   ""));
	$partitions[$partition_count++] = $fields_ref;
    }
    return(@partitions);
}

# Create an empty jffs2 block.
sub jffs2Block {
    return padBytes(pack("N3", 0x19852003, 0x0000000c, 0xf060dc98), $block_size);
}

# Write out $data to $filename,
sub writeOut {
    my($data, $filename) = @_;

    open FILE,">$filename" or die "Can't open file \"$filename\": $!\n";

    if (defined($data)) { print FILE $data;}

    close FILE or die "Can't close file \"$filename\": $!\n";
}

# Not used at the moment.
sub trailerData {
    my($product_id)       = 0x0001;
    my($protocol_id)      = 0x0000;
    my($firmware_version) = 0x2325;
    my($unknown1)         = 0x90f7;
    my($magic_number)     = 'eRcOmM';
    my($unknown2)         = 0x00b9;

    return pack("n4a6n",$product_id,$protocol_id,$firmware_version,$unknown1,$magic_number,$unknown2);
}

# Print the contents of the Sercomm RedBoot trailer.
sub printRedbootTrailer {
    my($redboot_data) = @_;

    my($correct_redboot_len) = 0x40000;
    my($redboot_data_len) = length($redboot_data);

    if ($redboot_data_len != $correct_redboot_len) {
	printf("Redboot length (0x%08X) is not 0x%08X\n", $redboot_data_len, $correct_redboot_len);
	return;
    }

    # The trailer is the last 80 bytes of the redboot partition.
    my($redboot_trailer) = substr($redboot_data, -80);

    writeOut($redboot_trailer, 'RedbootTrailer');

    my($mac_addr0, $mac_addr1, $mac_addr2, $unknown, $prefix, $ver_ctrl, $down_ctrl, $hid, $hver, $prodid, $prodidmask,
       $protid, $protidmask, $funcid, $funcidmask, $fver, $cseg, $csize, $postfix) =
	   unpack("n3Na7n2a32n10a7",$redboot_trailer);

    printf("MAC address is %04X%04X%04X\n", $mac_addr0, $mac_addr1, $mac_addr2);
    printf("unknown: %08X\n", $unknown);
    printf("%s:%04X:%04X:%s\n", $prefix, $ver_ctrl, $down_ctrl, $postfix);
    printf("VerControl: %04X\nDownControl: %04X\n", $ver_ctrl, $down_ctrl);
    printf("hid: %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X %04X\n", unpack("n16", $hid));
    printf("Hver: %04X\nProdID: %04X\nProtID: %04X\nFuncID: %04X\nFver: %04X\nCseg: %04X\nCsize: %04X\n",
	   $hver, $prodid, $protid, $funcid, $fver, $cseg, $csize);
}

# remove the optional Loader partition
sub removeOptionalLoader {
    my($partitions_ref) = @_;

    my $index;
    my $count = 0;
    map {
	if (not defined $index) {
	    if ($_->{'name'} eq "Loader") {
		$index = $count;
	    }
	    $count++;
	}
    } @$partitions_ref;
    
    defined $index or die "Cannot find the Loader partition\n";

    splice(@$partitions_ref, $index, 1);

    # Set fixed offsets and sizes for Kernel and Ramdisk
    map {
	if ($_->{'name'} eq 'Kernel') {
	    $_->{'offset'}   = $kernel_offset;
	    $_->{'size'}     = $kernel_size;
	    $_->{'variable'} = 0;
	}
	if ($_->{'name'} eq 'Ramdisk') {
	    $_->{'offset'}   = $ramdisk_offset;
	}
    } @$partitions_ref;

    return;
}


# populate @partitions based on the firmware's partition table
sub spliceFirmwarePartitions {
    my($firmware_buf, $partitions_ref) = @_;

    # we know that partition table, if it exists, begins at start of 'FIS directory' and has max length 0x1000
    my($partition_table);
    map {
	$_->{'name'} eq 'FIS directory' and
	    $partition_table = findPartitionTable(substr($firmware_buf, $_->{'offset'}, $_->{'size'}));
    } @$partitions_ref;

    # return 0 here if no partition table in FIS directory
    return if not $partition_table;

    my @new_partitions = parsePartitionTable($partition_table);

    # Remove the optional second stage bootloader if it is not found in the FIS directory.
    if (not grep { $_->[0] eq 'Loader' } @new_partitions) {
	removeOptionalLoader($partitions_ref);
    }

    my($partition_count) = 0;
    my($splice) = 0;
    map {

	# Skip pseudo partitions.
	while (($partition_count < scalar(@$partitions_ref)) and
	       $partitions_ref->[$partition_count]->{'pseudo'}) {
	    $debug and printf("Skipped <%s> (pseudo partition)\n", $partitions_ref->[$partition_count]->{'name'});
	    $partition_count++;
	}

	# If we are in a variable area, and we haven't reached the end of it,
	# then splice in another partition for use by the later code.
	if ($splice and ($partitions_ref->[$partition_count]->{'name'} ne $_->[0])) {
	    $debug and printf("Splicing new partition <%s> before <%s>\n",
			      $_->[0], $partitions_ref->[$partition_count]->{'name'});
	    splice(@{$partitions_ref}, $partition_count, 0, ({'name' => "",'variable'=>1,'header'=>0}));
	}

	my $partition = $partitions_ref->[$partition_count];

	# Variable partitions can be overridden by the real FIS directory
	if ($partition->{'variable'}) {

	    # Only override the filename if the partition name is not set or doesn't match
	    if ($partition->{'name'} ne $_->[0]) {

		if (length($partition->{'name'})) {
		    $debug and printf("Overwriting <%s> with <%s>\n",
				      $partitions_ref->[$partition_count]->{'name'}, $_->[0]);
		}

		$partition->{'name'} = $_->[0];
		$partition->{'file'} = $_->[0];
	    }

	    # Set the offset, size and skips based on the real partition table
	    $partition->{'offset'} = $_->[1];
	    $partition->{'size'}   = $_->[2];
	    $partition->{'skips'}  = $_->[3];

	    $debug and printf("Locating <%s> at 0x%08X (%s)\n",
			      $partition->{'name'}, $partition->{'offset'},
			      ($partition->{'size'} >= $block_size ?
			       sprintf("%d blocks", numBlocks($partition->{'size'})) :
			       sprintf("0x%05X bytes", $partition->{'size'})));

	    $splice = 1;
	}

	# Fixed partitions cannot be overridden
	else {
	    ($partition->{'name'} eq $_->[0]) or
		die "Unexpected partition <",$_->[0],"> (expecting <",$partition->{'name'},">)\n";

	    $debug and printf("Locating <%s> at 0x%08X (%s)\n",
			      $partition->{'name'}, $partition->{'offset'},
			      ($partition->{'size'} >= $block_size ?
			       sprintf("%d blocks", numBlocks($partition->{'size'})) :
			       sprintf("0x%05X bytes", $partition->{'size'})));
	    
	    $splice = 0;
	}
	
	$partition_count++;

    } @new_partitions;

    return;
}

# Read in an 8MB firmware file, and store the data into @partitions.
# Note that the data is only stored in a partition if 'offset' and 'size' are defined,
# and it does not already have data stored in it.
sub readInFirmware {
    my($filename, $partitions_ref) = @_;

    my($firmware_buf);

    open FILE,$filename or die "Can't find firmware image \"$filename\": $!\n";
    read FILE,$firmware_buf,$flash_len or die "Can't read $flash_len bytes from \"$filename\": $!\n";
    close FILE or die "Can't close \"$filename\": $!\n";

    $debug and printf("Read 0x%08X bytes from \"%s\"\n", length($firmware_buf), $filename);

    spliceFirmwarePartitions($firmware_buf, $partitions_ref);

    # Read the parts of the firmware file into the partitions table.
    map {
	if (defined $_->{'offset'} and defined $_->{'size'}) {

	    if (defined $_->{'data'}) {
		$debug and printf("Not overwriting data in <%s>\n", $_->{'name'});
	    }
	    else {

		# Slurp up the data, based on whether a header and/or data is present or not
		if ($_->{'header'}) {

		    # Read the length, and grab the data based on the length.
		    my($data_len) = unpack("N", substr($firmware_buf, $_->{'offset'}));

		    # A length of 0xFFFFFFFF means that the area is not initialised
		    if ($data_len != 0xFFFFFFFF) {
			$debug and printf("Found header size of 0x%08X bytes for <%s>\n", $data_len, $_->{'name'});
			$_->{'data'} = substr($firmware_buf, $_->{'offset'} + $_->{'header'}, $data_len);
		    }
		}
		elsif ($_->{'pseudo'} and not defined $_->{'file'} and
		       (substr($firmware_buf, $_->{'offset'}, $_->{'size'}) eq
			(pack("C", 0xff) x $_->{'size'}))) {
		    $debug and printf("Skipping empty pseudo partition <%s>\n", $_->{'name'});
		}
		else {

		    # Grab the whole partition, using the maximum size.
		    $_->{'data'} = substr($firmware_buf, $_->{'offset'}, $_->{'size'});
		}

		# If skip regions are defined, remove them from the data.
		if (defined $_->{'skips'}) {
		    my $removed = 0;
		    foreach my $region (@{$_->{'skips'}}) {
			if (($region->{'offset'} > 0) or
			    not ($_->{'header'} > 0)) {
			    $debug and printf("Removing 0x%05X bytes from offset 0x%05X\n",
					      $region->{'size'}, $region->{'offset'});
			    $region->{'data'} = substr($_->{'data'}, $region->{'offset'} - $removed, $region->{'size'}, '');
			}
			$removed += $region->{'size'};
		    }
		}

		$quiet or defined $_->{'data'} and printf("Read %s into <%s>\n",
							  (length($_->{'data'}) >= $block_size ?
							   sprintf("%d blocks", numBlocks(length($_->{'data'}))) :
							   sprintf("0x%05X bytes", length($_->{'data'}))), $_->{'name'});
	    }
	}
    } @$partitions_ref;
}

# Write the partition data stored in memory out into the files associated with each.
sub writeOutFirmwareParts {
    my(@partitions) = @_;

    # Write out the parts of the firmware file.
    map {

	# We can only write if 'data' and 'file' are defined.
	if (defined $_->{'file'} and defined $_->{'data'} and length($_->{'data'})) {
	    writeOut($_->{'data'}, $_->{'file'});
	    $quiet or printf("Wrote 0x%08X bytes from <%s> into \"%s\"\n",
			      length($_->{'data'}), $_->{'name'}, $_->{'file'});
	}
	else {
	    $debug and printf("Skipping <%s> (%s)\n", $_->{'name'},
			      (not defined $_->{'file'}) ?
			      "no filename specified" :
			      "no data to write");
	}

    } @partitions;

    return;
}

# Read in the partition data from the files associated with each and store in memory.
sub readInFirmwareParts {
    my(@partitions) = (@_);
    
    undef $/; # we want to slurp

    map {

	my $file = $_->{'file'};
	if (defined $file) {
	    open FILE,$file or die "Can't find firmware part \"$file\": $!\n";

	    # Slurp in the data
	    $_->{'data'} = <FILE>;

	    # close the file
	    close FILE or die "Can't close file \"$file\": $!\n";

	    # Optionally byteswap the data
	    if ($_->{'byteswap'}) {
		# Byte swap the data (which has to be padded to a multiple of 4 bytes).
		$_->{'data'} = pack("N*", unpack("V*", $_->{'data'}.pack("CCC", 0)));
	    }

	    # Keep track of the actual size.
	    my $size;

	    if ($_->{'header'}) {
		if ($_->{'pseudo'}) {
		    $size = $_->{'header'} + length($_->{'data'});
		}
		else {
		    $size = paddedSize($_->{'header'} + length($_->{'data'}));
		}
	    }
	    elsif (not $_->{'pseudo'}) {
		$size = paddedSize(length($_->{'data'}));
	    }
	    else {
		$size = length($_->{'data'});
	    }

	    # Check to make sure the file contents are not too large.
	    if (defined $_->{'size'} and ($size > $_->{'size'})) {
		die sprintf("Ran out of flash space in <%s> - %s too large.\n", $_->{'name'},
			    sprintf("0x%05X bytes", ($size - $_->{'size'})));
	    }

	    # If the partition does not have a fixed size, the calculate the size.
	    if (not defined $_->{'size'}) {
		$_->{'size'} = $size;
	    }

	    # Keep the user appraised ...
	    $quiet or printf("Read 0x%08X bytes from \"%s\" into <%s> (%s / %s)%s\n",
			     length($_->{'data'}), $_->{'file'}, $_->{'name'},
			     ($size >= $block_size ?
			      sprintf("%d blocks", numBlocks($size)) :
			      sprintf("0x%05X bytes", $size)),
			     ($_->{'size'} >= $block_size ?
			      sprintf("%d blocks", numBlocks($_->{'size'})) :
			      sprintf("0x%05X bytes", $_->{'size'})),
			     ($_->{'byteswap'} ? " (byte-swapped)" : ""));
	}

    } @partitions;

    return;
}

# layoutPartitions : this function must be ugly - it needs to verify RedBoot, SysConf, Kernel, Ramdisk, and
#     FIS directory partitions exist, are in the correct order, and do not have more data than can fit in
#     their lengths (fixed for all but Ramdisk, which has a minimum length of one block).
#     If Rootdisk and/or Userdisk exist, it must also verify that their block padded lengths are not
#     too great for the available space.
# input : an array of hashes, some of which are populated with data
# output: same reference with start and size (partition not data) also populated. this populated structure
#         can then be passed to buildPartitionTable() to generate the actual partition table data
sub layoutPartitions {
    my(@partitions) = @_;

    # Find the kernel partition, and save a pointer to it for later use
    my $kernel;
    map { ($_->{'name'} eq "Kernel") && ($kernel = $_); } @partitions;
    $kernel or die "Couldn't find the kernel partition\n";

    # Find the last variable size partition, and save a pointer to it for later use
    my $lastdisk;
    my $directory_offset;
    my $curdisk = $partitions[0];
    map {
	if (not defined $lastdisk) {
	    if ($_->{'name'} eq "FIS directory") {
		$lastdisk = $curdisk;
		$directory_offset = $_->{'offset'};
	    }
	    else {
		$curdisk = $_;
	    }
	}
    } @partitions;

    $lastdisk or die "Couldn't find the last variable size partition\n";

    $debug and printf("Last variable size partition is <%s>\n", $lastdisk->{'name'});

    #
    # here we go through the $partitions array ref and fill in all the values
    #

    # This points to where the next partition should be placed.
    my $pointer = $flash_start;

    map {

	$debug and printf("Pointer is 0x%08X\n", $pointer);

	# Determine the start and offset of the current partition.
	if (defined $_->{'offset'}) {
	    $_->{'start'} = $flash_start + $_->{'offset'};
	    # Check for running past the defined start of the partition.
	    if (($pointer > $_->{'start'}) and not $_->{'pseudo'}) {
		die sprintf("Ran out of flash space before <%s> - %s too large.\n", $_->{'name'},
			    sprintf("0x%05X bytes", ($pointer - $_->{'start'})));
	    }
	}

	# If offset is not defined, then calculate it.
	else {
	    $_->{'start'} = $pointer;
	    $_->{'offset'} = $_->{'start'} - $flash_start;
	}

	my $size = defined $_->{'data'} ? length($_->{'data'}) : 0;

	# Add skip regions for the partitions with headers.
	if ($_->{'header'} > 0) {
	    # Define the skip region for the initial Sercomm header.
	    push(@{$_->{'skips'}},
		 { 'offset' => 0, 'size' => $_->{'header'}, 'data' => undef });
	    # Allow for the Sercomm header to be prepended to the data.
	    $size += $_->{'header'};

	    # Determine if the partition overlaps the ramdisk boundary.
	    if (($_->{'offset'} < $ramdisk_offset) and
		(($_->{'offset'} + $size) > $ramdisk_offset)) {
		# Define the skip region for the inline Sercomm header.
		push(@{$_->{'skips'}},
		     { 'offset' => ($ramdisk_offset - $_->{'offset'}), 'size' => 16,
		       'data' => pack("N4", $block_size) });
		# Allow for the Sercomm header to be inserted in the data.
		$size += 16;
	    }
	}

	# Partitions without headers cannot have skip regions.
	elsif (($_->{'offset'} <= $ramdisk_offset) and
	       (($_->{'offset'} + $size) > $ramdisk_offset)) {
	    # Pad the kernel until it extends past the ramdisk offset.
	    push(@{$kernel->{'skips'}},
		 { 'offset' => ($ramdisk_offset - $kernel->{'offset'}), 'size' => 16,
		   'data' => pack("N4", $block_size) });
	    $kernel->{'size'} = $ramdisk_offset - $kernel->{'offset'} + $block_size;
	    $kernel->{'data'} = padBytes($kernel->{'data'},
					 $kernel->{'size'} - $kernel->{'header'} - 16);
	    $_->{'offset'} = $ramdisk_offset + $block_size;
	    $_->{'start'} = $flash_start + $_->{'offset'};
	    $pointer = $_->{'start'};
	    $debug and printf("Extending kernel partition past ramdisk offset.\n");
	}

	# If this is the last variable size partition, then fill the rest of the space.
	if ($_->{'name'} eq $lastdisk->{'name'}) {
	    $_->{'size'} = paddedSize($directory_offset + $flash_start - $pointer);
	    $debug and printf("Padding last variable partition <%s> to 0x%08X bytes\n", $_->{'name'}, $_->{'size'});
	}

	die sprintf("Partition size not defined in <%s>.\n", $_->{'name'})
	    unless defined $_->{'size'};

	# Extend to another block if required.
	if ($size > $_->{'size'}) {
	    if ($_->{'name'} eq $lastdisk->{'name'}) {
		die sprintf("Ran out of flash space in <%s> - %s too large.\n", $_->{'name'},
			    sprintf("0x%05X bytes", ($size - $_->{'size'})));
	    }
	    $_->{'size'} = $size;
	    printf("Extending partition <%s> to 0x%08X bytes\n", $_->{'name'}, $_->{'size'});
	}

	# Keep the user appraised ...
	$debug and printf("Allocated <%s> from 0x%08X to 0x%08X (%s / %s)\n",
			  $_->{'name'}, $_->{'start'}, $_->{'start'} + $_->{'size'},
			  ($size >= $block_size ?
			   sprintf("%d blocks", numBlocks($size)) :
			   sprintf("0x%05X bytes", $size)),
			  ($_->{'size'} >= $block_size ?
			   sprintf("%d blocks", numBlocks($_->{'size'})) :
			   sprintf("0x%05X bytes", $_->{'size'})));

	# Check to make sure we have not run out of room.
	if (($_->{'start'} + $_->{'size'}) > ($flash_start + $flash_len)) {
	    die "Ran out of flash space in <", $_->{'name'}, ">\n";
	}

	$debug and printf("Moving pointer from 0x%08X to 0x%08X (0x%08X + 0x%08X)\n",
			  $pointer, paddedSize($_->{'start'} + $_->{'size'}),
			  $_->{'start'}, $_->{'size'});

	# Move the pointer up, in preparation for the next partition.
	$pointer = paddedSize($_->{'start'} + $_->{'size'});

    } @partitions;

    return;
}

sub buildPartitionTable {
    my(@partitions) = @_;

    my($flash_start) = 0x50000000;
    my($partition_data) = '';

    map {

	# Collate the partition data for all known partitions.
	if (not $_->{'pseudo'} and defined $_->{'offset'} and defined $_->{'size'}) {

	    # Pack and append the binary table entry for this partition.
	    $partition_data .= createPartitionEntry($_->{'name'}, $_->{'offset'} + $flash_start,
						    $_->{'size'}, $_->{'skips'});

	    # If this is the FIS directory, then write the partition table data into it.
	    if ($_->{'name'} eq "FIS directory") {
		# Explicitly terminate the partition data.
		$partition_data .= pack("C",0xff) x 0x100;
		$_->{'data'} = padBytes($partition_data, $_->{'size'});
	    }

	    my $size = length($_->{'data'});

	    # Keep the user appraised ...
	    $debug and printf("Table entry <%s> from 0x%08X to 0x%08X (%s / %s)%s\n",
			      $_->{'name'}, $_->{'start'}, $_->{'start'} + $_->{'size'},
			      ($size >= $block_size ?
			       sprintf("%d blocks", numBlocks($size)) :
			       sprintf("0x%05X bytes", $size)),
			      ($_->{'size'} >= $block_size ?
			       sprintf("%d blocks", numBlocks($_->{'size'})) :
			       sprintf("0x%05X bytes", $_->{'size'})),
			      (defined $_->{'skips'} ?
			       sprintf("\nTable entry <%s> skip %s", $_->{'name'},
				       join(", ",
					    map { sprintf("0x%08X to 0x%08X", $_->{'offset'},
							  $_->{'offset'} + $_->{'size'} - 1) }
					    @{$_->{'skips'}})) :
			       "")
			      );
	}
	else {
	    $debug and print "No table entry required for <", $_->{'name'}, ">\n";
	}

    } @partitions;

    return;
}

sub writeOutFirmware {
    my($filename, @partitions) = @_;

    # Clear the image to start.
    my $image_buf = "";

    map {

	# We can only write a partition if it has an offset, a size, and some data to write.
	if (defined $_->{'offset'} and defined $_->{'size'} and defined $_->{'data'}) {

	    # Keep track of the end of the image.
	    my $end_point = length($image_buf);

	    # If the next partition is well past the end of the current image, then pad it.
	    if ($_->{'offset'} > $end_point) {
		$image_buf .= padBytes("", $_->{'offset'} - $end_point);
		$quiet or printf("Padded %s before <%s> in \"%s\"\n",
				 ((length($image_buf) - $end_point) >= $block_size ?
				  sprintf("%d blocks", numBlocks(length($image_buf) - $end_point)) :
				  sprintf("0x%05X bytes", length($image_buf) - $end_point)),
				 $_->{'name'}, $filename);
	    }

	    # If the next partition is before the end of the current image, then rewind.
	    elsif ($_->{'offset'} < $end_point) {
		$debug and printf("Rewound %s before <%s> in \"%s\"\n",
				  (($end_point - $_->{'offset'}) >= $block_size ?
				   sprintf("%d blocks", numBlocks($end_point - $_->{'offset'})) :
				   sprintf("0x%05X bytes", $end_point - $_->{'offset'})),
				  $_->{'name'}, $filename);
# 		if (($end_point - $_->{'offset'}) >= $block_size) {
# 		    die "Allocation error: rewound a full block or more ...\n";
# 		}
	    }

	    # If skip regions are defined, add them to the data.
	    if (defined $_->{'skips'}) {
		my $added = 0;
		foreach my $region (@{$_->{'skips'}}) {
		    if (($region->{'offset'} > 0) or
			not ($_->{'header'} > 0)) {
			$debug and printf("Inserted 0x%05X bytes (at offset 0x%05X) into <%s>\n",
					  $region->{'size'}, $region->{'offset'}, $_->{'name'});
			substr($_->{'data'},
			       $region->{'offset'} + $added - $_->{'header'},
			       0, $region->{'data'});
			$added += $region->{'size'};
		    }
		}
	    }

	    # Splice the data into the image at the appropriate place, padding as required.
	    substr($image_buf, $_->{'offset'}, $_->{'size'},
		   $_->{'header'} ?
		   padBytes(pack("N4",length($_->{'data'})).$_->{'data'}, $_->{'size'}) :
		   padBytes($_->{'data'}, $_->{'size'}));
	    
	    # Keep the user appraised ...
	    $quiet or printf("Wrote %s (0x%08X to 0x%08X) from <%s> into \"%s\"\n",
			     ($_->{'size'} >= $block_size ?
			      sprintf("%2d blocks", numBlocks($_->{'size'})) :
			      sprintf("0x%05X bytes", $_->{'size'})),
			     $_->{'offset'}, $_->{'offset'}+$_->{'size'}, $_->{'name'}, $filename);
	}

	# If we are not able to write a partition, then give debug information about why.
	else {
	    $debug and printf("Skipping <%s> (%s)\n", $_->{'name'},
			      (not defined $_->{'offset'}) ? "no offset defined" :
			      ((not defined $_->{'size'}) ? "no size defined" :
			       "no data available"));
	}

    } @partitions;

    # Write the image to the specified file.
    writeOut($image_buf, $filename);

    return;
}

# checkPartitionTable: sanity check partition table - for testing but might evolve into setting @partitions
#    so that we can write out jffs2 partitions from a read image
#    currently not nearly paranoid enough
sub checkPartitionTable {
    my($data) = @_;

    my($pointer) = 0;
    my($entry);

    my($name, $flash_base, $size, $done, $dummy_long, $padding);
    do {
	$entry = substr($data, $pointer, 0x100);

	($name,$flash_base,$dummy_long,$size,$dummy_long,$dummy_long,$padding,$dummy_long,$dummy_long) = unpack("a16N5x212N2",$entry);
	$name =~ s/\0//g;
	$debug and printf("pointer: %d\tname: %s%sflash_base: 0x%08X\tsize: 0x%08X\n",
			  $pointer, $name, (" " x (16 - length($name))), $flash_base, $size);
	$pointer += 0x100;
	$debug and printf("terminator: 0x%08X\n", unpack("C", substr($data, $pointer, 1)));
	if (unpack("C", substr($data, $pointer, 1)) eq 0xff) {
	    $done = 1;
	}
    } until $done;
}

sub printPartitions {
    my(@partitions) = @_;

    my($offset, $size, $skips);
    map {
#	defined $_->{'size'} ? $size = $_->{'size'} : $size = undef;

	if (defined  $_->{'size'}) {
	    $size = $_->{'size'};
	}
	else {
	    $size = undef;
	}
	if (defined  $_->{'offset'}) {
	    $offset = $_->{'offset'};
	}
	else {
	    $offset = undef;
	}
	if (defined  $_->{'skips'}) {
	    $skips = $_->{'skips'};
	}
	else {
	    $skips = undef;
	}
	printf("%s%s", $_->{'name'}, (" " x (16 - length($_->{'name'}))));
	if (defined $offset) { printf("0x%08X\t", $offset); } else { printf("(undefined)\t");   };
	if (defined $size)   { printf("0x%08X", $size); } else { printf("(undefined)"); };
	if (defined $skips) {
	    printf("\t[%s]",
		   join(", ",
			map { sprintf("0x%05X/0x%05X", $_->{'offset'}, $_->{'size'}); }
			@$skips));
	}
	printf("\n");
    } @partitions;
}

sub defaultPartitions {

    return ({'name'=>'RedBoot',          'file'=>'RedBoot',
	     'offset'=>0x00000000,        'size'=>0x00040000,
	     'variable'=>0, 'header'=>0,  'pseudo'=>0, 'data'=>undef, 'byteswap'=>0},
	    {'name'=>'EthAddr',           'file'=>undef,
	     'offset'=>0x0003ffb0,        'size'=>0x00000006,
	     'variable'=>0, 'header'=>0,  'pseudo'=>1, 'data'=>undef, 'byteswap'=>0},
	    {'name'=>'SysConf',           'file'=>'SysConf',
	     'offset'=>0x00040000,        'size'=>0x00020000,
	     'variable'=>0, 'header'=>0,  'pseudo'=>0, 'data'=>undef, 'byteswap'=>0},
	    {'name'=>'Loader',            'file'=>'apex.bin',
	     'offset'=>undef,             'size'=>undef,
	     'variable'=>1, 'header'=>16, 'pseudo'=>0, 'data'=>undef, 'byteswap'=>0},
	    {'name'=>'Kernel',            'file'=>'vmlinuz',
	     'offset'=>undef,             'size'=>undef,
	     'variable'=>1, 'header'=>16, 'pseudo'=>0, 'data'=>undef, 'byteswap'=>0},
	    {'name'=>'Ramdisk',           'file'=>'ramdisk.gz',
	     'offset'=>undef,             'size'=>undef,
	     'variable'=>1, 'header'=>16, 'pseudo'=>0, 'data'=>undef, 'byteswap'=>0},
	    {'name'=>'FIS directory',     'file'=>undef,
	     'offset'=>0x007e0000,        'size'=>0x00020000,
	     'variable'=>0, 'header'=>0,  'pseudo'=>0, 'data'=>undef, 'byteswap'=>0},
	    {'name'=>'Loader config',	  'file'=>undef,
	     'offset'=>0x007f8000,        'size'=>0x00004000,
	     'variable'=>0, 'header'=>0,  'pseudo'=>1, 'data'=>undef, 'byteswap'=>0},
	    {'name'=>'Microcode',	  'file'=>'NPE-B',
	     'offset'=>0x007fc000,        'size'=>0x00003fe0,
	     'variable'=>0, 'header'=>16, 'pseudo'=>1, 'data'=>undef, 'byteswap'=>0},
	    {'name'=>'Trailer',           'file'=>'Trailer',
	     'offset'=>0x007ffff0,        'size'=>0x00000010,
	     'variable'=>0, 'header'=>0,  'pseudo'=>1, 'data'=>undef, 'byteswap'=>0});
}

# Main routine starts here ...

my($unpack, $pack, $little, $fatflash, $input, $output, $redboot);
my($kernel, $sysconf, $ramdisk, $fisdir);
my($microcode, $trailer, $ethaddr, $loader);

END {
    # Remove temporary files
    for my $file (@cleanup) {
	unlink $file;
    }
}

if (!GetOptions("d|debug"       => \$debug,
		"q|quiet"       => \$quiet,
		"u|unpack"      => \$unpack,
		"p|pack"        => \$pack,
		"l|little"      => \$little,
		"F|fatflash"    => \$fatflash,
		"i|input=s"     => \$input,
		"o|output=s"    => \$output,
		"b|redboot=s"   => \$redboot,
		"k|kernel=s"    => \$kernel,
		"s|sysconf=s"   => \$sysconf,
		"r|ramdisk=s"   => \$ramdisk,
		"f|fisdir=s"    => \$fisdir,
		"m|microcode=s" => \$microcode,
		"t|trailer=s"   => \$trailer,
		"e|ethaddr=s"   => \$ethaddr,
		"L|loader=s"    => \$loader,
		) or (not defined $pack and not defined $unpack)) {
    print "Usage: slugimage <options>\n";
    print "\n";
    print "  [-d|--debug]			Turn on debugging output\n";
    print "  [-q|--quiet]			Turn off status messages\n";
    print "  [-u|--unpack]			Unpack a firmware image\n";
    print "  [-p|--pack]			Pack a firmware image\n";
    print "  [-l|--little]			Convert Kernel and Ramdisk to little-endian\n";
    print "  [-F|--fatflash]			Generate an image for 16MB flash\n";
    print "  [-i|--input]     <file>		Input firmware image filename\n";
    print "  [-o|--output]    <file>		Output firmware image filename\n";
    print "  [-b|--redboot]   <file>		Input/Output RedBoot filename\n";
    print "  [-s|--sysconf]   <file>		Input/Output SysConf filename\n";
    print "  [-L|--loader]    <file>		Second stage boot loader filename\n";
    print "  [-k|--kernel]    <file>		Input/Output Kernel filename\n";
    print "  [-r|--ramdisk]   <file>		Input/Output Ramdisk filename(s)\n";
    print "  [-f|--fisdir]    <file>		Input/Output FIS directory filename\n";
    print "  [-m|--microcode] <file>		Input/Output Microcode filename\n";
    print "  [-t|--trailer]   <file>		Input/Output Trailer filename\n";
    print "  [-e|--ethaddr]   <AABBCCDDEEFF>	Set the Ethernet address\n";

    # %%% TODO %%% Document --ramdisk syntax

    exit 1;
}

my(@partitions) = defaultPartitions();

if ($pack) {
    die "Output filename must be specified\n" unless defined $output;

    # If we're creating an image and no RedBoot, SysConf partition is
    # explicitly specified, simply write an empty one as the upgrade tools
    # don't touch RedBoot and SysConf anyway.  If no Trailer is specified,
    # put in one.
    if (not defined $redboot and not -e "RedBoot") {
	$redboot = tempfile();
	open TMP, ">$redboot" or die "Cannot open file $redboot: $!";
	push @cleanup, $redboot;
	# The RedBoot partition is 256 * 1024 = 262144; the trailer we add
	# is 70 bytes.
	print TMP "\0"x(262144-70);
	# Upgrade tools check for an appropriate Sercomm trailer.
	for my $i (@sercomm_redboot_trailer) {
	    print TMP pack "S", $i;
	}
	close TMP;
    }
    if (not defined $sysconf and not -e "SysConf") {
	$sysconf = tempfile();
	open TMP, ">$sysconf" or die "Cannot open file $sysconf: $!";
	push @cleanup, $sysconf;
	# The SysConf partition is 128 * 1024 = 131072
	print TMP "\0"x131072;
	close TMP;
    }
    if (not defined $trailer and not -e "Trailer") {
	$trailer = tempfile();
	open TMP, ">$trailer" or die "Cannot open file $trailer: $!";
	push @cleanup, $trailer;
	for my $i (@sercomm_flash_trailer) {
	    print TMP pack "S", $i;
	}
	close TMP;
    }

    # If the microcode was not specified, then don't complain that it's missing.
    if (not defined $microcode and not -e "NPE-B") {
	map { ($_->{'name'} eq 'Microcode') && ($_->{'file'} = undef);   } @partitions;
    }
}

# Go through the partition options, and set the names and files in @partitions
if (defined $redboot)   { map { ($_->{'name'} eq 'RedBoot')	  && ($_->{'file'} = $redboot);   } @partitions; }
if (defined $sysconf)   { map { ($_->{'name'} eq 'SysConf')	  && ($_->{'file'} = $sysconf);   } @partitions; }
if (defined $loader)    { map { ($_->{'name'} eq 'Loader')	  && ($_->{'file'} = $loader);    } @partitions; }
if (defined $kernel)    { map { ($_->{'name'} eq 'Kernel')	  && ($_->{'file'} = $kernel);    } @partitions; }
if (defined $fisdir)    { map { ($_->{'name'} eq 'FIS directory') && ($_->{'file'} = $fisdir);    } @partitions; }
if (defined $microcode) { map { ($_->{'name'} eq 'Microcode')	  && ($_->{'file'} = $microcode); } @partitions; }
if (defined $trailer)   { map { ($_->{'name'} eq 'Trailer')	  && ($_->{'file'} = $trailer);   } @partitions; }

if (defined $little)  {
    map {
	if (($_->{'name'} eq 'Loader') or
	    ($_->{'name'} eq 'Kernel') or
	    ($_->{'name'} eq 'Ramdisk')) {
	    $_->{'byteswap'} = 1;
	}
    } @partitions;
}

if (defined $fatflash)  {
    $flash_len = 0x01000000;
    map {
	if (($_->{'name'} eq 'FIS directory') or
	    ($_->{'name'} eq 'Loader config') or
	    ($_->{'name'} eq 'Microcode') or
	    ($_->{'name'} eq 'Trailer')) {
	    $_->{'offset'} += 0x00800000;
	}
    } @partitions;
}

if (defined $ethaddr) {
    map {
	if ($_->{'name'} eq 'EthAddr') {
	    $ethaddr =~ s/://g;
	    if (($ethaddr !~ m/^[0-9A-Fa-f]+$/) or (length($ethaddr) != 12)) {
		die "Invalid ethernet address specification: '".$ethaddr."'\n";
	    }
	    $_->{'data'} = pack("H12", $ethaddr);
	}
    } @partitions;
}

if (defined $ramdisk) {

    # A single filename is used for the ramdisk filename
    if ($ramdisk !~ m/[:,]/) {
	map { ($_->{'name'} eq 'Ramdisk') && ($_->{'file'} = $ramdisk); } @partitions;
    }

    # otherwise, it's a list of name:file mappings
    else {
	my @mappings = split(',', $ramdisk);

	# Find the index of the Ramdisk entry
	my $index;
	my $count = 0;
	map {
	    if (not defined $index) {
		if ($_->{'name'} eq "Ramdisk") {
		    $index = $count;
		}
		$count++;
	    }
	} @partitions;

	defined $index or die "Cannot find the Ramdisk partition\n";

	# Replace the Ramdisk entry with the new mappings
	splice(@partitions, $index, 1, map {

	    # Preserve the information from the ramdisk entry
	    my %entry = %{$partitions[$index]};

	    # Parse the mapping
	    ($_ =~ m/^([^:]+):([^:]+)(:([^:]+))?$/) or die "Invalid syntax in --ramdisk\n";
	    $entry{'name'} = $1; $entry{'file'} = $2; my $size = $4;

	    # If the mapping is not for the ramdisk, then undefine its attributes
	    if ($entry{'name'} ne 'Ramdisk') {
		$entry{'offset'} = undef;
		$entry{'size'} = undef;
		$entry{'variable'} = 1;
		$entry{'header'} = 0;
		$entry{'pseudo'} = 0;
		$entry{'data'} = undef;
		$entry{'byteswap'} = 0;
	    }

	    # Support specification of the number of blocks for empty jffs2
	    if ($entry{'file'} =~ m/^[0-9]+$/) {
		$size = $entry{'file'};
		$entry{'file'} = undef;
	    }

	    # If the user has specified a size, then respect their wishes
	    if (defined $size) {
		$entry{'size'} = $size * $block_size;
		# Create an empty partition of the requested size.
		$entry{'data'} = padBytes("", $entry{'size'} - $entry{'header'});
	    }

	    \%entry;

	} @mappings);
    }
}

# Read in the firmware image
if ($input) {
    if ($debug) {
	print "Initial partition map:\n";
	printPartitions(@partitions);
    }
    
    my $result = readInFirmware($input, \@partitions);

    if ($debug) {
	print "After reading firmware:\n";
	printPartitions(@partitions);
    }
}

# Unpack the firmware if requested
if ($unpack) {
    die "Input filename must be specified\n" unless defined $input;

#    map {
#	($_->{'name'} eq 'FIS directory') and @partitions = checkPartitionTable($_->{'data'});
#    } @partitions;

    writeOutFirmwareParts(@partitions);

}

# Pack the firmware if requested
if ($pack) {

    if (!defined $loader) {
	removeOptionalLoader(\@partitions);
    }

    if ($debug) {
	print "Initial partition map:\n";
	printPartitions(@partitions);
    }
    
    my $result = readInFirmwareParts(@partitions);

    if ($debug) {
	print "after readInFirmwareParts():\n";
	printPartitions(@partitions);
# 	map {
# 	    ($_->{'name'} eq 'RedBoot') && (printRedbootTrailer($_->{'data'}));
# 	} @partitions;
    }
    
    layoutPartitions(@partitions);

    if ($debug) {
 	print "after layoutPartitions():\n";
 	printPartitions(@partitions);
    }
    
    buildPartitionTable(@partitions);

    if ($debug) {
 	print "after buildPartitionTable():\n";
 	printPartitions(@partitions);

#  	my($lastblock);
#  	map {
#  	    if ($_->{'name'} eq 'FIS directory') {
#  		$lastblock = $_->{'data'};
#  	    }
#  	} @partitions;

#  	print "checkPartitionTable():\n";
#  	checkPartitionTable($lastblock);
    }
    
    writeOutFirmware($output, @partitions);

}

exit 0;
