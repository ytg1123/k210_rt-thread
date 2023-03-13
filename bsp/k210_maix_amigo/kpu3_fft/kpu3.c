#include <rtthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

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

static void fft_oneframe(uint32_t *fft_ramptr,uint32_t *outptr);
static void fft_func(uint32_t *inptr,uint32_t *outptr);

#define OV_WIDTH   LCD_X_MAX
#define OV_HIGHT   LCD_Y_MAX
#define QVGA_WIDTH 320
#define QVGA_HIGHT 240

static kpuframebuffer_t _framebuffer0 = {RT_NULL,RT_NULL,RT_NULL};
kpuframebuffer_t *kfb = &_framebuffer0;

static rt_thread_t kpu3thread = RT_NULL;

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
#define FRAME_1ST           320
#define FFT_FORWARD_SHIFT   0x0U
static float    hard_power[FFT_N];
static uint64_t fft_out_data[FFT_N / 2];
static uint64_t buffer_input[FFT_N];
static uint64_t buffer_output[FFT_N];

/* 320 x 240 x 32bit(4byte) = 38400 */
static uint32_t g_lcd_gram[38400] __attribute__((aligned(64)));

#define CLASS_NUMBER 20

static volatile uint32_t g_ai_done_flag;
static volatile uint8_t g_dvp_finish_flag;
static volatile uint8_t g_fft_finish_flag;

#define KMODEL_SIZE (3104 * 1024)
static uint8_t *model_data;

// static image_t kpu_image, display_image;
static uint8_t  *g_ai_buf;

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

static int dvp_irq(void *ctx) {
    rt_enter_critical();
    if (dvp_get_interrupt(DVP_STS_FRAME_FINISH)) {
        /* switch gram */
        dvp_set_display_addr( (uint32_t)g_lcd_gram);
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
    if (x1 >= 320) x1= 319;
    if (x2 >= 320) x2= 319;
    if (y1 >= 240) y1= 239;
    if (y2 >= 240) y2= 239;

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

static void fft_oneframe(uint32_t *fft_ramptr,uint32_t *outptr)  {
/* for fft */
    int i=0,j=0;
        /* fft run*/
        g_fft_finish_flag=0;
        for (i=0;i<QVGA_HIGHT;i++){
            //fft_func(  (uint32_t)&fft_ramptr[QVGA_WIDTH*i] ,(uint32_t)&outptr[QVGA_WIDTH*i]);
            for (j=0;j<QVGA_WIDTH/2;j++){
              //fft_ramptr[QVGA_WIDTH*i+2*j]   =i;
              //fft_ramptr[QVGA_WIDTH*i+2*j+1] =i;
              outptr[QVGA_WIDTH*i+2*j]  = i;
              outptr[QVGA_WIDTH*i+2*j+1]= i;
            }
        }
        g_fft_finish_flag=1;
}

static void fft_func(uint32_t *inptr,uint32_t *outptr)  {
/* for fft */
    int i=0,j;
    complex_hard_t data_hard[FFT_N] = {0};
    fft_data_t *output_data;
    fft_data_t *input_data;

if(SKIPFFT==1) return;
        for ( i = 0; i < FFT_N / 2; ++i)
        {
          input_data = (fft_data_t *)&buffer_input[i];
          if( i < (FRAME_1ST)/2 ){
            input_data->R1 = inptr[2*i];              // data_hard[2 * i].real;
if (KPU3FFT_DEBUG == 1) printf("fft_func 0: i=%d 2i=%d : inptr=[%x] data=[%x] \n",i,2*i  ,(uint32_t)&inptr[2*i]  ,inptr[2*i]);
            input_data->I1 = 0;
            input_data->R2 = inptr[2*i+1];            // data_hard[2 * i + 1].real;
if (KPU3FFT_DEBUG == 1) printf("fft_func 1: i=%d 2i=%d : inptr=[%x] data=[%x] \n",i,2*i+1,(uint32_t)&inptr[2*i+1],inptr[2*i+1]);
            input_data->I2 = 0;
          } else {
            input_data->R1 = 0;
            input_data->I1 = 0;
            input_data->R2 = 0;
            input_data->I2 = 0;
          }
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
        for (i = 0,j=0; i < 320 ; i++)
        {
                outptr[j+0]  = hard_power[i];
                outptr[j+1]  = hard_power[i];
                outptr[j+2]  = hard_power[i];
                outptr[j+3]  = hard_power[i];
                outptr[j+4]  = hard_power[i];
                outptr[j+5]  = hard_power[i];
                outptr[j+6]  = hard_power[i];
                outptr[j+7]  = hard_power[i];
                outptr[j+8]  = hard_power[i];
                outptr[j+9]  = hard_power[i];
                outptr[j+10] = hard_power[i];
                outptr[j+11] = hard_power[i];
                outptr[j+12] = hard_power[i];
                outptr[j+13] = hard_power[i];
                outptr[j+14] = hard_power[i];
                outptr[j+15] = hard_power[i];
                outptr[j+16] = hard_power[i];
                outptr[j+17] = hard_power[i];
                outptr[j+18] = hard_power[i];
                outptr[j+19] = hard_power[i];
                j+=20;
        }
}


#define PLL0_OUTPUT_FREQ 800000000UL
#define PLL1_OUTPUT_FREQ 400000000UL
void kpu3_main(void *parameter) {
     int sta= 0;
     uint32_t ret,length;

    /* Set CPU and dvp clk */
    sysctl_pll_set_freq(SYSCTL_PLL0,PLL0_OUTPUT_FREQ );
    sysctl_pll_set_freq(SYSCTL_PLL1,PLL1_OUTPUT_FREQ );
    sysctl_clock_enable(SYSCTL_CLOCK_AI);

    uarths_init();
    io_set_power();
    io_mux_init();
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
    kfb->pix_buf0 = g_lcd_gram;
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

    /*lv Init */
    printf("Call lv_init...\n");
    lv_init();
    rt_thread_mdelay(10);
    disp_init();
    rt_thread_mdelay(10);

    //lcd_draw_picture(0, 0, 320, 240, rgb_image);
    //lcd_set_direction(DIR_YX_RLDU); /* left up 0,0 */
    //printf("show pic\n");
    //lv_tutorial_hello_world();
    //lv_test2();
    //printf("done lv_tutorial hello world \n");

    //while (1) {
    //lv_task_handler();
    //lv_tick_inc(1); 
    //}

    /* DVP Camera Init */
    init_cam();

    /* ai address setting */
    dvp_set_ai_addr( (uint32_t)g_ai_buf, 
                     (uint32_t)(g_ai_buf + 320 * 224),
                     (uint32_t)(g_ai_buf + 320 * 224 * 2));
    /* lcd display address setting */
    dvp_set_display_addr((uint32_t)g_lcd_gram );

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

    g_dvp_finish_flag = 0;
    g_fft_finish_flag = 0;
    dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);

    /* init kpu */
    /* init face detect model */
    rt_thread_mdelay(10);
    if (kpu_load_kmodel(&face_detect_task, model_data) != 0) {
        printf("\nmodel init error\n");
        while (1) ;
    }

    printf("model init ok\n");

    detect_rl0.anchor_number= ANCHOR_NUM;
    detect_rl0.anchor= layer0_anchor;
    detect_rl0.threshold= 0.6;
    detect_rl0.nms_value= 0.3;
    region_layer_init(&detect_rl0, 10, 7, 75, 320, 224);

    printf("region_layer_init rl0 ok\n");

//    detect_rl1.anchor_number= ANCHOR_NUM;
//    detect_rl1.anchor= layer1_anchor;
//    detect_rl1.threshold= 0.6;
//    detect_rl1.nms_value= 0.3;
//    region_layer_init(&detect_rl1, 20, 14, 75, 320, 224);
//    printf("region_layer_init rl1 ok\n");

    while (1)
    {

        /* dvp finish*/
        while (g_dvp_finish_flag == 0)
            ;

        /* run face detect */
        kpu_run_kmodel(&face_detect_task, g_ai_buf, DMAC_CHANNEL5, ai_done, NULL);
        while (!g_ai_done_flag);
        g_ai_done_flag= 0;

        float *output0, *output1;
        size_t output_size0, output_size1;

        // NOTE output_size 是字节， float 是4字节
        sta= kpu_get_output(&face_detect_task, 0, (uint8_t **)&output0, &output_size0);
        detect_rl0.input= output0;
        region_layer_run(&detect_rl0, NULL);

//        sta= kpu_get_output(&face_detect_task, 1, (uint8_t **)&output1, &output_size1);
//        detect_rl1.input= output1;
//        region_layer_run(&detect_rl1, NULL);

        g_fft_finish_flag = 0;
        lcd_draw_picture(0, 0, 320, 240, g_lcd_gram);
        g_dvp_finish_flag = 0;

        /* run key point detect */
        region_layer_draw_boxes(&detect_rl0, drawboxes);
//        region_layer_draw_boxes(&detect_rl1, drawboxes);
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

     g_ai_buf   = (uint8_t  *)iomem_malloc(QVGA_WIDTH * QVGA_HIGHT * 3);


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
        iomem_free(g_ai_buf);

        printf("kpu_yolov3 stoped \n");

        }
}
//INIT_APP_EXPORT(kpu_v3);
MSH_CMD_EXPORT(kpu_v3,start kpu_v3 thread);
MSH_CMD_EXPORT(kpu_v3_stop,stop kpu_v3 thread);

