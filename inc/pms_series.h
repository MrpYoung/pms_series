/*
 * Copyright (c) 2020 MrpYoung <623216350@qq.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-04-01     MrpYoung  the first version
 */
 
#ifndef __PMS_SERIES_H__
#define __PMS_SERIES_H__

#define DBG_TAG "pms_series"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>
#include <rthw.h>
#include <rtdevice.h>

#if defined(PMS_SERIES_USING_PMS5003ST)
#define COMM_LEN            40              //data = 17  +  check 2  = 19
#else 
#define COMM_LEN            32
#endif

struct pms_device
{
    rt_device_t serial; 
    struct serial_configure config; 
    rt_uint16_t len;
    rt_uint16_t PM1_0_CF1;
    rt_uint16_t PM2_5_CF1;
    rt_uint16_t PM10_0_CF1;
    rt_uint16_t PM1_0_amb;
    rt_uint16_t PM2_5_amb;
    rt_uint16_t PM10_0_amb;
    rt_uint16_t air_0_3um;
    rt_uint16_t air_0_5um;
    rt_uint16_t air_1_0um;
    rt_uint16_t air_2_5um;
    rt_uint16_t air_5_0um;
    rt_uint16_t air_10_0um;
    
    rt_uint16_t hcho;
    rt_uint16_t temp;
    rt_uint16_t humi;
    
    rt_uint8_t  version;
    rt_uint8_t  errorCode;
    rt_uint16_t checksum;   
};
typedef struct pms_device *pms_device_t;

pms_device_t pms_init(const char *uart_name);

rt_err_t frame_check(pms_device_t dev,rt_uint8_t *buf,rt_uint16_t len);

rt_err_t pms_get_byte(pms_device_t dev, char data);

void pms_deinit(pms_device_t dev);

#endif 
