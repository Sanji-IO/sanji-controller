#!/bin/sh

source testcase_config
MQTT_SERVER_IP=127.0.0.1

###################################################
# Test Items:
###################################################

# create
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -h ${MQTT_SERVER_IP} -f ${JSON_CONTEXT}/ethernet/request_create_ethernet.json
sleep 3

# read capability
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -h ${MQTT_SERVER_IP} -f ${JSON_CONTEXT}/ethernet/request_readcap_ethernet.json
sleep 3

# read
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -h ${MQTT_SERVER_IP} -f ${JSON_CONTEXT}/ethernet/request_read_ethernet.json
sleep 3

# update
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -h ${MQTT_SERVER_IP} -f ${JSON_CONTEXT}/ethernet/request_update_ethernet.json
sleep 3

# delete
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -h ${MQTT_SERVER_IP} -f ${JSON_CONTEXT}/ethernet/request_delete_ethernet.json
sleep 3
