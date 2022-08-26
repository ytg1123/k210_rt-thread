#include <rtthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

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
#include "region_layer.h"
#include "image_process.h"

#ifndef KPU3FFT_DEBUG
#define KPU3FFT_DEBUG 0
//#define KPU3FFT_DEBUG 1
#endif
#ifndef SKIPFFT
#define SKIPFFT 0
//#define SKIPFFT 1
#endif

#if defined(RT_USING_SPI_MSD) && defined(RT_USING_DFS_ELMFAT)
#include <spi_msd.h>
#include <dfs_fs.h>
#include <dfs_file.h>
#include <dfs_private.h>
#endif

/* ramポインタ関連*/
extern uint16_t g_lcd_ram[QVGA_WIDTH * QVGA_HIGHT ]     ;
extern uint16_t g_fft_inp_ram[QVGA_WIDTH * QVGA_HIGHT ] ;
extern uint16_t g_fft_out_ram[QVGA_WIDTH * QVGA_HIGHT ] ;
extern uint8_t  g_ai_buf[QVGA_WIDTH * QVGA_HIGHT * 3]   ;

/* for fft */
/*
 * 0->95 96->96+320 416->416+96
 */
#define FRAME_LEN           512
#define FFT_N               512U
#define FRAME_1ST           320
#define FRAME_2ND           512
#define FFT_FORWARD_SHIFT   0x0U
#define SAMPLE_RATE         8000

//static int16_t  real[FFT_N];
//static int16_t  imag[FFT_N];
float    kpu3_hard_power[FFT_N];
uint64_t kpu3_buffer_input[FFT_N];
uint64_t kpu3_buffer_output[FFT_N];
uint16_t kpu3_fft_out_data[FFT_N];
uint16_t kpu3_fft_inp_data[FFT_N];

uint16_t fft_rgb565_conv(uint16_t indata)  {
    int16_t r,g,b;
    r=(indata >> 11) & 0x1f;r=r*r;
    g=(indata >> 5)  & 0x3f;g=g*g;
    b=(indata     )  & 0x1f;b=b*b;
    return r+g+b;
}

void fft_oneframe(void)  {
    int i=0,j=0;
    complex_hard_t data_hard[FFT_N] = {0};
    fft_data_t *output_data;
    fft_data_t *input_data;
    float pow,hard_p;

/* for fft */

     /* fft run*/

     for (j=0;j<QVGA_HIGHT;j++)
     {
#if 1
        for ( i = 0; i < FRAME_1ST; i++) { 
              kpu3_fft_inp_data[i] = g_fft_inp_ram[QVGA_WIDTH*j + i];
        }
#endif

        for ( i = 0; i < FFT_N / 2; ++i)
        {
            input_data = (fft_data_t *)&kpu3_buffer_input[i];
            input_data->R1 = kpu3_fft_inp_data[2*i];                         // data_hard[2 * i].real;
            input_data->I1 = 0;                                              // data_hard[2 * i].imag;
            input_data->R2 = kpu3_fft_inp_data[2*i+1];                       // data_hard[2 * i + 1].real;
            input_data->I2 = 0;                                              // data_hard[2 * i + 1].imag;
        }
        fft_complex_uint16_dma(DMAC_CHANNEL0, DMAC_CHANNEL1, FFT_FORWARD_SHIFT, FFT_DIR_FORWARD, kpu3_buffer_input, FFT_N, kpu3_buffer_output);
        for ( i = 0; i < FFT_N / 2; i++)
        {
            output_data = (fft_data_t*)&kpu3_buffer_output[i];
            data_hard[2 * i].imag = output_data->I1 ;
            data_hard[2 * i].real = output_data->R1 ;
            data_hard[2 * i + 1].imag = output_data->I2 ;
            data_hard[2 * i + 1].real = output_data->R2 ;
        }

        //for (i = 0; i < FFT_N; i++)
        for (i = 0; i < QVGA_WIDTH ; i++)
        {
            hard_p = sqrt(data_hard[i].real * data_hard[i].real + data_hard[i].imag * data_hard[i].imag);
            hard_p = 20*log(2*hard_p/FFT_N);
            kpu3_fft_out_data[i] = hard_p;
            g_fft_out_ram[QVGA_WIDTH*j + i]=  hard_p; 
        }


     } /* for end */

}

