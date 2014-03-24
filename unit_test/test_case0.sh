#!/bin/sh

source test_case_config

###################################################
# Test Items:
# 	1. Test 'method' with upper case and lowwer case.
# 	   'method' should be case-insensitive.
###################################################

# create
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_eth1.json

# delete
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_delete_model_eth1.json

