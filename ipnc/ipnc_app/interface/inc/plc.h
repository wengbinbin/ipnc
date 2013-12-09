
#ifndef __PLC_H__
#define __PLC_H__

/***********************************************************************************/

#define CHANNEL_NUM  8
#define RS_PORT_NUM  4
#define MODBUSTCP_NUM    8

#define DIS_NUM     64
#define REG_NUM     16

#define  PROTOCOL_MODBUS     1

/* data_type */
#define READ_DISCRETE            0x02
#define READ_HOLDING_REGISTER    0x03
#define READ_INPUT_REGISTER      0x04
#define WRITE_HOLDING_REGISTER   0x06


struct MODBUS_DATA         /* par tab*/
{
    int flag;            /* used or not */
    int dev_id;
    int data_type;
    int plc_ind;
    int reg_cnt;
    int start_addr;

    int speed;
    int databit;
    int stopbit;
    int parity;//0 1 2
    int streamctrl; // 0 1 2
};

#define DEFAULT_MODBUS_DATA         \
{                                   \
    0,      /*flag*/                \
    1,      /* dev_id */            \
    READ_HOLDING_REGISTER,          \
    0,     /* plc_id */             \
    1,      /* reg_cnt */           \
    0,      /* start add */         \
    9600,        /* speed */        \
    8,            /*databit*/       \
    1,            /*stopbit*/       \
    0,            /*parity*/        \
    0            /*streamctrl*/    \
}
        


struct MODBUS_RS
{
    int flag;    /* used or not */

    int protocol;
        
    int poll_intval;    /* ms */
    int expire;        /* ms */

    struct MODBUS_DATA  md[CHANNEL_NUM];     /* support 8 dev_id */
};

#define DEFAULT_MODBUS_RS           \
{                                   \
    0,      /*flag*/                \
    PROTOCOL_MODBUS,                \
    10000,     /* poll_interval */     \
    1000,      /* expire */            \
    { \
        DEFAULT_MODBUS_DATA, \
        DEFAULT_MODBUS_DATA, \
        DEFAULT_MODBUS_DATA, \
        DEFAULT_MODBUS_DATA, \
        DEFAULT_MODBUS_DATA, \
        DEFAULT_MODBUS_DATA, \
        DEFAULT_MODBUS_DATA, \
        DEFAULT_MODBUS_DATA \
    } \
}
      

struct MODBUS_TCP
{
    int flag;    /* used or not */
    char ip_addr[16];
    int tcp_port;
    int poll_intval;    /* ms */
    int expire;        /* ms */
    int dev_id;
    int data_type;
    int plc_ind;
    int reg_cnt;
    int start_addr;    

};

#define DEFAULT_MODBUS_TCP        \
{                                 \
    0,    /* flag */              \
    {"0.0.0.0"},                    \
    502, /* tcp_port,*/                 \
    10000,    /* poll_intval */      \
    3000,        /* expire; */       \
    1,        /*dev_id*/          \
    3,        /*data_type*/       \
    0,        /*plc_ind*/         \
    1,        /* reg_cnt*/        \
    0        /*start_addr*/      \
}

struct DATA_SOURCE
{
    struct MODBUS_RS    md_rs[RS_PORT_NUM];             /* support 4 serial port */
    struct MODBUS_TCP   md_tcp[MODBUSTCP_NUM];            /* support 8 tcp peer */

    int plcRun;
    long plcLogFileInd;
};

/***************************************************************************************/

/**
* @brief Commands used to communicate with file manager.

*/
typedef enum {
    PLC_MSG_START, ///< Let PLC Progress start.
    PLC_MSG_QUIT, ///< Let PLC Progress shutdown.
    PLC_MSG_RESTART, ///< Restart PLC Progress
} PlcCmd_t;

/**
* @brief PLC Progress buffer type

* This structure is used to communicate with file manager.
*/
typedef struct _PLC_MSG_BUF
{
    long Des; ///< Where this message go.
    PlcCmd_t cmd; ///< Message command ID.
    int src; ///< Where this message from.
    int shmid; ///< Share memory ID which is used to communicate.
    int length; ///< Data length in share memory.
    int shmidx; ///< Share memory index (0 or 1). 0 is used for system settings; and 1 is used for system log.
    int nPageNum; ///< System log page number.
    int nItemNum; ///< System log item number.
} PLC_MSG_BUF;

#define PLC_MSG_KEY        0x542c7ae7 ///< Message key used by system server.

#define PLC_LOG_FILE "/mnt/nand/plclogfile.log"

#define MAX_PLC_LOG_FILE_SIZE 10*1024

#define MAXplcLogFileSize 20*1024

extern char *getTime(void);

#define plcLogPrintf(debug,fmt,args...)  \
{ \
    if(debug) \
    { \
        printf("%s  PLC: "fmt,getTime(),##args); \
    } \
\
   sysInfoP->data_source.plcLogFileInd+= fprintf(plcLogFile,"%s  PLC: "fmt,getTime(),##args); \
\
    if(sysInfoP->data_source.plcLogFileInd>=MAXplcLogFileSize) \
    { \
        sysInfoP->data_source.plcLogFileInd=0; \
\
        fseek(plcLogFile,0,SEEK_SET); \
    } \
\
printf("plcLogFileInd=%ld\n",sysInfoP->data_source.plcLogFileInd); \
}

#ifndef __PLC_I_C__
void SendPlcStartCmd(void);
void SendPlcQuitCmd(void);
void SendPlcRestartCmd(void);
int plcInit(void);
#endif

#endif

