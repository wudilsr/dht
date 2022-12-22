/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __CUTILS_STR_PARMS_H
#define __CUTILS_STR_PARMS_H

#include <stdint.h>

struct str_parms;

struct str_parms* str_parms_create(void);
struct str_parms* str_parms_create_str(const char* _string);
void str_parms_destroy(struct str_parms* str_parms);

void str_parms_del(struct str_parms* str_parms, const char* key);

int str_parms_add_str(struct str_parms* str_parms, const char* key,
                      const char* value);
int str_parms_add_int(struct str_parms* str_parms, const char* key, int value);

int str_parms_add_float(struct str_parms* str_parms, const char* key,
                        float value);

int str_parms_get_str(struct str_parms* str_parms, const char* key,
                      char* out_val, int len);
int str_parms_get_int(struct str_parms* str_parms, const char* key,
                      int* out_val);
int str_parms_get_float(struct str_parms* str_parms, const char* key,
                        float* out_val);

char* str_parms_to_str(struct str_parms* str_parms);

/* debug */
void str_parms_dump(struct str_parms* str_parms);

#endif /* __CUTILS_STR_PARMS_H */
