#!/bin/sh

files=`ls | grep -E 'testcase_[0-9]+.sh'`

for file in ${files};
do
	echo "---> ${file}"
	sed -i "s/TOPIC_REGISTER/TOPIC_CONTROLLER/g" ${file}
done
