/*
 * Copyright (c) 2016 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <zephyr.h>
#include <tc_util.h>
#include <misc/slist.h>

static sys_slist_t test_list;

static sys_snode_t test_node_1;
static sys_snode_t test_node_2;
static sys_snode_t test_node_3;

static inline bool verify_emptyness(sys_slist_t *list)
{
	sys_snode_t *node;
	sys_snode_t *s_node;
	int count;

	if (!sys_slist_is_empty(list)) {
		return false;
	}

	if (sys_slist_peek_head(list)) {
		return false;
	}

	if (sys_slist_peek_tail(list)) {
		return false;
	}

	count = 0;
	SYS_SLIST_FOR_EACH_NODE(list, node) {
		count++;
	}

	if (count) {
		return false;
	}

	SYS_SLIST_FOR_EACH_NODE_SAFE(list, node, s_node) {
		count++;
	}

	if (count) {
		return false;
	}

	return true;
}

static inline bool verify_content_amount(sys_slist_t *list, int amount)
{
	sys_snode_t *node;
	sys_snode_t *s_node;
	int count;

	if (sys_slist_is_empty(list)) {
		return false;
	}

	if (!sys_slist_peek_head(list)) {
		return false;
	}

	if (!sys_slist_peek_tail(list)) {
		return false;
	}

	count = 0;
	SYS_SLIST_FOR_EACH_NODE(list, node) {
		count++;
	}

	if (count != amount) {
		return false;
	}

	count = 0;
	SYS_SLIST_FOR_EACH_NODE_SAFE(list, node, s_node) {
		count++;
	}

	if (count != amount) {
		return false;
	}

	return true;
}

static inline bool verify_tail_head(sys_slist_t *list,
				    sys_snode_t *head,
				    sys_snode_t *tail,
				    bool same)
{
	if (sys_slist_peek_head(list) != head) {
		return false;
	}

	if (sys_slist_peek_tail(list) != tail) {
		return false;
	}

	if (same) {
		if (sys_slist_peek_head(list) != sys_slist_peek_tail(list)) {
			return false;
		}
	} else {
		if (sys_slist_peek_head(list) == sys_slist_peek_tail(list)) {
			return false;
		}
	}

	return true;
}

void main(void)
{
	int status = TC_FAIL;

	TC_PRINT("Starting slist test\n");

	TC_PRINT(" - Initializing the list\n");
	sys_slist_init(&test_list);

	if (!verify_emptyness(&test_list)) {
		TC_ERROR("*** test_list should be empty\n");
		goto end;
	}

	TC_PRINT(" - Appending node 1\n");
	sys_slist_append(&test_list, &test_node_1);
	if (!verify_content_amount(&test_list, 1)) {
		TC_ERROR("*** test_list has wrong content\n");
		goto end;
	}

	if (!verify_tail_head(&test_list, &test_node_1, &test_node_1, true)) {
		TC_ERROR("*** test_list head/tail are wrong\n");
		goto end;
	}

	TC_PRINT(" - Finding and removing node 1\n");
	sys_slist_find_and_remove(&test_list, &test_node_1);
	if (!verify_emptyness(&test_list)) {
		TC_ERROR("*** test_list should be empty\n");
		goto end;
	}

	TC_PRINT(" - Prepending node 1\n");
	sys_slist_prepend(&test_list, &test_node_1);
	if (!verify_content_amount(&test_list, 1)) {
		TC_ERROR("*** test_list has wrong content\n");
		goto end;
	}

	if (!verify_tail_head(&test_list, &test_node_1, &test_node_1, true)) {
		TC_ERROR("*** test_list head/tail are wrong\n");
		goto end;
	}

	TC_PRINT(" - Removing node 1\n");
	sys_slist_remove(&test_list, NULL, &test_node_1);
	if (!verify_emptyness(&test_list)) {
		TC_ERROR("*** test_list should be empty\n");
		goto end;
	}

	TC_PRINT(" - Appending node 1\n");
	sys_slist_append(&test_list, &test_node_1);
	TC_PRINT(" - Prepending node 2\n");
	sys_slist_prepend(&test_list, &test_node_2);

	if (!verify_content_amount(&test_list, 2)) {
		TC_ERROR("*** test_list has wrong content\n");
		goto end;
	}

	if (!verify_tail_head(&test_list, &test_node_2, &test_node_1, false)) {
		TC_ERROR("*** test_list head/tail are wrong\n");
		goto end;
	}

	TC_PRINT(" - Appending node 3\n");
	sys_slist_append(&test_list, &test_node_3);

	if (!verify_content_amount(&test_list, 3)) {
		TC_ERROR("*** test_list has wrong content\n");
		goto end;
	}

	if (!verify_tail_head(&test_list, &test_node_2, &test_node_3, false)) {
		TC_ERROR("*** test_list head/tail are wrong\n");
		goto end;
	}

	if (sys_slist_peek_next(&test_node_2) != &test_node_1) {
		TC_ERROR("*** test_list node links are wrong\n");
		goto end;
	}

	TC_PRINT(" - Finding and removing node 1\n");
	sys_slist_find_and_remove(&test_list, &test_node_1);
	if (!verify_content_amount(&test_list, 2)) {
		TC_ERROR("*** test_list has wrong content\n");
		goto end;
	}

	if (!verify_tail_head(&test_list, &test_node_2, &test_node_3, false)) {
		TC_ERROR("*** test_list head/tail are wrong\n");
		goto end;
	}

	TC_PRINT(" - Removing node 3\n");
	sys_slist_remove(&test_list, &test_node_2, &test_node_3);
	if (!verify_content_amount(&test_list, 1)) {
		TC_ERROR("*** test_list has wrong content\n");
		goto end;
	}

	if (!verify_tail_head(&test_list, &test_node_2, &test_node_2, true)) {
		TC_ERROR("*** test_list head/tail are wrong\n");
		goto end;
	}

	TC_PRINT(" - Removing node 2\n");
	sys_slist_remove(&test_list, NULL, &test_node_2);
	if (!verify_emptyness(&test_list)) {
		TC_ERROR("*** test_list should be empty\n");
		goto end;
	}

	status = TC_PASS;

end:
	TC_END_RESULT(status);
	TC_END_REPORT(status);
}
