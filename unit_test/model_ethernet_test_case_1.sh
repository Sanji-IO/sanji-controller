#!/bin/sh

source test_case_config

###################################################
# Test Items:
###################################################

# create
${pub_tool} -t "${topic_controller}" -f ${model_json_context}/ethernet/req_create_ethernet.json
sleep 1

# read capability
${pub_tool} -t "${topic_controller}" -f ${model_json_context}/ethernet/req_read_ethernet_capability.json
sleep 1

# read
${pub_tool} -t "${topic_controller}" -f ${model_json_context}/ethernet/req_read_ethernet.json
sleep 1

# update
${pub_tool} -t "${topic_controller}" -f ${model_json_context}/ethernet/req_update_ethernet.json
sleep 1

# delete
${pub_tool} -t "${topic_controller}" -f ${model_json_context}/ethernet/req_delete_ethernet.json
sleep 1
