#ifndef _MY_SETTINGS_H_
#define _MY_SETTINGS_H_

#define TO_FEED 10

#define FAN_TASK_STACK_SIZE 1024 * 8
#define FAN_TASK_PRIO 25
#define UART_BUFF_SIZE 1000

#define MAX_CONCENTRATION 120
#define DATA_INTERVAL 3
#define KICK_INTERVAL 10

#define CLIENT_ID "6483468b1cacf07a38140e42_TJautoGd732_0_0_2023061003"
#define USERNAME "6483468b1cacf07a38140e42_TJautoGd732"
#define PASSWORD "7a3698238aa89252ee762d0fb21d46fc2509468230796842aaab9ea172fdaa3e"

#define MSGQUEUE_OBJECTS 16 

typedef unsigned char u8;
typedef signed char i8;
typedef unsigned short u16;
typedef signed short i16;
typedef unsigned int u32;
typedef signed int i32;

typedef enum{
    OUTPUT_LOW,
    OUTPUT_HIGH
}OutPutVal;

//代表着风扇的当前运转状态——关机、开机、低档位（通风）、高档位（强力）
typedef enum{
    MACHINE_OFF=-1,
    MACHINE_ON=0,
    MACHINE_LOWGEAR=1,
    MACHINE_HIGHGEAR=2
}MachineState;

//代表着由于污染物浓度所引起的自动强力模式,OFF——其他模式,ON——强力通风
typedef enum
{
	MQ2_MACHINE_OFF,
	MQ2_MACHINE_ON
}MQ2_State;

typedef enum
{
    en_msg_cmd = 0,
    en_msg_report,
} en_msg_type_t;

typedef struct
{
    char *request_id;
    char *payload;
} cmd_t;

typedef struct
{
    int temp;
    int hum;
    int concen;
} report_t;

typedef struct
{
    en_msg_type_t msg_type;
    union
    {
        cmd_t cmd;
        report_t report;
    } msg;
} app_msg_t;

#endif

