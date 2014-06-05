#!/bin/sh

source test_case_config

###################################################
# Test Items:
#	1. Test controller mechanism that match topic to resources.
#	   In the case of no resource found.
###################################################

# register
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_wlan1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_dhcp.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_ipsec.json

# request read
${pub_tool} -t "${topic_controller}" -f ${json_context}/req_read_eth1.json
