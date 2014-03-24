#!/bin/sh

source test_case_config

###################################################
# Test Items:
#	1. Basic CRUD method of register resource
#	2. Some error exception
#		- double create
#		- delete empty
###################################################

# create
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_eth1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_wlan1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_dhcp.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_ipsec.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_view_veth1.json

# double create
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_eth1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_wlan1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_dhcp.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_ipsec.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_view_veth1.json

# read/update
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_read_model_eth1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_update_model_eth1.json

# delete one and readd
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_delete_model_eth1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_eth1.json

# double delete one and readd
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_delete_model_eth1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_delete_model_eth1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_eth1.json

# delete all
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_delete_model_eth1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_delete_model_wlan1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_delete_model_dhcp.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_delete_model_ipsec.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_delete_view_eth1.json

# double delete all
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_delete_model_eth1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_delete_model_wlan1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_delete_model_dhcp.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_delete_model_ipsec.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_delete_view_eth1.json

# readd all
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_eth1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_wlan1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_dhcp.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_ipsec.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_view_veth1.json

