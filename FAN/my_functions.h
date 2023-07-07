#ifndef _MY_FUNCTONS_H_
#define _MY_FUNCTONS_H_

#include "my_settings.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_gpio.h"


/*DHT11*/
void dht11_reset(void);
u8 GPIOGETINPUT(WifiIotIoName id,WifiIotGpioValue *value);
u8 dht11_check(void);
u8 InitDHT11(void);
u8 dht11_read_bit(void);
u8 dht11_read_byte(void);
u8 dht11_read_data(u8 *tmpr,u8 *humi);

/*MQ2*/
//void InitMQ2(void);
void mq2_calibration(void);
float GetVoltage(void);
float mq2_getPPM(void);

/*OC*/
void deal_report_msg(report_t *report);

#endif