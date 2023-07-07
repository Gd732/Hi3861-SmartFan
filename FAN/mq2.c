#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "my_settings.h"
#include "wifiiot_errno.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "wifiiot_adc.h"

#define CAL_PPM 20  //校准环境中PPM值
#define RL 1        //RL阻值

static float R0;    //元件在洁净空气中的阻值


//读取ADC口的电压数据
float GetVoltage(void)
{
    unsigned int ret;
    unsigned short data;

    ret = AdcRead(WIFI_IOT_ADC_CHANNEL_6, &data, WIFI_IOT_ADC_EQU_MODEL_8, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0xff);
    if (ret != WIFI_IOT_SUCCESS)
    {
        printf("ADC Read Fail\n");
    }
    return (float)data * 1.8 * 4 / 4096.0;
}

//将读取到的电压数据转化为PPM值
float mq2_getPPM(void)
{
    float voltage, RS, ppm;

    voltage = GetVoltage();
    RS = (5 - voltage) / voltage * RL;    //计算RS
    ppm = 613.9f * pow(RS / R0, -2.074f); //计算ppm
    return ppm;
}

//校准MQ2传感器
void mq2_calibration(void)
{
    float voltage = GetVoltage();
    float RS = (5 - voltage) / voltage * RL;
    R0 = RS / pow(CAL_PPM / 613.9f, 1 / -2.074f);
}



