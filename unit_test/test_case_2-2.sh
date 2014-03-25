#!/bin/sh

source test_case_config

###################################################
# Test Items:
#	1. Test controller mechanism that match topic to resources.
###################################################

# register
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_eth1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_wlan1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_dhcp.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_ipsec.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_view_veth1.json

# request read
${pub_tool} -t "${topic_controller}" -f ${json_context}/req_read_eth1.json
${pub_tool} -t "${topic_controller}" -f ${json_context}/req_read_eth1_long_topic1.json
${pub_tool} -t "${topic_controller}" -f ${json_context}/req_read_eth1_long_topic2.json
${pub_tool} -t "${topic_controller}" -f ${json_context}/req_read_eth1_long_topic3.json

# response read
${pub_tool} -t "${topic_controller}" -f ${json_context}/res_read_eth1_succ_eth1.json
