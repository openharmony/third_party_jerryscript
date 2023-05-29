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

static jerry_value_t
vm_exec_stop_callback (void *user_p)
{
  int *int_p = (int *) user_p;

  if (*int_p > 0)
  {
    (*int_p)--;

    return jerry_create_undefined ();
  }

  return jerry_create_string ((const jerry_char_t *) "Abort script");
} /* vm_exec_stop_callback */

class ExecStopTest : public testing::Test{
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "ExecStopTest SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "ExecStopTest TearDownTestCase";
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
HWTEST_F(ExecStopTest, Test001, testing::ext::TestSize.Level1)
{
  TEST_INIT ();

  /* Test stopping an infinite loop. */
  if (!jerry_is_feature_enabled (JERRY_FEATURE_VM_EXEC_STOP))
  {
    jerry_port_log (JERRY_LOG_LEVEL_ERROR, "Exec Stop is disabled!\n");
    // jerry_cleanup ();
  }
  else{
    jerry_context_t *ctx_p = jerry_create_context (1024, context_alloc_fn, NULL);
    jerry_port_default_set_current_context (ctx_p);
    jerry_init (JERRY_INIT_EMPTY);

    int countdown = 6;
    jerry_set_vm_exec_stop_callback (vm_exec_stop_callback, &countdown, 16);

    const jerry_char_t inf_loop_code_src1[] = "while(true) {}";
    jerry_value_t parsed_code_val = jerry_parse (NULL,
                                                0,
                                                inf_loop_code_src1,
                                                sizeof (inf_loop_code_src1) - 1,
                                                JERRY_PARSE_NO_OPTS);

    TEST_ASSERT (!jerry_value_is_error (parsed_code_val));
    jerry_value_t res = jerry_run (parsed_code_val);
    TEST_ASSERT (countdown == 0);

    TEST_ASSERT (jerry_value_is_error (res));

    jerry_release_value (res);
    jerry_release_value (parsed_code_val);

    /* A more complex example. Although the callback error is captured
    * by the catch block, it is automatically thrown again. */

    /* We keep the callback function, only the countdown is reset. */
    countdown = 6;

    const jerry_char_t inf_loop_code_src2[] = TEST_STRING_LITERAL (
      "function f() { while (true) ; }\n"
      "try { f(); } catch(e) {}"
    );

    parsed_code_val = jerry_parse (NULL,
                                  0,
                                  inf_loop_code_src2,
                                  sizeof (inf_loop_code_src2) - 1,
                                  JERRY_PARSE_NO_OPTS);

    TEST_ASSERT (!jerry_value_is_error (parsed_code_val));
    res = jerry_run (parsed_code_val);
    TEST_ASSERT (countdown == 0);

    /* The result must have an error flag which proves that
    * the error is thrown again inside the catch block. */
    TEST_ASSERT (jerry_value_is_error (res));

    jerry_release_value (res);
    jerry_release_value (parsed_code_val);

    jerry_cleanup ();
    free (ctx_p);
  } 
}
