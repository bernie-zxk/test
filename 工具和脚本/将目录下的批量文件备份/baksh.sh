#!/bin/bash


SBIN_PATH=$HOME/src/bin/

#SH_FILES='ls $SBIN_PATH *.sh'
SH_FILES='*.sh'

echo $SH_FILES
 
for shfile in $SH_FILES
do
	if [ -f "$shfile" ];
		then
			echo "cp " $shfile "  " $shfile".20120606.bak"
			cp $shfile  $shfile".20120606.bak"
	fi

done
