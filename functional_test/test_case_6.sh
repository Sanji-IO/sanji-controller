#!/bin/sh

source test_case_config

###################################################
# Test Items:
#	1.	Test lock mechanism
###################################################

# register
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_eth1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_wlan1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_dhcp.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_ipsec.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_view_veth1.json

# write -> write
${pub_tool} -t "${topic_controller}" -f ${json_context}/req_write_eth1_1.json
${pub_tool} -t "${topic_controller}" -f ${json_context}/req_write_eth1_2.json
sleep 8

# read -> read
${pub_tool} -t "${topic_controller}" -f ${json_context}/req_read_eth1_1.json
${pub_tool} -t "${topic_controller}" -f ${json_context}/req_read_eth1_2.json
sleep 8

# write -> read
${pub_tool} -t "${topic_controller}" -f ${json_context}/req_write_eth1.json
${pub_tool} -t "${topic_controller}" -f ${json_context}/req_read_eth1.json
sleep 8

# read -> write
# Controller didn't lock this situation.
${pub_tool} -t "${topic_controller}" -f ${json_context}/req_read_eth1.json
${pub_tool} -t "${topic_controller}" -f ${json_context}/req_write_eth1.json


