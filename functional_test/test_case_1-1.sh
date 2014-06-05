#!/bin/sh

source test_case_config

###################################################
# Test Items:
#	1. delete models which listen multi-resources
#	2. with ramdom register sequence
#	3. with random deregister seuqence
###################################################

# create all
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_eth1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_wlan1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_iptable.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_view_veth1.json

# delete all
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_delete_model_eth1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_delete_model_wlan1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_delete_model_iptable.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_delete_view_eth1.json

# create all
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_view_veth1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_wlan1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_iptable.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_eth1.json

# delete all
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_delete_model_iptable.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_delete_model_eth1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_delete_model_wlan1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_delete_view_eth1.json

