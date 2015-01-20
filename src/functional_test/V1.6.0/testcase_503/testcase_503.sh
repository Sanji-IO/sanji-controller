#!/bin/sh

source ../testcase_config

###################################################
# Test Items:
# 	1. Test 'method' with upper case and lowwer case.
# 	   'method' should be case-insensitive.
###################################################

# create
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -f ${JSON_CONTEXT}/register/request_create_model_ethernet.json
sleep 1

# delete
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -f ${JSON_CONTEXT}/register/request_delete_model_ethernet.json

