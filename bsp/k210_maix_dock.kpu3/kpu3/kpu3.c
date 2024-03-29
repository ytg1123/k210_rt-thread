#include <rtthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "drv_ov2640.h"
#include "dvp.h"
#include "plic.h"
#include "fpioa.h"
#include "lcd.h"
#include "board_config.h"

#define OV_WIDTH   LCD_X_MAX
#define OV_HIGHT   LCD_Y_MAX

static kpuframebuffer_t _framebuffer0 = {RT_NULL,RT_NULL,RT_NULL,RT_NULL};
kpuframebuffer_t *kfb = &_framebuffer0;

#include "sysctl.h"
#include "uarths.h"
#include "nt35310.h"
#include "utils.h"
#include "kpu.h"
#include "region_layer.h"
#include "image_process.h"

static rt_thread_t kpu3thread = RT_NULL;

#if defined(RT_USING_SPI_MSD) && defined(RT_USING_DFS_ELMFAT)
#include <spi_msd.h>
#include <dfs_fs.h>
#include <dfs_file.h>
#include <dfs_private.h>
#endif

#define CLASS_NUMBER 20

static volatile uint32_t g_ai_done_flag;
static volatile uint8_t g_dvp_finish_flag;
static volatile uint8_t g_ram_mux = 0;

#define KMODEL_SIZE (3104 * 1024)
//#define KMODEL_SIZE (1800 * 1024)
//uint8_t model_data[KMODEL_SIZE];
//uint8_t *model_data_align = model_data;

static uint8_t *model_data;

// static image_t kpu_image, display_image;
static uint32_t *g_lcd_gram0;
static uint32_t *g_lcd_gram1;
static uint8_t *g_ai_buf;

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
        dvp_set_display_addr(g_ram_mux ? (uint32_t)g_lcd_gram0 : (uint32_t)g_lcd_gram1);
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
    char *str;
    uint16_t color;
    uint16_t height;
    uint16_t width;
    uint32_t *ptr;
} class_lable_t;

class_lable_t class_lable[CLASS_NUMBER]= {
    {"aeroplane", GREEN}, {"bicycle", GREEN},   {"bird", GREEN},        {"boat", GREEN},
    {"bottle", 0xF81F},   {"bus", GREEN},       {"car", GREEN},         {"cat", GREEN},
    {"chair", 0xFD20},    {"cow", GREEN},       {"diningtable", GREEN}, {"dog", GREEN},
    {"horse", GREEN},     {"motorbike", GREEN}, {"person", 0xF800},     {"pottedplant", GREEN},
    {"sheep", GREEN},     {"sofa", GREEN},      {"train", GREEN},       {"tvmonitor", 0xF9B6}};

static uint32_t lable_string_draw_ram[115 * 16 * 8 / 2];
#endif

static void lable_init(void) {
#if (CLASS_NUMBER > 1)
    uint8_t index;

    class_lable[0].height= 16;
    class_lable[0].width= 8 * strlen(class_lable[0].str);
    class_lable[0].ptr= lable_string_draw_ram;
    lcd_ram_draw_string(class_lable[0].str, class_lable[0].ptr, BLACK, class_lable[0].color);
    for (index= 1; index < CLASS_NUMBER; index++) {
        class_lable[index].height= 16;
        class_lable[index].width= 8 * strlen(class_lable[index].str);
        class_lable[index].ptr= class_lable[index - 1].ptr +
                                class_lable[index - 1].height * class_lable[index - 1].width / 2;
        lcd_ram_draw_string(class_lable[index].str, class_lable[index].ptr, BLACK,
                            class_lable[index].color);
    }
#endif
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

void kpu3_main(void *parameter) {
     int sta= 0;
     uint32_t ret,length;

     /* Set CPU and dvp clk */
     sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
     sysctl_pll_set_freq(SYSCTL_PLL1, 400000000UL);
     //sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
     sysctl_clock_enable(SYSCTL_CLOCK_AI);

     uarths_init();
     io_set_power();
     io_mux_init();
     plic_init();

     printf("kpu3_main start\n");

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

     /* DVP init */
         printf("DVP init\n");

     /*init dvp hardware interface*/
     dvp_init(8);
     dvp_set_xclk_rate(24000000);
     dvp_enable_burst();
     dvp_set_output_enable(DVP_OUTPUT_DISPLAY, 1);
     dvp_set_output_enable(DVP_OUTPUT_AI, 1);
     dvp_set_image_format(DVP_CFG_RGB_FORMAT);
     dvp_set_image_size(OV_WIDTH,OV_HIGHT);
     ov2640_init();
     //dvp_sccb_set_clk_rate(500000);

     init_cam();

     dvp_set_ai_addr((uint32_t)g_ai_buf, (uint32_t)(g_ai_buf + 320 * 224),
                     (uint32_t)(g_ai_buf + 320 * 224 * 2));
     dvp_set_display_addr((uint32_t)g_lcd_gram0 );

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
     printf("System start\n");
     g_ram_mux = 0;
     g_dvp_finish_flag = 0;
     dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
     dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);

    /* init kpu */
    /* init face detect model */
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

    detect_rl1.anchor_number= ANCHOR_NUM;
    detect_rl1.anchor= layer1_anchor;
    detect_rl1.threshold= 0.6;
    detect_rl1.nms_value= 0.3;
    region_layer_init(&detect_rl1, 20, 14, 75, 320, 224);

    printf("region_layer_init rl1 ok\n");

#if BOARD_LICHEEDAN == 1
    lcd_set_direction(DIR_YX_RLUD);
#else
    lcd_set_direction(DIR_YX_LRDU);
#endif

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

        sta= kpu_get_output(&face_detect_task, 1, (uint8_t **)&output1, &output_size1);
        detect_rl1.input= output1;
        region_layer_run(&detect_rl1, NULL);

        /* display result */
        g_ram_mux ^= 0x01;
        lcd_draw_picture(0, 0, 320, 240, g_ram_mux ? g_lcd_gram0 : g_lcd_gram1);
        g_dvp_finish_flag = 0;

        /* run key point detect */
        region_layer_draw_boxes(&detect_rl0, drawboxes);
        region_layer_draw_boxes(&detect_rl1, drawboxes);
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

     g_lcd_gram0 = (uint32_t *)iomem_malloc(320 * 240 * 2);
     g_lcd_gram1 = (uint32_t *)iomem_malloc(320 * 240 * 2);
     g_ai_buf = (uint8_t *)iomem_malloc(320 * 240 * 3);

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
        iomem_free(g_lcd_gram0);
        iomem_free(g_lcd_gram1);
        iomem_free(g_ai_buf);

        printf("kpu_yolov3 stoped \n");

        }
}

//INIT_APP_EXPORT(kpu_v3);
MSH_CMD_EXPORT(kpu_v3,start kpu_v3 thread);
MSH_CMD_EXPORT(kpu_v3_stop,stop kpu_v3 thread);


