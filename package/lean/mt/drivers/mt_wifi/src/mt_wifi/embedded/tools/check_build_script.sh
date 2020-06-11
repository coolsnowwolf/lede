#!/bin/bash

function showUsage(){
        echo "##########################################################"
        echo "./check_build_script [ argument1 argument2 ]"
        echo "argument 1:"
	echo "		 shall be chip model, such as mt7615, mt7603e...."
	echo ""
        echo "argument 2:"
	echo "		 could be empty, it will auto build AP & STA mode."
        echo "otherwise, you could assign the mode you would like to build."
        echo "##########################################################"
}

function build(){
	for modes in AP STA
	do
		if [ "$1" != "" ]; then
			if [ $1 = ${modes} ]; then
				echo "###### BUILD assign chip: ${chips}.... "
		                echo "###### BUILD assign mode: ${modes} "
				sleep 2
				make WIFI_MODE=${modes} CHIPSET=${chips} clean
        	                make WIFI_MODE=${modes} CHIPSET=${chips}  || { echo "######### WIFI_MODE=${modes}, CHIPSET=${chips} fail!!"; exit 1; }
	                        make WIFI_MODE=${modes} CHIPSET=${chips} clean
			else
				continue;
			fi
		else
			echo "###### BUILD chip: ${chips}.... "
			echo "###### Auto Build AP/STA mode to check.... "
	                echo "###### BUILD mode now: ${modes} "
	                sleep 2
	                make WIFI_MODE=${modes} CHIPSET=${chips} clean
	                make WIFI_MODE=${modes} CHIPSET=${chips}  || { echo "######### WIFI_MODE=${modes}, CHIPSET=${chips} fail!!"; exit 1; }
	                make WIFI_MODE=${modes} CHIPSET=${chips} clean
	                echo ""
		fi
        done
}

for chips in mt7636s mt7636u mt7603e mt7603u mt7615 mt7637e mt7637u
do
	if [ "$1" != "" ]; then
		if [ $1 = ${chips} ]; then
			build $2;
		else
			continue;
		fi
	else
		build;
	fi
done
