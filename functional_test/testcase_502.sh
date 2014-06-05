#!/bin/sh

source testcase_config

###################################################
# Test Items:
# 	1. Test registeration with incomplete 'data' (key with empty value) (1-8)
# 	2. Test registeration with incomplete 'data' (no key) (9-15)
###################################################

# create
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_incomplete1.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_incomplete2.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_incomplete3.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_incomplete4.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_incomplete5.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_incomplete6.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_incomplete7.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_incomplete8.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_incomplete9.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_incomplete10.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_incomplete11.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_incomplete12.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_incomplete13.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_incomplete14.json
${PUB_TOOL} -t "${TOPIC_REGISTER}" -f ${JSON_CONTEXT}/register/request_create_model_incomplete15.json

