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
#include "sysctl.h"
#include "dmac.h"
#include "region_layer.h"

typedef uint8_t BYTE;

/* 日本語フォントファイル切り替え */
#ifndef _FONT_JP_KANA
//#define _FONT_JP_KANA 0
#define _FONT_JP_KANA 1
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

/* 38400(=320x240) の内訳 */
/* 320 X 120 X 32bit値 */
/* 320 X 240 X 16bit値 */
#define  GRAM_SIZE 240 * 320 / 2
#define  GRAM_BYTE_SIZE GRAM_SIZE * 4
uint32_t g_lcd_gram0[GRAM_SIZE] __attribute__((aligned(64)));
uint32_t g_lcd_gram1[GRAM_SIZE] __attribute__((aligned(64)));

/* kpu 識別種類数定義 */
#define CLASS_NUMBER 20

/* 各種フラッグ*/
static volatile uint32_t g_ai_done_flag;
static volatile uint8_t g_dvp_finish_flag;
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

static int RGB565Tab5[32] = {
    0, 8, 16, 25, 33, 41, 49, 58, 66, 74, 82, 90, 99, 107, 115, 123,
    132, 140, 148, 156, 165, 173, 181, 189, 197, 206, 214, 222, 230, 239, 247, 255
};

static int RGB565Tab6[64] = {
    0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 45, 49, 53, 57, 61, 65, 69, 73, 77, 81,
    85, 89, 93, 97, 101, 105, 109, 113, 117, 121, 125, 130, 134, 138, 142, 146, 150,
    154, 158, 162, 166, 170, 174, 178, 182, 186, 190, 194, 198, 202, 206, 210, 215,
    219, 223, 227, 231, 235, 239, 243, 247, 251, 255
};


#define IMABS(a)     (float){ ( sqrt( (a).real * (a).real + (a).imag * (a).imag ) )  }


/* RGB565を8Bitモノクロ値に変換*/
#define MAX_LV        255
int16_t rgb5652gray(uint16_t rgb565 ){
BYTE rgbRed,rgbGreen,rgbBlue;
float r,g,b;
float fgray;
BYTE gray;
    rgbRed   = RGB565Tab5[(rgb565 & 0xF800) >> 11];
    rgbGreen = RGB565Tab6[(rgb565 & 0x07E0) >> 5];
    rgbBlue  = RGB565Tab5[(rgb565 & 0x001F)];
    r = rgbRed;
    g = rgbGreen;
    b = rgbBlue;
    fgray = r*0.299+g*0.587+b*0.144;
    gray = rint(fgray);
    //printf("rgb5652gray:r=%f g=%f b=%f r=[%d] g=[%d] b=[%d] gray=[%d]\n",r,g,b,rgbRed,rgbGreen,rgbBlue,gray);
    if ( gray > MAX_LV ) gray = MAX_LV;

    return gray ;
}


static void ai_done(void *ctx) { g_ai_done_flag= 1; }

static int dvp_irq(void *ctx) {
    rt_enter_critical();
    if (dvp_get_interrupt(DVP_STS_FRAME_FINISH)) {

        /* switch gram */
        /* 条件演算子 g_ram_muxがTRUEならg_lcd_gram1[0]を使うFALSEならg_lcd_gram0[0] */
        dvp_set_display_addr(g_ram_mux ? (uint32_t)&g_lcd_gram0[0] : (uint32_t)&g_lcd_gram1[0]);
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

#if   defined(CAM_USING_OV2640)

     dvp_init(8);
     dvp_set_xclk_rate(24000000);
     dvp_enable_burst();
     dvp_set_output_enable(DVP_OUTPUT_DISPLAY, 1);
     dvp_set_output_enable(DVP_OUTPUT_AI, 1);
     dvp_set_image_format(DVP_CFG_RGB_FORMAT);
     dvp_set_image_size(OV_WIDTH,OV_HIGHT);
     ov2640_init();

#elif defined(CAM_USING_OV5640)
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

#endif

    /* ai address setting: ai bufferとはkpu演算用RAMバッファ */
    /* カメラ画像をaiバッファに取り込む設定実施*/
    dvp_set_ai_addr( (uint32_t)g_ai_buf,                                        /* Red */
                     (uint32_t)(g_ai_buf + G_AI_WIDTH * G_AI_HIGHT),            /* Green */
                     (uint32_t)(g_ai_buf + G_AI_WIDTH * G_AI_HIGHT * 2) );      /* Blue */


    //g_ram_mux = FALSE;          /* 初期値 FALSE */
    //dvp_set_display_addr(g_ram_mux ? (uint32_t)&g_lcd_gram0[0] : (uint32_t)&g_lcd_gram1[0]);
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


        g_dvp_finish_flag = 0;

#if 1
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
        lcd_draw_picture(0, 0, QVGA_WIDTH, QVGA_HIGHT, g_ram_mux ? (uint32_t)&g_lcd_gram0[0] : (uint32_t)&g_lcd_gram1[0]);
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

        printf("kpu_yolov3 stoped \n");

        }
}
//INIT_APP_EXPORT(kpu_v3);
MSH_CMD_EXPORT(kpu_v3,start kpu_v3 thread);
MSH_CMD_EXPORT(kpu_v3_stop,stop kpu_v3 thread);

