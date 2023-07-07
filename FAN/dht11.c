#include <stdio.h>
#include <unistd.h>

#include "my_settings.h"
#include "cmsis_os2.h"
#include "ohos_init.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "hi_time.h"
#include "wifiiot_watchdog.h"

/**************************
  dht11_reset()
  每次初始化时，需要拉低总线，时长不小于18ms。
  然后拉高总线，使其回到高电平20-40us
  等待从机响应
***************************/
void dht11_reset(void)
{
    //GPIO11设为输出模式
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_11,OUTPUT_LOW);
    hi_udelay(20000);//拉低20ms
    GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_11,OUTPUT_HIGH);
    hi_udelay(35);//拉高35us
}

u8 GPIOGETINPUT(WifiIotIoName id,WifiIotGpioValue *value)
{
    GpioGetInputVal(id,value);
    return *value;
}

/**************************
  dht11_check()
  dht11的应答
  其将总线拉低80us
  后再拉高80us
***************************/
u8 dht11_check(void)
{
    u8 RETRY=0;
    WifiIotGpioValue INVAL;
    //GPIO11设为输入模式
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_11,WIFI_IOT_IO_PULL_NONE);
    //等待变为低电平(现为高电平)
    while((GPIOGETINPUT(WIFI_IOT_IO_NAME_GPIO_11,&INVAL))&&RETRY<100)
    {
        RETRY++;
        hi_udelay(1);}
    if(RETRY>=100)
    {
        return 1;
    }
    else 
    {
        RETRY=0;
    }
    //等待变为高电平(现为低电平)
    while((!GPIOGETINPUT(WIFI_IOT_IO_NAME_GPIO_11,&INVAL))&&RETRY<100)
    {
        RETRY++;
        hi_udelay(1);
    }
    //hi_udelay(40);
    //如果超时，返回1
    if(RETRY>=100)
    {
        printf("dht11 check failed\n");
        return 1;
    }
    else
    {
        return 0;
    }
}

/**************************
  dht11_init()
  dht11的初始化
  主机发出初始化信号
***************************/
u8 InitDHT11(void)
{
    GpioInit();

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_2,WIFI_IOT_IO_FUNC_GPIO_2_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_2,WIFI_IOT_GPIO_DIR_OUT);
    //GpioSetOutputVal(WIFI_IOT_GPIO_IDX_2,OUTPUT_HIGH);
    //GPIO11设为普通GPIO
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_11,WIFI_IOT_IO_FUNC_GPIO_11_GPIO);
    //GPIO11设为输出,高电平
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_11,OUTPUT_HIGH);
    dht11_reset();

    return dht11_check();
}

/**************************
  dht11_read_bit()
  检测到应答后,dht11会发出40bit的数据
  每bit数据持续40us,故每bit延时40us
***************************/
u8 dht11_read_bit(void)
{
    u8 RETRY=0;
    WifiIotGpioValue INVAL;
    while((GPIOGETINPUT(WIFI_IOT_IO_NAME_GPIO_11,&INVAL))&&RETRY<100)
    {
        RETRY++;
        hi_udelay(1);
    }
    RETRY=0;
    while(!(GPIOGETINPUT(WIFI_IOT_IO_NAME_GPIO_11,&INVAL))&&RETRY<100)
    {
        RETRY++;
        hi_udelay(1);
    }
    //每bit延时40us
    hi_udelay(40);
    if(GPIOGETINPUT(WIFI_IOT_IO_NAME_GPIO_11,&INVAL))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**************************
  dht11_read_byte()
  读八位存入data进行返回
***************************/
u8 dht11_read_byte(void)
{
    u8 data;
    data=0;
    for(u8 i=0;i<8;i++)
    {
        //右移读数(数据读取方式为湿度温度高八位、低八位)
        data<<=1;
        data|=dht11_read_bit();
    }
    return data;
}

/**************************
  dht11_read_data()
  读得温湿度数据并返回结果值
  一帧数据由湿度高八位、湿度低八位、温度高八位、温度低八位和校验位组成
  校验位的值为其余各位之和(截取前八位)
***************************/
u8 dht11_read_data(u8 *tmpr,u8 *humi)
{
    u8 buff[5]={0};
    dht11_reset();
    if(dht11_check()==0)
    {
        for(u8 i=0;i<5;i++)
        {
            buff[i]=dht11_read_byte();
        }
        if((buff[0]+buff[1]+buff[2]+buff[3])==buff[4])
        {
            *humi=buff[0];
            *tmpr=buff[2];
        }
    }
    else
    {
        return 1;
    }
    return 0;
}

void dht_sample(void)
{
    InitDHT11();
    u8 temperature=0,humidity=0;
    u8 feedcnt=1;
    while(1)
    {
        dht11_read_data(&temperature,&humidity);
        hi_udelay(500000);
        printf("temperature=%u,humidity=%u\n",temperature,humidity);

        feedcnt++;
        if(feedcnt==10)
        {
            WatchDogKick();
            feedcnt=1;
        }
    }
}


