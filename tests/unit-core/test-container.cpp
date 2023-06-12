/* Copyright JS Foundation and other contributors, http://js.foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "jerryscript.h"
#include "test-common.h"
#include "jerryscript-port.h"
#include "jerryscript-port-default.h"
#include <gtest/gtest.h>

class ContainerTest : public testing::Test{
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "ContainerTest SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "ContainerTest TearDownTestCase";
    }

    void SetUp() override {}
    void TearDown() override {}

};

static constexpr size_t JERRY_SCRIPT_MEM_SIZE = 50 * 1024 * 1024;
static void* context_alloc_fn(size_t size, void* cb_data)
{
    (void)cb_data;
    size_t newSize = size > JERRY_SCRIPT_MEM_SIZE ? JERRY_SCRIPT_MEM_SIZE : size;
    return malloc(newSize);
}

HWTEST_F(ContainerTest, Test001, testing::ext::TestSize.Level1)
{
  jerry_context_t *ctx_p = jerry_create_context (1024, context_alloc_fn, NULL);
  jerry_port_default_set_current_context (ctx_p);
  jerry_init (JERRY_INIT_EMPTY);

  if (!jerry_is_feature_enabled (JERRY_FEATURE_MAP)
      || !jerry_is_feature_enabled (JERRY_FEATURE_SET)
      || !jerry_is_feature_enabled (JERRY_FEATURE_WEAKMAP)
      || !jerry_is_feature_enabled (JERRY_FEATURE_WEAKSET))
  {
    jerry_port_log (JERRY_LOG_LEVEL_ERROR, "Containers are disabled!\n");
    jerry_cleanup ();
    return;
  }

  jerry_value_t instance_check;
  jerry_value_t global = jerry_get_global_object ();
  jerry_value_t map_str = jerry_create_string ((jerry_char_t *) "Map");
  jerry_value_t set_str = jerry_create_string ((jerry_char_t *) "Set");
  jerry_value_t weakmap_str = jerry_create_string ((jerry_char_t *) "WeakMap");
  jerry_value_t weakset_str = jerry_create_string ((jerry_char_t *) "WeakSet");
  jerry_value_t global_map = jerry_get_property (global, map_str);
  jerry_value_t global_set = jerry_get_property (global, set_str);
  jerry_value_t global_weakmap = jerry_get_property (global, weakmap_str);
  jerry_value_t global_weakset = jerry_get_property (global, weakset_str);

  jerry_release_value (global);

  jerry_release_value (map_str);
  jerry_release_value (set_str);
  jerry_release_value (weakmap_str);
  jerry_release_value (weakset_str);

  jerry_value_t empty_map = jerry_create_container (JERRY_CONTAINER_TYPE_MAP, NULL, 0);
  TEST_ASSERT (jerry_get_container_type (empty_map) == JERRY_CONTAINER_TYPE_MAP);
  instance_check = jerry_binary_operation (JERRY_BIN_OP_INSTANCEOF, empty_map, global_map);
  TEST_ASSERT (jerry_get_boolean_value (instance_check));
  jerry_release_value (instance_check);
  jerry_release_value (global_map);
  jerry_release_value (empty_map);

  jerry_value_t empty_set = jerry_create_container (JERRY_CONTAINER_TYPE_SET, NULL, 0);
  TEST_ASSERT (jerry_get_container_type (empty_set) == JERRY_CONTAINER_TYPE_SET);
  instance_check = jerry_binary_operation (JERRY_BIN_OP_INSTANCEOF, empty_set, global_set);
  TEST_ASSERT (jerry_get_boolean_value (instance_check));
  jerry_release_value (instance_check);
  jerry_release_value (global_set);
  jerry_release_value (empty_set);

  jerry_value_t empty_weakmap = jerry_create_container (JERRY_CONTAINER_TYPE_WEAKMAP, NULL, 0);
  TEST_ASSERT (jerry_get_container_type (empty_weakmap) == JERRY_CONTAINER_TYPE_WEAKMAP);
  instance_check = jerry_binary_operation (JERRY_BIN_OP_INSTANCEOF, empty_weakmap, global_weakmap);
  TEST_ASSERT (jerry_get_boolean_value (instance_check));
  jerry_release_value (instance_check);
  jerry_release_value (global_weakmap);
  jerry_release_value (empty_weakmap);

  jerry_value_t empty_weakset = jerry_create_container (JERRY_CONTAINER_TYPE_WEAKSET, NULL, 0);
  TEST_ASSERT (jerry_get_container_type (empty_weakset) == JERRY_CONTAINER_TYPE_WEAKSET);
  instance_check = jerry_binary_operation (JERRY_BIN_OP_INSTANCEOF, empty_weakset, global_weakset);
  TEST_ASSERT (jerry_get_boolean_value (instance_check));
  jerry_release_value (instance_check);
  jerry_release_value (global_weakset);
  jerry_release_value (empty_weakset);

  jerry_cleanup ();
  free (ctx_p);
}
