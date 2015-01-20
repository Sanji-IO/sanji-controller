#!/bin/sh

CHANGES_FILE="CHANGES"

# clear release note
rm -f ${CHANGES_FILE}

# flush release note for git tag
tags=`git tag | tac`
for tag in ${tags}
do
	message_index=`git show ${tag} --pretty=short | grep "^commit " -m 1 -n | awk -F : '{print $1}'`
	((message_index--))
	git show ${tag} --pretty=short | head -n ${message_index} | tail -n +4 >> ${CHANGES_FILE}
done
