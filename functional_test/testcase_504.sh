#!/bin/sh

source testcase_config

###################################################
# Test Items:
#	1. delete models which listen multi-resources
#	2. with ramdom register sequence
#	3. with random deregister seuqence
###################################################

# create all
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_eth1.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_wlan1.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_iptable.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_view_veth1.json

# delete all
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_delete_model_eth1.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_delete_model_wlan1.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_delete_model_iptable.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_delete_view_eth1.json

# create all
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_view_veth1.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_wlan1.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_iptable.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_eth1.json

# delete all
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_delete_model_iptable.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_delete_model_eth1.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_delete_model_wlan1.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_delete_view_eth1.json

