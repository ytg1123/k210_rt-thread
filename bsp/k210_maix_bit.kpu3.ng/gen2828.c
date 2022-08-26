
// 将灰度图转换为RGB565 映射表
// Grayscale的取值是0-255, 在转换的时候把grayscale图像深度改为6bit
// 然后映射到64个RGB565的取值
static uint16_t gray2rgb565[64]={
0x0000, 0x0020, 0x0841, 0x0861, 0x1082, 0x10a2, 0x18c3, 0x18e3, 
0x2104, 0x2124, 0x2945, 0x2965, 0x3186, 0x31a6, 0x39c7, 0x39e7, 
0x4208, 0x4228, 0x4a49, 0x4a69, 0x528a, 0x52aa, 0x5acb, 0x5aeb, 
0x630c, 0x632c, 0x6b4d, 0x6b6d, 0x738e, 0x73ae, 0x7bcf, 0x7bef, 
0x8410, 0x8430, 0x8c51, 0x8c71, 0x9492, 0x94b2, 0x9cd3, 0x9cf3, 
0xa514, 0xa534, 0xad55, 0xad75, 0xb596, 0xb5b6, 0xbdd7, 0xbdf7, 
0xc618, 0xc638, 0xce59, 0xce79, 0xd69a, 0xd6ba, 0xdedb, 0xdefb, 
0xe71c, 0xe73c, 0xef5d, 0xef7d, 0xf79e, 0xf7be, 0xffdf, 0xffff, 
};

// RGB565转换为28x28的灰度图， 数值取值范围是0-255
// 在进行缩放的时候同时也做了图像预处理!!
// 修正了原来中间亮四周暗的问题
void gen_2828(void)
{
    int x,y,i,j,sum,r,g,b,gray,graymax;
    int sx2,sx,dx,ex,stdx,r2;
    int gate,dat;
    //224 to 28
    sx=0; sx2=0; graymax=0;

    for(y=0;y<28;y++)
        for(x=0;x<28;x++)
        {
            sum=0;
            // 这里又有一个8x8的for循环嵌套
            for(i=0;i<8;i++)
                for(j=0;j<8;j++)
                {   
                    //  这里的代码其实不太规范
                    // 244x244是图像尺寸，最好可以调用
                    r= g_ai_buf_in[y*224*8+j*224+x*8+i];
                    g= g_ai_buf_in[y*224*8+j*224+x*8+i+224*224];
                    b= g_ai_buf_in[y*224*8+j*224+x*8+i+224*224*2];
                    // ? 详细过程
                    gray = (r*76 + g*150 + b*30) >> 8;
                    sum+=gray;
                }
            // 这行语句其实同时做了两件事
            // sum/8/8 是做了一个均值处理,将原图的8*8的像素区域转换为1个像素的取值
            sum=255-(sum/8/8);  //invert
            // 更新graymax的数值
            if(sum>graymax)graymax=sum;
            // 赋值
            g_buf28[y*28+x]=sum;
            // sx 其实是sum_x的意思 
            // 将28x28灰度图的所有数值累加
            sx+=g_buf28[y*28+x];
            // 计算灰度图的平方和
            sx2+=((int)g_buf28[y*28+x]*(int)g_buf28[y*28+x]);
        }
    //strech
    // 计算灰度的平均值
    ex=sx/28/28;
    // 计算方差
    dx=sx2/28/28-(sx*sx/28/28/28/28);
    // 计算标准差
    stdx=(int)sqrt(dx);
    // 均值 
    gate=ex;//+stdx/8;
    // 打印一些统计信息
    printf("ex=%d,dx=%d,stdx=%d,gate=%d,graymax=%d\r\n",ex,dx,stdx,gate,graymax);
    // 修正
    // 原来的镜头存在中间亮四周暗的情况
    // 这里是图像预处理的部分

    for(y=0;y<28;y++)
        for(x=0;x<28;x++)
        {
            // 获取原来的像素值
            dat=g_buf28[y*28+x];
            // 直方图均衡化
            // 因为反色后,背景是黑色灰度值为0,所以得到的均值是
            // 特别小的数值，这里直接将gate阈值设置为了均值
            // 小于这个值的也被当做背景
            // 将原来的灰度图的最大值放大为255
            // TODO: 这里其实有bug, 如果画面
            // 可以给graymax做个阈值限制
            dat=(dat-gate)*255/(graymax-gate);
            // 如果小于零, dat置为0
            // 如果大于255就设置为 255
            dat=dat<0?0:(dat>255?255:dat);
            // 该像素点,距离28x28图像中心的半径的平方
            r2=(x-14)*(x-14)+(y-14)*(y-14);
            // 另外,需要注意的就是这个28x28的图像四周暗，然后这个图像又经历了反色
            // 也就是这个灰度图四周的像素点偏大了
            // 程序就是在这里通过一个四次函数 dat = dat/(1+2*(r/14)^4),做了一下修正
            // 注: 这里2相当于一个比例系数，可以调节,数值越大,这种修正的强度也就越大。
            dat=(int)(dat/(1.0+2*r2*r2/14.0/14.0/14.0/14.0));  
            // 将计算得到的新的dat赋值给原来的28x28的图像。
            g_buf28[y*28+x]=dat;
        }
}

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
#include <stdio.h>
#include "kpu.h"
#include <platform.h>
#include <printf.h>
#include <string.h>
#include <stdlib.h>
#include <sysctl.h>
#include "nt35310.h"
#include "lcd.h"
#include "dvp.h"
#include "ov2640.h"
#include "uarths.h"
#include "encoding.h"
#include "gpiohs.h"
#include "sleep.h"
#include "fpioa.h"

#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX

#include "incbin.h"
#include "w25qxx.h"
#include "math.h"

// 是否是从Flash载入模型
// 定义了即为从Flash中载入
#define LOAD_FROM_FLASH 0

#define PLL0_OUTPUT_FREQ 800000000UL
#define PLL1_OUTPUT_FREQ 400000000UL
#define PLL2_OUTPUT_FREQ 45158400UL

// 采集原始图片的尺寸
#define PIC_W 224
#define PIC_H 224

// KMODEL模型计算完成标志位
volatile uint32_t g_ai_done_flag;
// DVP 拍照结束标志位
volatile uint8_t g_dvp_finish_flag = 0;

// **的原始图片, RGB565格式, 所以是用uint16_t存储每个像素
uint16_t g_lcd_buf[PIC_W*PIC_H];

// 这里没有用到这个变量 注释掉
// extern const char* mbnet_label_name[];

// 模型输入图片
uint8_t g_buf28[28*28];
// ?
uint8_t g_ai_buf_in[PIC_W * PIC_H *3] __attribute__((aligned(128)));
// 定义kmodel模型
kpu_model_context_t task1;

#if LOAD_FROM_FLASH
// KModel的尺寸
// 每个Kmodel都不一样，查看kmodel模型转换日志
#define KMODEL_SIZE (4220 * 1024)
// 模型地址位
#define MODEL_ADDRESS (0x600000)
uint8_t model_data[KMODEL_SIZE] __attribute__((aligned(128)));

#else
//还是把二进制模型编译到代码里？
// Include Binary File 
INCBIN(model, "m.kmodel");
#endif

// 模型输出
float *features;
// 无效变量，注释掉
// uint16_t label_index[1000];

// 输出日志的字符数组
char content[50];

// 用于记录时间
// 帧率计算用
unsigned long t0,t1;

/* KModel前向计算的回调函数　*/
static void ai_done(void* userdata)
{
    // 模型前向计算完毕之后设置标志位?
    g_ai_done_flag = 1;

}

/* 
  DVP的中断处理函数 
  图像开始采集以及图像采集结束
*/
static int on_irq_dvp(void* ctx)
{
    // 查看当前的DVP Status 是不是Frame采集结束
    if (dvp_get_interrupt(DVP_STS_FRAME_FINISH))
    {
        // 清除这个中断标志
        dvp_clear_interrupt(DVP_STS_FRAME_FINISH);
        // 设置
        g_dvp_finish_flag = 1;
    }
    else
    {
        if(g_dvp_finish_flag == 0)
            // DVP开始采集图像
            dvp_start_convert();
        // 清除Frame start中断标志
        dvp_clear_interrupt(DVP_STS_FRAME_START);
    }

    return 0;
}

/* 管脚功能设置*/
static void io_mux_init(void)
{   
    /* Init DVP IO map and function settings */
    /* DVP管脚设置 */
    fpioa_set_function(42, FUNC_CMOS_RST);
    fpioa_set_function(44, FUNC_CMOS_PWDN);
    fpioa_set_function(46, FUNC_CMOS_XCLK);
    fpioa_set_function(43, FUNC_CMOS_VSYNC);
    fpioa_set_function(45, FUNC_CMOS_HREF);
    fpioa_set_function(47, FUNC_CMOS_PCLK);
    fpioa_set_function(41, FUNC_SCCB_SCLK);
    fpioa_set_function(40, FUNC_SCCB_SDA);

    /* Init SPI IO map and function settings */
    /* 初始化SPI的IO管脚映射, LCD液晶屏需要用到*/
    fpioa_set_function(37, FUNC_GPIOHS0 + RST_GPIONUM);
    fpioa_set_function(38, FUNC_GPIOHS0 + DCX_GPIONUM);
    fpioa_set_function(36, FUNC_SPI0_SS3);
    fpioa_set_function(39, FUNC_SPI0_SCLK);

    sysctl_set_spi0_dvp_data(1);
}

/* 设置电源模式 */
static void io_set_power(void)
{
    /* Set dvp and spi pin to 1.8V */   
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
}


// 将灰度图转换为RGB565 映射表
// Grayscale的取值是0-255, 在转换的时候把grayscale图像深度改为6bit
// 然后映射到64个RGB565的取值
static uint16_t gray2rgb565[64]={
0x0000, 0x0020, 0x0841, 0x0861, 0x1082, 0x10a2, 0x18c3, 0x18e3, 
0x2104, 0x2124, 0x2945, 0x2965, 0x3186, 0x31a6, 0x39c7, 0x39e7, 
0x4208, 0x4228, 0x4a49, 0x4a69, 0x528a, 0x52aa, 0x5acb, 0x5aeb, 
0x630c, 0x632c, 0x6b4d, 0x6b6d, 0x738e, 0x73ae, 0x7bcf, 0x7bef, 
0x8410, 0x8430, 0x8c51, 0x8c71, 0x9492, 0x94b2, 0x9cd3, 0x9cf3, 
0xa514, 0xa534, 0xad55, 0xad75, 0xb596, 0xb5b6, 0xbdd7, 0xbdf7, 
0xc618, 0xc638, 0xce59, 0xce79, 0xd69a, 0xd6ba, 0xdedb, 0xdefb, 
0xe71c, 0xe73c, 0xef5d, 0xef7d, 0xf79e, 0xf7be, 0xffdf, 0xffff, 
};

// 空白图片，用于填写分类信息，显示在LCD上
volatile uint16_t info_pic[96*92];
// 28x28的输入图 RGB565格式,用于LCD显示
volatile uint16_t pic_2828[28*28];

// 时间，未用到
// static int times=0;

// 显示识别结果到LCD液晶屏
static void show_result(void)
{
    // **当前的时间
    t1=read_cycle();
    // 输出结果
    float *features;
    size_t count;
    int i;
    float pmax;
    const char* name;
    int i_max;

    // 在LCD的左上部分显示摄像头采集的画面
    lcd_draw_picture(0, 0, PIC_W, PIC_H, g_lcd_buf);
    // **KPU的输出 输出保存在features里面
    kpu_get_output(&task1, 0, (uint8_t **)&features, &count);
    // 看结果有多少个数值
    count /= sizeof(float);
    for (i = 0; i < count; i++)
    {
        if (i % 16 == 0)
            printf("\r\n%04x: ",i);
        printf("%.3f ",features[i]);
    }

    // 找到输出里面概率最大的label id
    pmax=0;
    i_max=0;
    for (i = 0; i < count; i++)
    {
        if(features[i]>pmax) 
        {
            pmax=features[i];
            i_max = i;
        }
    }
    printf("\r\n");

    // 遍历所有的像素
    for(i=0; i<28*28;i+=2)
    {   
        // 把每个灰度图转换为RGB565
        pic_2828[i]=gray2rgb565[g_buf28[i+1]/4];
        pic_2828[i+1]=gray2rgb565[g_buf28[i]/4];
    }

    printf("\r\ni_max=%d, pmax=%.3f", i_max, features[i_max]);
    // 输出结果日志
    snprintf(content, 100, "%d: %1.3f", i_max, features[i_max]);
    content[12]=0;
    // 创建一个空白的图
    memset(info_pic,0x00,sizeof(info_pic));
    // 在信息图上绘制分类信息
    lcd_ram_draw_string(content, info_pic, WHITE, BLACK,96);
    // 显示信息图
    lcd_draw_picture(224, 0, 96, 16, info_pic);
    // 绘制二值化的28x28的缩略图 输入数据
    lcd_draw_picture(224, 16, 28, 28, pic_2828);
}

// RGB565转换为28x28的灰度图， 数值取值范围是0-255
// 在进行缩放的时候同时也做了图像预处理!!
// 修正了原来中间亮四周暗的问题
void gen_2828(void)
{
    int x,y,i,j,sum,r,g,b,gray,graymax;
    int sx2,sx,dx,ex,stdx,r2;
    int gate,dat;
    //224 to 28
    sx=0; sx2=0; graymax=0;

    for(y=0;y<28;y++)
        for(x=0;x<28;x++)
        {
            sum=0;
            // 这里又有一个8x8的for循环嵌套
            for(i=0;i<8;i++)
                for(j=0;j<8;j++)
                {   
                    //  这里的代码其实不太规范
                    // 244x244是图像尺寸，最好可以调用
                    r= g_ai_buf_in[y*224*8+j*224+x*8+i];
                    g= g_ai_buf_in[y*224*8+j*224+x*8+i+224*224];
                    b= g_ai_buf_in[y*224*8+j*224+x*8+i+224*224*2];
                    // ? 详细过程
                    gray = (r*76 + g*150 + b*30) >> 8;
                    sum+=gray;
                }
            // 这行语句其实同时做了两件事
            // sum/8/8 是做了一个均值处理,将原图的8*8的像素区域转换为1个像素的取值
            sum=255-(sum/8/8);  //invert
            // 更新graymax的数值
            if(sum>graymax)graymax=sum;
            // 赋值
            g_buf28[y*28+x]=sum;
            // sx 其实是sum_x的意思 
            // 将28x28灰度图的所有数值累加
            sx+=g_buf28[y*28+x];
            // 计算灰度图的平方和
            sx2+=((int)g_buf28[y*28+x]*(int)g_buf28[y*28+x]);
        }
    //strech
    // 计算灰度的平均值
    ex=sx/28/28;
    // 计算方差
    dx=sx2/28/28-(sx*sx/28/28/28/28);
    // 计算标准差
    stdx=(int)sqrt(dx);
    // 均值 
    gate=ex;//+stdx/8;
    // **一些统计信息
    printf("ex=%d,dx=%d,stdx=%d,gate=%d,graymax=%d\r\n",ex,dx,stdx,gate,graymax);
    // 修正
    // 原来的镜头存在中间亮四周暗的情况
    // 这里是图像预处理的部分

    for(y=0;y<28;y++)
        for(x=0;x<28;x++)
        {
            // **原来的像素值
            dat=g_buf28[y*28+x];
            // 直方图均衡化
            // 因为反色后,背景是黑色灰度值为0,所以得到的均值是
            // 特别小的数值，这里直接将gate阈值设置为了均值
            // 小于这个值的也被当做背景
            // 将原来的灰度图的最大值放大为255
            // TODO: 这里其实有bug, 如果画面
            // 可以给graymax做个阈值限制
            dat=(dat-gate)*255/(graymax-gate);
            // 如果小于零, dat置为0
            // 如果大于255就设置为 255
            dat=dat<0?0:(dat>255?255:dat);
            // 该像素点,距离28x28图像中心的半径的平方
            r2=(x-14)*(x-14)+(y-14)*(y-14);
            // 另外,需要注意的就是这个28x28的图像四周暗，然后这个图像又经历了反色
            // 也就是这个灰度图四周的像素点偏大了
            // 程序就是在这里通过一个四次函数 dat = dat/(1+2*(r/14)^4),做了一下修正
            // 注: 这里2相当于一个比例系数，可以调节,数值越大,这种修正的强度也就越大。
            dat=(int)(dat/(1.0+2*r2*r2/14.0/14.0/14.0/14.0));  
            // 将计算得到的新的dat赋值给原来的28x28的图像。
            g_buf28[y*28+x]=dat;
        }
}


int main()
{
    int i;
    fpioa_set_function(8, FUNC_GPIOHS0);
    gpiohs_set_drive_mode(0, GPIO_DM_OUTPUT);
    gpiohs_set_pin(0, GPIO_PV_LOW);
    /* Set CPU and dvp clk */
    sysctl_pll_set_freq(SYSCTL_PLL0, PLL0_OUTPUT_FREQ);
    sysctl_pll_set_freq(SYSCTL_PLL1, PLL1_OUTPUT_FREQ);
    //sysctl_pll_set_freq(SYSCTL_PLL2, PLL2_OUTPUT_FREQ);
    // 初始化系统时钟
    sysctl_clock_enable(SYSCTL_CLOCK_AI);
    // 初始化UART
    uarths_init();
    io_mux_init();
    io_set_power();
    plic_init();

    /* LCD init*/
    printf("LCD init\r\n");
    // LCD初始化
    lcd_init();
    // 设置LCD的显示方向
    lcd_set_direction(DIR_YX_RLDU);
    // 黑屏
    lcd_clear(BLACK);

    /* DVP init */
    printf("DVP init\r\n");
    dvp_init(8);
    dvp_set_xclk_rate(24000000);
    dvp_enable_burst();
    // 设置允许导流到AI模型
    dvp_set_output_enable(0, 1);    //enable to ai
    // DVP不直接导流到LCD
    dvp_set_output_enable(1, 1);    //disable to lcd
    // 设置图像格式为RGB格式
    dvp_set_image_format(DVP_CFG_RGB_FORMAT);
    // 设置图像尺寸
    dvp_set_image_size(PIC_W, PIC_H);
    // 初始化摄像头
    ov2640_init();

    dvp_set_ai_addr((uint32_t)g_ai_buf_in, (uint32_t)(g_ai_buf_in + PIC_W * PIC_H), (uint32_t)(g_ai_buf_in + PIC_W * PIC_H * 2));
    dvp_set_display_addr((uint32_t)g_lcd_buf);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
    dvp_disable_auto();
    /* DVP interrupt config */
    printf("DVP interrupt config\r\n");
    plic_set_priority(IRQN_DVP_INTERRUPT, 1);
    // 设置plic irq的中断处理函数为on_irq_dvp
    plic_irq_register(IRQN_DVP_INTERRUPT, on_irq_dvp, NULL);
    plic_irq_enable(IRQN_DVP_INTERRUPT);

    #if LOAD_FROM_FLASH
    // 载入Kmodel模型信息 
    // MODEL_ADDRESS 模型地址信息
    w25qxx_init(3, 0);
    w25qxx_enable_quad_mode();
    w25qxx_read_data(MODEL_ADDRESS, model_data, KMODEL_SIZE, W25QXX_QUAD_FAST);
    printf("load model from flash ok!\r\n");
    #endif

    // 载入kmodel模型
    if (kpu_load_kmodel(&task1, model_data) != 0)
    {
        printf("Cannot load kmodel.\n");
        exit(-1);
    }
    /* enable global interrupt */
    // 全局允许中断
    sysctl_enable_irq();
    /* system start */
    printf("system start\r\n");
    // DVP图像采集完成标志位 
    g_dvp_finish_flag = 0;
    dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);

    while(1)
    {
        // 等待直到完成图像采集
        while (g_dvp_finish_flag == 0){};
        // 将RGB565的彩图转换为28x28的灰度图，数值取值范围是0-255
        gen_2828();

        g_ai_done_flag = 0;
        // 开始计时
        t0=read_cycle();

        // KPU运行kmodel模型
        // ai_done是kpu_run_kmodel的回调函数， ai_done会将g_ai_done_flag置为1
        if (kpu_run_kmodel(&task1, g_buf28, 5, ai_done, NULL) != 0)
        {
            printf("Cannot run kmodel.\n");
            exit(-1);
        }
        // 死循环，直到模型计算结束
        while (!g_ai_done_flag);
        // 清空标志位
        g_ai_done_flag=0;
        g_dvp_finish_flag = 0;
        // 显示可视化结果
        show_result();
    }  
}

