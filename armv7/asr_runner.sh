#!/bin/sh
for file in ./res/*; do
	if test -f $file ; then
		echo $file
		./asr offline $file
	fi
	sleep 1
done