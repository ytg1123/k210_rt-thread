/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018/09/30     Bernard      The first version
 */

#include <rtthread.h>
#include <stdio.h>
#include <fpioa.h>
#include "gpio.h"

static rt_thread_t led = RT_NULL;

void thread_blinky_led_entry(void* paramenter)
{
    gpio_init();
    fpioa_set_function(12,FUNC_GPIO0);
    fpioa_set_function(13,FUNC_GPIO1);
    fpioa_set_function(14,FUNC_GPIO2);
    gpio_set_drive_mode(0,GPIO_DM_OUTPUT);
    gpio_set_drive_mode(1,GPIO_DM_OUTPUT);
    gpio_set_drive_mode(2,GPIO_DM_OUTPUT);
	while(1)
	{
		gpio_set_pin(0,GPIO_PV_LOW);
		rt_thread_delay(RT_TICK_PER_SECOND/2);
		gpio_set_pin(0,GPIO_PV_HIGH);
		rt_thread_delay(RT_TICK_PER_SECOND/2);		
        gpio_set_pin(1,GPIO_PV_LOW);
		rt_thread_delay(RT_TICK_PER_SECOND/2);
		gpio_set_pin(1,GPIO_PV_HIGH);
		rt_thread_delay(RT_TICK_PER_SECOND/2);
        gpio_set_pin(2,GPIO_PV_LOW);
		rt_thread_delay(RT_TICK_PER_SECOND/2);
		gpio_set_pin(2,GPIO_PV_HIGH);
		rt_thread_delay(RT_TICK_PER_SECOND/2);
	}
}

int main(void)
{
	rt_uint16_t i,j;
	struct rt_device_rect_info rect;
	led = rt_thread_create(	"sys_led",
				thread_blinky_led_entry,
				RT_NULL,
				1024,
				10,
				3
				);	
	if(led != RT_NULL)
	{
		rt_thread_startup(led);
	}
	rt_thread_delay(RT_TICK_PER_SECOND);
                           
	return 0;
}                                                                                                         

