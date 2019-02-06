//
// Copyright (c) 2019
// IoTech
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_DATA_H_
#define _IOT_DATA_H_

#include "iot/os.h"

typedef enum iot_data_type_t
{
  IOT_DATA_INT8 = 0,
  IOT_DATA_UINT8 = 1,
  IOT_DATA_INT16 = 2,
  IOT_DATA_UINT16 = 3,
  IOT_DATA_INT32 = 4,
  IOT_DATA_UINT32 = 5,
  IOT_DATA_INT64 = 6,
  IOT_DATA_UINT64 = 7,
  IOT_DATA_FLOAT32 = 8,
  IOT_DATA_FLOAT64 = 9,
  IOT_DATA_BOOL = 10,
  IOT_DATA_STRING = 11,
  IOT_DATA_BLOB = 12,
  IOT_DATA_MAP = 13,
  IOT_DATA_ARRAY = 14
} iot_data_type_t;

typedef struct iot_data_t iot_data_t;

typedef struct iot_data_map_iter_t
{
  struct iot_data_map_t * map;
  struct iot_data_pair_t * pair;
} iot_data_map_iter_t;

typedef struct iot_data_array_iter_t
{
  struct iot_data_array_t * array;
  uint32_t index;
} iot_data_array_iter_t ;

extern void iot_data_free (iot_data_t * data);
extern iot_data_type_t iot_data_type (const iot_data_t * data);
extern const char * iot_data_type_name (const iot_data_t * data);

extern iot_data_t * iot_data_alloc_i8 (int8_t val);
extern iot_data_t * iot_data_alloc_ui8 (uint8_t val);
extern iot_data_t * iot_data_alloc_i16 (int16_t val);
extern iot_data_t * iot_data_alloc_ui16 (uint16_t val);
extern iot_data_t * iot_data_alloc_i32 (int32_t val);
extern iot_data_t * iot_data_alloc_ui32 (uint32_t val);
extern iot_data_t * iot_data_alloc_i64 (int64_t val);
extern iot_data_t * iot_data_alloc_ui64 (uint64_t val);
extern iot_data_t * iot_data_alloc_f32 (float val);
extern iot_data_t * iot_data_alloc_f64 (double val);
extern iot_data_t * iot_data_alloc_bool (bool val);
extern iot_data_t * iot_data_alloc_string (const char * val, bool copy);
extern iot_data_t * iot_data_alloc_blob (uint8_t * data, uint32_t size, bool copy);
extern iot_data_t * iot_data_map_alloc (iot_data_type_t key_type);
extern iot_data_t * iot_data_array_alloc (const uint32_t size);

extern int8_t iot_data_get_i8 (const iot_data_t * data);
extern uint8_t iot_data_get_ui8 (const iot_data_t * data);
extern int16_t iot_data_get_i16 (const iot_data_t * data);
extern uint16_t iot_data_get_ui16 (const iot_data_t * data);
extern int32_t iot_data_get_i32 (const iot_data_t * data);
extern uint32_t iot_data_get_ui32 (const iot_data_t * data);
extern int64_t iot_data_get_i64 (const iot_data_t * data);
extern uint64_t iot_data_get_ui64 (const iot_data_t * data);
extern float iot_data_get_f32 (const iot_data_t * data);
extern double iot_data_get_f64 (const iot_data_t * data);
extern bool iot_data_get_bool (const iot_data_t * data);
extern const char * iot_data_get_string (const iot_data_t * data);
extern const uint8_t * iot_data_get_blob (const iot_data_t * data, uint32_t * size);

extern void iot_data_map_add (iot_data_t * map, iot_data_t * key, iot_data_t * val);
extern const iot_data_t * iot_data_map_get (const iot_data_t * , const iot_data_t * key);

extern void iot_data_array_add (iot_data_t * array, const uint32_t index, iot_data_t * val);
extern const iot_data_t * iot_data_array_get (const iot_data_t * array, const uint32_t index);
extern uint32_t iot_data_array_size (const iot_data_t * array);

extern void iot_data_map_iter (const iot_data_t * map, iot_data_map_iter_t * iter);
extern bool iot_data_map_iter_next (iot_data_map_iter_t * iter);
extern const iot_data_t * iot_data_map_iter_get_key (const iot_data_map_iter_t * iter);
extern const iot_data_t * iot_data_map_iter_get_value (const iot_data_map_iter_t * iter);

extern void iot_data_array_iter (const iot_data_t * map, iot_data_array_iter_t * iter);
extern bool iot_data_array_iter_next (iot_data_array_iter_t * iter);
extern uint32_t iot_data_array_iter_get_index (const iot_data_array_iter_t * iter);
extern const iot_data_t * iot_data_array_iter_get_value (const iot_data_array_iter_t * iter);

#endif