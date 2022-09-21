/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <rthw.h>
#include <rtthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <bsp.h>

#include "fpioa.h"
#include "lcd.h"
#include "sysctl.h"
#include "nt35310.h"
#include "board_config.h"
#include "i2s.h"
#include "dmac.h"
#include "uarths.h"

#include "gpio.h"
#include "dvp.h"
#include "plic.h"
#include "utils.h"
#include "kpu.h"

#include "3dmain.hpp"
#include "gpiohs.h"
#include "3dconfig.hpp"
#include "fileio.h"
#include "sdcard.h"

#ifdef BSP_USING_K210MMD

#define K210_DBG
#undef K210_DBG

static rt_thread_t k210mmd = RT_NULL;
static rt_thread_t k210vtask = RT_NULL;
#define CORE_VOLTAGE_GPIONUM (5)

#if defined(RT_USING_SPI_MSD) && defined(RT_USING_DFS_ELMFAT)
#include <spi_msd.h>
#include <dfs_fs.h>
#include <dfs_file.h>
#include <dfs_private.h>
#endif

//uint32_t g_lcd_gram[LCD_X_MAX * LCD_Y_MAX / 2] __attribute__((aligned(128)));

static void io_set_power(void)
{
#if BOARD_LICHEEDAN == 1
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
#else
    sysctl_set_power_mode(SYSCTL_POWER_BANK1, SYSCTL_POWER_V18);
#endif
}

static void io_mux_init(void)
{
#if BOARD_LICHEEDAN == 1
    fpioa_set_function(27, FUNC_SPI1_SCLK);
    fpioa_set_function(28, FUNC_SPI1_D0);
    fpioa_set_function(26, FUNC_SPI1_D1);
    fpioa_set_function(29, FUNC_GPIOHS7);

    fpioa_set_function(11, FUNC_GPIOHS0 + CORE_VOLTAGE_GPIONUM);

    fpioa_set_function(38, FUNC_GPIOHS0 + DCX_GPIONUM);
    fpioa_set_function(36, FUNC_SPI0_SS3);
    fpioa_set_function(39, FUNC_SPI0_SCLK);
    fpioa_set_function(37, FUNC_GPIOHS0 + RST_GPIONUM);
    sysctl_set_spi0_dvp_data(1);
#else
    //fpioa_set_function(8, FUNC_GPIOHS0 + DCX_GPIONUM);
    //fpioa_set_function(6, FUNC_SPI0_SS3);
    //fpioa_set_function(7, FUNC_SPI0_SCLK);
    sysctl_set_spi0_dvp_data(1);
#endif
}

//for pc implementation:
int filopen(const char *pathname,struct dfs_fd *f){
  uint32_t ret;
  // O_RDWR | O_APPEND | O_CREAT
  ret = dfs_file_open(f,pathname,O_RDONLY) ;
  if ( ret < 0) return 1; // error 
  else return 0;	// OK
}

int filread(struct dfs_fd *fp,void *buff,size_t by){
  uint32_t len;
  len = dfs_file_read(fp,buff,by);
  //rt_kprintf("filread %d readed len:%d\n",by,len);
  if(len>0)return len;/*ok*/;
  return 0; /* fail */
}

int filclose(struct dfs_fd *fp){
  return  dfs_file_close(fp);
}

int core1_function(void *ctx)
{
    uint64_t core = current_coreid();
    printf("Core %ld Hello world\n", core);
    //vTask((void*)0);
    //while(1);
    k210vtask = rt_thread_create( "k210vtask",
                             vTask,
                             RT_NULL,
                             1024 * 8 ,
                             8,
                             3
                             );
    if(k210vtask != RT_NULL) {
        rt_thread_startup(k210vtask);
        printf("k210vtask started \n");
    }

}

#define CSR_CYCLE     0xc00U
#define CSR_MCYCLE    0xb00U
uint64_t get_time(void)
{
    //uint64_t v_cycle = read_cycle();
    uint64_t freq  ;
    uint64_t v_cycle = read_csr(0xB00U);

    freq = sysctl_clock_get_freq(SYSCTL_CLOCK_CPU);
#ifdef K210_DBG
    printf("SYSCTL_CLOCK_CPU=%d\n",SYSCTL_CLOCK_CPU);
    printf("sysctl_clock_get_freq=%d\n",freq);
#endif
    return v_cycle * 1000000 / freq;
}

void k210_mmd_main(void *parameter)
{
    //FATFS sdfs;
    uint32_t ret,length;
    struct dfs_fd fd;

    io_mux_init();
#if BOARD_LICHEEDAN == 1
 #if defined(OVER_VOLTAGE)
    gpiohs_set_drive_mode(CORE_VOLTAGE_GPIONUM, GPIO_DM_INPUT);
    gpiohs_set_pin(CORE_VOLTAGE_GPIONUM, GPIO_PV_LOW);
  //overclock settings, edit if you want
  #define MHz *1000000
    rt_thread_mdelay(100);
    sysctl_cpu_set_freq(600 MHz);
    rt_thread_mdelay(100);
  #undef MHz
 #endif
    io_set_power();
    lcd_init();
    lcd_set_direction(DIR_YX_RLDU);
#else
    lcd_set_direction(DIR_YX_RLUD);
#endif /* #if BOARD_LICHEEDAN */

#ifdef K210_DBG
    printf("k210_mmd \n");
#endif

    lcd_clear(BLACK);

    /* Clear stdin buffer before scanf */
    //sys_stdin_flush();

    // make another thread\n

    uint64_t core = current_coreid();
    printf("Core %ld Hello world\n", core);
#if PROCESSNUM==2
    printf("core 1 registering...\n");
    register_core1(core1_function, 0);
    core1_function((void*)0);
    for(volatile int i=0;i<10000;i++);
#endif

    /* system start */
#ifdef K210_DBG
    printf("system start\n");
#endif

    main3d("model.pmd","motion.vmd");
    while(1);
}

int k210_mmd(void)
{
        rt_uint16_t i,j;
        struct rt_device_rect_info rect;
        k210mmd = rt_thread_create( "k210mmd",
                                     k210_mmd_main,
                                     RT_NULL,
                                     1024 * 64 ,
                                     10,
                                     3
                                     );
        if(k210mmd != RT_NULL)
        {
                rt_thread_startup(k210mmd);
        }
        //rt_thread_delay(RT_TICK_PER_SECOND);

        return 0;
}

void k210_mmd_stop(void)
{
        if(k210mmd != RT_NULL)
        {
        rt_thread_delete(k210mmd);
        }
}

//INIT_APP_EXPORT(k210_mmd);
MSH_CMD_EXPORT(k210_mmd,start k210_mmd thread);
MSH_CMD_EXPORT(k210_mmd_stop,stop k210_mmd thread);

#endif /* BSP_USING_K210MMD */

