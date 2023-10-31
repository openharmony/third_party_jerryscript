/*
 *  Copyright (c)  Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *  Description:  jerry literal cache definition
 *  Author: Harmony OS Software group
 *  Create: 2020-11-19
 */
#ifndef JERRY_LITERAL_CACHE_H
#define JERRY_LITERAL_CACHE_H

#include <stdint.h>

void InitJerryLiteralCache(void);

void ClearJerryLiteralCache(void);

void AddJerryLiteralCache(int32_t hash, void *p);

void DelJerryLiteralCache(int32_t hash);

void *GetJerryLiteralCache(int32_t hash);
#endif