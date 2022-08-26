#include <rtthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

/* for K210 rt-theread */
/* kpu code at 2022.03.27 by yoshito tagashira */

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

/* 使うカメラの選択 */
#if   defined(CAM_USING_OV2640)
#include "drv_ov2640.h"
#elif defined(CAM_USING_OV5640)
#include "drv_ov5640.h"
#endif

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
#include "fft_soft.h"

/* 日本語フォントファイル切り替え */
#ifndef _FONT_JP_KANA
//#define _FONT_JP_KANA 0
#define _FONT_JP_KANA 1
#endif

#ifndef KPU3FFT_DEBUG
#define KPU3FFT_DEBUG 0
//#define KPU3FFT_DEBUG 1
#endif
#ifndef SKIPFFT
#define SKIPFFT 0
//#define SKIPFFT 1
#endif

/* 画面サイズ指定 */
#define OV_WIDTH   LCD_X_MAX 	/* 320 */
#define OV_HIGHT   LCD_Y_MAX 	/* 240 */
#define QVGA_WIDTH OV_WIDTH
#define QVGA_HIGHT OV_HIGHT
#define G_AI_WIDTH  320
#define G_AI_HIGHT  224
#define G_AI_LAST   240
#define G_AI_OFFSET 10
#define OV_TOP16    16

/* framebuffer 構造体定義 */
/* driver/camera/drv_ov2640.h */
/* driver/camera/drv_ov5640.h */
static kpuframebuffer_t _framebuffer0 = {RT_NULL,RT_NULL,RT_NULL,RT_NULL};
kpuframebuffer_t *kfb = &_framebuffer0;

static rt_thread_t kpu3thread = RT_NULL;

#if defined(RT_USING_SPI_MSD) && defined(RT_USING_DFS_ELMFAT)
#include <spi_msd.h>
#include <dfs_fs.h>
#include <dfs_file.h>
#include <dfs_private.h>
#endif

/* for fft */
#define FFT_N               512U
#define FFTLINE_320         320
#define FFTLINE_512         512
#define FFT_FORWARD_SHIFT   0x0U
#define FFTLINE_80          80
#define FFTLINE_192         192

/* 38400(=320x240) の内訳 */
/* 320 X 120 X 32bit値 */
/* 320 X 240 X 16bit値 */
#define  GRAM_SIZE 240 * 320 / 2
#define  GRAM_BYTE_SIZE GRAM_SIZE * 4
int32_t  g_fft_gram[GRAM_SIZE]  __attribute__((aligned(64)));
uint32_t g_lcd_gram0[GRAM_SIZE] __attribute__((aligned(64)));
uint32_t g_lcd_gram1[GRAM_SIZE] __attribute__((aligned(64)));

float    hard_power[FFT_N];
float    soft_power[FFT_N];
uint64_t fft_out_data[FFT_N / 2];
uint64_t buffer_input[FFT_N];
uint64_t buffer_output[FFT_N];

/* kpu 識別種類数定義 */
#define CLASS_NUMBER 20

/* 各種フラッグ*/
static volatile uint32_t g_ai_done_flag;
static volatile uint8_t g_dvp_finish_flag;
static volatile uint8_t g_fft_finish_flag;
static volatile uint8_t g_ram_mux = FALSE;


/* KPU学習済みモデルデータサイズ定義*/
#define KMODEL_SIZE (3104 * 1024)
/* KPU学習済みモデルデータ格納Pointer定義*/
static uint8_t *model_data;

/* gramポインタ関連*/
/* malloc して得られるポインタの保存先として*/
/* 320 X 240 X 24bit値*/
//static uint8_t  *g_ai_buf;
static uint8_t  g_ai_buf[QVGA_WIDTH * QVGA_HIGHT * 3];

/* CNN 物体識別するときのアンカーボックス定義*/
kpu_model_context_t face_detect_task;
static region_layer_t detect_rl0, detect_rl1;
static obj_info_t face_detect_info;
#define ANCHOR_NUM 3
// NOTE x,y

static float layer1_anchor[ANCHOR_NUM * 2]= {
    10. / 320. * 20, 14. / 224. * 14, 23. / 320. * 20,
    27. / 224. * 14, 37. / 320. * 20, 58. / 224. * 14,
};

static float layer0_anchor[ANCHOR_NUM * 2]= {
    81. / 320. * 10, 82. / 224. * 7,   135. / 320. * 10,
    169. / 224. * 7, 344. / 320. * 10, 319. / 224. * 7,
};


/* g_ai_bufからのカメラ映像を取り込む*/
static void cp_image_for_ai_buff_top16(uint32_t *ptr){
int i,j;
uint16_t dl,dh,d1,d2,d3;
uint32_t d;
uint8_t r,g,b;

    for(i=0;i<OV_TOP16 ;i++){
        for(j=0;j<G_AI_WIDTH / 2 ;j++){
         r = g_ai_buf[(i)*G_AI_WIDTH/2 + 2*j ];
         g = g_ai_buf[(i)*G_AI_WIDTH/2 + 2*j + G_AI_WIDTH/2 * G_AI_HIGHT     ];
         b = g_ai_buf[(i)*G_AI_WIDTH/2 + 2*j + G_AI_WIDTH/2 * G_AI_HIGHT * 2 ];
         r = (r>>3) & 0x1f;
         g = (g>>2) & 0x3f;
         b = (b<<3) & 0x1f;
         d1=(r     ) & 0x001f;
         d2=(g << 5) & 0x07e0;
         d3=(b <<11) & 0xF800;
         dl = d1|d2|d3;
         r = g_ai_buf[(i)*G_AI_WIDTH/2 + 2*j + 1 ];
         g = g_ai_buf[(i)*G_AI_WIDTH/2 + 2*j + 1 + G_AI_WIDTH/2 * G_AI_HIGHT     ];
         b = g_ai_buf[(i)*G_AI_WIDTH/2 + 2*j + 1 + G_AI_WIDTH/2 * G_AI_HIGHT * 2 ];
         r = (r>>3) & 0x1f;
         g = (g>>2) & 0x3f;
         b = (b<<3) & 0x1f;
         d1=(r     ) & 0x001f;
         d2=(g << 5) & 0x07e0;
         d3=(b <<11) & 0xF800;
         dh = d1|d2|d3;
         d = (dh<<16)&0xffff0000 | (dl&0xffff);
         ptr[(i)*G_AI_WIDTH/2 + 2*j] = d;
        }
    }

}

static void cp_image_for_lcd_buff_test(uint32_t *ptr){
int i,j;
uint32_t d;
uint16_t dh,dl;
uint16_t rgbh,rgbl;
//uint8_t rh,gh,bh;
//uint8_t rl,gl,bl;
float rh,gh,bh;
float rl,gl,bl;
float vh,vl;

    /* トップ位置から16行目まではカメラ画像をモノクロ16ビット化する*/
    for(i=0;i<OV_TOP16 ;i++){
        for(j=0;j<G_AI_WIDTH/2 ;j++){
#if 1
         if(j<8) d=0xffffffff;
         else if(j>=8 && j<16) d=0;
         else if(j>=16 && j<24) d=0xffffffff;
         else if(j>=24 && j<32) d=0;
         else if(j>=32 && j<40) d=0xffffffff;
         else if(j>=40 && j<48) d=0;
         else if(j>=48 && j<56) d=0xffffffff;
         else if(j>=56 && j<64) d=0;
         else if(j>=64 && j<72) d=0xffffffff;
         else if(j>=72 && j<80) d=0;
         else if(j>=80 && j<88) d=0xffffffff;
         else if(j>=88 && j<96) d=0;
         else if(j>=96 && j<114) d=0xffffffff;
         else if(j>=114 && j<122) d=0;
         else if(j>=122 && j<130) d=0xffffffff;
         else if(j>=130 && j<138) d=0;
         else if(j>=138 && j<146) d=0xffffffff;
         else if(j>=146 && j<154) d=0;
         else if(j>=154 && j<160) d=0xffffffff;
#else

         if(j<16) d=0xffffffff;
         else if(j>=16 && j<32) d=0;
         else if(j>=32 && j<48) d=0xffffffff;
         else if(j>=48 && j<64) d=0;
         else if(j>=64 && j<80) d=0xffffffff;
         else if(j>=80 && j<96) d=0;
         else if(j>=96 && j<112) d=0xffffffff;
         else if(j>=112 && j<128) d=0;
         else if(j>=128 && j<144) d=0xffffffff;
         else if(j>=144 && j<160) d=0;

#endif

         dh = (d>>16)&0xffff;
         dl = (d)&0xffff;

         if(KPU3FFT_DEBUG) rt_kprintf("cp_image lcdbuff:HIGHT=[%d] WIDTH=[%d] dh=[%x] dl=[%x] \n",i,j,dh,dl );

         ptr[(i)*G_AI_WIDTH/2 + j] = d;

        }
    } /* for(i=0;i<OV_TOP16 ;i++) */

}

static void cp_image_for_lcd_buff_outside_of_top16(uint32_t *ptr){
int i,j;
uint32_t d;
uint16_t dh,dl;
uint16_t rgbh,rgbl;
//uint8_t rh,gh,bh;
//uint8_t rl,gl,bl;
float rh,gh,bh;
float rl,gl,bl;
float vh,vl;
    /* トップ位置16行より最終行まではカメラ画像をそのまま使う*/
    for(i=OV_TOP16;i<G_AI_LAST ;i++){
        for(j=0;j<G_AI_WIDTH/2 ;j++){
         d = g_ram_mux ? g_lcd_gram1[(i)*G_AI_WIDTH/2 + j ] :  g_lcd_gram0[(i)*G_AI_WIDTH/2 + j ];
         ptr[(i)*G_AI_WIDTH/2 + j] = d;
        }
    }

}


/* g_lcd_gramからのカメラ映像を取り込む*/
/* g_lcd_gram : 画像フォーマットはRGB565の16Bit形式*/
/* */
static void cp_image_for_lcd_buff_top16(uint32_t *ptr){
int i,j;
uint32_t d;
uint16_t dh,dl;
uint16_t rgbh,rgbl;
//uint8_t rh,gh,bh;
//uint8_t rl,gl,bl;
float rh,gh,bh;
float rl,gl,bl;
float vh,vl;

    /* トップ位置から16行目まではカメラ画像をモノクロ16ビット化する*/
    for(i=0;i<OV_TOP16 ;i++){
        for(j=0;j<G_AI_WIDTH/2 ;j++){
         d = g_ram_mux ? g_lcd_gram1[(i)*G_AI_WIDTH/2 + j ] :  g_lcd_gram0[(i)*G_AI_WIDTH/2 + j ];
         //d = g_lcd_gram[(i)*G_AI_WIDTH/2 + j ];
         dh = (d>>16)&0xffff;
         dl = (d)&0xffff;

         /* RGB各色を合計してモノクロ16ビットにする*/
         rh = (dh>>11)&0x1f;
         /* Gは5bit化する*/
         gh = (dh>>6 )&0x1f;
         bh = (dh    )&0x1f;
         /* カラーをモノクロ化する*/
         vh = 0.3*rh + 0.6*gh + 0.1*bh;
         rgbh = vh;

         /* 小さい値は足切り*/
         //if(rgbh<0x10)rgbh=0;

         /* 値が8bitなので16bit化する*/
         dh = rgbh<<11;

         /* RGB各色を合計してモノクロ16ビットにする*/
         rl = (dl>>11)&0x1f;
         /* Gは5bit化する*/
         gl = (dl>>6 )&0x1f;
         bl = (dl    )&0x1f;
         /* カラーをモノクロ化する*/
         vl = 0.3*rl + 0.6*gl + 0.1*bl;
         rgbl = vl;

         /* 小さい値は足切り*/
         //if(rgbl<0x10)rgbl=0;

         /* 値が8bitなので16bit化する*/
         dl = rgbl<<11;

         if(KPU3FFT_DEBUG) rt_kprintf("cp_image lcdbuff:HIGHT=[%d] WIDTH=[%d] rgbh=[%x] rgbl=[%x] dh=[%x] dl=[%x] \n",i,j,rgbh,rgbl,dh,dl );

         d = (dh<<16)&0xffff0000 | (dl&0xffff);
         ptr[(i)*G_AI_WIDTH/2 + j] = d;
        }
    } /* for(i=0;i<OV_TOP16 ;i++) */

}

static void ai_done(void *ctx) { g_ai_done_flag= 1; }

static int dvp_irq(void *ctx) {
    rt_enter_critical();
    if (dvp_get_interrupt(DVP_STS_FRAME_FINISH)) {

        /* switch gram */
        /* 条件演算子 g_ram_muxがTRUEならg_lcd_gram1[0]を使うFALSEならg_lcd_gram0[0] */
        dvp_set_display_addr(g_ram_mux ? (uint32_t)&g_lcd_gram1[0] : (uint32_t)&g_lcd_gram0[0]);
        dvp_clear_interrupt(DVP_STS_FRAME_FINISH);
        rt_sem_release(kfb->lock);
        g_dvp_finish_flag= 1;

    } else {
        if(RT_EOK == rt_sem_trytake(kfb->lock))
        {
	dvp_start_convert();
        }
        dvp_clear_interrupt(DVP_STS_FRAME_START);
    }
    rt_exit_critical();
    return 0;
}

#if (CLASS_NUMBER > 1)
typedef struct {
    uint8_t  len;
    char *str;
    uint16_t color;
    uint16_t height;
    uint16_t width;
    uint32_t *ptr;
} class_lable_t;

/* 日本語フォントファイル切り替え */
#if (_FONT_JP_KANA == 0) 
#include "class_tbl_ascii.h"
#elif(_FONT_JP_KANA == 1) 
#include "class_tbl_jp.h"
#endif /* #if (_FONT_JP_KANA == 0) */

static uint32_t lable_string_draw_ram[115 * 16 * 8 / 2];
#endif /* #if (CLASS_NUMBER > 1) */

static void lable_init(void) {
#if (CLASS_NUMBER > 1)
    uint8_t index;

    class_lable[0].height= 16;
    //class_lable[0].width= 8 * rt_strlen(class_lable[0].str);
    class_lable[0].width= 8 * class_lable[0].len;
    class_lable[0].ptr= lable_string_draw_ram;
    lcd_ram_draw_string(class_lable[0].str, class_lable[0].ptr, BLACK, class_lable[0].color);
    for (index= 1; index < CLASS_NUMBER; index++) {
        class_lable[index].height= 16;
        //class_lable[index].width= 8 * rt_strlen(class_lable[index].str);
        class_lable[index].width= 8 * class_lable[index].len;
        class_lable[index].ptr= class_lable[index - 1].ptr +
                                class_lable[index - 1].height * class_lable[index - 1].width / 2;
        lcd_ram_draw_string(class_lable[index].str, class_lable[index].ptr, BLACK,
                            class_lable[index].color);
    }
#endif /* #if (CLASS_NUMBER > 1) */
}

static void drawboxes(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t class,
                      float prob) {
    if (x1 >= QVGA_WIDTH) x1= QVGA_WIDTH-1;
    if (x2 >= QVGA_WIDTH) x2= QVGA_WIDTH-1;
    if (y1 >= QVGA_HIGHT) y1= QVGA_HIGHT-1;
    if (y2 >= QVGA_HIGHT) y2= QVGA_HIGHT-1;

#if (CLASS_NUMBER > 1)
    lcd_draw_rectangle(x1, y1, x2, y2, 2, class_lable[class].color);
    lcd_draw_picture(x1 + 1, y1 + 1, class_lable[class].width, class_lable[class].height,
                     class_lable[class].ptr);
#else
    lcd_draw_rectangle(x1, y1, x2, y2, 2, RED);
#endif
}

static uint16_t fft_rgb565_conv(uint16_t indata)  {
    int16_t r,g,b;
    r=(indata >> 11) & 0x1f;r=r*r;
    g=(indata >> 5)  & 0x3f;g=g*g;
    b=(indata     )  & 0x1f;b=b*b;
    return r+g+b;
}

/* Software FFT */
static void fft_soft_func(void)  {
    int i=0,j,k,ofset;
    int8_t r,g,b;
    int8_t rh,gh,bh;
    int8_t rl,gl,bl;
    int16_t rgbl,rgbh;
    int16_t rgblm,rgbhm;
    int32_t rgb;
    complex data_soft[FFT_N] = {0};
    fft_data_t *input_data;
    float pmax=10;

    g_fft_finish_flag=0;

if(SKIPFFT==1) return;

    for (j=0;j<OV_TOP16;j++){
        for (i = 0; i < FFT_N/2 ; i++) /* 0 1 2 3 4 5 ... 255 */
        {
            if(i<QVGA_WIDTH/2) /* 0 1 2 3 4 5 ... 159 */
                rgb = g_fft_gram[(j)*QVGA_WIDTH/2+i];
            else
                rgb = 0;

            rgbh = (rgb >> 16)&0xffff;
            rgbl = (rgb      )&0xffff;

            data_soft[2*i].real = (int16_t)rgbh;
            data_soft[2*i].imag = (int16_t)0;
            data_soft[2*i+1].real = (int16_t)rgbl;
            data_soft[2*i+1].imag = (int16_t)0;
        }
        for (int i = 0; i < FFT_N / 2; ++i) /* 0 1 2 3 4 5 ... 255 */
        {
            input_data = (fft_data_t *)&buffer_input[i];
            input_data->R1 = data_soft[2 * i].real;
            input_data->I1 = data_soft[2 * i].imag;
            input_data->R2 = data_soft[2 * i + 1].real;
            input_data->I2 = data_soft[2 * i + 1].imag;
        }
        /* software FFT Run */
        fft_soft(data_soft, FFT_N);
        /* FFT Result Data Recalc RootMeanSquera */
        pmax=10;
        for (i = 0; i < FFT_N; i++)
        {
            soft_power[i] = sqrt(data_soft[i].real * data_soft[i].real + data_soft[i].imag * data_soft[i].imag) ;

            //Convert to dBFS
            //soft_power[i] = 20*log(2*soft_power[i]/FFT_N) ;
            //if( soft_power[i]>pmax) pmax = soft_power[i];
            if(KPU3FFT_DEBUG) rt_kprintf("fft_soft_func 1 :HIGHT=[%d] WIDTH=[%d] soft_power[i]= [%d] \n",j,i,soft_power[i]  );

        }

        /* copy FFT Result to buffer */
        for (i = 0; i < FFT_N/2 ; i++) /* i=0 1 2 3 4 5 ... 255 */
        {
            rgbh = soft_power[ (2*i) ];    /* 0 2 4 6 8 10 ... 510 */
            rgbl = soft_power[ (2*i) + 1]; /* 1 3 5 7 9 11 ... 511 */

            if(rgbh<0)rgbh=0;
            if(rgbl<0)rgbl=0;
            if(rgbh<1000)rgbh=0;
            if(rgbl<1000)rgbl=0;

            rgb  = (rgbh<<16)&0xffff0000 | (rgbl&0xffff);

            if(i<QVGA_WIDTH/2) /* 0 1 2 3 4 5 ... 159 */
                g_fft_gram[(j)*QVGA_WIDTH/2+i]=rgb;

            if(KPU3FFT_DEBUG) rt_kprintf("fft_soft_func 2 :HIGHT=[%3d] WIDTH=[%3d] rgb=[%8x] rgbh=[%8x] rgbl=[%8x] \n",j,i, rgb,rgbh,rgbl );
        }

    } /* for (j=0;j<OV_TOP16;j++) */

#if 0

    for (i = 0; i < QVGA_WIDTH/2 ; i++){
        rgbh=0;rgbl=0;
        for (j=0;j<OV_TOP16;j++){
            rgb = g_fft_gram[(j)*QVGA_WIDTH/2+i];
            rgbh += (rgb >> 16)&0xffff;
            rgbl += (rgb      )&0xffff;
        }
        rgbh /= 16; rgbl /= 16;
        for (j=0;j<OV_TOP16;j++){
            /* マイナス値のカット */
            if(rgbh<0)rgbh=0;
            if(rgbl<0)rgbl=0;
            /* 値の最大値計算 */
            rh = 0x1f; gh = 0x3f; bh = 0x1f;
            rl = 0x1f; gl = 0x3f; bl = 0x1f;
            rgblm = (rl<<11&0xf800)|(gl<<5&0x07e0)|(bl&0x1f);
            rgbhm = (rh<<11&0xf800)|(gh<<5&0x07e0)|(bh&0x1f);
            /* 値の調整実験 */
            //if(rgbh<1000)rgbh=0;else rgbh=rgbhm;
            //if(rgbl<1000)rgbl=0;else rgbl=rgblm;
            rgb  = (rgbh<<16)&0xffff0000 | (rgbl&0xffff);
            g_fft_gram[ (j)*QVGA_WIDTH/2 +i ] = rgb  ;
            if(KPU3FFT_DEBUG) rt_kprintf("fft_soft_func 3 :HIGHT=[%3d] WIDTH=[%3d] rgb=[%8x]  \n",j,i, rgb );
        }
    }
#endif

    g_fft_finish_flag=1;

}


/*---------------------------------------------------------------------------------------
packages/kendryte-sdk-v0.5.6/kendryte-standalone-sdk/lib/drivers/include/fft.h
typedef struct _fft_data
{
    int16_t I1;
    int16_t R1;
    int16_t I2;
    int16_t R2;
} fft_data_t;
----------------------------------------------------------------------------------------*/
/* Hardeware fft_func */
static void fft_func(void)  {
    int i=0,j,k,ofset;
    int8_t r,g,b;
    int8_t rh,gh,bh;
    int8_t rl,gl,bl;
    int16_t rgbl,rgbh;
    int16_t rgblm,rgbhm;
    int32_t rgb;
    complex_hard_t data_hard[FFT_N] = {0};
    fft_data_t *output_data;
    fft_data_t *input_data;
    float pmax=10;

    g_fft_finish_flag=0;

if(SKIPFFT==1) return;

    for (j=0;j<OV_TOP16;j++){
        for (i = 0; i < FFT_N/2 ; i++) /* 0 1 2 3 4 5 ... 255 */
        {
            if(i<QVGA_WIDTH/2) /* 0 1 2 3 4 5 ... 159 */
                rgb = g_fft_gram[(j)*QVGA_WIDTH/2+i];
            else 
                rgb = 0;

            rgbh = (rgb >> 16)&0xffff;
            rgbl = (rgb      )&0xffff;
            data_hard[2*i].real = (int16_t)rgbh;
            data_hard[2*i].imag = (int16_t)0;
            data_hard[2*i+1].real = (int16_t)rgbl;
            data_hard[2*i+1].imag = (int16_t)0;
        }
        for (int i = 0; i < FFT_N / 2; ++i) /* 0 1 2 3 4 5 ... 255 */
        {
            input_data = (fft_data_t *)&buffer_input[i];
            input_data->R1 = data_hard[2 * i].real;
            input_data->I1 = data_hard[2 * i].imag;
            input_data->R2 = data_hard[2 * i + 1].real;
            input_data->I2 = data_hard[2 * i + 1].imag;
        }

        /* FFT Run */
        fft_complex_uint16_dma(DMAC_CHANNEL0, DMAC_CHANNEL1,FFT_FORWARD_SHIFT,FFT_DIR_FORWARD,buffer_input,FFT_N,buffer_output);

        /* FFT Result Data Settings */
        for ( i = 0; i < FFT_N / 2; i++)
        {
            output_data = (fft_data_t*)&buffer_output[i];
            data_hard[2 * i].real = output_data->R1 ;
            data_hard[2 * i].imag = output_data->I1 ;
            data_hard[2 * i + 1].real = output_data->R2  ;
            data_hard[2 * i + 1].imag = output_data->I2  ;

        }
        /* FFT Result Data Recalc RootMeanSquera */
        pmax=10;
        for (i = 0; i < FFT_N; i++)
        {
            hard_power[i] = sqrt(data_hard[i].real * data_hard[i].real + data_hard[i].imag * data_hard[i].imag);

            //Convert to dBFS
            hard_power[i] = 20*log(2*hard_power[i]/FFT_N) ;
            if( hard_power[i]>pmax) pmax = hard_power[i];
            if(KPU3FFT_DEBUG) rt_kprintf("fft_func 1 :HIGHT=[%d] WIDTH=[%d] hard_power[i]= [%8x] \n",j,i,hard_power[i]  );

        }

        /* copy FFT Result to buffer 0->256 */
        for (i = 0; i < FFT_N/2 ; i++) /* 0 1 2 3 4 5 6 7 8 10 ... 255 */
        {
            rgbh = hard_power[ (2*i)  ];
            rgbl = hard_power[ (2*i) + 1];

            if(rgbh<0)rgbh=0;
            if(rgbl<0)rgbl=0;

            /* 値の最大値計算 */
            rh = 0x1f; gh = 0x3f; bh = 0x1f;
            rl = 0x1f; gl = 0x3f; bl = 0x1f;
            rgblm = (rl<<11&0xf800)|(gl<<5&0x07e0)|(bl&0x1f);
            rgbhm = (rh<<11&0xf800)|(gh<<5&0x07e0)|(bh&0x1f);

            /* 値の調整実験 */
            if(rgbh<0x40)rgbh=0;else rgbh=rgbhm;
            if(rgbl<0x40)rgbl=0;else rgbl=rgblm;

            rgb  = (rgbh<<16)&0xffff0000 | (rgbl&0xffff); 

            if(i<QVGA_WIDTH/2) /* 0 1 2 3 4 5 ... 159 */
                g_fft_gram[(j)*QVGA_WIDTH/2+i]=rgb;

            if(KPU3FFT_DEBUG) rt_kprintf("fft_func 2 :HIGHT=[%3d] WIDTH=[%3d] rgb=[%8x] rgbh=[%8x] rgbl=[%8x] \n",j,i, rgb,rgbh,rgbl );
        }

    } /* for (j=0;j<OV_TOP16;j++) */

#if 0

    for (i = 0; i < QVGA_WIDTH/2 ; i++){
        rgbh=0;rgbl=0;
        for (j=0;j<OV_TOP16;j++){
            rgb = g_fft_gram[(j)*QVGA_WIDTH/2+i];
            rgbh += (rgb >> 16)&0xffff;
            rgbl += (rgb      )&0xffff;
        }
        rgbh /= 16; rgbl /= 16;
        for (j=0;j<OV_TOP16;j++){
            /* マイナス値のカット */
            if(rgbh<0)rgbh=0;
            if(rgbl<0)rgbl=0;
            /* 値の最大値計算 */
            rh = 0x1f; gh = 0x3f; bh = 0x1f;
            rl = 0x1f; gl = 0x3f; bl = 0x1f;
            rgblm = (rl<<11&0xf800)|(gl<<5&0x07e0)|(bl&0x1f);
            rgbhm = (rh<<11&0xf800)|(gh<<5&0x07e0)|(bh&0x1f);
            /* 値の調整実験 */
            //if(rgbh<1000)rgbh=0;else rgbh=rgbhm;
            //if(rgbl<1000)rgbl=0;else rgbl=rgblm;
            rgb  = (rgbh<<16)&0xffff0000 | (rgbl&0xffff);
            g_fft_gram[ (j)*QVGA_WIDTH/2 +i ] = rgb  ;
            if(KPU3FFT_DEBUG) rt_kprintf("fft_func 3 :HIGHT=[%3d] WIDTH=[%3d] rgb=[%8x]  \n",j,i, rgb );
        }
    }
#endif
    g_fft_finish_flag=1;
}

static void color_func(uint16_t *outptr)  {
/* for fft */
    int i=0,j;
    uint8_t r,g,b;
    uint16_t rgb;

    for (i = 0,j=0; i < 320 ; i++,j++)
    {
                rgb=outptr[i];
                r=(rgb>>11)&0x1f;
                g=(rgb>>5)&0x3f;
                b=(rgb)&0x1f;

                if(r>0){r=0x1f;}
                if(g>0){g=0x3f;}
                if(b>0){b=0x1f;}

                rgb=r<<11|g<<5|b;
                outptr[j]=rgb;
    }

}

static void clear_rgb565_colorbar(int32_t *ptr)  {
int i,j;

    for (j=0;j<QVGA_HIGHT;j++){
        for ( i = 0; i < FFTLINE_320/2 ; i++) {
            ptr[(j)*FFTLINE_320/2 + i] = 0;
        }
    }

}

static void set_rgb565_data(int32_t *ptr)  {
    int i=0,j=0;
    int32_t rgb;
    uint16_t rgbh,rgbl;
    uint8_t r,g,b;

        for (j=0;j<16;j++){
            for (i=0;i<FFTLINE_320/2;i++){
              r = 0x1f;g=0;b=0;
              rgbh=r<<11|g<<5|b;
              rgbl=r<<11|g<<5|b;
              rgb = (rgbh<<16)&0xffff0000 | (rgbl&0xffff);
              ptr[ (j)*FFTLINE_320/2 + i] = rgb;
            }
        }
}

/* for lcd test function */
static void set_rgb565_colorbar(int32_t *ptr)  {
    int i=0,j=0;
    int32_t rgb;
    uint16_t rgbh,rgbl;
    uint8_t r,g,b;
        /* fft run*/

        for (j=0;j<QVGA_HIGHT;j++){
            for (i=0;i<20;i++){
              r = 0x1f;g=0;b=0;
              rgbh=r<<11|g<<5|b;
              rgbl=r<<11|g<<5|b;
              rgb = (rgbh<<16)&0xffff0000 | (rgbl&0xffff);
              ptr[ (j)*FFTLINE_320/2 + i] = rgb;
            }
        }
        for (j=0;j<QVGA_HIGHT;j++){
            for (i=20;i<40;i++){
              r=0;g=0;b=0;
              rgbh=r<<11|g<<5|b;
              rgbl=r<<11|g<<5|b;
              rgb = (rgbh<<16)&0xffff0000 | (rgbl&0xffff);
              ptr[ (j)*FFTLINE_320/2 + i] = rgb;
            }
        }
        for (j=0;j<QVGA_HIGHT;j++){
            for (i=40;i<60;i++){
              r = 0;g=0x3f;b=0;
              rgbh=r<<11|g<<5|b;
              rgbl=r<<11|g<<5|b;
              rgb = (rgbh<<16)&0xffff0000 | (rgbl&0xffff);
              ptr[ (j)*FFTLINE_320/2 + i] = rgb;
            }
        }
        for (j=0;j<QVGA_HIGHT;j++){
            for (i=60;i<80;i++){
              r=0;g=0;b=0;
              rgbh=r<<11|g<<5|b;
              rgbl=r<<11|g<<5|b;
              rgb = (rgbh<<16)&0xffff0000 | (rgbl&0xffff);
              ptr[ (j)*FFTLINE_320/2 + i] = rgb;
            }
        }
        for (j=0;j<QVGA_HIGHT;j++){
            for (i=80;i<100;i++){
              r=0;g=0;b=0x1f;
              rgbh=r<<11|g<<5|b;
              rgbl=r<<11|g<<5|b;
              rgb = (rgbh<<16)&0xffff0000 | (rgbl&0xffff);
              ptr[ (j)*FFTLINE_320/2 + i] = rgb;
            }
        }
        for (j=0;j<QVGA_HIGHT;j++){
            for (i=100;i<120;i++){
              r=0;g=0;b=0;
              rgbh=r<<11|g<<5|b;
              rgbl=r<<11|g<<5|b;
              rgb = (rgbh<<16)&0xffff0000 | (rgbl&0xffff);
              ptr[ (j)*FFTLINE_320/2 + i] = rgb;
            }
        }
        for (j=0;j<QVGA_HIGHT;j++){
            for (i=120;i<140;i++){
              r = 0x1f;g=0;b=0;
              rgbh=r<<11|g<<5|b;
              rgbl=r<<11|g<<5|b;
              rgb = (rgbh<<16)&0xffff0000 | (rgbl&0xffff);
              ptr[ (j)*FFTLINE_320/2 + i] = rgb;
            }
        }
        for (j=0;j<QVGA_HIGHT;j++){
            for (i=140;i<160;i++){
              r = 0;g=0;b=0;
              rgbh=r<<11|g<<5|b;
              rgbl=r<<11|g<<5|b;
              rgb = (rgbh<<16)&0xffff0000 | (rgbl&0xffff);
              ptr[ (j)*FFTLINE_320/2 + i] = rgb;
            }
        }

}


#define PLL0_OUTPUT_FREQ 800000000UL
//#define PLL1_OUTPUT_FREQ 400000000UL
#define PLL1_OUTPUT_FREQ 160000000UL
void kpu3_main(void *parameter) {
     int sta= 0;
     uint32_t ret,length;

    /* Set CPU and dvp clk */
    sysctl_pll_set_freq(SYSCTL_PLL0,PLL0_OUTPUT_FREQ );
    sysctl_pll_set_freq(SYSCTL_PLL1,PLL1_OUTPUT_FREQ );
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    sysctl_clock_enable(SYSCTL_CLOCK_AI);
    uarths_init();

    io_mux_init();
    io_set_power();
    /* init Platform Level Interrupt Controller */
    plic_init();

    /* Label init */
    lable_init();

    /* LCD init */
    printf("LCD init\n");
    lcd_init();
    lcd_clear(BLACK);

    /*init memory space*/
    kfb->lock = rt_sem_create("kfb lock",1,RT_IPC_FLAG_FIFO);
    kfb->pix_buf0 = g_lcd_gram0;
    kfb->pix_buf1 = g_lcd_gram1;
    kfb->pix_buf1 = g_lcd_gram0;
    kfb->ai_buf   = g_ai_buf;
    rt_thread_mdelay(10);

#if BOARD_LICHEEDAN == 1
#if   defined(BSP_USING_MAIXBIT)
    printf("MAIXBIT LCD \n");
    #if defined(CAM_USING_OV2640)
      lcd_set_direction(DIR_YX_RLDU);
    #elif defined(CAM_USING_OV5640)
      //lcd_set_direction(DIR_YX_LRDU);/* 上下逆映像 */
      //lcd_set_direction(DIR_YX_RLUD);
      lcd_set_direction(DIR_YX_RLDU);
    #endif
#elif defined(BSP_USING_MAIXM1DOCK)
    printf("MAIXM1DOCK LCD \n");
    lcd_set_direction(DIR_YX_RLDU);
#endif /* if defined(BSP_USING_MAIXBIT) */
#endif /* #if BOARD_LICHEEDAN == 1 */

    /*lv Init 初期化のみ実施*/
    printf("Call lv_init...\n");
    lv_init();
    rt_thread_mdelay(10);
    disp_init();
    rt_thread_mdelay(10);

    /* lv 表示 例*/
    //lcd_set_direction(DIR_YX_RLDU); /* left up 0,0 */
    //printf("show pic\n");
    //lv_tutorial_hello_world();
    //lv_test2();
    //printf("done lv_tutorial hello world \n");

    /* lv 表示タスク実行例  */
    //while (1) {
    //lv_task_handler();
    //lv_tick_inc(1); 
    //}

    /* DVP Camera Init */
    init_cam();

    /* ai address setting: ai bufferとはkpu演算用RAMバッファ */
    /* カメラ画像をaiバッファに取り込む設定実施*/
    dvp_set_ai_addr( (uint32_t)g_ai_buf,                                        /* Red */
                     (uint32_t)(g_ai_buf + G_AI_WIDTH * G_AI_HIGHT),            /* Green */
                     (uint32_t)(g_ai_buf + G_AI_WIDTH * G_AI_HIGHT * 2) );      /* Blue */


    //g_ram_mux = FALSE;          /* 初期値 FALSE */
    //dvp_set_display_addr(g_ram_mux ? (uint32_t)&g_lcd_gram1[0] : (uint32_t)&g_lcd_gram0[0]);
    dvp_set_display_addr( (uint32_t)&g_lcd_gram0[0] );

    /*init irq*/
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
    dvp_disable_auto();

    /* DVP interrupt config */
    printf("DVP interrupt config\n");
    plic_set_priority(IRQN_DVP_INTERRUPT, 2);
    plic_irq_register(IRQN_DVP_INTERRUPT, dvp_irq, NULL);
    plic_irq_enable(IRQN_DVP_INTERRUPT);

    /* enable global interrupt */
    sysctl_enable_irq();

    rt_thread_mdelay(10);
    enable_cam(1);

    /* system start */
    printf("kpu3_main start\n");

    g_ram_mux = FALSE;          /* 初期値 FALSE */
    g_dvp_finish_flag = 0;  /* カメラのフレーム終了割り込み時に１になる*/
    g_fft_finish_flag = 0;

    dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);

    /* init kpu */
    /* init face detect model */
    rt_thread_mdelay(10);
    /* KPU学習済みモデルデータを確保したポインタアドレスをkpuローダに取り込む設定する*/
    /* face_detect_task はkpu演算用タスク構造体*/
    if (kpu_load_kmodel(&face_detect_task, model_data) != 0) {
        printf("\nmodel init error\n");
        while (1) ;
    }

    g_fft_finish_flag = 0;

    /* 同時に2個のKPU演算レイア(detect_rl0,detect_rl1)を作成実行できるが今回は1個だけ使ってみる*/
    printf("model init ok\n");
    /* KPU演算レイアl0定義*/
    detect_rl0.anchor_number= ANCHOR_NUM;
    detect_rl0.anchor= layer0_anchor;
    detect_rl0.threshold= 0.6;
    detect_rl0.nms_value= 0.3;
    region_layer_init(&detect_rl0, 10, 7, 75, 320, 224);
    printf("region_layer_init rl0 ok\n");

    /* KPU演算レイアl1定義*/
//    detect_rl1.anchor_number= ANCHOR_NUM;
//    detect_rl1.anchor= layer1_anchor;
//    detect_rl1.threshold= 0.6;
//    detect_rl1.nms_value= 0.3;
//    region_layer_init(&detect_rl1, 20, 14, 75, 320, 224);
//    printf("region_layer_init rl1 ok\n");

    while (1)
    {

        /* dvp finish*/
        /* カメラのフレーム終了を待つ*/
        while (g_dvp_finish_flag == 0)
            ;

        //cp_image_for_ai_buff_top16(g_fft_gram);
        cp_image_for_lcd_buff_top16(g_fft_gram);
        cp_image_for_lcd_buff_outside_of_top16(g_fft_gram);
        //set_rgb565_colorbar( (uint32_t)&g_fft_gram[0] );
        fft_func(); 
        //fft_soft_func();
        while (g_fft_finish_flag == 0)
            ;

        //lcd_draw_picture(0, 0, QVGA_WIDTH, QVGA_HIGHT, (uint32_t)&g_lcd_gram0[0] );
        lcd_draw_picture(0, 0, QVGA_WIDTH, QVGA_HIGHT ,  (uint32_t)&g_fft_gram[0] );
        g_fft_finish_flag = 0;
        g_dvp_finish_flag = 0;

#if 0
        /* run face detect */
        /* KPU演算を実行開始する*/
        /* ai_bufのカメラ画像をKPU演算する*/
        /* KPU演算終了でg_ai_done_flagが1になる*/
        kpu_run_kmodel(&face_detect_task, g_ai_buf, DMAC_CHANNEL5, ai_done, NULL);
        while (!g_ai_done_flag);
        /* KPU演算終了フラッグをクリアする*/
        g_ai_done_flag= 0;  

        float *output0, *output1;
        size_t output_size0, output_size1;

        // NOTE output_size 是字节， float 是4字节
        /* レイア0の演算結果をゲットする*/
        sta= kpu_get_output(&face_detect_task, 0, (uint8_t **)&output0, &output_size0);
        detect_rl0.input= output0;
        region_layer_run(&detect_rl0, NULL);

        /* display result */
        //g_ram_mux ^= 0x01;

        /* レイア1の演算結果をゲットする*/
//        sta= kpu_get_output(&face_detect_task, 1, (uint8_t **)&output1, &output_size1);
//        detect_rl1.input= output1;
//        region_layer_run(&detect_rl1, NULL);

        /* 条件演算子 g_ram_muxがTRUEならg_lcd_gram1[0]を使うFALSEならg_lcd_gram0[0] */
        //lcd_draw_picture(0, 0, QVGA_WIDTH, QVGA_HIGHT, g_ram_mux ? (uint32_t)&g_lcd_gram1[0] : (uint32_t)&g_lcd_gram0[0]);
        //set_rgb565_colorbar(g_fft_gram);
        //lcd_draw_picture(0, 0, QVGA_WIDTH, QVGA_HIGHT,  g_lcd_gram) ;
        lcd_draw_picture(0, 0, QVGA_WIDTH, QVGA_HIGHT,  g_fft_gram) ;
        g_fft_finish_flag = 0;
        g_dvp_finish_flag = 0;

        /* run key point detect */
        /* KPUの判定結果を表示する */
        region_layer_draw_boxes(&detect_rl0, drawboxes);
//        region_layer_draw_boxes(&detect_rl1, drawboxes);

#endif
    }

}

int kpu_v3(void)
{
     rt_uint16_t i,j;
     uint32_t ret,length;
     char pathname[]="yolo.kmodel";
     //char pathname[]="yolo50.kmodel";
     struct dfs_fd fd;

     ret = dfs_file_open(&fd,pathname,O_RDONLY) ;
     rt_kprintf("dfs_file_open return=%x \n",ret);
     if ( ret < 0 ) return 1;

     rt_kprintf("kpu3_main\n");
     model_data = (uint8_t *)iomem_malloc(KMODEL_SIZE);
     length = dfs_file_read(&fd,model_data,KMODEL_SIZE);
     if(length>0)rt_kprintf("kpu3 Model data read OK(%d)\n",length);
     else rt_kprintf("kpu3 model data read NG(%d)\n",length);

     dfs_file_close(&fd);
     rt_kprintf("dfs_file_closed \n");

     //g_fft_gram  = (uint32_t *)iomem_malloc(QVGA_WIDTH * QVGA_HIGHT * 2);
     //g_ai_buf    = (uint8_t  *)iomem_malloc(QVGA_WIDTH * QVGA_HIGHT * 3);


     kpu3thread = rt_thread_create( "kpuv3",
                             kpu3_main,
                             RT_NULL,
                             64 * 1024 ,
                             10,
                             3
                             );

     if(kpu3thread != RT_NULL)
     {
                rt_thread_startup(kpu3thread);
     }
     //rt_thread_delay(RT_TICK_PER_SECOND);

     return 0;
}

void kpu_v3_stop(void)
{
        if(kpu3thread != RT_NULL)
        {
        rt_thread_delete(kpu3thread);

        iomem_free(model_data);
        //iomem_free(g_fft_gram);
        //iomem_free(g_ai_buf);

        printf("kpu_yolov3 stoped \n");

        }
}
//INIT_APP_EXPORT(kpu_v3);
MSH_CMD_EXPORT(kpu_v3,start kpu_v3 thread);
MSH_CMD_EXPORT(kpu_v3_stop,stop kpu_v3 thread);

