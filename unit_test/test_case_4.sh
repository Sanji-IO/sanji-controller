#!/bin/sh

source test_case_config

###################################################
# Test Items:
#	1.	Test routing procedure of write-like request
#		with some modules failed.
###################################################


# register
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_eth1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_wlan1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_dhcp.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_ipsec.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_view_veth1.json

# write
${pub_tool} -t "${topic_controller}" -f ${json_context}/req_write_eth1.json
sleep 1
${pub_tool} -t "${topic_controller}" -f ${json_context}/res_write_eth1_succ_eth1.json
sleep 1
${pub_tool} -t "${topic_controller}" -f ${json_context}/res_write_eth1_fail_dhcp.json
${pub_tool} -t "${topic_controller}" -f ${json_context}/res_write_eth1_succ_ipsec.json

