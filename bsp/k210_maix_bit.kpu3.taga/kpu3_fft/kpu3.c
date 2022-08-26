#include <rtthread.h>
#include <stdio.h>
#include "gpio.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>

#define KPU3_DEBUG 0
//#define KPU3_DEBUG 1

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
#include "image_process.h"
//#include "rgbimg.h"

/* 日本語フォントファイル切り替え */
#ifndef _FONT_JP_KANA
//#define _FONT_JP_KANA 0
#define _FONT_JP_KANA 1
#endif

#define FRAME_LEN           512
#define FFT_N               512U
#define FFT_FORWARD_SHIFT   0x0U
#define SAMPLE_RATE         8000

float    hardpower[FFT_N];
uint64_t fftout_data[FFT_N / 2];
uint64_t buff_input[FFT_N];
uint64_t buff_output[FFT_N];

/* 画面サイズ指定 */
#define G_AI_WIDTH 320
#define G_AI_HIGHT 224
#define QVGA_WIDTH 320
#define QVGA_HIGHT 240

/* framebuffer 構造体定義 */
/* driver/camera/drv_ov2640.h */
/* driver/camera/drv_ov5640.h */
static kpuframebuffer_t _framebuffer0 = {RT_NULL,RT_NULL,RT_NULL};
kpuframebuffer_t *kfb = &_framebuffer0;

static rt_thread_t kpu3thread = RT_NULL;

#if defined(RT_USING_SPI_MSD) && defined(RT_USING_DFS_ELMFAT)
#include <spi_msd.h>
#include <dfs_fs.h>
#include <dfs_file.h>
#include <dfs_private.h>
#endif

/* kpu 識別種類数定義 */
#define CLASS_NUMBER 20

/* 各種フラッグ*/
static volatile uint32_t g_ai_done_flag;
static volatile uint8_t g_dvp_finish_flag;
static volatile uint8_t g_fft_finish_flag;

/* KPU学習済みモデルデータサイズ定義*/
#define KMODEL_SIZE (3104 * 1024)

/* KPU学習済みモデルデータ格納Pointer定義*/
uint8_t model_data[KMODEL_SIZE];

/* ramポインタ関連*/
uint32_t g_lcd_ram[38400]                        __attribute__((aligned(64))) ;
uint32_t g_fft_inp_ram[38400]                    __attribute__((aligned(64))) ;
uint32_t g_fft_out_ram[38400]                    __attribute__((aligned(64))) ;
uint8_t  g_ai_buf[QVGA_WIDTH * QVGA_HIGHT * 3]   __attribute__((aligned(64))) ;

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

static void ai_done(void *ctx) { g_ai_done_flag= 1; }

static void io_mux_init(void) {
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

static void update_image(void){
int i,j;
uint16_t d,d1,d2,d3;
uint8_t r,g,b;

    for(i=0;i<240 ;i++){
        for(j=0;j<320 ;j++){
         d = g_lcd_ram[i*320+j] ;
         g_fft_inp_ram[i*320+j] = d;
         g_fft_out_ram[i*320+j] = d;
         g_fft_finish_flag= 1;
        }
    }

}


static int dvp_irq(void *ctx) {
    rt_enter_critical();
    if (dvp_get_interrupt(DVP_STS_FRAME_FINISH)) {

        /* switch gram */
        dvp_set_display_addr( (uint32_t)g_lcd_ram );
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

#define PLL0_OUTPUT_FREQ 800000000UL
#define PLL1_OUTPUT_FREQ 400000000UL

void kpu3_main(void *parameter) {
    int i,j,sta= 0;
    uint32_t ret,length;
    complex_hard_t data_hard[FFT_N] = {0};
    fft_data_t *output_data;
    fft_data_t *input_data;


    /* Set CPU and dvp clk */
    sysctl_pll_set_freq(SYSCTL_PLL0,PLL0_OUTPUT_FREQ );
    sysctl_pll_set_freq(SYSCTL_PLL1,PLL1_OUTPUT_FREQ );
    sysctl_clock_enable(SYSCTL_CLOCK_AI);

    uarths_init();
    io_mux_init();
    io_set_power();
    /* LCD init */
    printf("LCD init\n");
    lcd_init();
    lcd_clear(BLACK);

    /* init Platform Level Interrupt Controller */
    plic_init();

    /*init memory space*/
    kfb->lock = rt_sem_create("kfb lock",1,RT_IPC_FLAG_FIFO);
    kfb->pix_buf0 = g_lcd_ram;
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

    /* DVP Camera Init */
    init_cam();

    /* ai address setting: ai bufferとはkpu演算用RAMバッファ */
    /* カメラ画像をaiバッファに取り込む設定実施*/
    dvp_set_ai_addr( (uint32_t)g_ai_buf,			                /* Red */ 
                     (uint32_t)(g_ai_buf + G_AI_WIDTH * G_AI_HIGHT),		/* Green */
                     (uint32_t)(g_ai_buf + G_AI_WIDTH * G_AI_HIGHT * 2));	/* Blue */

    /* lcd display address setting */
    /* LCD表示用RAMアドレス設定 */
    dvp_set_display_addr( (uint32_t)g_lcd_ram );

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

    g_dvp_finish_flag = 0;  /* カメラのフレーム終了割り込み時に１になる*/
    g_fft_finish_flag = 0;
    dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);

    while (1)
    {

        /* dvp finish*/
        /* カメラのフレーム終了を待つ*/
        while (g_dvp_finish_flag == 0)
            ;

     for (j=0;j<QVGA_HIGHT;j++)
     {
        for ( i = 0; i < 320; i++) {
              g_fft_inp_ram[i] = g_lcd_ram[QVGA_WIDTH*j + i];
        }
        for ( i = 320; i < FFT_N; i++) {
              g_fft_inp_ram[i] = 0;
        }

        for ( i = 0; i < FFT_N / 2; ++i)
        {
            input_data = (fft_data_t *)&buff_input[i];
            if(i < 160 ){
            input_data->R1 = g_fft_inp_ram[2*i];
            //input_data->R1 = g_lcd_ram[2*i];
            input_data->I1 = 0;             
            input_data->R2 = g_fft_inp_ram[2*i+1]; 
            //input_data->R2 = g_lcd_ram[2*i+1]; 
            input_data->I2 = 0;             
            } else{
            input_data->R1 = 0;
            input_data->I1 = 0;             
            input_data->R2 = 0;
            input_data->I2 = 0;             
            }
        }

        fft_complex_uint16_dma(DMAC_CHANNEL0, DMAC_CHANNEL1, FFT_FORWARD_SHIFT, FFT_DIR_FORWARD, buff_input, FFT_N, buff_output);

        for ( i = 0; i < FFT_N / 2; i++)
        {
            output_data = (fft_data_t*)&buff_output[i];
            data_hard[2 * i].imag = output_data->I1 ;
            data_hard[2 * i].real = output_data->R1 ;
            data_hard[2 * i + 1].imag = output_data->I2 ;
            data_hard[2 * i + 1].real = output_data->R2 ;
        }

        for (i = 0; i < 320; i++)
        {
            hardpower[i] = sqrt(data_hard[i].real * data_hard[i].real + data_hard[i].imag * data_hard[i].imag);

            //Convert to dBFS
            hardpower[i] = 20*log(2*hardpower[i]/FFT_N);
            g_fft_out_ram[i] = hardpower[i];
        }

        lcd_draw_picture(0, 0, 320, 240, g_fft_out_ram);
        rt_thread_delay(RT_TICK_PER_SECOND/25);
        
        g_fft_finish_flag = 0;
        g_dvp_finish_flag = 0;
    } /* for (j=0;j<QVGA_HIGHT;j++) end  */
   } /* while end */

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
     //model_data = (uint8_t *)iomem_malloc(KMODEL_SIZE);
     length = dfs_file_read(&fd,model_data,KMODEL_SIZE);
     if(length>0)rt_kprintf("kpu3 Model data read OK(%d)\n",length);
     else rt_kprintf("kpu3 model data read NG(%d)\n",length);

     dfs_file_close(&fd);
     rt_kprintf("dfs_file_closed \n");

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

        printf("kpu_yolov3 stoped \n");

        }
}
//INIT_APP_EXPORT(kpu_v3);
MSH_CMD_EXPORT(kpu_v3,start kpu_v3 thread);
MSH_CMD_EXPORT(kpu_v3_stop,stop kpu_v3 thread);

