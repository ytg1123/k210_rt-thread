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
#include <rtthread.h>
#include <stdio.h>
#include "gpio.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include "dvp.h"
#include "fpioa.h"
#include "lcd.h"
#include "plic.h"
#include "sysctl.h"
#include "uarths.h"
#include "nt35310.h"
#include "utils.h"
#include "kpu.h"
#include "board_config.h"
#include "i2s.h"
#include "fft.h"
#include "sysctl.h"
#include "dmac.h"

#define OV_WIDTH    320
#define OV_HIGHT    240

uint32_t rx_buf[1024];
uint32_t g_index;
uint32_t g_tx_len;

#define FRAME_LEN           512
#define FFT_N               512U
#define FFT_FORWARD_SHIFT   0x0U
#define SAMPLE_RATE         8000 

int16_t  real[FFT_N];
int16_t  imag[FFT_N];
float    hard_power[FFT_N];
uint64_t fft_out_data[FFT_N / 2];
uint64_t buffer_input[FFT_N];
uint64_t buffer_output[FFT_N];

uint32_t g_lcd_gram[38400] __attribute__((aligned(64)));

static void io_set_power(void)
{
#if BOARD_LICHEEDAN
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
#else
    sysctl_set_power_mode(SYSCTL_POWER_BANK1, SYSCTL_POWER_V18);
#endif
}

uint16_t get_bit1_num(uint32_t data)
{
    uint16_t num;
    for (num = 0; data; num++)
        data &= data - 1;
    return num;
}

static void io_mux_init()
{

#if BOARD_LICHEEDAN
    fpioa_set_function(38, FUNC_GPIOHS2);   /* 26 */
    fpioa_set_function(36, FUNC_SPI0_SS3);  /* 15 */
    fpioa_set_function(39, FUNC_SPI0_SCLK); /* 17 */
    fpioa_set_function(37, FUNC_GPIOHS3);   /* 27 */
#else
    fpioa_set_function(8, FUNC_GPIOHS0 + DCX_GPIONUM);
    fpioa_set_function(6, FUNC_SPI0_SS3);
    fpioa_set_function(7, FUNC_SPI0_SCLK);
    sysctl_set_spi0_dvp_data(1);
#endif

#if defined(BSP_USING_MAIXBIT)
    fpioa_set_function(20, FUNC_I2S0_IN_D0);
    fpioa_set_function(19, FUNC_I2S0_WS);
    fpioa_set_function(18, FUNC_I2S0_SCLK);
#elif defined(BSP_USING_MAIXM1DOCK)
    fpioa_set_function(20, FUNC_I2S0_IN_D0);
    fpioa_set_function(30, FUNC_I2S0_WS);
    fpioa_set_function(32, FUNC_I2S0_SCLK);
#else
    fpioa_set_function(20, FUNC_I2S0_IN_D0);
    fpioa_set_function(30, FUNC_I2S0_WS);
    fpioa_set_function(32, FUNC_I2S0_SCLK);
#endif

}

void update_image( float* hard_power,  float pw_max, uint32_t* pImage, uint32_t color, uint32_t bkg_color  );

void fft_lcd_maix(void *parameter)
{
    int i;
    complex_hard_t data_hard[FFT_N] = {0};

/* typedef struct _fft_data ********************************
 *{
 *    int16_t I1;
 *    int16_t R1;
 *    int16_t I2;
 *    int16_t R2;
 *} fft_data_t; 
************************************************************/
    fft_data_t *output_data;
    fft_data_t *input_data;

    uarths_init();
    io_mux_init();
    io_set_power();
    lcd_init();

#if BOARD_LICHEEDAN == 1
    lcd_set_direction(DIR_XY_LRDU);
#endif

#if defined(BSP_USING_MAIXBIT)
    lcd_set_direction(DIR_XY_LRDU);
#elif defined(BSP_USING_MAIXM1DOCK)
    lcd_set_direction(DIR_XY_LRDU);
#else
    lcd_set_direction(DIR_XY_LRDU);
#endif

    lcd_clear(BLACK);

    i2s_init(I2S_DEVICE_0, I2S_RECEIVER, 0x3);
    i2s_set_sample_rate(I2S_DEVICE_0, 38640 );
    i2s_rx_channel_config(I2S_DEVICE_0, I2S_CHANNEL_0,
                          RESOLUTION_16_BIT, SCLK_CYCLES_32,
                          TRIGGER_LEVEL_4, STANDARD_MODE);


    while (1)
    {
        i2s_receive_data_dma(I2S_DEVICE_0, &rx_buf[0], FRAME_LEN * 2, DMAC_CHANNEL3);

        for ( i = 0; i < FFT_N / 2; ++i)
        {
            input_data = (fft_data_t *)&buffer_input[i];
            input_data->R1 = rx_buf[2*i];   // data_hard[2 * i].real;
            input_data->I1 = 0;             // data_hard[2 * i].imag;
            input_data->R2 = rx_buf[2*i+1]; // data_hard[2 * i + 1].real;
            input_data->I2 = 0;             // data_hard[2 * i + 1].imag;
        }

        fft_complex_uint16_dma(DMAC_CHANNEL0, DMAC_CHANNEL1, FFT_FORWARD_SHIFT, FFT_DIR_FORWARD, buffer_input, FFT_N, buffer_output);

        for ( i = 0; i < FFT_N / 2; i++)
        {
            output_data = (fft_data_t*)&buffer_output[i];
            data_hard[2 * i].imag = output_data->I1 ;
            data_hard[2 * i].real = output_data->R1 ;
            data_hard[2 * i + 1].imag = output_data->I2 ;
            data_hard[2 * i + 1].real = output_data->R2 ;
        }

        float pmax=10;
        for (i = 0; i < FFT_N; i++)
        {
            hard_power[i] = sqrt(data_hard[i].real * data_hard[i].real + data_hard[i].imag * data_hard[i].imag);

            //Convert to dBFS
            hard_power[i] = 20*log(2*hard_power[i]/FFT_N);

            if( hard_power[i]>pmax)
                pmax = hard_power[i];
        }

        update_image( hard_power, 140 /*MAX range dBFS*/, g_lcd_gram,  RED, BLACK  );
        lcd_draw_picture(0, 0, 320, 240, g_lcd_gram);
	rt_thread_delay(RT_TICK_PER_SECOND/25);
    }

}

void update_image( float* hard_power, float pw_max, uint32_t* pImage, uint32_t color, uint32_t bkg_color )
{

    uint32_t bcolor= (bkg_color << 16) | bkg_color;
    uint32_t fcolor= (color << 16) | color;

    int  h[80];

    int i=0;
    int x=0;

    for(int i=0; i<80; ++i)
    {
        h[i]=120*(hard_power[i])/pw_max;

        if( h[i]>120)
            h[i] = 120;
        if( h[i]<0)
            h[i] = 0;
    }

    for( i=0; i<53; ++i)  // 53* 38640/512 => ~4000Hz
    {
        x=i*3;
        for( int y=0; y<120; ++y)
        {
            if( y<h[i+2] )
            {
                pImage[y+x*240]=fcolor;
                pImage[y+120+x*240]=fcolor;
                pImage[y+(x+1)*240]=fcolor;
                pImage[y+120+(x+1)*240]=fcolor;
                pImage[y+(x+2)*240]=bcolor;
                pImage[y+120+(x+2)*240]=bcolor;
            }
            else
            {
                pImage[y+x*240]=bcolor;
                pImage[y+120+x*240]=bcolor;
                pImage[y+(x+1)*240]=bcolor;
                pImage[y+120+(x+1)*240]=bcolor;
                pImage[y+(x+2)*240]=bcolor;
                pImage[y+120+(x+2)*240]=bcolor;

            }
        }
    }

}

static rt_thread_t fftlcd = RT_NULL;

int fft_lcd(void)
{
     int i;

     fftlcd = rt_thread_create( "fftlcd",
                                                fft_lcd_maix,
                                                RT_NULL,
                                                1024 * 32,
                                                10,
                                                3
                                        );
        if(fftlcd != RT_NULL)
        {
                rt_thread_startup(fftlcd);
        }
        //rt_thread_delay(RT_TICK_PER_SECOND);

        return 0;
}

void fft_lcd_stop(void)
{
        if(fftlcd != RT_NULL)
        {
    	rt_thread_detach(fftlcd);
    	rt_thread_delete(fftlcd);
	fftlcd=RT_NULL;
        }
}
MSH_CMD_EXPORT(fft_lcd,start fft_lcd thread);
MSH_CMD_EXPORT(fft_lcd_stop,stop fft_lcd thread);
