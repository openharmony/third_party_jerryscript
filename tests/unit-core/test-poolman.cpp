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

/**
 * Unit test for pool manager.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
extern "C"
{
  #include "jmem.h"
}
#include "jerryscript-port.h"
#include "jerryscript-port-default.h"

#define JMEM_ALLOCATOR_INTERNAL
#include "jmem-allocator-internal.h"

#include "test-common.h"
#include <gtest/gtest.h>
/* Iterations count. */
const uint32_t test_iters = 1024;

/* Subiterations count. */
#define TEST_MAX_SUB_ITERS  1024
#define TEST_CHUNK_SIZE 8

uint8_t *ptrs[TEST_MAX_SUB_ITERS];
uint8_t data[TEST_MAX_SUB_ITERS][TEST_CHUNK_SIZE];

class PoolmanTest : public testing::Test{
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "PoolmanTest SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "PoolmanTest TearDownTestCase";
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
HWTEST_F(PoolmanTest, Test001, testing::ext::TestSize.Level1)
{
  TEST_INIT ();
  jerry_context_t *ctx_p = jerry_create_context (1024, context_alloc_fn, NULL);
  jerry_port_default_set_current_context (ctx_p);
  jmem_init ();
  for (uint32_t i = 0; i < test_iters; i++)
  {
    const size_t subiters = ((size_t) rand () % TEST_MAX_SUB_ITERS) + 1;
    /* jmem_heap_print (false); */
    for (size_t j = 0; j < subiters; j++)
    {
      if (rand () % 256 == 0)
      {
        jmem_pools_collect_empty ();
      }

      if (ptrs[j] != NULL)
      {
        TEST_ASSERT (!memcmp (data[j], ptrs[j], TEST_CHUNK_SIZE));
        jmem_pools_free (ptrs[j], TEST_CHUNK_SIZE);
      }
    }
  }

#ifdef JMEM_STATS
  jmem_heap_stats_print ();
#endif /* JMEM_STATS */
  jmem_finalize ();
  free (ctx_p);
  return;
}
