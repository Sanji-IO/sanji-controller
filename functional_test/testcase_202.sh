#!/bin/sh

source testcase_config

###################################################
# Test Items:
#	1. Test controller mechanism that match topic to resources.
###################################################

# register
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_eth1.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_wlan1.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_dhcp.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_ipsec.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_view_veth1.json

# request read
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -f ${JSON_CONTEXT}/request_read_eth1.json
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -f ${JSON_CONTEXT}/request_read_eth1-longtopic1.json
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -f ${JSON_CONTEXT}/request_read_eth1-longtopic2.json
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -f ${JSON_CONTEXT}/request_read_eth1-longtopic3.json

# response read
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -f ${JSON_CONTEXT}/response_read_eth1-succ-eth1.json
