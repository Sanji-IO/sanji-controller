#!/bin/sh

source ../testcase_config

###################################################
# Test Items:
#	1. create and delete with hook mechanism
###################################################

# create
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -f ${JSON_CONTEXT}/register/request_create_model_ethernet.json
sleep 1
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -f ${JSON_CONTEXT}/register/request_create_model_cellular.json
sleep 1
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -f ${JSON_CONTEXT}/register/request_create_model_dhcp.json
sleep 1
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -f ${JSON_CONTEXT}/register/request_create_model_ipsec.json
sleep 1
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -f ${JSON_CONTEXT}/register/request_create_view_v-ethernet.json
sleep 1

# delete
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -f ${JSON_CONTEXT}/register/request_delete_model_ethernet.json
sleep 1
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -f ${JSON_CONTEXT}/register/request_delete_model_cellular.json
sleep 1
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -f ${JSON_CONTEXT}/register/request_delete_model_dhcp.json
sleep 1
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -f ${JSON_CONTEXT}/register/request_delete_model_ipsec.json
sleep 1
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -f ${JSON_CONTEXT}/register/request_delete_view_v-ethernet.json
sleep 1

