/*
 * Copyright (c) 2020 MrpYoung <623216350@qq.com>
 *
 * SPDX-License-Identifier: MIT
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-04-01     MrpYoung  the first version
 */
#define DBG_ENABLE
#define DBG_SECTION_NAME "pms_series"
#define DBG_LEVEL DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>

#include <rtthread.h>
#include <finsh.h>

#include <pms_series.h>

#if defined(PMS_SERIES_SAMPLE_USING_UART1)
#define PMS_SERIES_UART       "uart1"
#elif defined(PMS_SERIES_SAMPLE_USING_UART2)
#define PMS_SERIES_UART       "uart2"
#elif defined(PMS_SERIES_SAMPLE_USING_UART3)
#define PMS_SERIES_UART       "uart3"
#elif defined(PMS_SERIES_SAMPLE_USING_UART4)
#define PMS_SERIES_UART       "uart4"
#elif defined(PMS_SERIES_SAMPLE_USING_UART5)
#define PMS_SERIES_UART       "uart5"
#elif defined(PMS_SERIES_SAMPLE_USING_UART6)
#define PMS_SERIES_UART       "uart6"
#endif

#define DBG_ENABLE

struct rx_msg
{
    rt_device_t dev;
    rt_size_t size;
};

#ifdef PMS_SERIES_SAMPLE_USING_DMA
struct rt_messagequeue pms_mq;
#else
struct rt_semaphore pms_sem;
#endif

void pms_series_debug(pms_device_t dev)
{
	LOG_D("*********************************begin***********************************");
	LOG_D("PM1_0_CF1 = %5d\tPM2_5_CF1 = %5d\tPM10_0_CF1 = %5d",dev->PM1_0_CF1,dev->PM2_5_CF1,dev->PM10_0_CF1);
	LOG_D("PM1_0_amb = %5d\tPM2_5_amb = %5d\tPM10_0_amb = %5d",dev->PM1_0_amb,dev->PM2_5_amb,dev->PM10_0_amb);

	LOG_D("air_0_3um = %5d\tair_0_5um = %5d\tair_1_0um  = %5d",dev->air_0_3um,dev->air_0_5um,dev->air_1_0um);	
	LOG_D("air_2_5um = %5d\tair_5_0um = %5d\tair_10_0um = %5d",dev->air_2_5um,dev->air_5_0um,dev->air_10_0um);
	LOG_D("hcho	 = %5d\ttemp      = %5d\thum        = %5d",dev->hcho,dev->temp,dev->humi);

	LOG_D("version   = %5d                    errorCode = %5d",dev->version,dev->errorCode);
	LOG_D("********************************over*************************************");			
}

static void serial_thread_entry(void *parameter)
{
#ifndef PMS_SERIES_SAMPLE_USING_DMA	
	rt_err_t result;
	char ch;
	pms_device_t dev = parameter;
    while (1)
    {
		while (rt_device_read(dev->serial, 0, &ch, 1) == 0)
		{
			rt_sem_control(&pms_sem, RT_IPC_CMD_RESET, RT_NULL);
			rt_sem_take(&pms_sem, RT_WAITING_FOREVER);
		}
		result = pms_get_byte(dev,ch);	
		if (result == RT_EOK)
		{
			pms_series_debug(dev);
		}	
    }
#endif 
	
#ifdef PMS_SERIES_SAMPLE_USING_DMA	
    struct rx_msg msg;
    rt_err_t result;
    rt_uint32_t rx_length;
    static rt_uint8_t rx_buffer[RT_SERIAL_RB_BUFSZ + 1];

	pms_device_t dev = parameter;
	
    while (1)
    {
        rt_memset(&msg, 0, sizeof(msg));
        result = rt_mq_recv(&pms_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
        if (result == RT_EOK)
        {
            rx_length = rt_device_read(msg.dev, 0, rx_buffer, msg.size);
            rx_buffer[rx_length] = '\0';
			result = frame_check(dev,rx_buffer,rx_length);
			if (result == RT_EOK)
			{
				pms_series_debug(dev);
				LOG_D("rx buff success");
			}
			else
			{
				LOG_E("rx buff error");
			}
		}
	}
#endif	
}

static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
	RT_ASSERT(dev);
#ifndef PMS_SERIES_SAMPLE_USING_DMA
    if (size > 0)
    {
        rt_sem_release(&pms_sem);
    }
    return RT_EOK;	
#endif
	
#ifdef PMS_SERIES_SAMPLE_USING_DMA
    rt_err_t result;	
    struct rx_msg msg;
    msg.dev = dev;
    msg.size = size;

    result = rt_mq_send(&pms_mq, &msg, sizeof(msg));
    if ( result == -RT_EFULL)
    {
        rt_kprintf("message queue full！\n");
    }
    return result;	
#endif
}

int pms_sample(int argc, char *argv[])
{
	static pms_device_t dev = NULL;
    rt_err_t ret = RT_EOK;
	dev = pms_init(PMS_SERIES_UART);
#ifndef PMS_SERIES_SAMPLE_USING_DMA
    rt_sem_init(&pms_sem, "pms_sem", 0, RT_IPC_FLAG_FIFO);

    rt_device_open(dev->serial, RT_DEVICE_FLAG_INT_RX);

    rt_device_set_rx_indicate(dev->serial, uart_input);
#endif
#ifdef 	PMS_SERIES_SAMPLE_USING_DMA
    static char msg_pool[256];	
	rt_err_t result;
    result = rt_mq_init(&pms_mq, "pms_mq",
               msg_pool,                 
               sizeof(struct rx_msg),    
               sizeof(msg_pool),         
               RT_IPC_FLAG_FIFO);        
    if (result != RT_EOK)
    {
        LOG_E("init message queue failed.\n");
    }
    result = rt_device_open(dev->serial, RT_DEVICE_FLAG_DMA_RX);
    if (result != RT_EOK)
    {
        LOG_E("open device failed.\n");
    }
    result = rt_device_set_rx_indicate(dev->serial, uart_input);
    if (result != RT_EOK)
    {
        LOG_E("set rx indicate failed.\n");
    }	
#endif	
    rt_thread_t thread = rt_thread_create("serial", serial_thread_entry, dev, 1024, 3, 10);
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        ret = RT_ERROR;
    }

    return ret;
	
}
MSH_CMD_EXPORT(pms_sample, pms_series drive sample);
