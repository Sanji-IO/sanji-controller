#!/bin/sh

header_file=`ls *.h`
source_file=`ls *.c`

for file in ${header_file} ${source_file};
do
	wc -l ${file}
done
