#!/bin/sh

source testcase_config

###################################################
# Test Items:
#	1. Test controller mechanism that match topic to resources.
#	   In the case of resource found (but only registered by 'view').
###################################################

# register
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_wlan1.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_dhcp.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_ipsec.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_view_veth1.json

# request read
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -f ${JSON_CONTEXT}/request_read_eth1.json
