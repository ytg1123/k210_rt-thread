#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>
#include <asm/arch/gpio.h>

#include "board.h"
#include "interrupt.h"
#include "iomem.h"
#include "drv_i2c.h"
#include "drv_clock.h"
#include "port.h"
#include "reg-ccu.h"
#include "clock_sun8iw5.h"
#include "timer_sun8iw5.h"
#include "cpu_sun8i.h"

#ifdef RT_USING_I2C

#ifdef RT_I2C_DEBUG
#define I2C_DBG(...)     rt_kprintf("[I2C]"),rt_kprintf(__VA_ARGS__)
#else
#define I2C_DBG(...)
#endif



#define I2C0_BASE_ADDR (0x50280000)
#define I2C0_CCU_BUS_SOFT_RST_BASE (A33_CCU_BASE | CCU_BUS_SOFT_RST4)
#define I2C0_CCU_BUS_SOFT_RST_BIT (1 << (0))

#define I2C1_BASE_ADDR (0x50290000)
#define I2C1_CCU_BUS_SOFT_RST_BASE (A33_CCU_BASE | CCU_BUS_SOFT_RST4)
#define I2C1_CCU_BUS_SOFT_RST_BIT (1 << (1))

#define I2C2_BASE_ADDR (0x502A0000)
#define I2C2_CCU_BUS_SOFT_RST_BASE (A33_CCU_BASE | CCU_BUS_SOFT_RST4)
#define I2C2_CCU_BUS_SOFT_RST_BIT (1 << (2))

enum {
    I2C_ADDR            = 0x000,
    I2C_XADDR           = 0x004,
    I2C_DATA            = 0x008,
    I2C_CNTR            = 0x00c,
    I2C_STAT            = 0x010,
    I2C_CCR             = 0x014,
    I2C_SRST            = 0x018,
    I2C_EFR             = 0x01c,
    I2C_LCR             = 0x020,
};

enum {
    I2C_STAT_BUS_ERROR  = 0x00,
    I2C_STAT_TX_START   = 0x08,
    I2C_STAT_TX_RSTART  = 0x10,
    I2C_STAT_TX_AW_ACK  = 0x18,
    I2C_STAT_TX_AW_NAK  = 0x20,
    I2C_STAT_TXD_ACK    = 0x28,
    I2C_STAT_TXD_NAK    = 0x30,
    I2C_STAT_LOST_ARB   = 0x38,
    I2C_STAT_TX_AR_ACK  = 0x40,
    I2C_STAT_TX_AR_NAK  = 0x48,
    I2C_STAT_RXD_ACK    = 0x50,
    I2C_STAT_RXD_NAK    = 0x58,
    I2C_STAT_IDLE       = 0xf8,
};

struct ingenic_i2c_bus
{
    struct rt_i2c_bus_device parent;

    rt_uint32_t hwaddr;  // I2C0_BASE_ADDR
    rt_uint32_t irqno;  //  TWI0_INTERRUPT
} ;

static inline unsigned short i2c_readl(struct ingenic_i2c_bus *i2c, unsigned short offset)
{
unsigned short ret;
    //ret= readl(i2c->hwaddr + offset);
    ret= read32(i2c->hwaddr + offset);
    return ret;
}

static inline void i2c_writel(struct ingenic_i2c_bus *i2c, unsigned short offset,
        unsigned short value)
{
    //writel(value, i2c->hwaddr + offset);
    write32( i2c->hwaddr + offset, value);
}


/* delay x useconds */
void __usdelay(unsigned long usec)
{
        rt_thread_delay(usec);
}

/* delay x mseconds */
void __msdelay(unsigned long msec)
{
        rt_thread_mdelay(msec);
}

void set_i2c_bus(int bus_num )
{
        struct sunxi_ccm_reg *ccm_reg = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
    int reg_value = 0;

    /* reset twi reset register 0x2d8 twi0=bit0 twi1=bit1 twi2=bit2 */
        reg_value = read32(SUNXI_CCM_BASE + 0x2d8);
        reg_value |= 0x01 << bus_num;
        write32(SUNXI_CCM_BASE + 0x2d8 ,reg_value);
        __msdelay(1);

    /* GATE mask */
        ccm_reg->apb1_gate &= ~(1<<bus_num);
        __msdelay(1);
        ccm_reg->apb1_gate |=  1 << bus_num;

	u32 tmp = readl(0x01c2006c);
	I2C_DBG("%s %d bus_clok_gating_reg_3=0x%x \n",__func__, __LINE__, tmp);
	
	tmp = readl(0x01c20054);
	I2C_DBG("%s %d cpu_ahb_apb0_cfg=0x%x \n",__func__, __LINE__, tmp);
	tmp = readl(0x01c20058);
	I2C_DBG("%s %d apb1_clk_div_cfg=0x%x \n",__func__, __LINE__, tmp);
}

void reset_i2c(struct ingenic_i2c_bus *i2c )
{
    int i = 0xffff;
    i2c_writel(i2c, I2C_SRST, 1 );
    while(( i2c_readl(i2c, I2C_SRST) ) && (i))
    {
        i --;
    }
}

void set_speed_i2c(struct ingenic_i2c_bus *i2c , int speed)
{
    int i,clk_n,clk_m;
    int _2_pow_clk_n=1;
    //I2C_DBG("%s %d set_speed_i2c start speed=%d \n", __func__, __LINE__, speed);
    /* i2c Line Control SCL_STATE/SDA_STATE check  */
    if((i2c_readl(i2c, I2C_LCR) & 0x30) != 0x30 )
    {
        /* toggle I2CSCL until bus idle */
        /* I2C0 SCL_CTL SDA_CTL Enabled  */
        i2c_writel(i2c, I2C_LCR, 0x05 );
        __msdelay(10);
        i = 10;
                while ((i > 0) && (( i2c_readl(i2c, I2C_LCR) & 0x02) != 2))
                {
                        i2c_writel(i2c, I2C_LCR, i2c_readl(i2c, I2C_LCR) | 0x08 );
                        __msdelay(1);
                        i2c_writel(i2c, I2C_LCR, i2c_readl(i2c, I2C_LCR) & ~0x08 );
                        __msdelay(1);
                        i--;
                }
                i2c_writel(i2c, I2C_LCR, 0 );
                __msdelay(1);
    }
    //I2C_DBG("set_speed_i2c SCL_STATE/SDA_STAT check done\n");
    if(speed < 100)
    {
            speed = 100;
    }
    else if(speed > 400)
    {
            speed = 400;
    }
    clk_n = 1;
    //calculate 2 pow clk_n:
    _2_pow_clk_n = _2_pow_clk_n<<clk_n;

    clk_m = (24000/10)/(_2_pow_clk_n * speed) - 1;

    i2c_writel(i2c, I2C_CCR, (clk_m<<3) | clk_n );
    i2c_writel(i2c, I2C_CNTR, 0x40 );
    i2c_writel(i2c, I2C_EFR, 0 );
    //I2C_DBG("set_speed_i2c tm=%d tn=%d speed=%d done\n",clk_m,clk_n,speed);
}


#define TIMEOUT             0xff

static const struct rt_i2c_bus_device_ops i2c0_ops =
{
    ingenic_i2c_xfer,
    RT_NULL,
    RT_NULL
};
static const struct rt_i2c_bus_device_ops i2c1_ops =
{
    ingenic_i2c_xfer,
    RT_NULL,
    RT_NULL
};
static const struct rt_i2c_bus_device_ops i2c2_ops =
{
    ingenic_i2c_xfer,
    RT_NULL,
    RT_NULL
};

void pr_hex_dump(unsigned char *buf,int size)
{
int i;
//	rt_kprintf("\n");
//	rt_kprintf("-0 -1 -2 -3 -4 -5 -6 -7 -8 -9 -A -B -C -D -E -F \n");
//	rt_kprintf("----------------------------------------------- \n");
//	for (i=0;i<size;i++) {
//	rt_kprintf("%02x ",buf[i]);
//	if (i%16==15)rt_kprintf("\n");
//	}
//	rt_kprintf("\n");
}

struct ingenic_i2c_bus ingenic_i2c0;
struct ingenic_i2c_bus ingenic_i2c1;
struct ingenic_i2c_bus ingenic_i2c2;

static int a33_i2c_wait_status(struct ingenic_i2c_bus *i2c)
{
    int timeout = TIMEOUT;
    int ret;
    //I2C_DBG("[%s]%d \n", __func__, __LINE__);
    while (!(i2c_readl(i2c, I2C_CNTR) & 0x08) && (--timeout > 0) ){
                 rt_thread_mdelay(1);
    }

    ret= i2c_readl(i2c, I2C_STAT) & 0xff ;
    //I2C_DBG("[%s]%d ret=%x\n", __func__, __LINE__,ret);

    if (timeout){
        timeout= i2c_readl(i2c,I2C_STAT) & 0xff;
        //I2C_DBG("[%s]%d ret=%d\n", __func__, __LINE__,timeout);
        return timeout;
    }
    return I2C_STAT_BUS_ERROR;
}

//static int a33_i2c_start(struct ingenic_i2c_bus *i2c)
int a33_i2c_start(struct ingenic_i2c_bus *i2c)
{
    int timeout = TIMEOUT;
    int ret;

    i2c_writel(i2c, I2C_EFR,  0 );
    i2c_writel(i2c, I2C_SRST, 1 );
//    i2c_writel(i2c, I2C_CNTR, i2c_readl(i2c,I2C_CNTR)|0x20 );
//    while((timeout--)&&(!(i2c_readl(i2c,I2C_CNTR) & 0x08)));
    i2c_writel(i2c,I2C_CNTR, (1 << 7) | (1 << 6) | (1 << 5) |  (1 << 2));
    
    /* Master Mode check*/
    while ((i2c_readl(i2c, I2C_CNTR) & (1 << 5)) && (--timeout > 0)) {
                 rt_thread_mdelay(1);
    }
    ret=a33_i2c_wait_status(i2c);
    while (!(i2c_readl(i2c, I2C_CNTR) & 0x08 ) && (--timeout > 0) ){
                 rt_thread_mdelay(1);
    }
    ret= i2c_readl(i2c, I2C_STAT) & 0xff ;
    if(timeout){
        timeout= i2c_readl(i2c,I2C_STAT) & 0xff ;
	ret=timeout;
    }
    return ret;
}


int a33_i2c_restart(struct ingenic_i2c_bus *i2c)
{
    int timeout = TIMEOUT;
    int ret;

    i2c_writel(i2c,I2C_CNTR, (1 << 7) | (1 << 6) | (1 << 5) |  (1 << 2));
    
    i2c_writel(i2c,I2C_CNTR, i2c_readl(i2c, I2C_CNTR) | (0x01<<3));
    /* Master Mode check*/
    while ((i2c_readl(i2c, I2C_CNTR) & (1 << 5)) && (--timeout > 0)) {
                 rt_thread_mdelay(1);
    }
    ret=a33_i2c_wait_status(i2c);
    while (!(i2c_readl(i2c, I2C_CNTR) & 0x08 ) && (--timeout > 0) ){
                 rt_thread_mdelay(1);
    }
    ret= i2c_readl(i2c, I2C_STAT) & 0xff ;
    if(timeout){
        timeout= i2c_readl(i2c,I2C_STAT) & 0xff ;
	ret=timeout;
    }
    return ret;
}

static int a33_i2c_stop(struct ingenic_i2c_bus *i2c)
{
    int timeout=TIMEOUT;

    i2c_writel(i2c, I2C_CNTR, (i2c_readl(i2c,I2C_CNTR) | (1 << 4)));
    i2c_writel(i2c, I2C_CNTR, (i2c_readl(i2c,I2C_CNTR) | (1 << 3)));

    while ((i2c_readl(i2c, I2C_CNTR) & (1 << 4)) && (--timeout > 0))
        rt_thread_mdelay(1);

    if(timeout <= 0){
        //I2C_DBG("[%s]%d \n", __func__, __LINE__);
        return -1;
    }
   
    timeout=TIMEOUT; 
    while ((i2c_readl(i2c, I2C_STAT) != I2C_STAT_IDLE) && (--timeout > 0))
        rt_thread_mdelay(1);

    if(timeout <= 0){
        return -1;
    }
 
    return 0;
}

static int a33_i2c_send_data(struct ingenic_i2c_bus *i2c,rt_uint8_t dat)
{
    i2c_writel(i2c,I2C_DATA, dat);
    i2c_writel(i2c,I2C_CNTR, i2c_readl(i2c, I2C_CNTR) | (0x01<<3));
    return a33_i2c_wait_status(i2c);
}

static int a33_i2c_read(struct ingenic_i2c_bus *i2c,struct rt_i2c_msg * msg)
{

    rt_uint8_t * p = msg->buf;
        int len = msg->len;

        if(a33_i2c_send_data(i2c, (rt_uint8_t)(msg->addr << 1 | 1)) != I2C_STAT_TX_AR_ACK)
        {
            return -1;
        }


        while(len > 0)
        {
            switch(a33_i2c_wait_status(i2c))
            {
                case I2C_STAT_TX_AR_ACK:
                    if(len == 1)
                        i2c_writel(i2c, I2C_CNTR, i2c_readl(i2c,I2C_CNTR) & ~(1 << 2));
                    i2c_writel(i2c,I2C_CNTR, i2c_readl(i2c,I2C_CNTR) | (1<<3));
                    break;
                case I2C_STAT_RXD_ACK:
                    *p++ = i2c_readl(i2c, I2C_DATA);
                    len--;
                    if(len == 1)
                        i2c_writel(i2c, I2C_CNTR, i2c_readl(i2c,I2C_CNTR) & ~(1 << 2));
                    i2c_writel(i2c, I2C_CNTR, i2c_readl(i2c, I2C_CNTR) | (1<<3));
                    break;
                case I2C_STAT_RXD_NAK:
                    *p++ = i2c_readl(i2c, I2C_DATA);
                    len--;
                    break;
                default:
                    return -1;
            }
        }
        return 0;
}

static int a33_i2c_write(struct ingenic_i2c_bus *i2c,struct rt_i2c_msg * msg)
{
    rt_uint8_t * p = msg->buf;
    int len = msg->len;

    if(a33_i2c_send_data(i2c, (rt_uint8_t)(msg->addr << 1)) != I2C_STAT_TX_AW_ACK)
    {
        return -1;
    }

    while(len > 0)
    {
        if(a33_i2c_send_data(i2c, *p++) != I2C_STAT_TXD_ACK){
            return -1;
        }
        len--;
    }
    return 0;
}

rt_size_t ingenic_i2c_xfer(struct rt_i2c_bus_device *bus,
                                         struct rt_i2c_msg msgs[],
                                         rt_uint32_t num){

        struct rt_i2c_msg * pmsg = msgs;
	int val;
       
	struct ingenic_i2c_bus *i2c=( struct ingenic_i2c_bus *)bus;
        int i, res;
	

        if(!msgs || num <= 0) {
			return 0;
	}

        val=a33_i2c_start(i2c) ;
        if( val != I2C_STAT_TX_START)
        {
            return 0;
        }

        for(i = 0; i < num; i++, pmsg++)
        {
            
            if(i != 0)
            {
        	val=a33_i2c_restart(i2c) ;
                if(val != I2C_STAT_TX_RSTART)
                {
                    break;
                }

            }
            if(pmsg->flags & RT_I2C_RD){
                res = a33_i2c_read(i2c, pmsg);
            }
            else{
                res = a33_i2c_write(i2c, pmsg);
            }
            
            if(res < 0)
                break;
        }
        a33_i2c_stop(i2c);
        return i;
}

static int ingenic_i2c_set_speed(struct ingenic_i2c_bus *i2c, int rate){
    rt_uint32_t pclk = apb_get_clk();
    rt_int32_t freq, delta, best = 0x7fffffffffffffffLL;
    int tm = 5, tn = 0;
    int m, n;

    for(n = 0; n <= 7; n++)
    {
        for(m = 0; m <= 15; m++)
        {
            freq = pclk / (10 * (m + 1) * (1 << n));
            delta = rate - freq;
            if(delta >= 0 && delta < best)
            {
                tm = m;
                tn = n;
                best = delta;
            }
            if(best == 0)
                break;
        }
    }
    i2c_writel(i2c,I2C_CCR, ((tm & 0xf) << 3) | ((tn & 0x7) << 0));
    return 0;
}

static void rt_hw_i2c0_isr(int vector, void *param)
{
    struct rt_i2c_bus_device *i2c0 = (struct rt_serial_device *)param;
    struct ingenic_i2c_bus *_i2c0bus = (struct ingenic_i2c_bus *)i2c0->parent.user_data;
    RT_ASSERT(_i2c0bus != RT_NULL);
}

static void rt_hw_i2c1_isr(int vector, void *param)
{
    struct rt_i2c_bus_device *i2c1 = (struct rt_serial_device *)param;
    struct ingenic_i2c_bus *_i2c1bus = (struct ingenic_i2c_bus *)i2c1->parent.user_data;
    RT_ASSERT(_i2c1bus != RT_NULL);
}

static void rt_hw_i2c2_isr(int vector, void *param)
{
    struct rt_i2c_bus_device *i2c2 = (struct rt_serial_device *)param;
    struct ingenic_i2c_bus *_i2c2bus = (struct ingenic_i2c_bus *)i2c2->parent.user_data;
    RT_ASSERT(_i2c2bus != RT_NULL);
}

void gpio_ph_cfg_0(uint pull,uint val)
{
        uint reg_val = 0;
        writel( val , (A33_GPIO_BASE + 0xfc) ); 
        reg_val = readl(A33_GPIO_BASE + 0x118);
        reg_val &= ~(0x03 << 4);
        reg_val |= ( pull << 4);
        writel(reg_val, (A33_GPIO_BASE + 0x118));

        I2C_DBG("gpio_ph_cfg_0 reg_val=0x%x,read=0x%x\n",reg_val,readl((A33_GPIO_BASE + 0x118)));
}
void gpio_pe_cfg_1(uint pull,uint val)
{
        uint reg_val = 0;
        writel( val , (A33_GPIO_BASE + 0x94) ); 
        reg_val = readl(A33_GPIO_BASE + 0xac);
        reg_val &= ~(0x03 << 4);
        reg_val |= ( pull << 4);
        writel(reg_val, (A33_GPIO_BASE + 0xac));
        I2C_DBG("gpio_pe_cfg_1 reg_val=0x%x,read=0x%x\n",reg_val,readl((A33_GPIO_BASE + 0xac)));
}

void gpio_pd_cfg_0(uint pull,uint val)
{
        uint reg_val = 0;
        writel( val , (A33_GPIO_BASE + 0x6c) ); 
        reg_val = readl(A33_GPIO_BASE + 0x88);
        reg_val &= ~(0x03 << 4);
        reg_val |= ( pull << 4);
        writel(reg_val, (A33_GPIO_BASE + 0x88));
        I2C_DBG("gpio_pd_cfg_0 reg_val=0x%x,read=0x%x\n",reg_val,readl((A33_GPIO_BASE + 0x88)));
}
void gpio_pd_cfg_1(uint pull,uint val)
{
        uint reg_val = 0;
        writel( val , (A33_GPIO_BASE + 0x70) ); 
        reg_val = readl(A33_GPIO_BASE + 0x88);
        reg_val &= ~(0x03 << 4);
        reg_val |= ( pull << 4);
        writel(reg_val, (A33_GPIO_BASE + 0x88));
        I2C_DBG("gpio_pd_cfg_1 reg_val=0x%x,read=0x%x\n",reg_val,readl((A33_GPIO_BASE + 0x88)));
}
void gpio_pd_cfg_2(uint pull,uint val)
{
        uint reg_val = 0;
        writel( val , (A33_GPIO_BASE + 0x74) ); 
        reg_val = readl(A33_GPIO_BASE + 0x8c);
        reg_val &= ~(0x03 << 4);
        reg_val |= ( pull << 4);
        writel(reg_val, (A33_GPIO_BASE + 0x8c));
        I2C_DBG("gpio_pd_cfg_2 reg_val=0x%x,read=0x%x\n",reg_val,readl((A33_GPIO_BASE + 0x8c)));
}
void gpio_pd_cfg_3(uint pull,uint val)
{
        uint reg_val = 0;
        writel( val , (A33_GPIO_BASE + 0x78) ); 
        reg_val = readl(A33_GPIO_BASE + 0x8c);
        reg_val &= ~(0x03 << 4);
        reg_val |= ( pull << 4);
        writel(reg_val, (A33_GPIO_BASE + 0x8c));
        I2C_DBG("gpio_pd_cfg_3 reg_val=0x%x,read=0x%x\n",reg_val,readl((A33_GPIO_BASE + 0x8c)));
}

struct rt_i2c_bus_device *i2c0_bus ;
struct rt_i2c_bus_device *i2c1_bus ;
struct rt_i2c_bus_device *i2c2_bus ;
unsigned short srst;

int rt_hw_i2c_init(void)
{
    struct sunxi_ccm_reg *ccm_reg = (struct sunxi_ccm_reg *)A33_CCU_BASE;
    int bus_num,speed,reg_value,i,clk_m,clk_n;
    int timeout = TIMEOUT;
    I2C_DBG("rt_hw_i2c_init start \n");
    /* PB7 LED */
    gpio_set_mode(SUNXI_GPB(7), PIN_TYPE(SUNXI_GPIO_OUTPUT)|PULL_UP);

    /* PB7 LED OFF */
    sunxi_gpio_output(SUNXI_GPB(7), 0);

    /* EP952 RBG Blue6bit port config GPIO  */
    gpio_pd_cfg_0(0x02,0x22222222);
    /* EP952 RBG Gleen6bit port config GPIO  */
    gpio_pd_cfg_1(0x02,0x22222222);
    /* EP952 RBG Red6bit port config GPIO  */
    gpio_pd_cfg_2(0x02,0x22222222);
    /* EP952 Control Line config GPIO  */
    gpio_pd_cfg_3(0x02,0x00002222);

 
    /* twi0 init */
    I2C_DBG("[%s]%d  \n", __func__, __LINE__);
    struct ingenic_i2c_bus *i2c0;
    i2c0 = &ingenic_i2c0;
    rt_memset((void *)i2c0, 0, sizeof(struct ingenic_i2c_bus));
    i2c0->hwaddr = I2C0_BASE_ADDR;
    i2c0->irqno = TWI0_INTERRUPT;
    I2C_DBG("[%s]%d hwaddr=%x irqno=%x \n", __func__, __LINE__,i2c0->hwaddr,i2c0->irqno);

    /* i2c clock init */
    set_i2c_bus(0);
    /* i2c reset */
    reset_i2c(i2c0);
    set_speed_i2c(i2c0,400);

    bus_gate_clk_enalbe(TWI0_GATING);
    //0x01c202d0

//    rt_int64_t  val = read32(I2C0_CCU_BUS_SOFT_RST_BASE);
//    val |= I2C0_CCU_BUS_SOFT_RST_BIT;
    write32(I2C0_CCU_BUS_SOFT_RST_BASE, read32(I2C0_CCU_BUS_SOFT_RST_BASE)|I2C0_CCU_BUS_SOFT_RST_BIT);

  //  bus_software_reset_enalbe(TWI0_GATING);
    /* DDC  Set PH3 PH2 in func2 (I2C0) */
    gpio_set_func(GPIO_PORT_H, 2, 3);
    gpio_set_func(GPIO_PORT_H, 3, 3);
    //gpio_set_pull_mode(GPIO_PORT_H, 2,1);
    //gpio_set_pull_mode(GPIO_PORT_H, 3,1);

  //  bus_software_reset_enalbe(TWI0_GATING);
  //  bus_software_reset_disalbe(TWI0_GATING);

    i2c0_bus = &i2c0->parent;
    i2c0_bus->ops = &i2c0_ops;

    rt_i2c_bus_device_register(i2c0_bus, "i2c0");
  //  rt_hw_interrupt_install(TWI0_INTERRUPT, rt_hw_i2c0_isr, &i2c0, "i2c0");
  //   rt_hw_interrupt_umask(TWI0_INTERRUPT);

//    /* twi1 init */
//    struct ingenic_i2c_bus *i2c1;
//    i2c1 = &ingenic_i2c1;
//    rt_memset((void *)i2c1, 0, sizeof(struct ingenic_i2c_bus));
//    i2c1->hwaddr = I2C1_BASE_ADDR;
//    i2c1->irqno = TWI1_INTERRUPT;
//    I2C_DBG("[%s]%d hwaddr=%x irqno=%x \n", __func__, __LINE__,i2c1->hwaddr,i2c1->irqno);

//    /* i2c clock init */
//    set_i2c_bus(1);
//    reset_i2c(i2c1);
//    set_speed_i2c(i2c1,400);
//    bus_gate_clk_enalbe(TWI1_GATING);

//    rt_int64_t  val = read32(I2C1_CCU_BUS_SOFT_RST_BASE);
//    val |= I2C1_CCU_BUS_SOFT_RST_BIT;
    write32(I2C1_CCU_BUS_SOFT_RST_BASE, read32(I2C1_CCU_BUS_SOFT_RST_BASE)|I2C1_CCU_BUS_SOFT_RST_BIT);

  //  bus_software_reset_enalbe(TWI1_GATING);
  //  bus_software_reset_disalbe(TWI1_GATING);

//    i2c1_bus = &i2c1->parent;
//    i2c1_bus->ops = &i2c1_ops;

//    rt_i2c_bus_device_register(i2c1_bus, "i2c1");
  //  rt_hw_interrupt_install(TWI1_INTERRUPT, rt_hw_i2c1_isr, &i2c1, "i2c1");
  //  rt_hw_interrupt_umask(TWI1_INTERRUPT);

    /* twi2 init */
    struct ingenic_i2c_bus *i2c2;
    i2c2 = &ingenic_i2c2;
    rt_memset((void *)i2c2, 0, sizeof(struct ingenic_i2c_bus));
    i2c2->hwaddr = I2C2_BASE_ADDR;
    i2c2->irqno = TWI2_INTERRUPT;
    I2C_DBG("[%s]%d hwaddr=%x irqno=%x \n", __func__, __LINE__,i2c2->hwaddr,i2c2->irqno);

    write32(I2C2_CCU_BUS_SOFT_RST_BASE, read32(I2C2_CCU_BUS_SOFT_RST_BASE)|I2C2_CCU_BUS_SOFT_RST_BIT);

    /* HDMI  Set PE13 PE12 in func2 (I2C2) */
    //gpio_set_func(GPIO_PORT_E, 12, 3);
    //gpio_set_func(GPIO_PORT_E, 13, 3);
    //gpio_set_pull_mode(GPIO_PORT_E, 12, 1);
    //gpio_set_pull_mode(GPIO_PORT_E, 13, 1);

  //  bus_software_reset_enalbe(TWI2_GATING);
  //  bus_software_reset_disalbe(TWI2_GATING);

    i2c2_bus = &i2c2->parent;
    i2c2_bus->ops = &i2c2_ops;

    /* i2c clock init */
    set_i2c_bus(2);
    reset_i2c(i2c2);
    set_speed_i2c(i2c2,400);
    bus_gate_clk_enalbe(TWI2_GATING);
    reg_value=a33_i2c_start(i2c2);
    I2C_DBG("%s:%d  a33_i2c_start reg_value=%x I2C_STAT_TX_START==0x08 \n", __func__, __LINE__,reg_value);


    //rt_i2c_bus_device_register(i2c2_bus, "i2c2");
  //  rt_hw_interrupt_install(TWI2_INTERRUPT, rt_hw_i2c2_isr, &i2c2, "i2c2");
  //  rt_hw_interrupt_umask(TWI2_INTERRUPT);

    /* PB7 LED ON */
    sunxi_gpio_output(SUNXI_GPB(7), 1);

    I2C_DBG("rt_hw_i2c_init end \n");

      return 0;
}

//INIT_ENV_EXPORT(rt_hw_i2c_init);
INIT_DEVICE_EXPORT(rt_hw_i2c_init);

#endif
