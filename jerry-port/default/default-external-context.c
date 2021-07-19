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

#include "jerryscript-port.h"
#include "jerryscript-port-default.h"

#if defined (JERRY_FOR_IAR_CONFIG) && (JERRY_EXTERNAL_CONTEXT == 1)

#include "generate-bytecode.h"
#include "los_task.h"
#include "target_config.h"

/**
 * use array to record the correspondence between task id and jerry-heap/context
 */
#define MAX_CONTEXT_NUM (LOSCFG_BASE_CORE_TSK_LIMIT+1)
ContextRecord g_contextRecords[MAX_CONTEXT_NUM] = {0};

void jerry_switch_context();
extern jerry_context_t *jerry_dynamic_global_context_p;

/**
 * set context function: store task id and context
 */
void
jerry_port_default_set_current_context (jerry_context_t *context_p) /**< store created context */
{
  uint32_t curTaskId = LOS_CurTaskIDGet();
  g_contextRecords[curTaskId].context_p = context_p;
  jerry_dynamic_global_context_p = context_p;
}

void jerry_switch_context()
{
  jerry_dynamic_global_context_p = g_contextRecords[LOS_NextTaskIDGet()].context_p;
}

/**
 * when task ends, the context_pointer point to NULL
 */
void
jerry_port_default_remove_current_context_record () /**< remove current task's context record in Array */
{
  uint32_t curTaskId = LOS_CurTaskIDGet();
  g_contextRecords[curTaskId].context_p = NULL;
  jerry_dynamic_global_context_p = NULL;
}

/**
 * key: global dynamic context_p for current context
 */
jerry_context_t *
jerry_port_get_current_context (void) /**< points to current task's context */
{
  return jerry_dynamic_global_context_p;
}

#else // not defined JERRY_FOR_IAR_CONFIG || not enabled JERRY_EXTERNAL_CONTEXT

/**
 * Pointer to the current context.
 * Note that it is a global variable, and is not a thread safe implementation.
 */
static jerry_context_t *current_context_p = NULL;

/**
 * Set the current_context_p as the passed pointer.
 */
void
jerry_port_default_set_current_context (jerry_context_t *context_p) /**< points to the created context */
{
  current_context_p = context_p;
} /* jerry_port_default_set_current_context */

/**
 * Get the current context.
 *
 * @return the pointer to the current context
 */
jerry_context_t *
jerry_port_get_current_context (void)
{
  return current_context_p;
} /* jerry_port_get_current_context */

#endif // defined (JERRY_FOR_IAR_CONFIG) && (JERRY_EXTERNAL_CONTEXT == 1)
