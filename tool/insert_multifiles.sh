#!/bin/sh

files=`ls | grep "request*"`

for file in ${files};
do
	echo "---> ${file}"
	sed -i 's/.*data.*/\t\"tunnel\" : \"1\",\n&/' ${file}
done
