#!/bin/sh

# Check for ath10k (and maybe other) bugs, package them up,
# and let user know what to do with them.

TMPLOC=/tmp
CRASHDIR=$TMPLOC/bugcheck
FOUND_BUG=0

# set -x

bugcheck_generic()
{
    echo "OpenWrt crashlog report" > $CRASHDIR/info.txt
    date >> $CRASHDIR/info.txt
    echo >> $CRASHDIR/info.txt
    echo "uname" >> $CRASHDIR/info.txt
    uname -a >> $CRASHDIR/info.txt
    echo >> $CRASHDIR/info.txt
    echo "os-release" >> $CRASHDIR/info.txt
    cat /etc/os-release >> $CRASHDIR/info.txt
    echo >> $CRASHDIR/info.txt
    echo "os-release" >> $CRASHDIR/info.txt
    cat /etc/os-release >> $CRASHDIR/info.txt
    echo >> $CRASHDIR/info.txt
    echo "dmesg output" >> $CRASHDIR/info.txt
    dmesg >> $CRASHDIR/info.txt
    if [ -x /usr/bin/lspci ]
	then
	echo >> $CRASHDIR/info.txt
	echo "lspci" >> $CRASHDIR/info.txt
	lspci >> $CRASHDIR/info.txt
    fi
    echo >> $CRASHDIR/info.txt
    echo "cpuinfo" >> $CRASHDIR/info.txt
    cat /proc/cpuinfo >> $CRASHDIR/info.txt
    echo >> $CRASHDIR/info.txt
    echo "meminfo" >> $CRASHDIR/info.txt
    cat /proc/cpuinfo >> $CRASHDIR/info.txt
    echo >> $CRASHDIR/info.txt
    echo "cmdline" >> $CRASHDIR/info.txt
    cat /proc/cmdline >> $CRASHDIR/info.txt
    echo >> $CRASHDIR/info.txt
    echo "lsmod" >> $CRASHDIR/info.txt
    lsmod >> $CRASHDIR/info.txt
}

roll_crashes()
{
    # Roll any existing crashes
    if [ -d $CRASHDIR ]
	then
	if [ -d $CRASHDIR.1 ]
	    then
	    rm -fr $CRASHDIR.1
	fi
	mv $CRASHDIR $CRASHDIR.1
    fi

    # Prepare location
    mkdir -p $CRASHDIR
}

# ath10k, check debugfs entries.
for i in /sys/kernel/debug/ieee80211/*/ath10k/fw_crash_dump
do
  #echo "Checking $i"
  if cat $i > $TMPLOC/ath10k_crash.bin 2>&1
      then
      FOUND_BUG=1

      #echo "Found ath10k crash data in $i"
      roll_crashes

      ADIR=${i/fw_crash_dump/}

      CTFW=0
      if grep -- -ct- $TMPLOC/ath10k_crash.bin > /dev/null 2>&1
	  then
	  CTFW=1
      fi

      echo "Send bug reports to:" > $CRASHDIR/report_to.txt
      if [ -f $ADIR/ct_special -o $CTFW == "1" ]
	  then
	  # Looks like this is CT firmware or driver...
	  echo "greearb@candelatech.com" >> $CRASHDIR/report_to.txt
	  echo "and/or report or check for duplicates here:" >> $CRASHDIR/report_to.txt
	  echo "https://github.com/greearb/ath10k-ct/issues" >> $CRASHDIR/report_to.txt
      else
	  # Not sure who would want these bug reports for upstream...
	  echo "https://www.lede-project.org/" >> $CRASHDIR/report_to.txt
      fi
      echo >> $CRASHDIR/report_to.txt
      echo "Please attach all files in this directory to bug reports." >> $CRASHDIR/report_to.txt

      mv $TMPLOC/ath10k_crash.bin $CRASHDIR

      # Add any more ath10k specific stuff here.

      # And call generic bug reporting logic
      bugcheck_generic
  fi
done

if [ $FOUND_BUG == "1" ]
    then
    # Notify LUCI somehow?
    echo "bugcheck.sh found an issue to be reported" > /dev/kmsg
    echo "See $CRASHDIR for details on how to report this" > /dev/kmsg
    # Let calling code know something was wrong.
    exit 1
fi

exit 0
