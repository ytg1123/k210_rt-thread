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


#define QVGA_WIDTH 320
#define QVGA_HIGHT 240

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

/* for fft */
/*
 * 0->95 96->96+320 416->416+96
 */
#define FRAME_LEN           512
#define FFT_N               512U
#define FRAME_1ST           96
#define FRAME_2ND           416
#define FFT_FORWARD_SHIFT   0x0U
#define SAMPLE_RATE         8000
//static int16_t  real[FFT_N];
//static int16_t  imag[FFT_N];
static float    hard_power[FFT_N];
static uint64_t buffer_input[FFT_N];
static uint64_t buffer_output[FFT_N];
static int16_t fft_out_data[FFT_N];
static int16_t fft_inp_data[FFT_N];

int16_t fft_rgb565_conv(int16_t indata)  {
    int16_t r,g,b;
    r=(indata >> 11) & 0x1f;r=r*r;
    g=(indata >> 5)  & 0x3f;g=g*g;
    b=(indata     )  & 0x1f;b=b*b;
    return r+g+b;
}

void fft_cpy_from_data(int16_t *ptr)  {
    int i=0;
    /* fft run*/
    for ( i = 0; i < FRAME_1ST; i++) { fft_inp_data[i]=0;fft_out_data[i]=0;}
    for ( i = FRAME_1ST; i < FRAME_2ND; i++) { fft_inp_data[i]=ptr[i-FRAME_1ST];fft_out_data[i]=ptr[i-FRAME_1ST];}
    for ( i = FRAME_2ND; i < FRAME_LEN; i++) { fft_inp_data[i]=0;fft_out_data[i]=0;}

    for (i = FRAME_1ST; i < FRAME_2ND; i++ ) {
     printf("fft_cpy_from_data ptr : i=%d : ptr=%x              data=[%x] \n",i , (int16_t *)&ptr[i-FRAME_1ST]  ,(int16_t)ptr[i-FRAME_1ST] );
     printf("fft_cpy_from_data ptr : i=%d : fft_inp_data.ptr=%x data=[%x] \n",i , (int16_t *)&fft_inp_data[i] ,(int16_t)fft_inp_data[i] );
     printf("fft_cpy_from_data ptr : i=%d : fft_out_data.ptr=%x data=[%x] \n",i , (int16_t *)&fft_out_data[i] ,(int16_t)fft_out_data[i] );
    }
}
void fft_cpy_to_data(int16_t *ptr)  {
    int i=0;
    /* fft run*/
    for ( i = 0; i < QVGA_WIDTH; i++) ptr[i]=fft_out_data[i];

    for ( i = 0; i < QVGA_WIDTH; i++) {
     printf("fft_cpy_to_data ptr : i=%d : fft_inp_data.ptr=%x data=[%x] \n",i , (int16_t *)&fft_inp_data[i]  ,(int16_t)fft_inp_data[i] );
     printf("fft_cpy_to_data ptr : i=%d : fft_out_data.ptr=%x data=[%x] \n",i , (int16_t *)&fft_out_data[i]  ,(int16_t)fft_out_data[i] );
     printf("fft_cpy_to_data ptr : i=%d : ptr=%x              data=[%x] \n",i , (int16_t *)&ptr[i] ,(int16_t)ptr[i] );
    }
}

void fft_clr_frame(uint16_t *ptr)  {
    int i=0,j=0;
    /* fft run*/
    for (i=0;i<QVGA_HIGHT;i++)
       for ( j = 0; j < QVGA_WIDTH; j++) ptr[QVGA_WIDTH*i+j]=0;
}

void fft_oneframe(uint16_t *inptr)  {
/* for fft */
    int i=0,j=0;
     printf("parameter address for fft_oneframe(%x) \n", (uint32_t)&inptr[QVGA_WIDTH*i] );

        /* fft run*/
     for (i=0;i<QVGA_HIGHT;i++)
     {
        fft_cpy_from_data( (int16_t *)&inptr[QVGA_WIDTH*i] );
        //fft_func(     (int16_t *)&inptr[QVGA_WIDTH*i] ,i );
        fft_cpy_to_data( (int16_t *)&inptr[QVGA_WIDTH*i] );
     } /* for end */

}

void fft_func(uint16_t *inptr,int j)  {
/* for fft */
    int i=0;
    complex_hard_t data_hard[FFT_N] = {0};
    fft_data_t *output_data;
    fft_data_t *input_data;

if(SKIPFFT==1) return;

        for ( i = 0; i < FFT_N / 2; ++i)
        {
            input_data = (fft_data_t *)&buffer_input[i];
            input_data->R1 = fft_rgb565_conv( fft_inp_data[2*i] );      // data_hard[2 * i].real;
            input_data->I1 = 0;                                         // data_hard[2 * i].imag;
            input_data->R2 = fft_rgb565_conv( fft_inp_data[2*i+1] );    // data_hard[2 * i + 1].real;
            input_data->I2 = 0;                                         // data_hard[2 * i + 1].imag;
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

        for (i = 0; i < FFT_N; i++)
        {
            hard_power[i] = sqrt(data_hard[i].real * data_hard[i].real + data_hard[i].imag * data_hard[i].imag);
            //Convert to dBFS
            hard_power[i] = 20*log(2*hard_power[i]/FFT_N);
            fft_out_data[i] = hard_power[i];
            printf("fft_func output data: i=%d : outptr=%x data=[%x] \n",i, (uint32_t)&fft_out_data[i], fft_out_data[i]);
        }

}
