#!/bin/sh
# for file in ./*
# do
#     if [[ test -f $file ]]; then
#         # arr=(${arr[*]} $file)
#         echo "pass"
#     fi
# done
# echo ${arr[@]}
for file in ./res/*; do
	if test -f $file ; then
		# echo "$file " >> result
		./asr offline $file
	fi
	sleep 1
done