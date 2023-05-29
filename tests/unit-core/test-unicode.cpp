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
#include <gtest/gtest.h>
#include "jerryscript-port.h"
#include "jerryscript-port-default.h"

static bool
test_syntax_error (const char *script_p) /**< script */
{
  jerry_value_t parse_result = jerry_parse (NULL,
                                            0,
                                            (const jerry_char_t *) script_p,
                                            strlen (script_p),
                                            JERRY_PARSE_NO_OPTS);

  bool result = false;

  if (jerry_value_is_error (parse_result))
  {
    result = true;
    TEST_ASSERT (jerry_get_error_type (parse_result) == JERRY_ERROR_SYNTAX);
  }

  jerry_release_value (parse_result);
  return result;
} /* test_syntax_error */

class UnicodeTest : public testing::Test{
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "UnicodeTest SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "UnicodeTest TearDownTestCase";
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
HWTEST_F(UnicodeTest, Test001, testing::ext::TestSize.Level1)
{
  jerry_context_t *ctx_p = jerry_create_context (1024, context_alloc_fn, NULL);
  jerry_port_default_set_current_context (ctx_p);
  jerry_init (JERRY_INIT_EMPTY);

  if (!test_syntax_error ("\\u{61}"))
  {
    TEST_ASSERT (!test_syntax_error ("\xF0\x90\xB2\x80: break \\u{10C80}"));
    /* The \u surrogate pairs are ignored. The \u{hex} form must be used. */
    TEST_ASSERT (test_syntax_error ("\xF0\x90\xB2\x80: break \\ud803\\udc80"));
    /* The utf8 code point and the cesu8 surrogate pair must match. */
    TEST_ASSERT (!test_syntax_error ("\xF0\x90\xB2\x80: break \xed\xa0\x83\xed\xb2\x80"));

    TEST_ASSERT (!test_syntax_error ("$\xF0\x90\xB2\x80$: break $\\u{10C80}$"));
    TEST_ASSERT (test_syntax_error ("$\xF0\x90\xB2\x80$: break $\\ud803\\udc80$"));
    TEST_ASSERT (!test_syntax_error ("$\xF0\x90\xB2\x80$: break $\xed\xa0\x83\xed\xb2\x80$"));
  }

  jerry_cleanup ();
  free(ctx_p);
}
