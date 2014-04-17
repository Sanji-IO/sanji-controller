#!/bin/sh

source test_case_config

###################################################
# Test Items:
# 	1. Test registeration with incomplete 'data' (key with empty value) (1-8)
# 	2. Test registeration with incomplete 'data' (no key) (9-15)
###################################################

# create
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_incomplete1.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_incomplete2.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_incomplete3.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_incomplete4.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_incomplete5.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_incomplete6.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_incomplete7.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_incomplete8.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_incomplete9.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_incomplete10.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_incomplete11.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_incomplete12.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_incomplete13.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_incomplete14.json
${pub_tool} -t "${topic_register}" -f ${json_context}/register_req_create_model_incomplete15.json

