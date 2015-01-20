#!/bin/sh

dir=$1
dir=${dir:=.}

header_file=`ls ${dir}/*.h 2> /dev/null`
source_file=`ls ${dir}/*.c 2> /dev/null`
script_file=`ls ${dir}/*.sh 2> /dev/null`
json_file=`ls ${dir}/*.json 2> /dev/null`

total=0
for file in ${header_file} ${source_file} ${script_file} ${json_file};
do
	wc -l ${file}

	line=`wc -l ${file} | awk '{print $1}'`
	((total+=line))
done
echo "Total ${total}"
