#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_watchdog.h"

#include "wifi_connect.h"
#include "lwip/sockets.h"
#include "my_settings.h"
#include "oc_mqtt.h"


extern osMessageQueueId_t mid_MsgQueue;

//将传感器获取的数据report到云端
void deal_report_msg(report_t *report)
{
    oc_mqtt_profile_service_t service;
    oc_mqtt_profile_kv_t temperature;
    oc_mqtt_profile_kv_t humidity;
    oc_mqtt_profile_kv_t concentration;

    //进行json数据的拼装

    service.event_time = NULL;//null means the platform time
    service.service_id = "Cooking";//服务id,需与产品创建时的服务信息一致
    service.service_property = &temperature;//服务属性which could not be null
    service.nxt = NULL;//没有下一个service了

    //key-属性名  value-属性值(从data中获取) type-数据类型(int/string) nxt-指示后面是否还有数据类型
    temperature.key = "Temperature";
    temperature.value = &report->temp;
    temperature.type = EN_OC_MQTT_PROFILE_VALUE_INT;
    temperature.nxt = &humidity;

    humidity.key = "Humidity";
    humidity.value = &report->hum;
    humidity.type = EN_OC_MQTT_PROFILE_VALUE_INT;
    humidity.nxt = &concentration;

    concentration.key = "Concentration";
    concentration.value = &report->concen;
    concentration.type = EN_OC_MQTT_PROFILE_VALUE_INT;
    concentration.nxt = NULL;

    //通过mqtt协议,将service对应的服务属性传送到USERNAME对应的云端账号
    oc_mqtt_profile_propertyreport(USERNAME, &service);
    return;
}

