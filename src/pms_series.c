/*
 * Copyright (c) 2020 MrpYoung <623216350@qq.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * Change Logs:
 * Date           Author         Notes
 * 2020-04-01     MrpYoung       the first version
 * 2020-04-12     Aspiriniii     add support for pms5003t
 */
#define DBG_SECTION_NAME "pms_series"
#define DBG_LEVEL DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>

#include "pms_series.h"

#define COMM_START1			0x42
#define COMM_START2			0x4D

#define FRAME_HEAD1			0x00
#define FRAME_HEAD2			0x01
#define FRAME_LENH			0x02
#define FRAME_LENL			0x03
#define FRAME_RECEIVE		0x04
#define FRAME_CHECK			0x05

rt_err_t frame_check(pms_device_t dev,rt_uint8_t *buf,rt_uint16_t len)
{
	rt_uint16_t sum=0;
    RT_ASSERT(dev);
	for(uint8_t i=0;i<(len-2);i++)
	{
		sum += buf[i];
	}	
	if((buf[len-1] == (sum&0xFF)) && (buf[len-2] == (sum >> 8)))
	{
		dev->PM1_0_CF1  = ((rt_uint16_t)(buf[4])<<8) | buf[5];
		dev->PM2_5_CF1  = ((rt_uint16_t)(buf[6])<<8) | buf[7];
		dev->PM10_0_CF1 = ((rt_uint16_t)(buf[8])<<8) | buf[9];
		dev->PM1_0_amb  = ((rt_uint16_t)(buf[10])<<8) | buf[11];
		dev->PM2_5_amb  = ((rt_uint16_t)(buf[12])<<8) | buf[13];
		dev->PM10_0_amb = ((rt_uint16_t)(buf[14])<<8) | buf[15];
		dev->air_0_3um  = ((rt_uint16_t)(buf[16])<<8) | buf[17];
		dev->air_0_5um  = ((rt_uint16_t)(buf[18])<<8) | buf[19];
		dev->air_1_0um  = ((rt_uint16_t)(buf[20])<<8) | buf[21];
		dev->air_2_5um  = ((rt_uint16_t)(buf[22])<<8) | buf[23];
#if !defined(PMS_SERIES_USING_PMS5003T)
		dev->air_5_0um  = ((rt_uint16_t)(buf[24])<<8) | buf[25];
		dev->air_10_0um = ((rt_uint16_t)(buf[26])<<8) | buf[27];
#else
		dev->temp 		= ((rt_uint16_t)(buf[24])<<8) | buf[25];
		dev->humi 		= ((rt_uint16_t)(buf[26])<<8) | buf[27];		
#endif
#if defined(PMS_SERIES_USING_PMS5003S)
		dev->hcho 		= ((rt_uint16_t)(buf[28])<<8) | buf[29];		
#endif
#if defined(PMS_SERIES_USING_PMS5003ST)
		dev->hcho 		= ((rt_uint16_t)(buf[28])<<8) | buf[29];
		dev->temp 		= ((rt_uint16_t)(buf[30])<<8) | buf[31];
		dev->humi 		= ((rt_uint16_t)(buf[32])<<8) | buf[33];
#endif

#if defined(PMS_SERIES_USING_PMS5003) || defined(PMS_SERIES_USING_PMS5003T) || defined(PMS_SERIES_USING_PMS5003ST) || defined(PMS_SERIES_USING_PMSA003) || defined(PMS_SERIES_USING_PMS7003M)
		dev->version	= buf[len - 4];
		dev->errorCode	= buf[len - 3];
#endif
		return RT_EOK;
	}
	return RT_ERROR;
}

rt_err_t pms_get_byte(pms_device_t dev, char data)
{
	rt_err_t result;
	static uint8_t state = FRAME_HEAD1;
	static uint8_t cnt = 0;
	static rt_uint8_t buf[40] = {0};
	RT_ASSERT(dev);
	if(state == FRAME_HEAD1 && data == COMM_START1)
	{
		buf[cnt++] = data;
		state = FRAME_HEAD2;
	}
	else if (state == FRAME_HEAD2 && data == COMM_START2)
	{
		buf[cnt++] = data;		
		state = FRAME_LENH;
	}
	else if (state == FRAME_LENH)
	{
		buf[cnt++] = data;			
		state = FRAME_LENL;
	}
	else if (state == FRAME_LENL)
	{
		buf[cnt++] = data;
		state = FRAME_RECEIVE;
	}
	else if (state == FRAME_RECEIVE)
	{
		buf[cnt++] = data;
		if(cnt >= COMM_LEN - 1)
			state = FRAME_CHECK;
	} 
	else if (state == FRAME_CHECK)
	{
		buf[cnt++] = data;
		state = FRAME_HEAD1;
		cnt = 0;
		result = frame_check(dev, buf, COMM_LEN);
		if (result == RT_EOK)
		{
			LOG_D("check success");
			return result;			
		}
		else
		{
			LOG_E("check error");
		}
	}
	else {}
	return result;		
}

pms_device_t pms_init(const char *uart_name)
{
	pms_device_t dev;	
    RT_ASSERT(uart_name);

    dev = rt_calloc(1, sizeof(struct pms_device));
    if (dev == RT_NULL)
    {
        LOG_E("Can't allocate memory for pms device %s",uart_name);
        return RT_NULL;
    }	
    dev->serial = rt_device_find(uart_name);	
    if (!dev->serial)
    {
		rt_free(dev);
        rt_kprintf("find %s failed!\n", uart_name);
    } 
	else
	{		
		dev->config.baud_rate = BAUD_RATE_9600;
		dev->config.data_bits = DATA_BITS_8;
		dev->config.stop_bits = STOP_BITS_1;
		dev->config.parity = PARITY_NONE;
		dev->config.bit_order = BIT_ORDER_LSB;
		dev->config.invert = NRZ_NORMAL;
		dev->config.bufsz = RT_SERIAL_RB_BUFSZ;
		dev->config.reserved = 0;
		rt_device_control(dev->serial, RT_DEVICE_CTRL_CONFIG, &dev->config);	
	}
	return dev;
}

void pms_deinit(pms_device_t dev)
{
	RT_ASSERT(dev);
    rt_free(dev);	
}
