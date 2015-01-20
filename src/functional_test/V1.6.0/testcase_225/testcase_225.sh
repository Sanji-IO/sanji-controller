#!/bin/sh

source ../testcase_config

###################################################
# Test Items:
#	1. routing wirte-like request (model listen interacted resources)
#
# Importantances:
#   1. Model can NOT subscribe interacted resources
###################################################

# register
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -f ${JSON_CONTEXT}/register/request_create_model_ethernet.json
sleep 1
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -f ${JSON_CONTEXT}/register/request_create_view_v-ethernet.json
sleep 1

# read
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -f ${JSON_CONTEXT}/request_read_ethernet.json
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -f ${JSON_CONTEXT}/response_read_succ_ethernet.json
sleep 5

# update
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -f ${JSON_CONTEXT}/request_update_ethernet.json
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -f ${JSON_CONTEXT}/response_update_succ_ethernet.json

