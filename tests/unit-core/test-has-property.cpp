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

#include "config.h"
#include "jerryscript.h"
#include "jerryscript-port.h"
#include "jerryscript-port-default.h"
#include "test-common.h"
#include <gtest/gtest.h>

static void
assert_boolean_and_release (jerry_value_t result, bool expected)
{
  TEST_ASSERT (jerry_value_is_boolean (result));
  TEST_ASSERT (jerry_get_boolean_value (result) == expected);
  jerry_release_value (result);
} /* assert_boolean_and_release */

class HasPropertyTest : public testing::Test{
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "HasPropertyTest SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "HasPropertyTest TearDownTestCase";
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
HWTEST_F(HasPropertyTest, Test001, testing::ext::TestSize.Level1)
{
  jerry_context_t *ctx_p = jerry_create_context (1024, context_alloc_fn, NULL);
  jerry_port_default_set_current_context (ctx_p);
  TEST_INIT ();

  jerry_init (JERRY_INIT_EMPTY);

  jerry_value_t object = jerry_create_object ();
  jerry_value_t prop_name = jerry_create_string_from_utf8 ((jerry_char_t *) "something");
  jerry_value_t prop_value = jerry_create_boolean (true);
  jerry_value_t proto_object = jerry_create_object ();

  /* Assert that an empty object does not have the property in question */
  assert_boolean_and_release (jerry_has_property (object, prop_name), false);
  assert_boolean_and_release (jerry_has_own_property (object, prop_name), false);

  assert_boolean_and_release (jerry_set_prototype (object, proto_object), true);

  /* If the object has a prototype, that still means it doesn't have the property */
  assert_boolean_and_release (jerry_has_property (object, prop_name), false);
  assert_boolean_and_release (jerry_has_own_property (object, prop_name), false);

  assert_boolean_and_release (jerry_set_property (proto_object, prop_name, prop_value), true);

  /* After setting the property on the prototype, it must be there, but not on the object */
  assert_boolean_and_release (jerry_has_property (object, prop_name), true);
  assert_boolean_and_release (jerry_has_own_property (object, prop_name), false);

  TEST_ASSERT (jerry_delete_property (proto_object, prop_name));
  assert_boolean_and_release (jerry_set_property (object, prop_name, prop_value), true);

  /* After relocating the property onto the object, it must be there */
  assert_boolean_and_release (jerry_has_property (object, prop_name), true);
  assert_boolean_and_release (jerry_has_own_property (object, prop_name), true);

  jerry_release_value (object);
  jerry_release_value (prop_name);
  jerry_release_value (prop_value);
  jerry_release_value (proto_object);

  jerry_cleanup ();

  free (ctx_p);
}
