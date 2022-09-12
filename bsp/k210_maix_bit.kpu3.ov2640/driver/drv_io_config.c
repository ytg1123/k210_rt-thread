/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-19     ZYH          first version
 */

#include <rtthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#ifdef CAM_USING_OV2640
#include "drv_ov2640.h"
#endif

#ifdef CAM_USING_OV5640
#include "drv_ov5640.h"
#endif

#include "dvp.h"
#include "plic.h"
#include "fpioa.h"
#include "lcd.h"
#include "board_config.h"
#include "drv_io_config.h"
#include "gpio_common.h"
#include "nt35310.h"
#include "sysctl.h"

#define OV_WIDTH   LCD_X_MAX
#define OV_HIGHT   LCD_Y_MAX

#define DCMI_RESET_LOW()      dvp->cmos_cfg &= ~DVP_CMOS_RESET
#define DCMI_RESET_HIGH()     dvp->cmos_cfg |= DVP_CMOS_RESET
#define DCMI_PWDN_LOW()       dvp->cmos_cfg &= ~DVP_CMOS_POWER_DOWN
#define DCMI_PWDN_HIGH()      dvp->cmos_cfg |= DVP_CMOS_POWER_DOWN

#define HS_GPIO(n) (FUNC_GPIOHS0 + n)
#define CORE_VOLTAGE_GPIONUM (5)

#define IOCONFIG(pin,func)  {pin, func, #func}

static struct io_config
{
    int io_num;
    fpioa_function_t func;
    const char * func_name;
} io_config[] = 
{
#if BOARD_LICHEEDAN == 1
    IOCONFIG(BSP_SPI1_CLK_PIN, FUNC_SPI1_SCLK),          /* 27 */
    IOCONFIG(BSP_SPI1_D0_PIN, FUNC_SPI1_D0),             /* 28 */
    IOCONFIG(BSP_SPI1_D1_PIN, FUNC_SPI1_D1),             /* 26 */
    IOCONFIG(BSP_SPI1_SS0_PIN, FUNC_GPIOHS7),            /* 29 */
    //IOCONFIG(BSP_SPI1_SS0_PIN, FUNC_GPIOHS2),            /* 29 */

    IOCONFIG(11, HS_GPIO(CORE_VOLTAGE_GPIONUM)),         /* 11  */

    IOCONFIG(BSP_LCD_DC_PIN, FUNC_GPIOHS2),              /* 38 LCD DC PIN */
    IOCONFIG(BSP_LCD_CS_PIN, FUNC_SPI0_SS3),             /* 36 LCD CS PIN */
    IOCONFIG(BSP_LCD_WR_PIN, FUNC_SPI0_SCLK),            /* 39 LCD WR PIN */
    IOCONFIG(BSP_LCD_RST_PIN,FUNC_GPIOHS3),              /* 37 BSP_LCD_RST_PIN*/

    // ????
    //IOCONFIG(BSP_LCD_DC_PIN, 26),       /* 38 LCD DC PIN */
    //IOCONFIG(BSP_LCD_CS_PIN, 15),       /* 36 LCD CS PIN */
    //IOCONFIG(BSP_LCD_WR_PIN, 17),       /* 39 LCD WR PIN */
    //IOCONFIG(BSP_LCD_RST_PIN,27),       /* 37 BSP_LCD_RST_PIN*/

#else
    IOCONFIG(8, HS_GPIO(DCX_GPIONUM)), 	          
    IOCONFIG(6, FUNC_SPI0_SS3),
    IOCONFIG(7, FUNC_SPI0_SCLK ), 
#endif

#if defined(BSP_USING_MAIXBIT)
    IOCONFIG(20, FUNC_I2S0_IN_D0),
    IOCONFIG(30, FUNC_I2S0_WS),
    IOCONFIG(32, FUNC_I2S0_SCLK),
#elif defined(BSP_USING_MAIXM1DOCK)
    IOCONFIG(20, FUNC_I2S0_IN_D0),
    IOCONFIG(19, FUNC_I2S0_WS),
    IOCONFIG(18, FUNC_I2S0_SCLK),
#else 
    IOCONFIG(20, FUNC_I2S0_IN_D0),
    IOCONFIG(30, FUNC_I2S0_WS),
    IOCONFIG(32, FUNC_I2S0_SCLK),
#endif

#ifdef BSP_USING_CAMERA
    IOCONFIG(BSP_CAMERA_SCCB_SDA_PIN, FUNC_SCCB_SDA),    /* 40 */
    IOCONFIG(BSP_CAMERA_SCCB_SCLK_PIN, FUNC_SCCB_SCLK),	 /* 41 */
    IOCONFIG(BSP_CAMERA_CMOS_RST_PIN, FUNC_CMOS_RST),    /* 42 */
    IOCONFIG(BSP_CAMERA_CMOS_VSYNC_PIN, FUNC_CMOS_VSYNC),/* 43 */
    IOCONFIG(BSP_CAMERA_CMOS_PWDN_PIN, FUNC_CMOS_PWDN),	 /* 44 */
    IOCONFIG(BSP_CAMERA_CMOS_XCLK_PIN, FUNC_CMOS_XCLK),	 /* 46 */
    IOCONFIG(BSP_CAMERA_CMOS_PCLK_PIN, FUNC_CMOS_PCLK),	 /* 47 */
    IOCONFIG(BSP_CAMERA_CMOS_HREF_PIN, FUNC_CMOS_HREF),	 /* 45 */
#endif

};

void io_set_power(void) {
#if BOARD_LICHEEDAN == 1
    /* Set dvp and spi pin to 1.8V */
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
#else
    /* Set dvp and spi pin to 1.8V */
    sysctl_set_power_mode(SYSCTL_POWER_BANK0, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK1, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK2, SYSCTL_POWER_V18);
#endif
}

// overclock and voltageboost suported XD
//use  to configure core voltage.
int set_cpu_freq(uint32_t f){//MHz
  if(f<600){
    gpiohs_set_drive_mode(CORE_VOLTAGE_GPIONUM, GPIO_DM_INPUT);
    gpiohs_set_pin(CORE_VOLTAGE_GPIONUM, GPIO_PV_LOW);
  }else{
    gpiohs_set_drive_mode(CORE_VOLTAGE_GPIONUM, GPIO_DM_INPUT);
  }
  //Wait for voltage setting done.
  for(volatile int i=0;i<1000;i++);
#define MHz *1000000
  return sysctl_cpu_set_freq(f MHz)/1000000;
#undef MHz
}

void io_mux_init(void) {
#if BOARD_LICHEEDAN == 1

    /* Init DVP IO map and function settings */
    fpioa_set_function(42, FUNC_CMOS_RST);
    fpioa_set_function(44, FUNC_CMOS_PWDN);
    fpioa_set_function(46, FUNC_CMOS_XCLK);
    fpioa_set_function(43, FUNC_CMOS_VSYNC);
    fpioa_set_function(45, FUNC_CMOS_HREF);
    fpioa_set_function(47, FUNC_CMOS_PCLK);
    fpioa_set_function(41, FUNC_SCCB_SCLK);
    fpioa_set_function(40, FUNC_SCCB_SDA);

    /* Init SPI IO map and function settings */
    fpioa_set_function(38, FUNC_GPIOHS2);   /* 26 */
    fpioa_set_function(36, FUNC_SPI0_SS3);  /* 15 */
    fpioa_set_function(39, FUNC_SPI0_SCLK); /* 17 */
    fpioa_set_function(37, FUNC_GPIOHS3);   /* 27 */

    sysctl_set_spi0_dvp_data(1);
#else
    /* Init DVP IO map and function settings */
    fpioa_set_function(11, FUNC_CMOS_RST);
    fpioa_set_function(13, FUNC_CMOS_PWDN);
    fpioa_set_function(14, FUNC_CMOS_XCLK);
    fpioa_set_function(12, FUNC_CMOS_VSYNC);
    fpioa_set_function(17, FUNC_CMOS_HREF);
    fpioa_set_function(15, FUNC_CMOS_PCLK);
    fpioa_set_function(10, FUNC_SCCB_SCLK);
    fpioa_set_function(9, FUNC_SCCB_SDA);

    /* Init SPI IO map and function settings */
    fpioa_set_function(8, FUNC_GPIOHS0 + 2);
    fpioa_set_function(6, FUNC_SPI0_SS3);
    fpioa_set_function(7, FUNC_SPI0_SCLK);

    sysctl_set_spi0_dvp_data(1);
    fpioa_set_function(26, FUNC_GPIOHS0 + 8);
    gpiohs_set_drive_mode(8, GPIO_DM_INPUT);
#endif

    fpioa_set_function(20, FUNC_I2S0_IN_D0);
    fpioa_set_function(30, FUNC_I2S0_WS);
    fpioa_set_function(32, FUNC_I2S0_SCLK);

}

void io_mux_init_drv(void)
{
#if BOARD_LICHEEDAN == 1

    //fpioa_set_function(26, FUNC_SPI1_D1);
    //fpioa_set_function(27, FUNC_SPI1_SCLK);
    //fpioa_set_function(28, FUNC_SPI1_D0);
    //fpioa_set_function(29, FUNC_GPIOHS7);

    fpioa_set_function(11, FUNC_GPIOHS0 + CORE_VOLTAGE_GPIONUM);

    fpioa_set_function(38, FUNC_GPIOHS0 + DCX_GPIONUM); /* 26 GPIOHS2 */
    fpioa_set_function(36, FUNC_SPI0_SS3);              /* 15 */
    fpioa_set_function(39, FUNC_SPI0_SCLK);             /* 17 */
    fpioa_set_function(37, FUNC_GPIOHS0 + RST_GPIONUM); /* 27 GPIOHS3 */

#else
    fpioa_set_function(8, FUNC_GPIOHS0 + DCX_GPIONUM);
    fpioa_set_function(6, FUNC_SPI0_SS3);
    fpioa_set_function(7, FUNC_SPI0_SCLK);
#endif
}

void init_cam(void){

    /*init camera*/
    DCMI_PWDN_HIGH();
    rt_thread_mdelay(10);
    DCMI_PWDN_LOW();
    rt_thread_mdelay(10);

#if defined(CAM_USING_OV2640)

    /*init camera*/
    printf("ov2640 init\n");
    DCMI_PWDN_HIGH();
    rt_thread_mdelay(10);
    DCMI_PWDN_LOW();
    rt_thread_mdelay(10);
    ov2640_init();
    rt_thread_mdelay(10);
    ov2640_set_pixformat(PIXFORMAT_RGB565);
    rt_thread_mdelay(10);
    ov2640_set_framesize(FRAMESIZE_QVGA);
    rt_thread_mdelay(10);
    //light
    ov2640_set_light(0);
    rt_thread_mdelay(2);
    //saturation
    ov2640_set_saturation(3);
    rt_thread_mdelay(2);
    //contrast
    ov2640_set_contrast(3);
    rt_thread_mdelay(2);
    //brightness
    ov2640_set_brightness(4);
    rt_thread_mdelay(2);
    //effect
    ov2640_set_special_effects(0);
    rt_thread_mdelay(2);
    //exposure
    ov2640_set_exposure(4);
    rt_thread_mdelay(2);

#elif defined(CAM_USING_OV5640)
    /*init dvp camera hardware interface*/
    /* DVP init */
    dvp_init(16);
    dvp_set_xclk_rate(50000000);
    dvp_enable_burst();
    dvp_set_output_enable(0, 1);              /* AI Out Enabled */
    dvp_set_output_enable(1, 1);              /* DISP Out Enabled */
    dvp_set_image_format(DVP_CFG_RGB_FORMAT); /* RGB 565 Format */
    dvp_set_image_size(320, 240);
    ov5640_init();
    rt_thread_mdelay(10);
    printf("ov5640 init\n");

#if 0
//    OV5640_Focus_Init();
    OV5640_Light_Mode(2);      //set auto
    OV5640_Color_Saturation(6); //default
    OV5640_Brightness(8);   //default
    OV5640_Contrast(3);     //default
//    OV5640_Sharpness(33);   //set auto
//    OV5640_Auto_Focus();
#endif /* #if 0 end */

#endif
}

void enable_cam(uint8_t enable)
{
        if(enable)
        {
                dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
                plic_irq_enable(IRQN_DVP_INTERRUPT);
                dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
        }
        else{
                plic_irq_disable(IRQN_DVP_INTERRUPT);
                dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
                dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
        }
}

static int print_io_config()
{
    int i;
    rt_kprintf("IO Configuration Table\n");
    rt_kprintf("┌───────┬────────────────────────┐\n");
    rt_kprintf("│Pin    │Function                │\n");
    rt_kprintf("├───────┼────────────────────────┤\n");
    for(i = 0; i < sizeof io_config / sizeof io_config[0]; i++)
    {
        rt_kprintf("│%-2d     │%-24.24s│\n", io_config[i].io_num, io_config[i].func_name);
    }
    rt_kprintf("└───────┴────────────────────────┘\n");
    return 0;
}
MSH_CMD_EXPORT_ALIAS(print_io_config, io, print io config);

int io_config_init(void)
{
    int count = sizeof(io_config) / sizeof(io_config[0]);
    int i;

    sysctl_set_power_mode(SYSCTL_POWER_BANK0, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK2, SYSCTL_POWER_V18);

    for(i = 0; i < count; i++)
    {
        fpioa_set_function(io_config[i].io_num, io_config[i].func);
    }
// test
    //io_mux_init_drv();

/* import from k210_mmd/k210.c io_set_power() */
//#if BOARD_LICHEEDAN == 1
//    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
//    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
//#else
//    sysctl_set_power_mode(SYSCTL_POWER_BANK1, SYSCTL_POWER_V18);
//#endif

#if defined(BSP_USING_CAMERA) || defined(BSP_USING_LCD)
    sysctl_set_spi0_dvp_data(1);
#endif

//#if BOARD_LICHEEDAN == 1
// #if defined(OVER_VOLTAGE)
//    gpiohs_set_drive_mode(CORE_VOLTAGE_GPIONUM, GPIO_DM_INPUT);
//    gpiohs_set_pin(CORE_VOLTAGE_GPIONUM, GPIO_PV_LOW);
//  //overclock settings, edit if you want
//  #define MHz *1000000
//    usleep(100000);
//    sysctl_cpu_set_freq(600 MHz);
//    usleep(100000);
//  #undef MHz
// #endif
//#endif /* BOARD_LICHEEDAN */

}
INIT_BOARD_EXPORT(io_config_init);
