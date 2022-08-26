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

#ifndef __DRV_OV5640_H
#define __DRV_OV5640_H
#include "rtthread.h"
#ifdef CAM_USING_OV5640
#include <stdint.h>
//#include "ov5640cfg.h"
//#include "ov5640af.h"

#include <stdint.h>

#define OV5640_ID           0X5640
#define OV5640_ADDR         0X78
#define OV5640_CHIPIDH      0X300A
#define OV5640_CHIPIDL      0X300B
#define OV5640_DEFAULT_SLAVE_ID 0x3c
/* min/typical/max system clock (xclk) frequencies */
#define OV5640_XCLK_MIN  6000000
#define OV5640_XCLK_MAX 54000000


#define DCMI_RESET_LOW()      dvp->cmos_cfg &= ~DVP_CMOS_RESET
#define DCMI_RESET_HIGH()     dvp->cmos_cfg |= DVP_CMOS_RESET
#define DCMI_PWDN_LOW()       dvp->cmos_cfg &= ~DVP_CMOS_POWER_DOWN
#define DCMI_PWDN_HIGH()      dvp->cmos_cfg |= DVP_CMOS_POWER_DOWN

#define XSIZE               320
#define YSIZE               240
#define LCD_GRAM_ADDRESS    0x60020000

#define QQVGA_160_120       0
#define QCIF_176_144        1
#define QVGA_320_240        2
#define WQVGA_400_240       3
#define CIF_352_288         4

#define OV5640_DEFAULT_SLAVE_ID 0x3c

#define OV5640_REG_SYS_RESET02          0x3002
#define OV5640_REG_SYS_CLOCK_ENABLE02   0x3006
#define OV5640_REG_SYS_CTRL0            0x3008
#define OV5640_REG_CHIP_ID              0x300a
#define OV5640_REG_IO_MIPI_CTRL00       0x300e
#define OV5640_REG_PAD_OUTPUT_ENABLE01  0x3017
#define OV5640_REG_PAD_OUTPUT_ENABLE02  0x3018
#define OV5640_REG_PAD_OUTPUT00         0x3019
#define OV5640_REG_SYSTEM_CONTROL1      0x302e
#define OV5640_REG_SC_PLL_CTRL0         0x3034
#define OV5640_REG_SC_PLL_CTRL1         0x3035
#define OV5640_REG_SC_PLL_CTRL2         0x3036
#define OV5640_REG_SC_PLL_CTRL3         0x3037
#define OV5640_REG_SLAVE_ID             0x3100
#define OV5640_REG_SCCB_SYS_CTRL1       0x3103
#define OV5640_REG_SYS_ROOT_DIVIDER     0x3108
#define OV5640_REG_AWB_R_GAIN           0x3400
#define OV5640_REG_AWB_G_GAIN           0x3402
#define OV5640_REG_AWB_B_GAIN           0x3404
#define OV5640_REG_AWB_MANUAL_CTRL      0x3406
#define OV5640_REG_AEC_PK_EXPOSURE_HI   0x3500
#define OV5640_REG_AEC_PK_EXPOSURE_MED  0x3501
#define OV5640_REG_AEC_PK_EXPOSURE_LO   0x3502
#define OV5640_REG_AEC_PK_MANUAL        0x3503
#define OV5640_REG_AEC_PK_REAL_GAIN     0x350a
#define OV5640_REG_AEC_PK_VTS           0x350c
#define OV5640_REG_TIMING_DVPHO         0x3808
#define OV5640_REG_TIMING_DVPVO         0x380a
#define OV5640_REG_TIMING_HTS           0x380c
#define OV5640_REG_TIMING_VTS           0x380e
#define OV5640_REG_TIMING_TC_REG20      0x3820
#define OV5640_REG_TIMING_TC_REG21      0x3821
#define OV5640_REG_AEC_CTRL00           0x3a00
#define OV5640_REG_AEC_B50_STEP         0x3a08
#define OV5640_REG_AEC_B60_STEP         0x3a0a
#define OV5640_REG_AEC_CTRL0D           0x3a0d
#define OV5640_REG_AEC_CTRL0E           0x3a0e
#define OV5640_REG_AEC_CTRL0F           0x3a0f
#define OV5640_REG_AEC_CTRL10           0x3a10
#define OV5640_REG_AEC_CTRL11           0x3a11
#define OV5640_REG_AEC_CTRL1B           0x3a1b
#define OV5640_REG_AEC_CTRL1E           0x3a1e
#define OV5640_REG_AEC_CTRL1F           0x3a1f
#define OV5640_REG_HZ5060_CTRL00        0x3c00
#define OV5640_REG_HZ5060_CTRL01        0x3c01
#define OV5640_REG_SIGMADELTA_CTRL0C    0x3c0c
#define OV5640_REG_FRAME_CTRL01         0x4202
#define OV5640_REG_FORMAT_CONTROL00     0x4300
#define OV5640_REG_POLARITY_CTRL00      0x4740
#define OV5640_REG_MIPI_CTRL00          0x4800
#define OV5640_REG_DEBUG_MODE           0x4814
#define OV5640_REG_ISP_FORMAT_MUX_CTRL  0x501f
#define OV5640_REG_PRE_ISP_TEST_SET1    0x503d
#define OV5640_REG_SDE_CTRL0            0x5580
#define OV5640_REG_SDE_CTRL1            0x5581
#define OV5640_REG_SDE_CTRL3            0x5583
#define OV5640_REG_SDE_CTRL4            0x5584
#define OV5640_REG_SDE_CTRL5            0x5585
#define OV5640_REG_AVG_READOUT          0x56a1

#define MEDIA_BUS_FMT_FIXED                     0x0001

/* RGB - next is        0x101b */
#define MEDIA_BUS_FMT_RGB444_1X12               0x1016
#define MEDIA_BUS_FMT_RGB444_2X8_PADHI_BE       0x1001
#define MEDIA_BUS_FMT_RGB444_2X8_PADHI_LE       0x1002
#define MEDIA_BUS_FMT_RGB555_2X8_PADHI_BE       0x1003
#define MEDIA_BUS_FMT_RGB555_2X8_PADHI_LE       0x1004
#define MEDIA_BUS_FMT_RGB565_1X16               0x1017
#define MEDIA_BUS_FMT_BGR565_2X8_BE             0x1005
#define MEDIA_BUS_FMT_BGR565_2X8_LE             0x1006
#define MEDIA_BUS_FMT_RGB565_2X8_BE             0x1007
#define MEDIA_BUS_FMT_RGB565_2X8_LE             0x1008
#define MEDIA_BUS_FMT_RGB666_1X18               0x1009
#define MEDIA_BUS_FMT_RBG888_1X24               0x100e
#define MEDIA_BUS_FMT_RGB666_1X24_CPADHI        0x1015
#define MEDIA_BUS_FMT_RGB666_1X7X3_SPWG         0x1010
#define MEDIA_BUS_FMT_BGR888_1X24               0x1013
#define MEDIA_BUS_FMT_GBR888_1X24               0x1014
#define MEDIA_BUS_FMT_RGB888_1X24               0x100a
#define MEDIA_BUS_FMT_RGB888_2X12_BE            0x100b
#define MEDIA_BUS_FMT_RGB888_2X12_LE            0x100c
#define MEDIA_BUS_FMT_RGB888_1X7X4_SPWG         0x1011
#define MEDIA_BUS_FMT_RGB888_1X7X4_JEIDA        0x1012
#define MEDIA_BUS_FMT_ARGB8888_1X32             0x100d
#define MEDIA_BUS_FMT_RGB888_1X32_PADHI         0x100f
#define MEDIA_BUS_FMT_RGB101010_1X30            0x1018
#define MEDIA_BUS_FMT_RGB121212_1X36            0x1019
#define MEDIA_BUS_FMT_RGB161616_1X48            0x101a

/* YUV (including grey) - next is       0x202d */
#define MEDIA_BUS_FMT_Y8_1X8                    0x2001
#define MEDIA_BUS_FMT_UV8_1X8                   0x2015
#define MEDIA_BUS_FMT_UYVY8_1_5X8               0x2002
#define MEDIA_BUS_FMT_VYUY8_1_5X8               0x2003
#define MEDIA_BUS_FMT_YUYV8_1_5X8               0x2004
#define MEDIA_BUS_FMT_YVYU8_1_5X8               0x2005
#define MEDIA_BUS_FMT_UYVY8_2X8                 0x2006
#define MEDIA_BUS_FMT_VYUY8_2X8                 0x2007
#define MEDIA_BUS_FMT_YUYV8_2X8                 0x2008
#define MEDIA_BUS_FMT_YVYU8_2X8                 0x2009
#define MEDIA_BUS_FMT_Y10_1X10                  0x200a
#define MEDIA_BUS_FMT_Y10_2X8_PADHI_LE          0x202c
#define MEDIA_BUS_FMT_UYVY10_2X10               0x2018
#define MEDIA_BUS_FMT_VYUY10_2X10               0x2019
#define MEDIA_BUS_FMT_YUYV10_2X10               0x200b
#define MEDIA_BUS_FMT_YVYU10_2X10               0x200c
#define MEDIA_BUS_FMT_Y12_1X12                  0x2013
#define MEDIA_BUS_FMT_UYVY12_2X12               0x201c
#define MEDIA_BUS_FMT_VYUY12_2X12               0x201d
#define MEDIA_BUS_FMT_YUYV12_2X12               0x201e
#define MEDIA_BUS_FMT_YVYU12_2X12               0x201f
#define MEDIA_BUS_FMT_UYVY8_1X16                0x200f
#define MEDIA_BUS_FMT_VYUY8_1X16                0x2010
#define MEDIA_BUS_FMT_YUYV8_1X16                0x2011
#define MEDIA_BUS_FMT_YVYU8_1X16                0x2012
#define MEDIA_BUS_FMT_YDYUYDYV8_1X16            0x2014
#define MEDIA_BUS_FMT_UYVY10_1X20               0x201a
#define MEDIA_BUS_FMT_VYUY10_1X20               0x201b
#define MEDIA_BUS_FMT_YUYV10_1X20               0x200d
#define MEDIA_BUS_FMT_YVYU10_1X20               0x200e
#define MEDIA_BUS_FMT_VUY8_1X24                 0x2024
#define MEDIA_BUS_FMT_YUV8_1X24                 0x2025
#define MEDIA_BUS_FMT_UYYVYY8_0_5X24            0x2026
#define MEDIA_BUS_FMT_UYVY12_1X24               0x2020
#define MEDIA_BUS_FMT_VYUY12_1X24               0x2021
#define MEDIA_BUS_FMT_YUYV12_1X24               0x2022
#define MEDIA_BUS_FMT_YVYU12_1X24               0x2023
#define MEDIA_BUS_FMT_YUV10_1X30                0x2016
#define MEDIA_BUS_FMT_UYYVYY10_0_5X30           0x2027
#define MEDIA_BUS_FMT_AYUV8_1X32                0x2017
#define MEDIA_BUS_FMT_UYYVYY12_0_5X36           0x2028
#define MEDIA_BUS_FMT_YUV12_1X36                0x2029
#define MEDIA_BUS_FMT_YUV16_1X48                0x202a
#define MEDIA_BUS_FMT_UYYVYY16_0_5X48           0x202b
/* Bayer - next is      0x3021 */
#define MEDIA_BUS_FMT_SBGGR8_1X8                0x3001
#define MEDIA_BUS_FMT_SGBRG8_1X8                0x3013
#define MEDIA_BUS_FMT_SGRBG8_1X8                0x3002
#define MEDIA_BUS_FMT_SRGGB8_1X8                0x3014
#define MEDIA_BUS_FMT_SBGGR10_ALAW8_1X8         0x3015
#define MEDIA_BUS_FMT_SGBRG10_ALAW8_1X8         0x3016
#define MEDIA_BUS_FMT_SGRBG10_ALAW8_1X8         0x3017
#define MEDIA_BUS_FMT_SRGGB10_ALAW8_1X8         0x3018
#define MEDIA_BUS_FMT_SBGGR10_DPCM8_1X8         0x300b
#define MEDIA_BUS_FMT_SGBRG10_DPCM8_1X8         0x300c
#define MEDIA_BUS_FMT_SGRBG10_DPCM8_1X8         0x3009
#define MEDIA_BUS_FMT_SRGGB10_DPCM8_1X8         0x300d
#define MEDIA_BUS_FMT_SBGGR10_2X8_PADHI_BE      0x3003
#define MEDIA_BUS_FMT_SBGGR10_2X8_PADHI_LE      0x3004
#define MEDIA_BUS_FMT_SBGGR10_2X8_PADLO_BE      0x3005
#define MEDIA_BUS_FMT_SBGGR10_2X8_PADLO_LE      0x3006
#define MEDIA_BUS_FMT_SBGGR10_1X10              0x3007
#define MEDIA_BUS_FMT_SGBRG10_1X10              0x300e
#define MEDIA_BUS_FMT_SGRBG10_1X10              0x300a
#define MEDIA_BUS_FMT_SRGGB10_1X10              0x300f
#define MEDIA_BUS_FMT_SBGGR12_1X12              0x3008
#define MEDIA_BUS_FMT_SGBRG12_1X12              0x3010
#define MEDIA_BUS_FMT_SGRBG12_1X12              0x3011
#define MEDIA_BUS_FMT_SRGGB12_1X12              0x3012
#define MEDIA_BUS_FMT_SBGGR14_1X14              0x3019
#define MEDIA_BUS_FMT_SGBRG14_1X14              0x301a
#define MEDIA_BUS_FMT_SGRBG14_1X14              0x301b
#define MEDIA_BUS_FMT_SRGGB14_1X14              0x301c
#define MEDIA_BUS_FMT_SBGGR16_1X16              0x301d
#define MEDIA_BUS_FMT_SGBRG16_1X16              0x301e
#define MEDIA_BUS_FMT_SGRBG16_1X16              0x301f
#define MEDIA_BUS_FMT_SRGGB16_1X16              0x3020
/* JPEG compressed formats - next is    0x4002 */
#define MEDIA_BUS_FMT_JPEG_1X8                  0x4001

/* Vendor specific formats - next is    0x5002 */

/* S5C73M3 sensor specific interleaved UYVY and JPEG */
#define MEDIA_BUS_FMT_S5C_UYVY_JPEG_1X8         0x5001

/* HSV - next is        0x6002 */
#define MEDIA_BUS_FMT_AHSV8888_1X32             0x6001

typedef struct framebuffer
{
    rt_sem_t lock;
    uint8_t *pix_buf;
    uint8_t *ai_buf;
}__attribute__((aligned(8))) framebuffer_t;

typedef struct kpuframebuffer
{
    rt_sem_t lock;
    uint8_t *pix_buf0;
    uint8_t *pix_buf1;
    uint8_t *ai_buf;
}__attribute__((aligned(8))) kpuframebuffer_t;

#define jpeg_buf_size       (30*1024)

uint8_t ov5640_init(void);
int ov5640_set_framefmt( uint32_t code);
void ov5640_flash_lamp(uint8_t sw);
void OV5640_Brightness(uint8_t bright);
void OV5640_Exposure(uint8_t exposure);
void OV5640_Light_Mode(uint8_t mode);
void OV5640_Color_Saturation(uint8_t sat);
void OV5640_Brightness(uint8_t bright);
void OV5640_Contrast(uint8_t contrast);
void OV5640_Sharpness(uint8_t sharp);
void OV5640_Special_Effects(uint8_t eft);
uint8_t OV5640_Focus_Init(void);
uint8_t OV5640_Auto_Focus(void);

#endif

#endif
