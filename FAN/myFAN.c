#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

#include "my_settings.h"
#include "my_functions.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_pwm.h"
#include "wifiiot_errno.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_adc.h"
#include "wifiiot_uart.h"
#include "wifiiot_watchdog.h"
#include "hi_time.h"
#include "oc_mqtt.h"
#include "oc_mqtt_profile_package.h"
#include "wifi_connect.h"

//MSGQUEUE_OBJ_t msg;
osMessageQueueId_t mid_MsgQueue; //消息队列的ID

static void InitLED(void)
{
    GpioInit();
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_2,WIFI_IOT_IO_FUNC_GPIO_2_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_2,WIFI_IOT_GPIO_DIR_OUT);
}

static void InitFan(void)
{
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_IO_FUNC_GPIO_10_PWM1_OUT);
    //设置GPIO_10引脚为输出模式
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_GPIO_DIR_OUT);
    //初始化PWM1端口
    PwmInit(WIFI_IOT_PWM_PORT_PWM1);
}

static void AllInit(void)
{
    u32 ret;

    WifiIotUartAttribute uart_attr = {

        //baud_rate: 9600
        .baudRate = 9600,

        //data_bits: 8bits
        .dataBits = 8,
        .stopBits = 1,
        .parity = 0,
    };

    //连接Wifi(手机热点)
    WifiConnect("Mi10S","hellworld");
    //配置华为云平台用户信息
    device_info_init(CLIENT_ID,USERNAME,PASSWORD);
    //初始化mqtt通讯功能
    oc_mqtt_init();

    //Initialize uart driver
    ret = UartInit(WIFI_IOT_UART_IDX_1, &uart_attr, NULL);
    if (ret != WIFI_IOT_SUCCESS)
    {
        printf("Failed to init uart! Err code = %d\n", ret);
        return;
    }
    printf("UART Test Start\n");
    //初始化各个IO端口
    InitLED();
    InitFan();
    InitDHT11();
    //InitMQ2();
    usleep(1000000);
    mq2_calibration();
}


//请开机:WAKE\r\n  通风模式:LOWGEAR\r\n  强力模式:HIGHGEAR\r\n  请关机:SLEEP\r\n  请重置:REWAKE\r\n
static void dealSpeechInput(u8 uart_buff[],MachineState* pstate)
{
    switch((u8)(uart_buff[0]))
    {
        case (u8)'W':
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2,1);
            *pstate=MACHINE_ON;
            break;
        case (u8)'L':
            PwmStart(WIFI_IOT_PWM_PORT_PWM1, 20000, 40000);
            *pstate=MACHINE_LOWGEAR;
            break;
        case (u8)'H':
            PwmStart(WIFI_IOT_PWM_PORT_PWM1, 40000, 40000);
            *pstate=MACHINE_HIGHGEAR;
            break;
        case (u8)'S':
            PwmStop(WIFI_IOT_PWM_PORT_PWM1);
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2,0);
            *pstate=MACHINE_OFF;
            break;
        case (u8)'R':
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2,1);
            *pstate=MACHINE_ON;
            break;
        default:
            break;
    }

}


static void Fan_Task(void)
{
    AllInit();

    //预先设定的状态量
    //指示电机运作状态
    MachineState state = MACHINE_OFF;
    //指示蜂鸣器状态
    MQ2_State MQstatus = MQ2_MACHINE_OFF;

    //需要读取的数据值
    u8 temperature=0,humidity=0;
    float PPM=0;


    app_msg_t *app_msg_get;
    app_msg_t *app_msg_put;
    while (1)
    {
        //通过串口1接收来自语音模块的数据
        u8 uart_buff[UART_BUFF_SIZE] = {0};
        u8 *uart_buff_ptr = uart_buff;
        UartRead(WIFI_IOT_UART_IDX_1, uart_buff_ptr, UART_BUFF_SIZE);
        //语音输入,驱动电机
        dealSpeechInput(uart_buff,&state);
        
        //获取data,并发出消息队列
        if(1)
        {
            dht11_read_data(&temperature,&humidity);
            PPM=mq2_getPPM();

            printf("Temperature=%u°C,Humidity=%u%%,PPM=%u\n",temperature,humidity,(u32)(PPM));

            app_msg_put=malloc(sizeof(app_msg_t));

            if(app_msg_put != NULL)
            {
                //struct report_t report,含temp,hum,concen三种属性
                //data中的各变量为float类型故需强制转换
                app_msg_put->msg_type = en_msg_report;
                app_msg_put->msg.report.temp=(int)(temperature);
                app_msg_put->msg.report.hum=(int)(humidity);
                app_msg_put->msg.report.concen=(int)(PPM);
                if(0 != osMessageQueuePut(mid_MsgQueue, &app_msg_put, 0U, 0U))
                {
                    free(app_msg_put);
                }
            }
        }

        //消息队列接收
        if(1)
        {
            app_msg_get = NULL;
            (void)osMessageQueueGet(mid_MsgQueue,(void**)&app_msg_get,NULL,0U);
            if(app_msg_get!=NULL)
            {
                if(app_msg_get->msg_type==en_msg_report)
                {
                    deal_report_msg(&app_msg_get->msg.report);
                }
                free(app_msg_get);
            }
        }
        
        if(state!=MACHINE_OFF)//若已经开机,才作变化
        {
            if(PPM>=MAX_CONCENTRATION+10&&MQstatus==MQ2_MACHINE_OFF)
            {
                PwmStart(WIFI_IOT_PWM_PORT_PWM1, 40000, 40000);
                MQstatus=MQ2_MACHINE_ON;
                state=MACHINE_HIGHGEAR;
            }
            else if(PPM<MAX_CONCENTRATION-10&&MQstatus==MQ2_MACHINE_ON)
            {
                PwmStart(WIFI_IOT_PWM_PORT_PWM1, 20000, 40000);
                MQstatus=MQ2_MACHINE_OFF;
                state=MACHINE_LOWGEAR;
            }
        }

        hi_udelay(1000000);
    }
}

static void Fan_ExampleEntry(void)
{
    mid_MsgQueue = osMessageQueueNew(MSGQUEUE_OBJECTS, 10, NULL);
    if (mid_MsgQueue == NULL)
    {
        printf("Falied to create Message Queue!\n");
    }

    WatchDogDisable();
    osThreadAttr_t attr;

    attr.name = "Fan_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = FAN_TASK_STACK_SIZE;
    attr.priority = FAN_TASK_PRIO;

    if (osThreadNew((osThreadFunc_t)Fan_Task, NULL, &attr) == NULL)
    {
        printf("[ADCExample] Falied to create Fan_Task!\n");
    }
}

APP_FEATURE_INIT(Fan_ExampleEntry);