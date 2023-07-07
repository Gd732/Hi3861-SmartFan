1、FAN为我们所编写的程序所在的目录，myFAN为主函数接口！！！

2、由于本作业使用BearPi提供的源代码，其src中各头文件的存储位置和hi3861_hdu_iot_application-master有所不同，故在本目录中放入存在路径差异的各个头文件所对应的文件夹，具体操作如下：
（1）Wifi连接部分：请将BUILD.gn中的"//foundation/communication/interfaces/kits/wifi_lite/wifiservice"注释掉，并将# "//foundation/communication/wifi_lite/interfaces/wifiservice"的注释取消;

（2）MQTT部分：请在目录"//third_party/"下加入本文档所在目录中的Paho-mqtt文件夹;

（3）基本函数部分：若hi3861_hdu_iot_application-master的include文件夹中的函数与本作业使用的函数不同，请将本文档所在目录中的include文件夹中的所有头文件粘贴到hi3861_hdu_iot_application-master所对应的"//utils/native/lite/include"文件夹中！！！同时，可以将BUILD.gn中的"//vendor/hisi/hi3861/hi3861/include"注释掉！！！