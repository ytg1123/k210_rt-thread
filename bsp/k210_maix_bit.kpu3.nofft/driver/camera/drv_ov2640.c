#include "rtthread.h"
#ifdef CAM_USING_OV2640
#include "ov2640reg.h"
#include "ov2640cfg.h"
#include "drv_ov2640.h"
#include "dvp.h"
#include "rtdbg.h"

static const uint8_t yuv422_regs[][2] = {
        { BANK_SEL, BANK_SEL_DSP },
        { RESET,   RESET_DVP},
        { 0xD7,     0x01 },
        { IMAGE_MODE, IMAGE_MODE_YUV422 },
        { 0xE1,     0x67 },
        { RESET,    0x00 },
        {0, 0},
};

static const uint8_t rgb565_regs[][2] = {
        { BANK_SEL,   BANK_SEL_DSP },
        { RESET,      RESET_DVP},
        { 0xD7,       0x03},
        { IMAGE_MODE, IMAGE_MODE_RGB565 },
        { 0xE1,       0x77 },
        { RESET,      0x00 },
        {0,           0},
};

static const uint8_t jpeg_regs[][2] = {
        { BANK_SEL, BANK_SEL_DSP },
        { RESET,   RESET_DVP},
        { IMAGE_MODE, IMAGE_MODE_JPEG_EN|IMAGE_MODE_RGB565 },
        { 0xD7,     0x03 },
        { 0xE1,     0x77 },
        { QS,       0x0C },
        { RESET,    0x00 },
        {0, 0},
};

void ov2640_set_pixformat(pixformat_t pixformat)
{
    uint16_t i=0;
    const uint8_t (*regs)[2]=NULL;

    /* read pixel format reg */
    switch (pixformat) {
        case PIXFORMAT_RGB565:
            regs = rgb565_regs;
            break;
        case PIXFORMAT_YUV422:
            regs = yuv422_regs;
            break;
        case PIXFORMAT_GRAYSCALE:
            LOG_E("pixformat not supported");
            break;
        case PIXFORMAT_JPEG:
            regs = jpeg_regs;
            break;
        default:
            LOG_E("pixformat not supported");
    }

    /* Write initial regsiters */
    while (regs[i][0]) {
        dvp_sccb_send_data(OV2640_ADDR, regs[i][0], regs[i][1]);
        i++;
    }
    /* delay n ms */
    rt_thread_mdelay(30);
    return;
}

uint8_t ov2640_init(void)
{
    uint16_t i=0;
    const uint8_t (*regs)[2];
    uint16_t reg = 0;
   
    /* Reset all registers */
    dvp_sccb_send_data(OV2640_ADDR, BANK_SEL, BANK_SEL_SENSOR);
    dvp_sccb_send_data(OV2640_ADDR, COM7, COM7_SRST);

    /* delay n ms */
    rt_thread_mdelay(10);

    /* read sensor id */
    reg = dvp_sccb_receive_data(OV2640_ADDR,REG_PID);
    reg <<= 8;
    reg |= dvp_sccb_receive_data(OV2640_ADDR,REG_VER);

    if(reg != OV2640_ID)
    {
        LOG_E("sensor ID: 0x%4x This is not ov2640!",reg);
        return 1;
    }else
    {
        LOG_I("ov2640 found! Sensor ID: 0x%4x",reg);
    }
    i = 0;
    regs = ov2640_default;//default_regs,ov2640_default
    /* Write initial regsiters */
    while (regs[i][0]) {
        dvp_sccb_send_data(OV2640_ADDR, regs[i][0], regs[i][1]);
        i++;
    }
    
    i = 0;
    regs = svga_config;
    /* Write initial regsiters */
    while (regs[i][0]) {
        dvp_sccb_send_data(OV2640_ADDR, regs[i][0], regs[i][1]);
        i++;
    }    

//    ov2640_set_image_win(0,0,800,600);
    return 0;
}

void ov2640_set_framesize(framesize_t framesize)
{
    uint8_t clkrc;
    uint16_t w = cam_resolution[framesize][0];
    uint16_t h = cam_resolution[framesize][1];

    int i=0;
    const uint8_t (*regs)[2];

    if ((w <= 800) && (h <= 600)) {
        clkrc =0x80;
        regs = svga_config;
//		regs = ov2640_config;
    } else {
        clkrc =0x81;
        regs = uxga_regs;
    }

    /* Disable DSP */
    dvp_sccb_send_data(OV2640_ADDR, BANK_SEL, BANK_SEL_DSP);
    dvp_sccb_send_data(OV2640_ADDR, R_BYPASS, R_BYPASS_DSP_BYPAS);

    /* Set CLKRC */
	if(clkrc == 0x81)
	{
	    dvp_sccb_send_data(OV2640_ADDR, BANK_SEL, BANK_SEL_SENSOR);
	    dvp_sccb_send_data(OV2640_ADDR, CLKRC, clkrc);
	}

    /* Write DSP input regsiters */
    while (regs[i][0]) {
        dvp_sccb_send_data(OV2640_ADDR, regs[i][0], regs[i][1]);
        i++;
    }
	 /* Write output width */
	dvp_sccb_send_data(OV2640_ADDR,0xe0,0x04 ); /* OUTH[8]/OUTW[9:8] */
    dvp_sccb_send_data(OV2640_ADDR, ZMOW, (w>>2)&0xFF); /* OUTW[7:0] (real/4) */
    dvp_sccb_send_data(OV2640_ADDR, ZMOH, (h>>2)&0xFF); /* OUTH[7:0] (real/4) */
    dvp_sccb_send_data(OV2640_ADDR, ZMHH, ((h>>8)&0x04)|((w>>10)&0x03)); /* OUTH[8]/OUTW[9:8] */
	dvp_sccb_send_data(OV2640_ADDR,0xe0,0x00 ); /* OUTH[8]/OUTW[9:8] */

    /* Enable DSP */
    dvp_sccb_send_data(OV2640_ADDR, BANK_SEL, BANK_SEL_DSP);
    dvp_sccb_send_data(OV2640_ADDR, R_BYPASS, R_BYPASS_DSP_EN);

    /* delay n ms */
    rt_thread_mdelay(30);
	dvp_set_image_size(w, h);
    return;
}

//返回0：设置成功
//其他：设置失败
uint8_t ov2640_set_image_win(uint16_t offx,uint16_t offy,uint16_t width,uint16_t height)
{
	uint16_t hsize;
	uint16_t vsize;
	uint8_t temp; 
	if(width%4)return 1;
	if(height%4)return 2;

	hsize=width/4;
	vsize=height/4;

    dvp_sccb_send_data(OV2640_ADDR,0XFF,0X00);
    dvp_sccb_send_data(OV2640_ADDR,0XE0,0X04);
    dvp_sccb_send_data(OV2640_ADDR,0X51,hsize&0XFF);    //设置H_SIZE的低8位
    dvp_sccb_send_data(OV2640_ADDR,0X52,vsize&0XFF);    //设置V_SIZE的低8位
    dvp_sccb_send_data(OV2640_ADDR,0X53,offx&0XFF);     //设置offx的低8位
    dvp_sccb_send_data(OV2640_ADDR,0X54,offy&0XFF);     //设置offy的低8位

	temp=(vsize>>1)&0X80;
	temp|=(offy>>4)&0X70;
	temp|=(hsize>>5)&0X08;
	temp|=(offx>>8)&0X07; 

    dvp_sccb_send_data(OV2640_ADDR,0X55,temp);          //设置H_SIZE/V_SIZE/OFFX,OFFY的高8位
    dvp_sccb_send_data(OV2640_ADDR,0X57,(hsize>>2)&0X80);//设置H_SIZE/V_SIZE/OFFX,OFFY的高8位
    dvp_sccb_send_data(OV2640_ADDR,0XE0,0X00);

    return 0;    
}

//自动曝光设置参数共有5个等级
const static uint8_t OV2640_AUTOEXPOSURE_LEVEL[5][8]=
{
	{
		0xFF,0x01,
		0x24,0x20,
		0x25,0x18,
		0x26,0x60,
	},
	{
		0xFF,0x01,
		0x24,0x34,
		0x25,0x1c,
		0x26,0x00,
	},
	{
		0xFF,0x01,	
		0x24,0x3e,	
		0x25,0x38,
		0x26,0x81,
	},
	{
		0xFF,0x01,
		0x24,0x48,
		0x25,0x40,
		0x26,0x81,
	},
	{
		0xFF,0x01,	
		0x24,0x58,	
		0x25,0x50,	
		0x26,0x92,	
	},
}; 
//OV2640自动曝光等级设置
//level:0~4
void ov2640_set_exposure(uint8_t level)
{  
	uint8_t i;
	uint8_t *p=(uint8_t*)OV2640_AUTOEXPOSURE_LEVEL[level];
	for(i=0;i<4;i++)
	{ 
		dvp_sccb_send_data(OV2640_ADDR,p[i*2],p[i*2+1]); 
	} 
}  

//白平衡设置
//0:自动
//1:太阳sunny
//2,阴天cloudy
//3,办公室office
//4,家里home
void ov2640_set_light(uint8_t mode)
{
	uint8_t regccval=0X5E;//Sunny 
	uint8_t regcdval=0X41;
	uint8_t regceval=0X54;
	switch(mode)
	{ 
		case 0://auto 
			dvp_sccb_send_data(OV2640_ADDR,0XFF,0X00);	 
			dvp_sccb_send_data(OV2640_ADDR,0XC7,0X10);//AWB ON 
			return;  	
		case 2://cloudy
			regccval=0X65;
			regcdval=0X41;
			regceval=0X4F;
			break;	
		case 3://office
			regccval=0X52;
			regcdval=0X41;
			regceval=0X66;
			break;	
		case 4://home
			regccval=0X42;
			regcdval=0X3F;
			regceval=0X71;
			break;	
	}
	dvp_sccb_send_data(OV2640_ADDR,0XFF,0X00);	 
	dvp_sccb_send_data(OV2640_ADDR,0XC7,0X40);	//AWB OFF 
	dvp_sccb_send_data(OV2640_ADDR,0XCC,regccval); 
	dvp_sccb_send_data(OV2640_ADDR,0XCD,regcdval); 
	dvp_sccb_send_data(OV2640_ADDR,0XCE,regceval);  
}
//色度设置
//0:-2
//1:-1
//2,0
//3,+1
//4,+2
void ov2640_set_saturation(uint8_t sat)
{ 
	uint8_t reg7dval=((sat+2)<<4)|0X08;
	dvp_sccb_send_data(OV2640_ADDR,0XFF,0X00);		
	dvp_sccb_send_data(OV2640_ADDR,0X7C,0X00);		
	dvp_sccb_send_data(OV2640_ADDR,0X7D,0X02);				
	dvp_sccb_send_data(OV2640_ADDR,0X7C,0X03);			
	dvp_sccb_send_data(OV2640_ADDR,0X7D,reg7dval);			
	dvp_sccb_send_data(OV2640_ADDR,0X7D,reg7dval); 		
}
//亮度设置
//0:(0X00)-2
//1:(0X10)-1
//2,(0X20) 0
//3,(0X30)+1
//4,(0X40)+2
void ov2640_set_brightness(uint8_t bright)
{
	dvp_sccb_send_data(OV2640_ADDR,0xff, 0x00);
	dvp_sccb_send_data(OV2640_ADDR,0x7c, 0x00);
	dvp_sccb_send_data(OV2640_ADDR,0x7d, 0x04);
	dvp_sccb_send_data(OV2640_ADDR,0x7c, 0x09);
	dvp_sccb_send_data(OV2640_ADDR,0x7d, bright<<4); 
	dvp_sccb_send_data(OV2640_ADDR,0x7d, 0x00); 
}
//对比度设置
//0:-2
//1:-1
//2,0
//3,+1
//4,+2
void ov2640_set_contrast(uint8_t contrast)
{
	uint8_t reg7d0val=0X20;
	uint8_t reg7d1val=0X20;
  	switch(contrast)
	{
		case 0://-2
			reg7d0val=0X18;	 	 
			reg7d1val=0X34;	 	 
			break;	
		case 1://-1
			reg7d0val=0X1C;	 	 
			reg7d1val=0X2A;	 	 
			break;	
		case 3://1
			reg7d0val=0X24;	 	 
			reg7d1val=0X16;	 	 
			break;	
		case 4://2
			reg7d0val=0X28;	 	 
			reg7d1val=0X0C;	 	 
			break;	
	}
	dvp_sccb_send_data(OV2640_ADDR,0xff,0x00);
	dvp_sccb_send_data(OV2640_ADDR,0x7c,0x00);
	dvp_sccb_send_data(OV2640_ADDR,0x7d,0x04);
	dvp_sccb_send_data(OV2640_ADDR,0x7c,0x07);
	dvp_sccb_send_data(OV2640_ADDR,0x7d,0x20);
	dvp_sccb_send_data(OV2640_ADDR,0x7d,reg7d0val);
	dvp_sccb_send_data(OV2640_ADDR,0x7d,reg7d1val);
	dvp_sccb_send_data(OV2640_ADDR,0x7d,0x06);
}
//特效设置
//0:普通模式 
//1,负片
//2,黑白   
//3,偏红
//4,偏绿
//5,偏蓝
//6,复古	    
void ov2640_set_special_effects(uint8_t eft)
{
	uint8_t reg7d0val=0X00;
	uint8_t reg7d1val=0X80;
	uint8_t reg7d2val=0X80; 
	switch(eft)
	{
		case 1://负片
			reg7d0val=0X40; 
			break;	
		case 2://黑白
			reg7d0val=0X18; 
			break;	 
		case 3://偏红
			reg7d0val=0X18; 
			reg7d1val=0X40;
			reg7d2val=0XC0; 
			break;	
		case 4://偏绿
			reg7d0val=0X18; 
			reg7d1val=0X40;
			reg7d2val=0X40; 
			break;	
		case 5://偏蓝
			reg7d0val=0X18; 
			reg7d1val=0XA0;
			reg7d2val=0X40; 
			break;	
		case 6://复古
			reg7d0val=0X18; 
			reg7d1val=0X40;
			reg7d2val=0XA6; 
			break;	 
	}
	dvp_sccb_send_data(OV2640_ADDR,0xff,0x00);
	dvp_sccb_send_data(OV2640_ADDR,0x7c,0x00);
	dvp_sccb_send_data(OV2640_ADDR,0x7d,reg7d0val);
	dvp_sccb_send_data(OV2640_ADDR,0x7c,0x05);
	dvp_sccb_send_data(OV2640_ADDR,0x7d,reg7d1val);
	dvp_sccb_send_data(OV2640_ADDR,0x7d,reg7d2val); 
}

//彩条测试
//sw:0,关闭彩条
//   1,开启彩条(注意OV2640的彩条是叠加在图像上面的)
void ov2640_color_bar(uint8_t sw)
{
	uint8_t reg;
	dvp_sccb_send_data(OV2640_ADDR,0XFF,0X01);
	reg=dvp_sccb_receive_data(OV2640_ADDR,0X12);
	reg&=~(1<<1);
	if(sw)reg|=1<<1; 
	dvp_sccb_send_data(OV2640_ADDR,0X12,reg);
}
#endif