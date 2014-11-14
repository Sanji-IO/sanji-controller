#!/bin/sh

source testcase_config
MQTT_SERVER_IP=127.0.0.1

###################################################
# Test Items:
###################################################

# create
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -h ${MQTT_SERVER_IP} -f ${JSON_CONTEXT}/model/request_create_model.json
sleep 3

# read capability
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -h ${MQTT_SERVER_IP} -f ${JSON_CONTEXT}/model/request_readcap_model.json
sleep 3

# read
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -h ${MQTT_SERVER_IP} -f ${JSON_CONTEXT}/model/request_read_model.json
sleep 3

# update
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -h ${MQTT_SERVER_IP} -f ${JSON_CONTEXT}/model/request_update_model.json
sleep 3

# delete
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -h ${MQTT_SERVER_IP} -f ${JSON_CONTEXT}/model/request_delete_model.json
sleep 3

# other method
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -h ${MQTT_SERVER_IP} -f ${JSON_CONTEXT}/model/request_other_model.json
sleep 3

# response procedure
${PUB_TOOL} -t "${TOPIC_CONTROLLER}" -h ${MQTT_SERVER_IP} -f ${JSON_CONTEXT}/model/response_read_model.json
sleep 3
