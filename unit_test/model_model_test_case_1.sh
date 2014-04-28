#!/bin/sh

source test_case_config

###################################################
# Test Items:
###################################################

# create
${pub_tool} -t "${topic_controller}" -f ${model_json_context}/model/req_create_model.json
sleep 1

# read capability
${pub_tool} -t "${topic_controller}" -f ${model_json_context}/model/req_read_model_capability.json
sleep 1

# read
${pub_tool} -t "${topic_controller}" -f ${model_json_context}/model/req_read_model.json
sleep 1

# update
${pub_tool} -t "${topic_controller}" -f ${model_json_context}/model/req_update_model.json
sleep 1

# delete
${pub_tool} -t "${topic_controller}" -f ${model_json_context}/model/req_delete_model.json
sleep 1

# other method
${pub_tool} -t "${topic_controller}" -f ${model_json_context}/model/req_other_model.json
sleep 1

# response procedure
${pub_tool} -t "${topic_controller}" -f ${model_json_context}/model/res_read_model.json
