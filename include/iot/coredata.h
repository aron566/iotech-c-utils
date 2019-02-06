//
// Copyright (c) 2019
// IoTech
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef _IOT_COREDATA_H_
#define _IOT_COREDATA_H_

#include "iot/data.h"

typedef struct iot_coredata_t iot_coredata_t;
typedef struct iot_coredata_sub_t iot_coredata_sub_t;
typedef struct iot_coredata_pub_t iot_coredata_pub_t;

typedef iot_data_t * (*iot_data_pub_cb_fn_t) (void * self);
typedef void (*iot_data_sub_fn_t) (iot_data_t * data, void * self, const char * match);

extern iot_coredata_t * iot_coredata_alloc (void);
extern void iot_coredata_free (iot_coredata_t * cd);

/* Data subscription */

extern iot_coredata_sub_t * iot_coredata_sub_alloc (iot_coredata_t * cd, void * self, iot_data_sub_fn_t callback, const char * pattern);
extern void iot_coredata_sub_free (iot_coredata_sub_t * sub);

/* Data publication */

extern iot_coredata_pub_t * iot_coredata_pub_alloc (iot_coredata_t * cd, void * self, iot_data_pub_cb_fn_t callback, const char * topic);
extern void iot_coredata_pub_free  (iot_coredata_pub_t * pub);
extern void iot_coredata_publish (iot_coredata_pub_t * pub, iot_data_t * data);

#endif