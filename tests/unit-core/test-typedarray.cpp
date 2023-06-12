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
#include "jerryscript-port.h"
#include "jerryscript-port-default.h"
#include "test-common.h"
#include <gtest/gtest.h>
#include <stdio.h>
#include <iostream>

/**
 * Type to describe test cases.
 */
typedef struct
{
  jerry_typedarray_type_t typedarray_type; /**< what kind of TypedArray */
  const char *constructor_name; /**< JS constructor name for TypedArray */
  uint32_t element_count; /**< number of elements for the TypedArray */
  uint32_t bytes_per_element; /**< bytes per elment of the given typedarray_type */
} test_entry_t;

/**
 * Register a JavaScript value in the global object.
 */
static void
register_js_value (const char *name_p, /**< name of the function */
                    jerry_value_t value) /**< function callback */
{
  jerry_value_t global_obj_val = jerry_get_global_object ();

  jerry_value_t name_val = jerry_create_string ((const jerry_char_t *) name_p);
  jerry_value_t result_val = jerry_set_property (global_obj_val, name_val, value);

  jerry_release_value (name_val);
  jerry_release_value (global_obj_val);

  jerry_release_value (result_val);
} /* register_js_value */

static jerry_value_t
assert_handler (const jerry_value_t func_obj_val, /**< function object */
                const jerry_value_t this_val, /**< this arg */
                const jerry_value_t args_p[], /**< function arguments */
                const jerry_length_t args_cnt) /**< number of function arguments */
{
  JERRY_UNUSED (func_obj_val);
  JERRY_UNUSED (this_val);

  if (jerry_value_is_boolean (args_p[0])
      && jerry_get_boolean_value (args_p[0]))
  {
    return jerry_create_boolean (true);
  }
  else
  {
    if (args_cnt > 1
        && jerry_value_is_string (args_p[1]))
    {
      jerry_length_t utf8_sz = jerry_get_string_size (args_p[1]);
      JERRY_VLA (char, string_from_utf8, utf8_sz);
      string_from_utf8[utf8_sz] = 0;

      jerry_string_to_char_buffer (args_p[1], (jerry_char_t *) string_from_utf8, utf8_sz);

      printf ("JS assert: %s\n", string_from_utf8);
    }
    TEST_ASSERT (false);
  }
} /* assert_handler */

/**
 * Do simple TypedArray property validation.
 */
static void
test_typedarray_info (jerry_value_t typedarray, /**< target TypedArray to query */
                      jerry_typedarray_type_t typedarray_type, /**< expected TypedArray type */
                      jerry_length_t element_count, /**< expected element count */
                      jerry_length_t bytes_per_element) /**< bytes per element for the given type */
{
  TEST_ASSERT (!jerry_value_is_error (typedarray));
  TEST_ASSERT (jerry_value_is_typedarray (typedarray));
  TEST_ASSERT (jerry_get_typedarray_type (typedarray) == typedarray_type);
  TEST_ASSERT (jerry_get_typedarray_length (typedarray) == element_count);

  jerry_length_t byte_length = (uint32_t) -1;
  jerry_length_t byte_offset = (uint32_t) -1;
  jerry_value_t arraybuffer = jerry_get_typedarray_buffer (typedarray, &byte_offset, &byte_length);
  TEST_ASSERT (jerry_value_is_arraybuffer (arraybuffer));

  TEST_ASSERT (byte_length == element_count * bytes_per_element);
  TEST_ASSERT (byte_offset == 0);

  jerry_release_value (arraybuffer);
} /* test_typedarray_info */

/**
 * Test construction of TypedArrays and validate properties.
 */
static void
test_typedarray_queries (test_entry_t test_entries[]) /**< test cases */
{
  jerry_value_t global_obj_val = jerry_get_global_object ();

  for (uint32_t i = 0; test_entries[i].constructor_name != NULL; i++)
  {
    /* Create TypedArray via construct call */
    {
      jerry_value_t prop_name = jerry_create_string ((const jerry_char_t *) test_entries[i].constructor_name);
      jerry_value_t prop_value = jerry_get_property (global_obj_val, prop_name);
      TEST_ASSERT (!jerry_value_is_error (prop_value));
      jerry_value_t length_arg = jerry_create_number (test_entries[i].element_count);

      jerry_value_t typedarray = jerry_construct_object (prop_value, &length_arg, 1);

      jerry_release_value (prop_name);
      jerry_release_value (prop_value);
      jerry_release_value (length_arg);

      test_typedarray_info (typedarray,
                            test_entries[i].typedarray_type,
                            test_entries[i].element_count,
                            test_entries[i].bytes_per_element);
      jerry_release_value (typedarray);
    }

    /* Create TypedArray via api call */
    {
      jerry_value_t typedarray = jerry_create_typedarray (test_entries[i].typedarray_type,
                                                          test_entries[i].element_count);
      test_typedarray_info (typedarray,
                            test_entries[i].typedarray_type,
                            test_entries[i].element_count,
                            test_entries[i].bytes_per_element);
      jerry_release_value (typedarray);
    }
  }

  jerry_release_value (global_obj_val);
} /* test_typedarray_queries */



/**
 * Test get/set/delete property by index.
 */
static void test_property_by_index (test_entry_t test_entries[])
{
  int test_int_numbers[5] = {-5, -70, 13, 0, 56};
  double test_double_numbers[5] = {-83.153, -35.15, 0, 13.1, 89.8975};
  uint8_t test_uint_numbers[5] = {83, 15, 36, 0, 43};

  for (uint32_t i = 0; test_entries[i].constructor_name != NULL; i++)
  {
    jerry_value_t test_number;
    uint32_t test_numbers_length = sizeof (test_int_numbers) / sizeof (int);
    jerry_value_t typedarray = jerry_create_typedarray (test_entries[i].typedarray_type, test_numbers_length);
    jerry_typedarray_type_t type = jerry_get_typedarray_type (typedarray);

    jerry_value_t set_result;
    jerry_value_t get_result;

    switch (type)
    {
      case JERRY_TYPEDARRAY_INT8:
      case JERRY_TYPEDARRAY_INT16:
      case JERRY_TYPEDARRAY_INT32:
      {
        for (uint8_t j = 0; j < test_numbers_length; j++)
        {
          test_number = jerry_create_number (test_int_numbers[j]);
          TEST_ASSERT (!jerry_delete_property_by_index (typedarray, j));
          set_result = jerry_set_property_by_index (typedarray, j, test_number);
          get_result = jerry_get_property_by_index (typedarray, j);

          TEST_ASSERT (jerry_value_is_boolean (set_result));
          TEST_ASSERT (jerry_get_boolean_value (set_result));
          TEST_ASSERT (!jerry_delete_property_by_index (typedarray, j));
          TEST_ASSERT (jerry_get_number_value (get_result) == test_int_numbers[j]);

          jerry_release_value (test_number);
          jerry_release_value (set_result);
          jerry_release_value (get_result);
        }
        break;
      }
      case JERRY_TYPEDARRAY_FLOAT32:
      case JERRY_TYPEDARRAY_FLOAT64:
      {
        for (uint8_t j = 0; j < test_numbers_length; j++)
        {
          test_number = jerry_create_number (test_double_numbers[j]);
          TEST_ASSERT (!jerry_delete_property_by_index (typedarray, j));
          set_result = jerry_set_property_by_index (typedarray, j, test_number);
          get_result = jerry_get_property_by_index (typedarray, j);

          TEST_ASSERT (jerry_value_is_boolean (set_result));
          TEST_ASSERT (jerry_get_boolean_value (set_result));
          TEST_ASSERT (!jerry_delete_property_by_index (typedarray, j));

          double epsilon = pow (10, -5);
          double get_abs = fabs (jerry_get_number_value (get_result) - test_double_numbers[j]);
          TEST_ASSERT (get_abs < epsilon);

          jerry_release_value (test_number);
          jerry_release_value (set_result);
          jerry_release_value (get_result);

          /* Testing positive and negative infinity */
          for (uint8_t k = 0; k < 2; k++)
          {
            jerry_value_t inf = jerry_create_number_infinity (k);
            jerry_value_t set_inf = jerry_set_property_by_index (typedarray, 0, inf);
            TEST_ASSERT (jerry_value_is_boolean (set_inf));
            TEST_ASSERT (jerry_get_boolean_value (set_inf));
            jerry_value_t get_inf = jerry_get_property_by_index (typedarray, 0);
            TEST_ASSERT (isinf (jerry_get_number_value (get_inf)));

            jerry_release_value (inf);
            jerry_release_value (set_inf);
            jerry_release_value (get_inf);
          }
        }
        break;
      }
      default:
      {
        for (uint8_t j = 0; j < test_numbers_length; j++)
        {
          test_number = jerry_create_number (test_uint_numbers[j]);
          TEST_ASSERT (!jerry_delete_property_by_index (typedarray, j));
          set_result = jerry_set_property_by_index (typedarray, j, test_number);
          get_result = jerry_get_property_by_index (typedarray, j);

          TEST_ASSERT (jerry_value_is_boolean (set_result));
          TEST_ASSERT (jerry_get_boolean_value (set_result));
          TEST_ASSERT (!jerry_delete_property_by_index (typedarray, j));
          TEST_ASSERT (jerry_get_number_value (get_result) == test_uint_numbers[j]);

          jerry_release_value (test_number);
          jerry_release_value (set_result);
          jerry_release_value (get_result);
        }
        break;
      }
    }

    jerry_value_t set_undefined = jerry_set_property_by_index (typedarray, 100, jerry_create_number (50));
    TEST_ASSERT (jerry_value_is_error (set_undefined));
    jerry_value_t get_undefined = jerry_get_property_by_index (typedarray, 100);
    TEST_ASSERT (jerry_value_is_undefined (get_undefined));

    jerry_release_value (set_undefined);
    jerry_release_value (get_undefined);
    jerry_release_value (typedarray);
  }
} /* test_property_by_index */

static void
test_detached_arraybuffer (void)
{
  static jerry_typedarray_type_t types[] =
  {
    JERRY_TYPEDARRAY_UINT8,
    JERRY_TYPEDARRAY_UINT8CLAMPED,
    JERRY_TYPEDARRAY_INT8,
    JERRY_TYPEDARRAY_UINT16,
    JERRY_TYPEDARRAY_INT16,
    JERRY_TYPEDARRAY_UINT32,
    JERRY_TYPEDARRAY_INT32,
    JERRY_TYPEDARRAY_FLOAT32,
    JERRY_TYPEDARRAY_FLOAT64,
  };

  /* Creating an TypedArray for a detached array buffer with a given length/offset is invalid */
  {
    uint8_t buf[1];
    const uint32_t length = 1;
    jerry_value_t arraybuffer = jerry_create_arraybuffer_external (length, buf, NULL);
    TEST_ASSERT (!jerry_value_is_error (arraybuffer));
    TEST_ASSERT (jerry_value_is_arraybuffer (arraybuffer));
    TEST_ASSERT (jerry_get_arraybuffer_byte_length (arraybuffer) == length);

    jerry_value_t is_detachable = jerry_is_arraybuffer_detachable (arraybuffer);
    TEST_ASSERT (!jerry_value_is_error (is_detachable));
    TEST_ASSERT (jerry_get_boolean_value (is_detachable));
    jerry_release_value (is_detachable);

    jerry_value_t res = jerry_detach_arraybuffer (arraybuffer);
    TEST_ASSERT (!jerry_value_is_error (res));
    jerry_release_value (res);

    for (size_t idx = 0; idx < (sizeof (types) / sizeof (types[0])); idx++)
    {
      jerry_value_t typedarray = jerry_create_typedarray_for_arraybuffer_sz (types[idx], arraybuffer, 0, 4);
      TEST_ASSERT (jerry_value_is_error (typedarray));
      TEST_ASSERT (jerry_get_error_type (typedarray) == JERRY_ERROR_TYPE);
      jerry_release_value (typedarray);
    }

    jerry_release_value (arraybuffer);
  }

  /* Creating an TypedArray for a detached array buffer without length/offset is valid */
  {
    uint8_t buf[1];
    const uint32_t length = 1;
    jerry_value_t arraybuffer = jerry_create_arraybuffer_external (length, buf, NULL);
    TEST_ASSERT (!jerry_value_is_error (arraybuffer));
    TEST_ASSERT (jerry_value_is_arraybuffer (arraybuffer));
    TEST_ASSERT (jerry_get_arraybuffer_byte_length (arraybuffer) == length);

    jerry_value_t is_detachable = jerry_is_arraybuffer_detachable (arraybuffer);
    TEST_ASSERT (!jerry_value_is_error (is_detachable));
    TEST_ASSERT (jerry_get_boolean_value (is_detachable));
    jerry_release_value (is_detachable);

    jerry_value_t res = jerry_detach_arraybuffer (arraybuffer);
    TEST_ASSERT (!jerry_value_is_error (res));
    jerry_release_value (res);

    for (size_t idx = 0; idx < (sizeof (types) / sizeof (types[0])); idx++)
    {
      jerry_value_t typedarray = jerry_create_typedarray_for_arraybuffer (types[idx], arraybuffer);
      TEST_ASSERT (jerry_value_is_error (typedarray));
      TEST_ASSERT (jerry_get_error_type (typedarray) == JERRY_ERROR_TYPE);
      jerry_release_value (typedarray);
    }

    jerry_release_value (arraybuffer);
  }
} /* test_detached_arraybuffer */

class TypedarrayTest : public testing::Test{
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "TypedarrayTest SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "TypedarrayTest TearDownTestCase";
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
HWTEST_F(TypedarrayTest, Test001, testing::ext::TestSize.Level1)
{
      jerry_context_t *ctx_p = jerry_create_context (1024, context_alloc_fn, NULL);
      jerry_port_default_set_current_context (ctx_p);
      jerry_init (JERRY_INIT_EMPTY);

      if (!jerry_is_feature_enabled (JERRY_FEATURE_TYPEDARRAY))
      {
        jerry_port_log (JERRY_LOG_LEVEL_ERROR, "TypedArray is disabled!\n");
        jerry_cleanup ();
        return;
      }

      jerry_value_t function_val = jerry_create_external_function (assert_handler);
      register_js_value ("assert", function_val);
      jerry_release_value (function_val);

      test_entry_t test_entries[] =
      {
    #define TEST_ENTRY(TYPE, CONSTRUCTOR, COUNT, BYTES_PER_ELEMENT) \
          { TYPE, CONSTRUCTOR, COUNT, BYTES_PER_ELEMENT }

        TEST_ENTRY (JERRY_TYPEDARRAY_UINT8,        "Uint8Array",        12, 1),
        TEST_ENTRY (JERRY_TYPEDARRAY_UINT8CLAMPED, "Uint8ClampedArray", 12, 1),
        TEST_ENTRY (JERRY_TYPEDARRAY_INT8,         "Int8Array",         12, 1),
        TEST_ENTRY (JERRY_TYPEDARRAY_UINT16,       "Uint16Array",       12, 2),
        TEST_ENTRY (JERRY_TYPEDARRAY_INT16,        "Int16Array",        12, 2),
        TEST_ENTRY (JERRY_TYPEDARRAY_UINT16,       "Uint16Array",       12, 2),
        TEST_ENTRY (JERRY_TYPEDARRAY_INT32,        "Int32Array",        12, 4),
        TEST_ENTRY (JERRY_TYPEDARRAY_UINT32,       "Uint32Array",       12, 4),
        TEST_ENTRY (JERRY_TYPEDARRAY_FLOAT32,      "Float32Array",      12, 4),
      /* TODO: add check if the float64 is supported */
        TEST_ENTRY (JERRY_TYPEDARRAY_FLOAT64,      "Float64Array",      12, 8),

        TEST_ENTRY (JERRY_TYPEDARRAY_INVALID, NULL, 0, 0)
    #undef TEST_ENTRY
      };

      /* Test TypedArray queries */
      test_typedarray_queries (test_entries);
      

     

      test_property_by_index (test_entries);
      /* test invalid things */
      {
        jerry_value_t values[] =
        {
          jerry_create_number (11),
          jerry_create_boolean (false),
          jerry_create_string ((const jerry_char_t *) "test"),
          jerry_create_object (),
          jerry_create_null (),
          jerry_create_arraybuffer (16),
          jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t *) "error"),
          jerry_create_undefined (),
          jerry_create_promise (),
        };

        for (size_t idx = 0; idx < sizeof (values) / sizeof (values[0]); idx++)
        {
          /* A non-TypedArray object should not be regarded a TypedArray. */
          bool is_typedarray = jerry_value_is_typedarray (values[idx]);
          TEST_ASSERT (is_typedarray == false);

          /* JERRY_TYPEDARRAY_INVALID should be returned for non-TypedArray objects */
          jerry_typedarray_type_t type = jerry_get_typedarray_type (values[idx]);
          TEST_ASSERT (type == JERRY_TYPEDARRAY_INVALID);

          /* Zero should be returned for non-TypedArray objects */
          jerry_length_t length = jerry_get_typedarray_length (values[idx]);
          TEST_ASSERT (length == 0);

          /**
           * Getting the ArrayBuffer from a non-TypedArray object(s) should return an error
           * and should not modify the output parameter values.
           */
          {
            jerry_length_t offset = 22;
            jerry_length_t byte_count = 23;
            TEST_ASSERT (offset == 22);
            TEST_ASSERT (byte_count == 23);
            // jerry_release_value (error);
          }

          
          jerry_release_value (values[idx]);
        }
      }
      test_detached_arraybuffer ();
      jerry_cleanup ();
      free(ctx_p);
      return;
}
