#!/bin/sh

source test_case_config

###################################################
# Test Items:
#	1. Test routing procedure of read-like request.
###################################################

# register
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_eth1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_wlan1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_dhcp.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_ipsec.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_view_veth1.json

# read
${pub_tool} -t "${topic_controller}" -f ${json_context}/req_read_eth1.json
${pub_tool} -t "${topic_controller}" -f ${json_context}/res_read_eth1_succ_eth1.json

