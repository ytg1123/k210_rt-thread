/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 */

#include <rthw.h>
#include <rtthread.h>

#include "board.h"
#include "tick.h"

#include "drv_uart.h"

#include "encoding.h"
#include "fpioa.h"
#include "dmac.h"

void init_bss(void)
{
    unsigned int *dst;

    dst = &__bss_start;
    while (dst < &__bss_end)
    {
        *dst++ = 0;
    }
}

void primary_cpu_entry(void)
{
    extern void entry(void);

    /* disable global interrupt */
    init_bss();
    rt_hw_interrupt_disable();
    entry();
}

#include <clint.h>
#include <sysctl.h>

int freq(void)
{
    rt_uint64_t value = 0;

    value = sysctl_clock_get_freq(SYSCTL_CLOCK_PLL0);
    rt_kprintf("PLL0: %d\n", value);
    value = sysctl_clock_get_freq(SYSCTL_CLOCK_PLL1);
    rt_kprintf("PLL1: %d\n", value);
    value = sysctl_clock_get_freq(SYSCTL_CLOCK_PLL2);
    rt_kprintf("PLL2: %d\n", value);
    value = sysctl_clock_get_freq(SYSCTL_CLOCK_CPU);
    rt_kprintf("CPU : %d\n", value);
    value = sysctl_clock_get_freq(SYSCTL_CLOCK_APB0);
    rt_kprintf("APB0: %d\n", value);
    value = sysctl_clock_get_freq(SYSCTL_CLOCK_APB1);
    rt_kprintf("APB1: %d\n", value);
    value = sysctl_clock_get_freq(SYSCTL_CLOCK_APB2);
    rt_kprintf("APB2: %d\n", value);
    value = sysctl_clock_get_freq(SYSCTL_CLOCK_HCLK);
    rt_kprintf("HCLK: %d\n", value);

    value = clint_get_time();
    rt_kprintf("mtime: %d\n", value);

    return 0;
}
MSH_CMD_EXPORT(freq, show freq info);

#ifdef RT_USING_SMP
extern int rt_hw_clint_ipi_enable(void);
#endif

void rt_hw_board_init(void)
{
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 400000000UL);
    // PLL2 set for k210_mmd
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);

    /* Init FPIOA */
    fpioa_init();
    /* Dmac init */
    dmac_init();

    /* initalize interrupt */
    rt_hw_interrupt_init();
    /* initialize hardware interrupt */
    rt_hw_uart_init();

    rt_hw_tick_init();

#ifdef RT_USING_SMP
    rt_hw_clint_ipi_enable();
#endif

#ifdef RT_USING_CONSOLE
    /* set console device */
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif /* RT_USING_CONSOLE */

#ifdef RT_USING_HEAP
    rt_kprintf("heap: [0x%08x - 0x%08x]\n", (rt_ubase_t) RT_HW_HEAP_BEGIN, (rt_ubase_t) RT_HW_HEAP_END);
    /* initialize memory system */
    rt_system_heap_init(RT_HW_HEAP_BEGIN, RT_HW_HEAP_END);
#endif

#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif
}
void rt_hw_cpu_reset(void)
{
    sysctl->soft_reset.soft_reset = 1;
    while(1);
}

/* IO definitions (access restrictions to peripheral registers) */
/**
    \defgroup CMSIS_glob_defs CMSIS Global Defines

    <strong>IO Type Qualifiers</strong> are used
    \li to specify the access to peripheral variables.
    \li for automatic generation of peripheral register debug information.
*/
#ifdef __cplusplus
  #define   __I     volatile             /*!< Defines 'read only' permissions                 */
#else
  #define   __I     volatile const       /*!< Defines 'read only' permissions                 */
#endif
#define     __O     volatile             /*!< Defines 'write only' permissions                */
#define     __IO    volatile             /*!< Defines 'read / write' permissions              */

void rt_hw_gt_maixpy(void)
{
    uint32_t jump_addr;
    void (*jump_to_maixpy)(void);

    /* convert user application address */
    /* 0000000080300000 */
    jump_addr = *(__IO uint32_t*) (0x80300000);
    jump_to_maixpy = (void (*)(void)) jump_addr;
    jump_to_maixpy();

}

MSH_CMD_EXPORT_ALIAS(rt_hw_cpu_reset, reboot, reset machine);
