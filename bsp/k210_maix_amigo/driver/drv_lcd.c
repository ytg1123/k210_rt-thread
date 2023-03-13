/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-12     ZYH          first version
 */


#include <rtthread.h>

#ifdef BSP_USING_LCD

#include <rtdevice.h>
#include "drv_lcd.h"
#include <board.h>
#include <gpiohs.h>
#include <fpioa.h>
#include "gpio.h"
#include <spi.h>
#include <drv_io_config.h>
#include <rthw.h>

#define DBG_TAG  "LCD"
#define DBG_LVL  DBG_WARNING
#include <rtdbg.h>

#include "nt35310.h"
#include "lcd.h"

#define NO_OPERATION            0x00
#define SOFTWARE_RESET          0x01
#define READ_ID                 0x04
#define READ_STATUS             0x09
#define READ_POWER_MODE         0x0A
#define READ_MADCTL             0x0B
#define READ_PIXEL_FORMAT       0x0C
#define READ_IMAGE_FORMAT       0x0D
#define READ_SIGNAL_MODE        0x0E
#define READ_SELT_DIAG_RESULT   0x0F
#define SLEEP_ON                0x10
#define SLEEP_OFF               0x11
#define PARTIAL_DISPALY_ON      0x12
#define NORMAL_DISPALY_ON       0x13
#define INVERSION_DISPALY_OFF   0x20
#define INVERSION_DISPALY_ON    0x21
#define GAMMA_SET               0x26
#define DISPALY_OFF             0x28
#define DISPALY_ON              0x29
#define HORIZONTAL_ADDRESS_SET  0x2A
#define VERTICAL_ADDRESS_SET    0x2B
#define MEMORY_WRITE            0x2C
#define COLOR_SET               0x2D
#define MEMORY_READ             0x2E
#define PARTIAL_AREA            0x30
#define VERTICAL_SCROL_DEFINE   0x33
#define TEAR_EFFECT_LINE_OFF    0x34
#define TEAR_EFFECT_LINE_ON     0x35
#define MEMORY_ACCESS_CTL       0x36
#define VERTICAL_SCROL_S_ADD    0x37
#define IDLE_MODE_OFF           0x38
#define IDLE_MODE_ON            0x39
#define PIXEL_FORMAT_SET        0x3A
#define WRITE_MEMORY_CONTINUE   0x3C
#define READ_MEMORY_CONTINUE    0x3E
#define SET_TEAR_SCANLINE       0x44
#define GET_SCANLINE            0x45
#define WRITE_BRIGHTNESS        0x51
#define READ_BRIGHTNESS         0x52
#define WRITE_CTRL_DISPALY      0x53
#define READ_CTRL_DISPALY       0x54
#define WRITE_BRIGHTNESS_CTL    0x55
#define READ_BRIGHTNESS_CTL     0x56
#define WRITE_MIN_BRIGHTNESS    0x5E
#define READ_MIN_BRIGHTNESS     0x5F
#define READ_ID1                0xDA
#define READ_ID2                0xDB
#define READ_ID3                0xDC
#define RGB_IF_SIGNAL_CTL       0xB0
#define NORMAL_FRAME_CTL        0xB1
#define IDLE_FRAME_CTL          0xB2
#define PARTIAL_FRAME_CTL       0xB3
#define INVERSION_CTL           0xB4
#define BLANK_PORCH_CTL         0xB5
#define DISPALY_FUNCTION_CTL    0xB6
#define ENTRY_MODE_SET          0xB7
#define BACKLIGHT_CTL1          0xB8
#define BACKLIGHT_CTL2          0xB9
#define BACKLIGHT_CTL3          0xBA
#define BACKLIGHT_CTL4          0xBB
#define BACKLIGHT_CTL5          0xBC
#define BACKLIGHT_CTL7          0xBE
#define BACKLIGHT_CTL8          0xBF
#define POWER_CTL1              0xC0
#define POWER_CTL2              0xC1
#define VCOM_CTL1               0xC5
#define VCOM_CTL2               0xC7
#define NV_MEMORY_WRITE         0xD0
#define NV_MEMORY_PROTECT_KEY   0xD1
#define NV_MEMORY_STATUS_READ   0xD2
#define READ_ID4                0xD3
#define POSITIVE_GAMMA_CORRECT  0xE0
#define NEGATIVE_GAMMA_CORRECT  0xE1
#define DIGITAL_GAMMA_CTL1      0xE2
#define DIGITAL_GAMMA_CTL2      0xE3
#define INTERFACE_CTL           0xF6

#define LCD_SPI_CHANNEL         0
#define LCD_SPI_CHIP_SELECT     SPI_SLAVE_SELECT

#if LCD_SWAP_COLOR_BYTES
rt_uint16_t const gray2rgb565[64]=
{
0x0000, 0x0020, 0x0841, 0x0861, 0x1082, 0x10a2, 0x18c3, 0x18e3,
0x2104, 0x2124, 0x2945, 0x2965, 0x3186, 0x31a6, 0x39c7, 0x39e7,
0x4208, 0x4228, 0x4a49, 0x4a69, 0x528a, 0x52aa, 0x5acb, 0x5aeb,
0x630c, 0x632c, 0x6b4d, 0x6b6d, 0x738e, 0x73ae, 0x7bcf, 0x7bef,
0x8410, 0x8430, 0x8c51, 0x8c71, 0x9492, 0x94b2, 0x9cd3, 0x9cf3,
0xa514, 0xa534, 0xad55, 0xad75, 0xb596, 0xb5b6, 0xbdd7, 0xbdf7,
0xc618, 0xc638, 0xce59, 0xce79, 0xd69a, 0xd6ba, 0xdedb, 0xdefb,
0xe71c, 0xe73c, 0xef5d, 0xef7d, 0xf79e, 0xf7be, 0xffdf, 0xffff,
};
#else
rt_uint16_t const gray2rgb565[64] = {
0x0000, 0x2000, 0x4108, 0x6108, 0x8210, 0xa210, 0xc318, 0xe318,
0x0421, 0x2421, 0x4529, 0x6529, 0x8631, 0xa631, 0xc739, 0xe739,
0x0842, 0x2842, 0x494a, 0x694a, 0x8a52, 0xaa52, 0xcb5a, 0xeb5a,
0x0c63, 0x2c63, 0x4d6b, 0x6d6b, 0x8e73, 0xae73, 0xcf7b, 0xef7b,
0x1084, 0x3084, 0x518c, 0x718c, 0x9294, 0xb294, 0xd39c, 0xf39c,
0x14a5, 0x34a5, 0x55ad, 0x75ad, 0x96b5, 0xb6b5, 0xd7bd, 0xf7bd,
0x18c6, 0x38c6, 0x59ce, 0x79ce, 0x9ad6, 0xbad6, 0xdbde, 0xfbde,
0x1ce7, 0x3ce7, 0x5def, 0x7def, 0x9ef7, 0xbef7, 0xdfff, 0xffff
};
#endif


typedef struct lcd_8080_device
{
    struct rt_device parent;
    struct rt_device_graphic_info lcd_info;
    int spi_channel;
    int cs;
    int dc_pin;
    int dma_channel;
} * lcd_8080_device_t;

lcd_8080_device_t lcd_backup = NULL;

static void  init_dcx(void)
{
    gpiohs_set_drive_mode(DCX_GPIONUM, GPIO_DM_OUTPUT);
    gpiohs_set_pin(DCX_GPIONUM, GPIO_PV_HIGH);

}

static void set_dcx_control(void)
{
    gpiohs_set_pin(DCX_GPIONUM, GPIO_PV_LOW);

}

static void set_dcx_data(void)
{
    gpiohs_set_pin(DCX_GPIONUM, GPIO_PV_HIGH);
}

#if BOARD_LICHEEDAN
static void init_rst(void)
{
    gpiohs_set_drive_mode(RST_GPIONUM, GPIO_DM_OUTPUT);
    gpiohs_set_pin(RST_GPIONUM, GPIO_PV_LOW);
    rt_thread_mdelay(100);
    gpiohs_set_pin(RST_GPIONUM, GPIO_PV_HIGH);
    rt_thread_mdelay(100);

}
#endif


static void drv_lcd_cmd(lcd_8080_device_t lcd, rt_uint8_t cmd)
{
    set_dcx_control();
    spi_init(SPI_CHANNEL, SPI_WORK_MODE_0, SPI_FF_OCTAL, 8, 0);
    spi_init_non_standard(SPI_CHANNEL, 
                                     8/*instrction length*/, 
                                     0/*address length*/, 
                                     0/*wait cycles*/,
                          SPI_AITM_AS_FRAME_FORMAT/*spi address trans mode*/);
    spi_send_data_normal_dma(DMAC_CHANNEL0, 
                             SPI_CHANNEL, 
                             SPI_SLAVE_SELECT, 
                             (uint8_t *)(&cmd), 1,SPI_TRANS_CHAR);

}

static void drv_lcd_data_byte(lcd_8080_device_t lcd, rt_uint8_t *data_buf, rt_uint32_t length)
{
    gpiohs_set_pin(lcd->dc_pin, GPIO_PV_HIGH);
    set_dcx_data();
    spi_init(SPI_CHANNEL, SPI_WORK_MODE_0, SPI_FF_OCTAL, 8, 0);
    spi_init_non_standard(SPI_CHANNEL, 0 /*instrction length*/, 8 /*address length*/, 0 /*wait cycles*/,
                          SPI_AITM_AS_FRAME_FORMAT /*spi address trans mode*/);
    spi_send_data_normal_dma(DMAC_CHANNEL0, 
                             SPI_CHANNEL, 
                             SPI_SLAVE_SELECT, 
                             data_buf, length, SPI_TRANS_CHAR);
}

static void drv_lcd_data_half_word(lcd_8080_device_t lcd, rt_uint16_t *data_buf, rt_uint32_t length)
{
    gpiohs_set_pin(lcd->dc_pin, GPIO_PV_HIGH);
    spi_init(lcd->spi_channel, SPI_WORK_MODE_0, SPI_FF_OCTAL, 16, 0);
    spi_init_non_standard(lcd->spi_channel, 0 /*instrction length*/, 16 /*address length*/, 0 /*wait cycles*/,
                          SPI_AITM_AS_FRAME_FORMAT /*spi address trans mode*/);
    spi_send_data_normal_dma(lcd->dma_channel, lcd->spi_channel, lcd->cs, data_buf, length, SPI_TRANS_SHORT);
}

static void drv_lcd_data_word(lcd_8080_device_t lcd, rt_uint32_t *data_buf, rt_uint32_t length)
{
    gpiohs_set_pin(lcd->dc_pin, GPIO_PV_HIGH);
    spi_init(lcd->spi_channel, SPI_WORK_MODE_0, SPI_FF_OCTAL, 32, 0);
    spi_init_non_standard(lcd->spi_channel, 0 /*instrction length*/, 32 /*address length*/, 0 /*wait cycles*/,
                          SPI_AITM_AS_FRAME_FORMAT /*spi address trans mode*/);
    spi_send_data_normal_dma(lcd->dma_channel, lcd->spi_channel, lcd->cs, data_buf, length, SPI_TRANS_INT);
}

void tft_hard_init(void)
{
    init_dcx();
    spi_init(SPI_CHANNEL, SPI_WORK_MODE_0, SPI_FF_OCTAL, 8, 0);
#if BOARD_LICHEEDAN == 1
    init_rst();
    spi_set_clk_rate(SPI_CHANNEL, 20000000);
#else
    spi_set_clk_rate(SPI_CHANNEL, 25000000);
#endif
}

void tft_write_command(uint8_t cmd)
{
    set_dcx_control();
    spi_init(SPI_CHANNEL, SPI_WORK_MODE_0, SPI_FF_OCTAL, 8, 0);
    spi_init_non_standard(SPI_CHANNEL, 
                          8 /*instrction length*/, 
                          0 /*address length*/, 0 /*wait cycles*/,
                          SPI_AITM_AS_FRAME_FORMAT /*spi address trans mode*/);
    spi_send_data_normal_dma(DMAC_CHANNEL0, SPI_CHANNEL, 
                             SPI_SLAVE_SELECT, &cmd, 1, SPI_TRANS_CHAR);
}
void tft_write_byte(uint8_t *data_buf, uint32_t length)
{
    set_dcx_data();
    spi_init(SPI_CHANNEL, SPI_WORK_MODE_0, SPI_FF_OCTAL, 8, 0);
    spi_init_non_standard(SPI_CHANNEL, 
                          8 /*instrction length*/, 
                          0 /*address length*/, 
                          0 /*wait cycles*/,
                          SPI_AITM_AS_FRAME_FORMAT /*spi address trans mode*/);
    spi_send_data_normal_dma(DMAC_CHANNEL0, SPI_CHANNEL, SPI_SLAVE_SELECT, 
                             data_buf, length, SPI_TRANS_CHAR);
}
void tft_write_half(uint16_t *data_buf, uint32_t length)
{
    set_dcx_data();
    spi_init(SPI_CHANNEL, SPI_WORK_MODE_0, SPI_FF_OCTAL, 16, 0);
    spi_init_non_standard(SPI_CHANNEL, 16/*instrction length*/, 
                                        0/*address length*/, 
                                        0/*wait cycles*/,
                          SPI_AITM_AS_FRAME_FORMAT/*spi address trans mode*/);
    spi_send_data_normal_dma(DMAC_CHANNEL0, SPI_CHANNEL, SPI_SLAVE_SELECT,
                             data_buf, length, SPI_TRANS_SHORT);

}
void tft_write_word(uint32_t *data_buf, uint32_t length, uint32_t flag)
{
    set_dcx_data();
    spi_init(SPI_CHANNEL, SPI_WORK_MODE_0, SPI_FF_OCTAL, 32, 0);
    spi_init_non_standard(SPI_CHANNEL, 0/*instrction length*/, 
                                      32/*address length*/,
                                       0/*wait cycles*/,
                          SPI_AITM_AS_FRAME_FORMAT/*spi address trans mode*/);
    spi_send_data_normal_dma(DMAC_CHANNEL0, SPI_CHANNEL, SPI_SLAVE_SELECT,
                             data_buf, length, SPI_TRANS_INT);


}
void tft_fill_data(uint32_t *data_buf, uint32_t length)
{
    set_dcx_data();
    spi_init(SPI_CHANNEL, SPI_WORK_MODE_0, SPI_FF_OCTAL, 32, 0);
    spi_init_non_standard(SPI_CHANNEL, 0/*instrction length*/,
                                      32/*address length*/,
                                       0/*wait cycles*/,
                          SPI_AITM_AS_FRAME_FORMAT/*spi address trans mode*/);
    spi_fill_data_dma(DMAC_CHANNEL0, SPI_CHANNEL, SPI_SLAVE_SELECT,
                      data_buf, length);

}

static void drv_lcd_hw_init(lcd_8080_device_t lcd)
{

    init_dcx();
    spi_init(SPI_CHANNEL, SPI_WORK_MODE_0, SPI_FF_OCTAL, 8, 0);
#if BOARD_LICHEEDAN
    init_rst();
    spi_set_clk_rate(SPI_CHANNEL, 20000000);
#else
    spi_set_clk_rate(SPI_CHANNEL, 25000000);
#endif
}

static void drv_lcd_set_direction(lcd_8080_device_t lcd, lcd_dir_t dir)
{
    if (dir & DIR_XY_MASK)
    {
        lcd->lcd_info.width = BSP_LCD_Y_MAX;
        lcd->lcd_info.height = BSP_LCD_X_MAX;
    }
    else
    {
        lcd->lcd_info.width = BSP_LCD_X_MAX;
        lcd->lcd_info.height = BSP_LCD_Y_MAX;
    }

    drv_lcd_cmd(lcd, MEMORY_ACCESS_CTL);
    drv_lcd_data_byte(lcd, (rt_uint8_t *)&dir, 1);
}

static void drv_lcd_set_area(lcd_8080_device_t lcd, rt_uint16_t x1, rt_uint16_t y1, rt_uint16_t x2, rt_uint16_t y2)
{
    rt_uint8_t data[4] = {0};

    data[0] = (rt_uint8_t)(x1 >> 8);
    data[1] = (rt_uint8_t)(x1);
    data[2] = (rt_uint8_t)(x2 >> 8);
    data[3] = (rt_uint8_t)(x2);
    drv_lcd_cmd(lcd, HORIZONTAL_ADDRESS_SET);
    drv_lcd_data_byte(lcd, data, 4);

    data[0] = (rt_uint8_t)(y1 >> 8);
    data[1] = (rt_uint8_t)(y1);
    data[2] = (rt_uint8_t)(y2 >> 8);
    data[3] = (rt_uint8_t)(y2);
    drv_lcd_cmd(lcd, VERTICAL_ADDRESS_SET);
    drv_lcd_data_byte(lcd, data, 4);

    drv_lcd_cmd(lcd, MEMORY_WRITE);
}

static void drv_lcd_set_pixel(lcd_8080_device_t lcd, uint16_t x, uint16_t y, uint16_t color)
{
    drv_lcd_set_area(lcd, x, y, x, y);
    drv_lcd_data_half_word(lcd, &color, 1);
}

static void drv_lcd_clear(lcd_8080_device_t lcd, uint16_t color)
{
    uint32_t data = ((uint32_t)color << 16) | (uint32_t)color;

    drv_lcd_set_area(lcd, 0, 0, lcd->lcd_info.width - 1, lcd->lcd_info.height - 1);
    gpiohs_set_pin(lcd->dc_pin, GPIO_PV_HIGH);
    spi_init(SPI_CHANNEL, SPI_WORK_MODE_0, SPI_FF_OCTAL, 32, 0);
    spi_init_non_standard(SPI_CHANNEL, 0 /*instrction length*/, 32 /*address length*/, 0 /*wait cycles*/,
                          SPI_AITM_AS_FRAME_FORMAT /*spi address trans mode*/);
    spi_fill_data_dma(DMAC_CHANNEL0, SPI_CHANNEL,SPI_SLAVE_SELECT , 
                          (const uint32_t *)&data, 
                          lcd->lcd_info.width * lcd->lcd_info.height / 2);
}

static void rt_bitblt(rt_uint16_t * dest, int dest_segment, int dest_common, int dest_x, int dest_y, int width, int height,
        rt_uint16_t *src, int src_segment, int src_common, int src_x, int src_y) 
{
    int sx0, sx1, sy0, sy1;
    int dx0, dx1, dy0, dy1;
    rt_uint16_t *buff_src;
    rt_uint16_t *buff_dest;
    int x, y;

    if (width <= 0) {
        return;
    }
    if (height <= 0) {
        return;
    }

    sx0 = src_x;
    sy0 = src_y;
    sx1 = sx0 + width - 1;
    sy1 = sy0 + height - 1;
    dx0 = dest_x;
    dy0 = dest_y;
    dx1 = dx0 + width - 1;
    dy1 = dy0 + height - 1;

    if (sx0 < 0) {
        dx0 -= sx0;
        sx0 = 0;
    }
    if (sy0 < 0) {
        dy0 -= sy0;
        sy0 = 0;
    }
    if (sx1 >= src_segment) {
        dx1 -= (sx1 - src_segment + 1);
        sx1 = src_segment - 1;
    }
    if (sy1 >= src_common) {
        dy1 -= (sy1 - src_common + 1);
        sy1 = src_common - 1;
    }

    if (dx0 < 0) {
        sx0 -= dx0;
        dx0 = 0;
    }
    if (dy0 < 0) {
        sy0 -= dy0;
        dy0 = 0;
    }
    if (dx1 >= dest_segment) {
        sx1 -= (dx1 - dest_segment + 1);
        dx1 = dest_segment - 1;
    }
    if (dy1 >= dest_common) {
        sy1 -= (dy1 - dest_common + 1);
        dy1 = dest_common - 1;
    }

    if (sx1 < 0 || sx0 >= src_segment) {
        return;
    }
    if (sy1 < 0 || sy0 >= src_common) {
        return;
    }
    if (dx1 < 0 || dx0 >= dest_segment) {
        return;
    }
    if (dy1 < 0 || dy0 >= dest_common) {
        return;
    }

    if ((rt_ubase_t)dest < (rt_ubase_t)src) {
        buff_src = src + (sy0 * src_segment) + sx0;
        buff_dest = dest + (dy0 * dest_segment) + dx0;
        for (y = sy0; y <= sy1; y++) {
            src = buff_src;
            dest = buff_dest;
            for (x = sx0; x <= sx1; x++) {
                *dest++ = *src++;
            }
            buff_src += src_segment;
            buff_dest += dest_segment;
        }
    } else {
        buff_src = src + (sy1 * src_segment) + sx1;
        buff_dest = dest + (dy1 * dest_segment) + dx1;
        for (y = sy1; y >= sy0; y--) {
            src = buff_src;
            dest = buff_dest;
            for (x = sx1; x >= sx0; x--) {
                *dest-- = *src--;
            }
            buff_src -= src_segment;
            buff_dest -= dest_segment;
        }
    }
}

static void drv_lcd_rect_update(lcd_8080_device_t lcd, uint16_t x1, uint16_t y1, uint16_t width, uint16_t height)
{
    static rt_uint16_t * rect_buffer = RT_NULL;
    if(!rect_buffer)
    {
        rect_buffer = rt_malloc_align(lcd->lcd_info.height * lcd->lcd_info.width * (lcd->lcd_info.bits_per_pixel / 8), 64);
        if(!rect_buffer)
        {
            return;
        }
    }
    if(x1 == 0 && y1 == 0 && width == lcd->lcd_info.width && height == lcd->lcd_info.height)
    {
        drv_lcd_set_area(lcd, x1, y1, x1 + width - 1, y1 + height - 1);
        drv_lcd_data_half_word(lcd, (rt_uint16_t *)lcd->lcd_info.framebuffer, width * height );
    }
    else
    {
        rt_bitblt(rect_buffer, width, height, 0, 0, width, height,
        (rt_uint16_t *)lcd->lcd_info.framebuffer, lcd->lcd_info.width, lcd->lcd_info.height, x1, y1);
        drv_lcd_set_area(lcd, x1, y1, x1 + width - 1, y1 + height - 1);
        drv_lcd_data_half_word(lcd, (rt_uint16_t *)rect_buffer, width * height );
    }
}

static rt_err_t drv_lcd_init(rt_device_t dev)
{
    rt_err_t ret = RT_EOK;
    lcd_8080_device_t lcd = (lcd_8080_device_t)dev;
    rt_uint8_t data = 0;
    uint32_t i;
    // lcd local copy
    if( lcd != NULL) lcd_backup = lcd;

    if(!lcd)
    {
        return RT_ERROR;
    }
    drv_lcd_hw_init(lcd);
    /* reset LCD */
    drv_lcd_cmd(lcd, SOFTWARE_RESET);
    rt_thread_mdelay(100);

    /* Enter normal status */
    drv_lcd_cmd(lcd, SLEEP_OFF);
    rt_thread_mdelay(100);

    /* pixel format rgb565 */
    drv_lcd_cmd(lcd, PIXEL_FORMAT_SET);
    data = 0x55;
    drv_lcd_data_byte(lcd, &data, 1);
    rt_thread_mdelay(10);
    /* set direction */
    drv_lcd_set_direction(lcd, DIR_YX_RLDU);

    lcd->lcd_info.framebuffer = rt_malloc_align(lcd->lcd_info.height * lcd->lcd_info.width * (lcd->lcd_info.bits_per_pixel / 8), 64);
    RT_ASSERT(lcd->lcd_info.framebuffer);
    for(i=0;i<lcd->lcd_info.height * lcd->lcd_info.width * (lcd->lcd_info.bits_per_pixel / 8);i++)
        lcd->lcd_info.framebuffer[i] = 0xff;
    /*display on*/
    drv_lcd_cmd(lcd,NORMAL_DISPALY_ON);
    rt_thread_mdelay(10);
    drv_lcd_cmd(lcd, DISPALY_ON);
    rt_thread_mdelay(10);
    /* set to black */
    drv_lcd_clear(lcd, 0x0000);
    return ret;
}

static rt_err_t drv_lcd_open(rt_device_t dev, rt_uint16_t oflag)
{

    /* Not need */

    return RT_EOK;
}

static rt_err_t drv_lcd_close(rt_device_t dev)
{

    /* Not need */

    return RT_EOK;
}

static rt_size_t drv_lcd_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    
    /* Not need */

    return 0;
}

static rt_size_t drv_lcd_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    
    /* Not need */

    return 0;
}

static rt_err_t drv_lcd_control(rt_device_t dev, int cmd, void *args)
{
    rt_err_t ret = RT_EOK;
    lcd_8080_device_t lcd = (lcd_8080_device_t)dev;
    rt_base_t level;
    struct rt_device_rect_info* rect_info = (struct rt_device_rect_info*)args;

    RT_ASSERT(dev != RT_NULL);

    switch (cmd)
    {
    case RTGRAPHIC_CTRL_RECT_UPDATE:
        if(!rect_info)
        {
            LOG_E("RTGRAPHIC_CTRL_RECT_UPDATE error args");
            return -RT_ERROR;
        }
        drv_lcd_rect_update(lcd, rect_info->x, rect_info->y, rect_info->width, rect_info->height);
        break;

    case RTGRAPHIC_CTRL_POWERON:
        /* Todo: power on */
        ret = -RT_ENOSYS;
        break;

    case RTGRAPHIC_CTRL_POWEROFF:
        /* Todo: power off */
        ret = -RT_ENOSYS;
        break;

    case RTGRAPHIC_CTRL_GET_INFO:
        *(struct rt_device_graphic_info *)args = lcd->lcd_info;
        break;

    case RTGRAPHIC_CTRL_SET_MODE:
        ret = -RT_ENOSYS;
        break;
    case RTGRAPHIC_CTRL_GET_EXT:
        ret = -RT_ENOSYS;
        break;
    default:
        LOG_E("drv_lcd_control cmd: %d", cmd);
        break;
    }

    return ret;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops drv_lcd_ops =
{
    drv_lcd_init,
    drv_lcd_open,
    drv_lcd_close,
    drv_lcd_read,
    drv_lcd_write,
    drv_lcd_control
};
#endif

int rt_hw_lcd_init(void)
{
    rt_err_t ret = RT_EOK;
    lcd_8080_device_t lcd_dev = (lcd_8080_device_t)rt_malloc(sizeof(struct lcd_8080_device));
    if(!lcd_dev)
    {
        return -1;
    }

    lcd_dev->cs                         = SPI_SLAVE_SELECT;
    lcd_dev->dc_pin                     = DCX_GPIONUM;
    lcd_dev->dma_channel                = DMAC_CHANNEL0;
    lcd_dev->spi_channel                = SPI_CHANNEL;
    lcd_dev->lcd_info.bits_per_pixel    = 16;
    lcd_dev->lcd_info.pixel_format      = RTGRAPHIC_PIXEL_FORMAT_RGB565;

    lcd_dev->parent.type        = RT_Device_Class_Graphic;
    lcd_dev->parent.rx_indicate = RT_NULL;
    lcd_dev->parent.tx_complete = RT_NULL;

#ifdef RT_USING_DEVICE_OPS
    lcd_dev->parent.ops        = &drv_lcd_ops;
#else
    lcd_dev->parent.init    = drv_lcd_init;
    lcd_dev->parent.open    = drv_lcd_open;
    lcd_dev->parent.close   = drv_lcd_close;
    lcd_dev->parent.read    = drv_lcd_read;
    lcd_dev->parent.write   = drv_lcd_write;
    lcd_dev->parent.control = drv_lcd_control;
#endif

    lcd_dev->parent.user_data = RT_NULL;

    ret = rt_device_register(&lcd_dev->parent, "lcd", RT_DEVICE_FLAG_RDWR);

    return ret;
}
INIT_DEVICE_EXPORT(rt_hw_lcd_init);


static uint16_t str2int16(const char* ptr)
{
    uint16_t ret = 0,i;
    for(i = 0;i < 4;i++)
    {
        uint8_t temp = ptr[i];
        if(temp > 96)
            temp = temp - 96 + 9;
        else if(temp > 64)
            temp = temp - 64 + 9;
        else
            temp -= 0x30;
        ret <<= 4;
        ret |= temp;
    }   
    return ret;
}

//lcd demo. Delete it if it is unneceserry.
static void lcd(int argc, char**argv)
{

    static rt_device_t lcd = RT_NULL;
    if (argc < 2)
    {
        rt_kprintf("Please input cmd\n");
        return;
    }
    if (!rt_strcmp(argv[1], "find"))
    {
        lcd = rt_device_find("lcd");
        if(lcd != RT_NULL)
            rt_kprintf("device found!\n");
    }else if(!rt_strcmp(argv[1], "init"))
    {
        rt_device_init(lcd);
    }else if(!rt_strcmp(argv[1], "clr"))
    {
        uint16_t color = 0;
        color = str2int16(argv[2]);
        rt_kprintf("set color:0x%04x\n",color);
        drv_lcd_clear((lcd_8080_device_t)lcd,color);
    }else if(!rt_strcmp(argv[1],"p"))
    {
        uint16_t x,y,color=0;
        x=atoi(argv[2]);
        y=atoi(argv[3]);
        color = str2int16(argv[4]);
        rt_kprintf("set point (%d,%d) at 0x%04x\n",x,y,color);
        drv_lcd_set_pixel((lcd_8080_device_t)lcd,x,y,color);
        drv_lcd_set_pixel((lcd_8080_device_t)lcd,x+1,y,color);
        drv_lcd_set_pixel((lcd_8080_device_t)lcd,x,y+1,color);
        drv_lcd_set_pixel((lcd_8080_device_t)lcd,x+1,y+1,color);
    }else if(!rt_strcmp(argv[1],"red"))
    {
        rt_kprintf("set color:red\n");
        drv_lcd_clear((lcd_8080_device_t)lcd,0xF800);        
    }else if(!rt_strcmp(argv[1],"green"))
    {
        rt_kprintf("set color:green\n");
        drv_lcd_clear((lcd_8080_device_t)lcd,0x07E0);           
    }else if(!rt_strcmp(argv[1],"blue"))
    {
        rt_kprintf("set color:blue\n");
        drv_lcd_clear((lcd_8080_device_t)lcd,0x001F);           
    }else if (!rt_strcmp(argv[1],"ctl"))
    {
        lcd_8080_device_t lcd_dev = (lcd_8080_device_t)lcd;
        uint32_t i,j;
        struct rt_device_rect_info rect;
        rect.height = 236;
        rect.width = 316;
        rect.x = 2;
        rect.y = 2;
        for(i=0;i<lcd_dev->lcd_info.height * lcd_dev->lcd_info.width * (lcd_dev->lcd_info.bits_per_pixel / 8);i++)
            lcd_dev->lcd_info.framebuffer[i] = 0x00;
        rt_device_control(lcd,RTGRAPHIC_CTRL_RECT_UPDATE,&rect);
        rt_kprintf("rect update!\n");
    }else   
    {
        rt_kprintf("unknown cmd!\n");
    }
    
}
MSH_CMD_EXPORT(lcd, lcd_test);

#endif
