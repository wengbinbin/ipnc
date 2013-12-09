#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <termios.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

#include <sys_env_type.h>

static int proc_timer_run;


#define RS232   1
#define RS485   2
#define PRO_MODBUS_RTU    20
#define PRO_KUNLUN    100
#define uchar unsigned char
#define uint unsigned int


#define DEV "/dev/ttyXM1"
#define DI_1  "/proc/gio/gio85"
#define DI_2  "/proc/gio/gio86"
#define DI_3  "/proc/gio/gio87"
#define DI_4  "/proc/gio/gio88"
#define DO_1  "/proc/gio/gio49"
#define DO_2  "/proc/gio/gio35"
#define DO_3  "/proc/gio/gio36"
#define DO_4  "/proc/gio/gio37"


#define DEBUG 0
#define MODBUSPORT   502
#define IEC104PORT 2404    // the port users will be connecting to

#define BACKLOG 8     // how many pending connections queue will hold

#define BUF_SIZE 1024

#define IEC104_FRAME_MAX_SIZE     255
#define IEC104_ASDU_MAX_SIZE      249
#define IEC104_S_FRAME_LEN        6
#define IEC104_U_FRAME_LEN        6
#define IEC104_S_BIT     0x01
#define IEC104_U_BIT     0x03

#define IEC104_START_BIT      0x68
#define IEC104_U_TESTFR_V       0x43
#define IEC104_U_TESTFR_C       0x83
#define IEC104_U_STOPDT_V      0x13
#define IEC104_U_STOPDT_C      0x23
#define IEC104_U_STARTDT_V     0x07
#define IEC104_U_STARTDT_C     0x0b

#define  IEC104_INIT   0
#define  START_CONF    1
#define  TEST_CONF     2





/* data_type */


/************************************************************************************/

struct DEV_TAB
{
    int flag;
    int addr;     /* dev_id */
    int data_type;    /* 0x03: read holding register, 0x04: read input register */
	int start_addr;
	int reg_cnt;
	int plc_ind;
	
	int speed;
    int parity;  /* 0:不校验/1：奇校验/2：偶校验 */
    int stopbit;        /* 1/2 */
    int databit;        /* 7/8 */
	int streamctrl;
    
};


struct RS_PORT_TAB
{
	int flag;
    /*int  rs_id;*/
    int  sub_id;
    int protocol;
    int poll_int;
    int expire;
    
    char dev_name[12];
    int fd;
    char type;      /* rs232/rs485 */
    int t10;        /* poll interval,default is 5s */
    int t5;         /* expire */
    
    struct DEV_TAB    tab[CHANNEL_NUM];
    
    int buf_len;
    char asyn_buf[256];
    
};


/* sys par */
extern SysInfo *sysInfoP;


/* local par */
struct DATA_SOURCE   data_source;
struct MODBUS_RS     *m_rs;/*[RS_PORT_NUM];*/
struct MODBUS_TCP    *m_tcp;/*[MODBUSTCP_NUM];*/

unsigned char    plc_hold_reg[REG_NUM];
unsigned char    plc_input_reg[REG_NUM];
unsigned char     plc_discrete[DIS_NUM];

int maxsock;

#if 0
struct PLC_ALARM
{
    int flag;
    int int_type;
    char int_name[20];
    int dev_id;
    char reason[20];
};

#define PLC_ALARM_NUM   10
struct PCL_ALARM    plc_alarm_tab[PLC_ALARM_NUM];
#endif

int file_d1,file_d2,file_d3,file_d4,file_d5,file_d6,file_d7,file_d8;

struct MODBUS_MBAP
{
    unsigned short  obj_seq;
    unsigned short  pro_id;
    unsigned short  len;
    unsigned char   addr;
};

struct  MODBUSTCP_CLIENT
{
    int fd;    // accepted connection fd
    struct sockaddr_in client_addr;
    int buf_len;
	unsigned char tcp_buf[IEC104_FRAME_MAX_SIZE];
	unsigned char modbus_frame[IEC104_FRAME_MAX_SIZE];    /* just for test */
};

struct  MODBUSTCP_GET
{
    
    int flag;    /* used or not */
	char ip_addr[16];
	int tcp_port;
	int t10;    /* poll_intval */
	int t20;        /* expire */

    int poll_intval;
    int expire;
    
	int dev_id;
	int data_type;
	int plc_ind;
	int reg_cnt;
	int  start_addr;      /* support 8 dev_id */

	int fd;    // accepted connection fd
    int buf_len;
	unsigned char tcp_buf[IEC104_FRAME_MAX_SIZE];
	unsigned char modbus_frame[IEC104_FRAME_MAX_SIZE];    /* just for test */

};

struct IEC104_APCI
{
	unsigned char start_byte;
	unsigned char apdu_len;
	unsigned char clrt_byte1;
    unsigned char clrt_byte2;
	unsigned char clrt_byte3;
	unsigned char clrt_byte4;

};


struct IEC104_I_FRAME
{
	
	struct IEC104_APCI    apci;

	unsigned char asdu[IEC104_ASDU_MAX_SIZE];

};

struct IEC104_ASDU
{
	unsigned char  type_id;
	unsigned char  asdu_sq;
	unsigned char  tx_reason;
	/*unsigned char  addr;
	unsigned short  info_addr;
	unsigned char  info_data[243];*/
	
};


struct  IEC104_CLIENT
{
    int fd;    // accepted connection fd
    struct sockaddr_in client_addr;
    int client_state;
    int vr;
    int vs;
    int t1;
    int t2;
    int t3;
    int win_k;
    int win_w;
	int buf_len;
	unsigned char tcp_buf[IEC104_FRAME_MAX_SIZE];
	unsigned char iec104_frame[IEC104_FRAME_MAX_SIZE];    /* just for test */
};

/*
t1: 发送I帧或S帧后，等待确认时间，否则会关闭链接并重建， default: 15s
t2: 接受方在收到I帧后t2时间没有再收到I帧，必须发送S帧确认, default: 10s
t3: 无报文下发送测试帧超时，default 20s
win_k:  在没有收到确认前，可以连续发送k个I帧，在被控站侧 default: 12
win_w:  连续收到w个I帧才发送确认，而不一定要等到超时，被控站 default: 1

*/

struct  IEC104_PAR
{
    int t1;
    int t2;
    int t3;
    int win_k;
    int win_w;
};

struct IEC104_CLIENT      client_tab[BACKLOG];
struct MODBUSTCP_CLIENT      modbus_tab[BACKLOG];
struct MODBUSTCP_GET      modbus_tcpget_tab[MODBUSTCP_NUM];
struct RS_PORT_TAB   rs_tab[RS_PORT_NUM];
int conn_amount_104,conn_amount_modbus;

struct IEC104_PAR   iec104_par;
static sem_t sem;



int speed_arr[] = { B38400, B19200, B9600, B4800, B2400, B1200, B300,
B38400, B19200, B9600, B4800, B2400, B1200, B300, };
int name_arr[] = {38400, 19200, 9600, 4800, 2400, 1200, 300, 38400,
19200, 9600, 4800, 2400, 1200, 300, };


const uchar auchCRCLo[]=
{ 
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 
0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 
0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7, 
0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 
0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 
0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 
0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91, 


0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 
0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 
0x40 
};


/* Table of CRC values for highšCorder byte */  
const uchar auchCRCHi[]=
{
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 
0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 
0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 
0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 
0x40 
};


uint getCRC16(uchar* array,uchar length)
{
	uchar uchCRCHi = 0xFF ; 
	uchar uchCRCLo = 0xFF ; 
	uchar uIndex ; 

	while(length--)
	{
		uIndex=uchCRCHi^*array++;
		uchCRCHi=uchCRCLo^auchCRCHi[uIndex];
		uchCRCLo=auchCRCLo[uIndex];
	}
	
	return ((uchCRCHi<<8)|uchCRCLo);
}


void start_t1_timer(int tab_id)
{
    client_tab[tab_id].t1=iec104_par.t1;
}

void stop_t1_timer(int tab_id)
{
	client_tab[tab_id].t1=0;
}

void start_t2_timer(int tab_id)
{
    client_tab[tab_id].t2=iec104_par.t2;
    #if DEBUG==0
    printf("client_tab[%u] start_t2_timer=%u\n",tab_id,client_tab[tab_id].t2);
    #endif
}
void stop_t2_timer(int tab_id)
{
	client_tab[tab_id].t2=0;
}


void start_t3_timer(int tab_id)
{
    client_tab[tab_id].t3=iec104_par.t3;
}

void stop_t3_timer(int tab_id)
{
	client_tab[tab_id].t3=0;
}

void set_speed(int fd, int speed)
{
	int i;
	int status;
	struct termios Opt;

	tcgetattr(fd, &Opt);
	for ( i= 0; i < sizeof(speed_arr) / sizeof(int); i++) 
	{
		if (speed == name_arr[i]) 
		{
		    #if DEBUG==20
		    printf("name_arr[%d].speed=%u\n",i,speed);
		    #endif
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&Opt, speed_arr[i]);
			cfsetospeed(&Opt, speed_arr[i]);
			
			status = tcsetattr(fd, TCSANOW, &Opt);
			if (status != 0) 
			{
				perror("tcsetattr fd");
				return;
			}
			tcflush(fd,TCIOFLUSH);
			
			return;
		}
	}
}

void set_Parity(int fd,int databits,int stopbits,int parity)
{
	struct termios options;

	if ( tcgetattr( fd,&options) != 0) 
	{
		perror("SetupSerial 1");
		return;
	}
	options.c_cflag &= ~CSIZE;   /* 设置屏蔽位 */

	switch (databits) /*设置数据位数*/
	{
		case 7:
		options.c_cflag |= CS7;
		break;
		case 8:
		#if DEBUG==20
		printf("databits=CS8\n");
		#endif
		options.c_cflag |= CS8;
		break;
		default:
			printf("Unsupported data size\n"); 
			return;
	}

	switch (parity)
	{
		case 0:
			options.c_cflag &= ~PARENB; /* Clear parity enable ，两个停止位*/
			/*options.c_iflag &= ~INPCK;*/ /* Enable parity checking */
		break;
		case 1:
			options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
			/*options.c_iflag |= INPCK;*/ /* Disnable parity checking */
		break;
		case 2:
			options.c_cflag |= PARENB; /* Enable parity */
			options.c_cflag &= ~PARODD; /* 转换为偶效验*/
			/*options.c_iflag |= INPCK;*/ /* Disnable parity checking */
		break;
		case 4: /*as no parity*/
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;      /* 一个停止位 */
			break;
		
	}
/* 设置停止位*/
	switch (stopbits)
	{
		case 1:
		#if DEBUG==20
		printf("stopbits=1\n");
		#endif
			options.c_cflag &= ~CSTOPB;
		break;
		case 2:
			options.c_cflag |= CSTOPB;
		break;
		default:
			fprintf(stderr,"Unsupported stop bits\n");
		return;
	}

	options.c_oflag  &= ~OPOST; /* 取消输出后处理，使用原始数据 */
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);   /* 使用原始输入数据，不使能规范输入 */

	
	tcflush(fd,TCIFLUSH);
	options.c_cc[VTIME] = 100; /* 设置超时15 seconds*/
	options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
	if (tcsetattr(fd,TCSANOW,&options) != 0)
	{
		perror("SetupSerial 3");
		return;
	}
	return;
}


void set_rs_par(int tab_id,int sub_id)
{
    struct termios newtio; 
    int fd;
    
    fd=rs_tab[tab_id].fd;    
        
    set_speed(fd, rs_tab[tab_id].tab[sub_id].speed);
	set_Parity(fd,rs_tab[tab_id].tab[sub_id].databit,rs_tab[tab_id].tab[sub_id].stopbit,rs_tab[tab_id].tab[sub_id].parity);

	tcgetattr(fd,&newtio);
	if(rs_tab[tab_id].tab[sub_id].streamctrl==0)
	{
			newtio.c_cflag &= ~CRTSCTS;         /*  不流控  */
	}
	else if(rs_tab[tab_id].tab[sub_id].streamctrl==1)      /* 硬件流控 */
	{
			newtio.c_cflag |= CRTSCTS;
	}
	else
	{
			newtio.c_cflag |= IXON|IXOFF|IXANY;
	}

	tcflush(fd,TCIFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);
}


int init_dev(int tab_id)
{ 
        int fd;
              
        
#if DEBUG==10
        printf("init_dev!\n");
#endif
               
        fd = open(rs_tab[tab_id].dev_name, O_RDWR|O_NOCTTY|O_NDELAY);      
        
        if (fd ==-1)
        {
            printf("error open\n");
            
            return -1;
        }
        else    
        {
            fcntl(fd, F_SETFL, 0);
        }
        
        rs_tab[tab_id].fd=fd;
        
        if(fd>maxsock)
        {
            maxsock=fd;
        }
       
        return fd;
}




void send_I_frame(int tab_id,char *buf,int len)
{
    unsigned char send_buf[IEC104_FRAME_MAX_SIZE];
    int i;
    
    #if DEBUG==0
    printf("send_iec104_I_frame\n");
    #endif

	if(client_tab[tab_id].client_state==IEC104_INIT)
	{
		return;
	}

	send_buf[0]=IEC104_START_BIT;
    send_buf[1]=12+len;
    send_buf[2]=((client_tab[tab_id].vs % 256)<<1) & 0xfe;
    send_buf[3]=client_tab[tab_id].vs /256;
    send_buf[4]=((client_tab[tab_id].vr % 256)<<1) & 0xfe;
    send_buf[5]=client_tab[tab_id].vr /256;
    
    send_buf[6]=130;     /* used defined ASDU id */
    send_buf[7]=1;       /* 一个信息体,SQ=0 */
    send_buf[8]=43;      /*  传送原因 */
    send_buf[9]=0;       /* 公共地址 */
    send_buf[10]=0;
    send_buf[11]=2;      /* 信息体地址 */
    
	memcpy(&send_buf[12],buf,len);   /* 信息体内容 */

    #if DEBUG==0
    printf("send_I_frame:  ");
    for(i=0;i<send_buf[1];i++)
    {
        printf("%0x ",send_buf[i]);
    }
    printf("\n");
    #endif

	if(client_tab[tab_id].win_k)
	{
		client_tab[tab_id].win_k--;
		send(client_tab[tab_id].fd,send_buf,send_buf[1],0);
		start_t1_timer(tab_id);
		stop_t2_timer(tab_id);
		client_tab[tab_id].vs++;
	}
	else
	{
		/* add to send_queue,coming soon  */
		#if DEBUG==0
		printf("add to send_queue\n");
		#endif

	}

}


void showclient()
{
    
    printf("client amount 104: %d\n", conn_amount_104);
    printf("client amount modbusrs: %d\n", conn_amount_modbus);
    printf("\n");
}

void process_iec104_S(int tab_id)
{
	int  nr;
	struct IEC104_APCI   apci;

	apci=*(struct IEC104_APCI *)(client_tab[tab_id].iec104_frame);

	nr=(apci.clrt_byte4 <<8)+(apci.clrt_byte3 >> 1);

	#if DEBUG==0
	printf("S frame NR=%u\n",nr);
	#endif

	client_tab[tab_id].win_k=iec104_par.win_k-client_tab[tab_id].vs+nr;

}

void send_iec104_startdt_c(int tab_id)
{
    unsigned char send_buf[6];
    
    #if DEBUG==0
    printf("send_iec104_startdt_c\n");
    #endif
    
    send_buf[0]=IEC104_START_BIT;
    send_buf[1]=IEC104_U_FRAME_LEN-2;
    send_buf[2]=IEC104_U_STARTDT_C;
    send_buf[3]=0;
    send_buf[4]=0;
    send_buf[5]=0;
    
    send(client_tab[tab_id].fd,send_buf,6,0);
    
}

void send_iec104_testfr_c(int tab_id)
{
    unsigned char send_buf[6];
    
    #if DEBUG==0
    printf("send_iec104_testfr_c\n");
    #endif
    


    send_buf[0]=IEC104_START_BIT;
    send_buf[1]=IEC104_U_FRAME_LEN-2;
    send_buf[2]=IEC104_U_TESTFR_C;
    send_buf[3]=0;
    send_buf[4]=0;
    send_buf[5]=0;
    
    send(client_tab[tab_id].fd,send_buf,6,0);
    
}

void send_iec104_stopdt_c(int tab_id)
{
    unsigned char send_buf[6];
    
    #if DEBUG==0
    printf("send_iec104_testfr_c\n");
    #endif
    
    send_buf[0]=IEC104_START_BIT;
    send_buf[1]=IEC104_U_FRAME_LEN-2;
    send_buf[2]=IEC104_U_STOPDT_C;
    send_buf[3]=0;
    send_buf[4]=0;
    send_buf[5]=0;
    
    send(client_tab[tab_id].fd,send_buf,6,0);
    
}



void process_iec104_U(int tab_id)
{
	struct IEC104_APCI   apci;
	
	
	#if DEBUG==0
	printf("process_iec104_u\n");
	#endif

	apci=*(struct IEC104_APCI *)(client_tab[tab_id].iec104_frame);

    switch(apci.clrt_byte1)
	{
	case IEC104_U_TESTFR_V:
	    #if DEBUG==0
	    printf("recv_testfr_v\n");
	    #endif
	    /*client_tab[tab_id].client_state=START_CONF;*/
	    send_iec104_testfr_c(tab_id);
		break;
	case IEC104_U_TESTFR_C:
		break;
	case IEC104_U_STOPDT_V:
	    #if DEBUG==0
	    printf("recv_stopdt_v\n");
	    #endif
	    client_tab[tab_id].client_state=IEC104_INIT;
	    send_iec104_stopdt_c(tab_id);
		break;
	case IEC104_U_STOPDT_C:
		break;
	case IEC104_U_STARTDT_V:
	    #if DEBUG==0
	    printf("recv_startdt_v\n");
	    #endif
	    client_tab[tab_id].client_state=START_CONF;
	    send_iec104_startdt_c(tab_id);
		break;
	case IEC104_U_STARTDT_C:
		break;

	}

}

void process_iec104_I(int tab_id)
{
	int  ns,nr;
	struct IEC104_APCI   apci;
	struct IEC104_ASDU   asdu;
	

	
	apci=*(struct IEC104_APCI *)(client_tab[tab_id].iec104_frame);

	ns=(apci.clrt_byte2 <<8)+(apci.clrt_byte1 >> 1);

	nr=(apci.clrt_byte4 <<8)+(apci.clrt_byte3 >> 1);

    #if DEBUG==0
	printf("tab_id=%u,I frame NS=%u, NR=%u\n",tab_id,ns,nr);
	#endif

	if(ns==client_tab[tab_id].vr)
	{
		#if DEBUG==0
		printf("receive send_seq==V(R)\n");
		#endif
		if(nr==client_tab[tab_id].vs)
		{
			#if DEBUG==0
			printf("receive right ack,right frame\n");
			#endif

			client_tab[tab_id].win_k=iec104_par.win_k-client_tab[tab_id].vs+nr;

			client_tab[tab_id].vr++;
			start_t2_timer(tab_id);

			asdu=*(struct IEC104_ASDU *)(&(client_tab[tab_id].iec104_frame[6]));
			switch(asdu.type_id)
			{
				case 100:     /* 总召唤 */
					/*send_I_frame(tab_id,send_buf,len);*/
					#if DEBUG==0
					printf("recv 100,send_i_frame null\n");
					#endif
					break;
				case 101:     /* 计数量召唤命令  */
					break;
				case 102:     /* 读命令  */
					break;
				case 103:     /* 时钟同步命令  */
					break;
				case 105:     /* 复位进程命令 */
					break;
				case 200:     /* 自定义 */
					break;
			}

		}
		else
		{
			#if DEBUG==0
			printf("error nr: %u,local vs: %u,reboot channel\n",nr,client_tab[tab_id].vs);
			#endif

			close(client_tab[tab_id].fd);
		}
	}
	else
	{
		#if DEBUG==0
		printf("recv wrong seq,reboot channel\n");
		#endif

		close(client_tab[tab_id].fd);

	}
	
	
	
}

void get_modbus_frame(int tab_id)
{
    struct MODBUS_MBAP   mbap;
    unsigned char send_buf[12];
    int plc_id,reg_cnt;
    int i,j,ind;
	int cnt,tmp;
    
    #if DEBUG==20
    printf("get_modbustcp_frame: tab_id=%u\n",tab_id);
    #endif
    
    mbap=*(struct MODBUS_MBAP *)(modbus_tab[tab_id].modbus_frame);
        
    memcpy(&send_buf[0],&(modbus_tab[tab_id].modbus_frame[0]),4);
    
    if(modbus_tab[tab_id].modbus_frame[7]==READ_HOLDING_REGISTER)
    {
        plc_id=modbus_tab[tab_id].modbus_frame[9];
		
        reg_cnt=modbus_tab[tab_id].modbus_frame[11];

		if((plc_id>=DIS_NUM) && (plc_id<REG_NUM+DIS_NUM))
		{
			#if DEBUG==0
			printf("plc_id=%d,reg_cnt=%d\n",plc_id,reg_cnt);
			#endif
			plc_id=plc_id-DIS_NUM;
			send_buf[4]=0;
			send_buf[5]=reg_cnt*2+3;
			send_buf[6]=modbus_tab[tab_id].modbus_frame[6];
			send_buf[7]=READ_HOLDING_REGISTER;
			send_buf[8]=reg_cnt*2;
			
			for(i=0;i<reg_cnt*2;i++)
			{
				send_buf[9+i]=plc_hold_reg[plc_id+i];
				
			}
			
		}
		else
		{
			send_buf[4]=0;
			send_buf[5]=0x03;
			send_buf[6]=modbus_tab[tab_id].modbus_frame[6];
			send_buf[7]=READ_HOLDING_REGISTER | 0x80;
			send_buf[8]=0x02;     /* just do this error code */
		}
        
        
        #if DEBUG==0
        for(j=0;j<6+send_buf[5];j++)             
        {
            printf(" %x ", send_buf[j]);
        }
        #endif
        
        send(modbus_tab[tab_id].fd,send_buf,6+send_buf[5],0);

    }
	else if(modbus_tab[tab_id].modbus_frame[7]==READ_INPUT_REGISTER)
    {
        plc_id=modbus_tab[tab_id].modbus_frame[9];
		
        reg_cnt=modbus_tab[tab_id].modbus_frame[11];

		if((plc_id>=(REG_NUM+DIS_NUM)) && ((plc_id+reg_cnt)<(2*REG_NUM+DIS_NUM)))
		{
			#if DEBUG==0
			printf("plc_id=%d,reg_cnt=%d\n",plc_id,reg_cnt);
			#endif
			plc_id=plc_id-DIS_NUM-REG_NUM;
			send_buf[4]=0;
			send_buf[5]=reg_cnt*2+3;
			send_buf[6]=modbus_tab[tab_id].modbus_frame[6];
			send_buf[7]=READ_INPUT_REGISTER;
			send_buf[8]=reg_cnt*2;
			
			for(i=0;i<reg_cnt*2;i++)
			{
				send_buf[9+i]=plc_input_reg[plc_id+i];
				
			}
		}
		else
		{
			send_buf[4]=0;
			send_buf[5]=0x03;
			send_buf[6]=modbus_tab[tab_id].modbus_frame[6];
			send_buf[7]=READ_INPUT_REGISTER | 0x80;
			send_buf[8]=0x02;     /* just do this error code */
		}
        
        
        #if DEBUG==0
        for(j=0;j<6+send_buf[5];j++)             
        {
            printf(" %x ", send_buf[j]);
        }
        #endif
        
        send(modbus_tab[tab_id].fd,send_buf,6+send_buf[5],0);
    }
	else if(modbus_tab[tab_id].modbus_frame[7]==READ_DISCRETE)
    {
        #if DEBUG==0
        printf("read discrete\n");
        #endif
        plc_id=modbus_tab[tab_id].modbus_frame[9];
		
        reg_cnt=modbus_tab[tab_id].modbus_frame[11];

		if((plc_id>=0) && (plc_id<DIS_NUM) && ((plc_id+reg_cnt)<DIS_NUM))
		{
			#if DEBUG==0
			printf("plc_id=%d,reg_cnt=%d\n",plc_id,reg_cnt);
			#endif

			cnt=reg_cnt/8;
			tmp=reg_cnt%8;

			send_buf[4]=0;
			if(tmp==0)
			{
				send_buf[5]=cnt+3;
			}
			else
			{
				send_buf[5]=cnt+1+3;
			}
			send_buf[6]=modbus_tab[tab_id].modbus_frame[6];
			send_buf[7]=READ_DISCRETE;
			send_buf[8]=send_buf[5]-3;
			
			for(i=0;i<cnt;i++)
			{		              
				ind=0;	          
			    while(8-ind)
				{
					send_buf[9+i]=send_buf[9+i] | (plc_discrete[plc_id+i*8+ind] <<  ind ) ;
					ind++;
			    }
				
			}
			#if DEBUG==0
			printf("i=%d,send_buf[9+i]=%u\n",i,send_buf[9+i]);
			#endif
			ind=0;
			while(tmp-ind)
			{
				send_buf[9+i]=send_buf[9+i] | (plc_discrete[plc_id+i*8+ind] <<  ind ) ;
				ind++;
			}

			
		}
		else
		{
			send_buf[4]=0;
			send_buf[5]=0x03;
			send_buf[6]=modbus_tab[tab_id].modbus_frame[6];
			send_buf[7]=READ_DISCRETE | 0x80;
			send_buf[8]=0x02;     /* just do this error code */
		}
        
        
        #if DEBUG==0
        for(j=0;j<6+send_buf[5];j++)             
        {
            printf(" %x ", send_buf[j]);
        }
        #endif
        
        send(modbus_tab[tab_id].fd,send_buf,6+send_buf[5],0);
    }
    else if(modbus_tab[tab_id].modbus_frame[7]==WRITE_HOLDING_REGISTER)
    {
        #if DEBUG==0
        printf("plc_id=%d,reg=%x  %x\n",modbus_tab[tab_id].modbus_frame[9],modbus_tab[tab_id].modbus_frame[10],modbus_tab[tab_id].modbus_frame[11]);
        #endif
        
        if((modbus_tab[tab_id].modbus_frame[10]==0) && ((modbus_tab[tab_id].modbus_frame[11]==1) || (modbus_tab[tab_id].modbus_frame[11]==0)))
        {
        
            plc_id=modbus_tab[tab_id].modbus_frame[9];
            plc_hold_reg[plc_id]=modbus_tab[tab_id].modbus_frame[10];
            plc_hold_reg[plc_id+1]=modbus_tab[tab_id].modbus_frame[11];

            
            /*if((plc_id>=20)  && (plc_id<24))
            {
                #if DEBUG==0
                printf("plc_id=%d,setgio=%d\n",plc_id,plc_hold_reg[plc_id].reg[1]);
                #endif
                
                if(plc_id==20)
                {
                                        
                    if(plc_hold_reg[plc_id].reg[1]==1)
                    {   
                        #if DEBUG==0
                        printf("set 1 to gio49\n");
                        #endif
                        
                        write(file_d5,"1",strlen("1"));
                    }
                    else
                    {
                        write(file_d5,"0",strlen("0"));
                    }
                    
                    
                }
                else if(plc_id==21)
                {
                                        
                    if(plc_hold_reg[plc_id].reg[1]==1)
                    {   
                        #if DEBUG==0
                        printf("set 1 to gio35\n");
                        #endif
                        
                        write(file_d6,"1",strlen("1"));
                    }
                    else
                    {
                        write(file_d6,"0",strlen("0"));
                    }
                    
                }
                else if(plc_id==22)
                {
                                        
                    if(plc_hold_reg[plc_id].reg[1]==1)
                    {   
                        #if DEBUG==0
                        printf("set 1 to gio36\n");
                        #endif
                        
                        write(file_d7,"1",strlen("1"));
                    }
                    else
                    {
                        write(file_d7,"0",strlen("0"));
                    }
                    
                }
                else if(plc_id==23)
                {
                                                            
                    if(plc_hold_reg[plc_id].reg[1]==1)
                    {   
                        #if DEBUG==0
                        printf("set 1 to gio37\n");
                        #endif
                        
                        write(file_d8,"1",strlen("1"));
                    }
                    else
                    {
                        write(file_d8,"0",strlen("0"));
                    }
                   
                }
            }
           */
            
            memcpy(&send_buf[4],&(modbus_tab[tab_id].modbus_frame[4]),8);     
            
            #if DEBUG==0
            for(j=0;j<6+send_buf[5];j++)             
            {
                printf(" %x ", send_buf[j]);
            }
            #endif
            
            send(modbus_tab[tab_id].fd,send_buf,6+send_buf[5],0);
        }
        else
        {
            send_buf[4]=0;
            send_buf[5]=2;
            send_buf[6]=0x86;
            send_buf[7]=0x03;
            
            #if DEBUG==0
            for(j=0;j<6+send_buf[5];j++)             
            {
                printf(" %x ", send_buf[j]);
            }
            #endif
            
            send(modbus_tab[tab_id].fd,send_buf,6+send_buf[5],0);
            
        }
    }
	else
	{
		send_buf[4]=0;
		send_buf[5]=0x03;
		send_buf[6]=modbus_tab[tab_id].modbus_frame[6];
		send_buf[7]=modbus_tab[tab_id].modbus_frame[7] | 0x80;
		send_buf[8]=0x01;     /* not support this func code */
	}
    
    
}

void get_modbus_tcp(int tab_id)
{
    int plc_id,reg_len;
	int cnt,ind,i;
    
    #if DEBUG==0
    printf("get_modbustcp_frame_from server: tab_id=%u\n",tab_id);
    #endif
    
	if(modbus_tcpget_tab[tab_id].dev_id!=modbus_tcpget_tab[tab_id].modbus_frame[6])
	{
#if DEBUG==0
		printf("main data error from device %d\n",modbus_tcpget_tab[tab_id].dev_id);
#endif
		
		
	}
	else
	{
		switch(modbus_tcpget_tab[tab_id].modbus_frame[7])
		{
			case READ_HOLDING_REGISTER:
				plc_id=modbus_tcpget_tab[tab_id].plc_ind;
				if(plc_id>=REG_NUM)
			    {
			        #if DEBUG==0
			        printf("illegle address\n");
			        #endif
			        
			        return;
			    }
				reg_len=modbus_tcpget_tab[tab_id].modbus_frame[5];
				memcpy(&plc_hold_reg[plc_id],&(modbus_tcpget_tab[tab_id].modbus_frame[9]),reg_len-3);
				break;
			case READ_INPUT_REGISTER:
				plc_id=modbus_tcpget_tab[tab_id].plc_ind;
				if(plc_id>=REG_NUM)
			    {
			        #if DEBUG==0
			        printf("illegle address\n");
			        #endif
			        
			        return;
			    }
				reg_len=modbus_tcpget_tab[tab_id].modbus_frame[5];
				memcpy(&plc_input_reg[plc_id],&(modbus_tcpget_tab[tab_id].modbus_frame[9]),reg_len-3);
				
				#if DEBUG==0
					        printf("\n!!! plc_read input register: ");
					        for(i=0;i<REG_NUM;i++)
					        {
						        printf("%0x ",plc_input_reg[i]);
					        }
					        printf("\n");
		        #endif
				
				break;
			case READ_DISCRETE:
			    #if DEBUG==0
			    printf("read_discrete,plc_ind=%d,frame[9]=%d\n",modbus_tcpget_tab[tab_id].plc_ind,modbus_tcpget_tab[tab_id].modbus_frame[9]);
			    #endif
				plc_id=modbus_tcpget_tab[tab_id].plc_ind;
				if((plc_id<4) || (plc_id>=DIS_NUM))
			    {
			        #if DEBUG==0
			        printf("illegle address\n");
			        #endif
			        
			        return;
			    }
				cnt=modbus_tcpget_tab[tab_id].modbus_frame[8];
				reg_len=modbus_tcpget_tab[tab_id].reg_cnt;
				 				
				#if DEBUG==0
				printf("cnt=%d,reg_cnt=%d\n",cnt,reg_len);
				#endif
				
				
				for(i=0;i<cnt;i++)
				{
					ind=0;
					while(8-ind)
					{
						plc_discrete[plc_id+i*8+ind]=((modbus_tcpget_tab[tab_id].modbus_frame[9+i]) >> ind) & 0x01;
						ind++;
						reg_len--;
						if(reg_len==0)
						{
						    break;
						}
					}
						            
				}
						        
				#if DEBUG==0
				printf("\n!!! plc_read discrete  ");
				for(i=0;i<DIS_NUM;i++)
				{
					printf("%0x ",plc_discrete[i]);
				}
				printf("\n");
				#endif

				break;
			default:
				printf("get modbus tcp,unknown type\n");
				return;
		}
	}
    

	close(modbus_tcpget_tab[tab_id].fd);
	modbus_tcpget_tab[tab_id].t20=0;
	modbus_tcpget_tab[tab_id].fd=0;
    
}

void get_iec104_frame(int tab_id)
{
    struct IEC104_APCI   apci;
    
    #if DEBUG==0
    printf("get_iec104_frame: tab_id=%u\n",tab_id);
    #endif

	apci=*(struct IEC104_APCI *)(client_tab[tab_id].iec104_frame);

	switch ((apci.clrt_byte1) & 0x03)
	{
	case IEC104_S_BIT:
		process_iec104_S(tab_id);
		break;
	case IEC104_U_BIT:
		process_iec104_U(tab_id);
		break;
	default:
	    #if DEBUG==0
	    printf("iec104_i_bit\n");
	    #endif
		if((client_tab[tab_id].client_state) > IEC104_INIT)
		{
			process_iec104_I(tab_id);
		}
		break;
	}
}

void process_tcp104_stream(int tab_id,char *tcp_buf,int buf_len)
{
    int j,ind,frame_len,local_ind;
    
    printf("process_tcp104_data: client[%d].srcaddr=%s, src_port=%u\n",tab_id,inet_ntoa(client_tab[tab_id].client_addr.sin_addr),ntohs(client_tab[tab_id].client_addr.sin_port));
    
    for(j=0;j<buf_len;j++)             
    {
        printf(" %x ", tcp_buf[j]);
    }
    printf("\nfirst put to tcp_buf,then get one whole frame, maybe I/S/U frame\n");
    
    /*send(client_tab[tab_id].fd,"hello world\n",14,0);*/

	ind=0;

	while(buf_len)
	{

		if(client_tab[tab_id].buf_len==0)
		{
		    #if DEBUG==0
			printf("buf is zero\n");
			#endif
			
			if(tcp_buf[ind]==IEC104_START_BIT)
			{
				if((tcp_buf[ind+1]<=253) && (tcp_buf[ind+1]>=6-2))
				{
					/* maybe the right frame head */
					if(buf_len>=tcp_buf[ind+1]+2)
					{
						/* contain a whole frame */
						memcpy(&(client_tab[tab_id].iec104_frame[0]),&tcp_buf[ind],tcp_buf[ind+1]);
						get_iec104_frame(tab_id);

						
						buf_len -= (tcp_buf[ind+1]+2);
						ind += (tcp_buf[ind+1]+2);

												
					}
					else
					{
						printf("not a whole frame\n");
						memcpy(client_tab[tab_id].tcp_buf,&tcp_buf[ind],buf_len);
						client_tab[tab_id].buf_len=buf_len;

						buf_len=0;

					}

				}
				else
				{
				    printf("error frame 2\n");
				    close(client_tab[tab_id].fd);
				    return;
				}
			}
			else
			{
				printf("error frame\n");
				close(client_tab[tab_id].fd);
				return;
				
			}
		}
		else
		{
			printf("buf is not empty\n");
			if(client_tab[tab_id].buf_len==1)
			{
				frame_len=tcp_buf[ind];
				if(buf_len>=frame_len+1)
				{
					printf("get a whole frame\n");
					client_tab[tab_id].iec104_frame[0]=client_tab[tab_id].tcp_buf[0];
					memcpy(&(client_tab[tab_id].iec104_frame[1]),&tcp_buf[ind],tcp_buf[ind]+1);
					get_iec104_frame(tab_id);
					client_tab[tab_id].buf_len=0;
					ind += tcp_buf[ind]+1;
					buf_len -= tcp_buf[ind]+1;
				}
				else
				{
					local_ind=client_tab[tab_id].buf_len;
					memcpy(&(client_tab[tab_id].tcp_buf[local_ind]),&tcp_buf[ind],buf_len);
					client_tab[tab_id].buf_len += buf_len;
					
					buf_len=0;

				}

			}
			else
			{
				frame_len=client_tab[tab_id].tcp_buf[1];
				if(buf_len>=frame_len)
				{
					printf("get a whole frame\n");
					client_tab[tab_id].iec104_frame[0]=client_tab[tab_id].tcp_buf[0];
					client_tab[tab_id].iec104_frame[1]=client_tab[tab_id].tcp_buf[1];
					memcpy(&(client_tab[tab_id].iec104_frame[2]),&tcp_buf[ind],frame_len);
					get_iec104_frame(tab_id);
					client_tab[tab_id].buf_len=0;
					ind += frame_len;
					buf_len -= frame_len;
				}
				else
				{
					local_ind=client_tab[tab_id].buf_len;
					memcpy(&(client_tab[tab_id].tcp_buf[local_ind]),&tcp_buf[ind],buf_len);
					client_tab[tab_id].buf_len += buf_len;
					
					buf_len=0;
				}

			}

		}
	}

}

void process_tcpmodbus_stream(int tab_id,char *tcp_buf,int buf_len)
{
    int j,ind,frame_len,local_ind;
    
    printf("process_tcpmodbus_data: client[%d].srcaddr=%s, src_port=%u\n",tab_id,inet_ntoa(modbus_tab[tab_id].client_addr.sin_addr),ntohs(modbus_tab[tab_id].client_addr.sin_port));
    
    for(j=0;j<buf_len;j++)             
    {
        printf(" %x ", tcp_buf[j]);
    }
    
    
	ind=0;

	while(buf_len)
	{

		if(modbus_tab[tab_id].buf_len==0)
		{
			if(buf_len<6)
			{
			    memcpy(&(modbus_tab[tab_id].modbus_frame[0]),&tcp_buf[ind],buf_len);
			    modbus_tab[tab_id].buf_len=buf_len;
			    buf_len=0;
			}
			else 
			{
			    frame_len=(tcp_buf[4] << 8) + tcp_buf[5];
			    #if DEBUG==0
			    printf("frame_len=%u\n",frame_len);
			    #endif
			    if(frame_len+6<=buf_len)
				{	
				    /* contain a whole frame */
					memcpy(&(modbus_tab[tab_id].modbus_frame[0]),&tcp_buf[ind],frame_len+6);
					get_modbus_frame(tab_id);
					
					buf_len -= (frame_len+6);
					ind += (frame_len+6);
				}
                else
                {
                    #if DEBUG==0
                    printf("no a whole frame\n");
                    #endif
                    
                    memcpy(modbus_tab[tab_id].tcp_buf,&tcp_buf[ind],buf_len);
					modbus_tab[tab_id].buf_len=buf_len;
					
					buf_len=0;
												
				}
			}
					
		}
		else
		{
		    #if DEBUG==0
			printf("buf is not empty\n");
			#endif
			
			if(modbus_tab[tab_id].buf_len<6)
			{
				
				local_ind=modbus_tab[tab_id].buf_len;
				modbus_tab[tab_id].tcp_buf[local_ind]=tcp_buf[ind];
				modbus_tab[tab_id].buf_len ++;
					
				buf_len--;
				ind++;
			}
			else
			{
				frame_len=*(unsigned short *)(&modbus_tab[tab_id].tcp_buf[4]);
				if(buf_len>=frame_len+6-modbus_tab[tab_id].buf_len)
				{
				    #if DEBUG==0
					printf("get a whole frame\n");
					#endif
					
					memcpy(&(modbus_tab[tab_id].modbus_frame[0]),&(modbus_tab[tab_id].tcp_buf[0]),modbus_tab[tab_id].buf_len);
					local_ind=modbus_tab[tab_id].buf_len;
					memcpy(&(modbus_tab[tab_id].modbus_frame[local_ind]),&tcp_buf[ind],frame_len+6-local_ind);
					get_modbus_frame(tab_id);
					modbus_tab[tab_id].buf_len=0;
					
					ind += (frame_len+6-local_ind);
					buf_len -= (frame_len+6-local_ind);
					
				}
				else
				{
					local_ind=modbus_tab[tab_id].buf_len;
					memcpy(&(modbus_tab[tab_id].tcp_buf[local_ind]),&tcp_buf[ind],buf_len);
					modbus_tab[tab_id].buf_len += buf_len;
					
					buf_len=0;
				}

			}

		}
	}

}

void process_tcpget_modbus(int tab_id,char *tcp_buf,int buf_len)
{
    int j,ind,frame_len,local_ind;
    
#if DEBUG==0
	printf("process_tcpget_modbus\n");
    for(j=0;j<buf_len;j++)             
    {
        printf(" %0x ", tcp_buf[j]);
    }
#endif
    
    
	ind=0;

	while(buf_len)
	{

		if(modbus_tcpget_tab[tab_id].buf_len==0)
		{
			if(buf_len<6)
			{
			    memcpy(&(modbus_tcpget_tab[tab_id].modbus_frame[0]),&tcp_buf[ind],buf_len);
			    modbus_tcpget_tab[tab_id].buf_len=buf_len;
			    buf_len=0;
			}
			else 
			{
			    frame_len=(tcp_buf[4] << 8) + tcp_buf[5];
			    #if DEBUG==0
			    printf("frame_len=%u\n",frame_len);
			    #endif
			    if(frame_len+6<=buf_len)
				{	
				    /* contain a whole frame */
					memcpy(&(modbus_tcpget_tab[tab_id].modbus_frame[0]),&tcp_buf[ind],frame_len+6);
					get_modbus_tcp(tab_id);
					
					buf_len -= (frame_len+6);
					ind += (frame_len+6);
				}
                else
                {
                    #if DEBUG==0
                    printf("no a whole frame\n");
                    #endif
                    
                    memcpy(modbus_tcpget_tab[tab_id].tcp_buf,&tcp_buf[ind],buf_len);
					modbus_tcpget_tab[tab_id].buf_len=buf_len;
					
					buf_len=0;
												
				}
			}
					
		}
		else
		{
		    #if DEBUG==0
			printf("buf is not empty\n");
			#endif
			
			if(modbus_tcpget_tab[tab_id].buf_len<6)
			{
				
				local_ind=modbus_tcpget_tab[tab_id].buf_len;
				modbus_tcpget_tab[tab_id].tcp_buf[local_ind]=tcp_buf[ind];
				modbus_tcpget_tab[tab_id].buf_len ++;
					
				buf_len--;
				ind++;
			}
			else
			{
				frame_len=*(unsigned short *)(&modbus_tcpget_tab[tab_id].tcp_buf[4]);
				if(buf_len>=frame_len+6-modbus_tcpget_tab[tab_id].buf_len)
				{
				    #if DEBUG==0
					printf("get a whole frame\n");
					#endif
					
					memcpy(&(modbus_tcpget_tab[tab_id].modbus_frame[0]),&(modbus_tcpget_tab[tab_id].tcp_buf[0]),modbus_tcpget_tab[tab_id].buf_len);
					local_ind=modbus_tcpget_tab[tab_id].buf_len;
					memcpy(&(modbus_tcpget_tab[tab_id].modbus_frame[local_ind]),&tcp_buf[ind],frame_len+6-local_ind);
					get_modbus_tcp(tab_id);
					modbus_tcpget_tab[tab_id].buf_len=0;
					
					ind += (frame_len+6-local_ind);
					buf_len -= (frame_len+6-local_ind);
					
				}
				else
				{
					local_ind=modbus_tcpget_tab[tab_id].buf_len;
					memcpy(&(modbus_tcpget_tab[tab_id].tcp_buf[local_ind]),&tcp_buf[ind],buf_len);
					modbus_tcpget_tab[tab_id].buf_len += buf_len;
					
					buf_len=0;
				}

			}

		}
	}

}


void init_RS_port()
{
    int i,k;
    
    for(i=0;i<RS_PORT_NUM;i++)
    {
		rs_tab[i].flag=0;
		/*rs_tab[i].rs_id=-1;*/
        rs_tab[i].fd=0;
        rs_tab[i].sub_id=-1;
        rs_tab[i].t10=0;
        rs_tab[i].t5=0;
        rs_tab[i].buf_len=0;
        
        rs_tab[i].poll_int=10;
        rs_tab[i].expire=3;
        rs_tab[i].protocol=PROTOCOL_MODBUS;

        memset(rs_tab[i].asyn_buf,0,256);
        
        for(k=0;k<CHANNEL_NUM;k++)
        {
            rs_tab[i].tab[k].flag=0;
           
            rs_tab[i].tab[k].addr=0;
			rs_tab[i].tab[k].data_type=READ_HOLDING_REGISTER;
            rs_tab[i].tab[k].plc_ind=0;
			rs_tab[i].tab[k].reg_cnt=0;
			rs_tab[i].tab[k].start_addr=0;
			
			rs_tab[i].tab[k].speed=9600;
			rs_tab[i].tab[k].parity=0;
			rs_tab[i].tab[k].databit=8;
			rs_tab[i].tab[k].stopbit=1;
			rs_tab[i].tab[k].streamctrl=0;

        }
    }

	rs_tab[0].type=RS232;
	rs_tab[1].type=RS232;
	rs_tab[2].type=RS485;
	rs_tab[3].type=RS485;
    
    memcpy(rs_tab[0].dev_name,"/dev/ttyXX0",11);
    memcpy(rs_tab[1].dev_name,"/dev/ttyXX1",11);
    memcpy(rs_tab[2].dev_name,"/dev/ttyXM0",11);
    memcpy(rs_tab[3].dev_name,"/dev/ttyXM1",11);
    
}

void proc_sub_data(int tab_id,char *ptr,int len)
{
    int i;
	int plc_id;
	uint fcs;
	int sub_id;
	unsigned char crc_hi,crc_lo;
	
    #if DEBUG==0
    printf("\n!!! proc_sub_data: ");
    for(i=0;i<len;i++)
    {
        printf("%0x  ",ptr[i]);
    }
    printf("\n");
    #endif
    
    sub_id=rs_tab[tab_id].sub_id;
    
    if(ptr[0]!=rs_tab[tab_id].tab[sub_id].addr)
    {
        #if DEBUG==0
        printf("\nerror addr,return\n");
        #endif
        return;
    }
    
    rs_tab[tab_id].t5=0;
      	        
    fcs=getCRC16((unsigned char *)ptr,len-2);
    crc_hi=(fcs>>8) & 0xff;
    crc_lo=fcs & 0x00ff;
                
    #if DEBUG==0
    printf("crc_hi=%0x, crc_lo=%0x\n",crc_hi,crc_lo);
    #endif
                
    if((crc_hi!=ptr[len-2]) || (crc_lo!=ptr[len-1]))
    {
        #if DEBUG==0
        printf("device %d CRC error\n",rs_tab[tab_id].tab[sub_id].addr);
        #endif
                    
        return;
     }
      	        
     if((ptr[1] & 0x7f)!=rs_tab[tab_id].tab[sub_id].data_type) 
	 {
	    #if DEBUG==0
	    printf("device %d func code error,data_type=%d,sub_id=%d\n",rs_tab[tab_id].tab[sub_id].addr,rs_tab[tab_id].tab[sub_id].data_type,sub_id);
	    #endif
				    
		return;
	}
	else if( (ptr[1] & 0x80)==0 )         
	{    
	    switch(ptr[1])
	    { 	
	        case READ_INPUT_REGISTER:
				plc_id=rs_tab[tab_id].tab[sub_id].plc_ind;

				memcpy(&plc_input_reg[plc_id],&ptr[3],ptr[2]);
				break;
	        case READ_HOLDING_REGISTER: 
				plc_id=rs_tab[tab_id].tab[sub_id].plc_ind;

				memcpy(&plc_hold_reg[plc_id],&ptr[3],ptr[2]);

			    #if DEBUG==0
				printf("\n!!! plc_read holding register: ");
				for(i=0;i<REG_NUM;i++)
				{
				    printf("%0x ",plc_hold_reg[i]);
				}
				printf("\n");
				#endif
					        
				break;
					    	        
		}

    }
    else
    {
		switch(ptr[2])
	    {
		    case 0x01:
				#if DEBUG==0
				printf("device %d func code error222\n",rs_tab[tab_id].tab[sub_id].addr);
				#endif
				break;
			case 0x02:
				#if DEBUG==0
				printf("device %d address error\n",rs_tab[tab_id].tab[sub_id].addr);
				#endif
				break;
		    case 0x03:
				#if DEBUG==0
				printf("device %d count error\n",rs_tab[tab_id].tab[sub_id].addr);
				#endif
				break;
			case 0x04:
				#if DEBUG==0
				printf("device %d read/write error\n",rs_tab[tab_id].tab[sub_id].addr);
				#endif
				break;

		}
    }  	    

}

void check_modbus_frame(int tab_id,char *ptr,int len)
{
    int frame_len;
    int ind;
    
    if(rs_tab[tab_id].buf_len==0)
    {
        if(len>=3)
        {
            if(ptr[1]&0x80)
            {
                if(5<=len)
                {
                    proc_sub_data(tab_id,ptr,5); 
                                      
                }
                else
                {
                    memcpy(rs_tab[tab_id].asyn_buf,ptr,len);
                                  
                    rs_tab[tab_id].buf_len=len;
                }
            }
            else
            {
                frame_len=ptr[2]; 
                if(5+frame_len<=len)
                {
                    proc_sub_data(tab_id,ptr,5+frame_len);                         
                                    
                } 
                else
                {
                    memcpy(rs_tab[tab_id].asyn_buf,ptr,len);
                                 
                    rs_tab[tab_id].buf_len=len;
                }
            }
        }
        else  /* len<3 */
        {
            memcpy(rs_tab[tab_id].asyn_buf,ptr,len);
                            
            rs_tab[tab_id].buf_len=len;
        }
                        
    }
    else  /* asyn_buf is not null  */
    {
        ind=rs_tab[tab_id].buf_len;
        if(ind+len<=256)
        {
            memcpy(&rs_tab[tab_id].asyn_buf[ind],ptr,len);
            rs_tab[tab_id].buf_len=ind+len;
        }
        else   /* ?????  */
        {
            memcpy(&rs_tab[tab_id].asyn_buf[ind],ptr,256-ind);
            rs_tab[tab_id].buf_len=256;
        }
                        
        if(rs_tab[tab_id].buf_len==256)
        {
            proc_sub_data(tab_id,(rs_tab[tab_id].asyn_buf),256);
        }
        else
        {
            if(rs_tab[tab_id].buf_len>=3)
            {
                if(rs_tab[tab_id].asyn_buf[1] & 0x80)
                {
                    if(rs_tab[tab_id].buf_len>=5)
                    {
                        proc_sub_data(tab_id,rs_tab[tab_id].asyn_buf,5);
                        rs_tab[tab_id].buf_len=0;
                                       
                    }
                                   
                }
                else
                {
                    frame_len=rs_tab[tab_id].asyn_buf[2];
                    if(rs_tab[tab_id].buf_len>=5+frame_len)
                    {
                        proc_sub_data(tab_id,rs_tab[tab_id].asyn_buf,5+frame_len);
                        rs_tab[tab_id].buf_len=0;
                                        
                    }
                    
                                    
                }
            }
        }
    }
     
}

void process_RS_data(int tab_id,char *ptr,int len)
{
    switch(rs_tab[tab_id].protocol)
    {
        case PROTOCOL_MODBUS:
            check_modbus_frame(tab_id,ptr,len);   
            break;
        
    }
}

void send_s_frame(int tab_id)
{
    unsigned char send_buf[6];
    
    #if DEBUG==0
    printf("send_iec104_s_frame,vr=%u\n",client_tab[tab_id].vr);
    #endif
    
    send_buf[0]=IEC104_START_BIT;
    send_buf[1]=IEC104_U_FRAME_LEN-2;
    send_buf[2]=IEC104_S_BIT;
    send_buf[3]=0;
    send_buf[4]=(((client_tab[tab_id].vr) % 256)<<1) & 0xfe;
    send_buf[5]=client_tab[tab_id].vr /256;
    
    send(client_tab[tab_id].fd,send_buf,6,0);
}

void send_rs_frame(int i)
{
    int j,result,sub_id;
    uint  fcs;
    int send_len;
	unsigned char cmd[256];
    
    if(rs_tab[i].type==RS485)
    {
        for(j=0;j<CHANNEL_NUM;j++)
        {
            rs_tab[i].sub_id++;
            rs_tab[i].sub_id=rs_tab[i].sub_id % CHANNEL_NUM;
            sub_id=rs_tab[i].sub_id;
            if(rs_tab[i].tab[sub_id].flag)
            {
                break;
            }
        }
        if(j==CHANNEL_NUM)
        {
            return;
        }
    }
    else
    {
        sub_id=0;
        rs_tab[i].sub_id=0;
        if(rs_tab[i].tab[0].flag==0)
        {
            return;
        }
    }
    #if DEBUG==0
    printf("sub_id=%u\n",sub_id);
    #endif
    
    
    
	
    switch(rs_tab[i].protocol)
    {
        case PROTOCOL_MODBUS:
            
            set_rs_par(i,sub_id);  
			cmd[0]=rs_tab[i].tab[sub_id].addr;
            cmd[1]=rs_tab[i].tab[sub_id].data_type;
            cmd[2]=((rs_tab[i].tab[sub_id].start_addr)>>8) & 0xff;
            cmd[3]=(rs_tab[i].tab[sub_id].start_addr) & 0xff;
            cmd[4]=0x00;
            cmd[5]=rs_tab[i].tab[sub_id].reg_cnt;
            fcs=getCRC16(cmd,6);
            cmd[6]=(fcs>>8) & 0xff;
            cmd[7]=fcs & 0x00ff;
            send_len=8;
            break;
    }
         
    #if DEBUG==0
    printf("send:  tab[%d]: ",i); 
    for(j=0;j<send_len;j++)
    {
        printf("%x ",cmd[j]);
    } 
    printf("\n");
    #endif  
             
    result = write(rs_tab[i].fd,cmd,send_len);      
                        
    send_len=0;
                   

    if (result < 0)
    {
        printf("Write command:failed\n");

        close(rs_tab[i].fd);

        exit(-1);

    }  
    
    rs_tab[i].t10=rs_tab[i].poll_int;
    rs_tab[i].t5=rs_tab[i].expire;
             
}

void poll_modbustcp(int tab_id)
{
	int new_fd;  // listen on sock_fd, new connection on new_fd
    struct sockaddr_in server_addr;    // server address information
    /*struct sockaddr_in client_addr; */// connector's address information
    
    int yes = 1;
	int ret,j;
	unsigned char cmd[256];

	if ((new_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
		perror("socket");
		return;
	}

	if (setsockopt(new_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		perror("setsockopt");
		return;
	}
	    
	server_addr.sin_family = AF_INET;         // host byte order
	server_addr.sin_port = htons(modbus_tcpget_tab[tab_id].tcp_port);     // short, network byte order
	server_addr.sin_addr.s_addr = inet_addr(modbus_tcpget_tab[tab_id].ip_addr);
	memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));
	

	ret=connect(new_fd,(struct sockaddr*)&server_addr,sizeof(server_addr));
	if(ret==-1)
	{
		printf("connect fail\n");
		return;
	}

	modbus_tcpget_tab[tab_id].fd=new_fd;
	
	if(new_fd>maxsock)
	{
	    maxsock=new_fd;
	}

	cmd[0]=0x00;
	cmd[1]=0x01;
	cmd[2]=0x00;
	cmd[3]=0x00;
	cmd[4]=0x00;
	cmd[5]=0x06;
	cmd[6]=modbus_tcpget_tab[tab_id].dev_id;
	cmd[7]=modbus_tcpget_tab[tab_id].data_type;
	cmd[8]=((modbus_tcpget_tab[tab_id].start_addr)>>8) & 0xff;
	cmd[9]=(modbus_tcpget_tab[tab_id].start_addr) & 0xff;
	cmd[10]=0x00;
	cmd[11]=modbus_tcpget_tab[tab_id].reg_cnt;
	
	#if DEBUG==0
    printf("modbustcp send:  "); 
    for(j=0;j<6+6;j++)
    {
        printf("%x ",cmd[j]);
    } 
    printf("\n");
    #endif  

    send(new_fd,cmd,6+6,0);

	modbus_tcpget_tab[tab_id].t10=modbus_tcpget_tab[tab_id].poll_intval;
    modbus_tcpget_tab[tab_id].t20=modbus_tcpget_tab[tab_id].expire;


}

void send_IEC104_I_frame(char *buf,int len)
{
    int i;
    
    for(i=0;i<BACKLOG;i++)
    {
        if(client_tab[i].fd)
        {
            send_I_frame(i,buf,len);
        }
    }
}

void get_di_register(void)
{
    int cnt;
    char buff[10];

    
    file_d1=open(DI_1,O_RDWR);
    if(file_d1)
    {
        /*cnt=fread(buff,1,sizeof(buff),file_d1);*/
        cnt = read(file_d1,buff,sizeof(buff));  
        #if DEBUG==20
        printf("cnt1 len=%d\n",cnt);
        if(cnt)
        {
            printf(" %c\n",buff[0]);
        }
        #endif
        
        if((memcmp(&buff[0],"0",1)==0) && (plc_discrete[0]==1))
        {
            plc_discrete[0]=0;
            /*send_IEC104_I_frame((char *)&plc_discrete[0],1);*/
        }
        else if((memcmp(&buff[0],"1",1)==0) && (plc_discrete[0]==0))
        {
            plc_discrete[0]=1;
            /*send_IEC104_I_frame((char *)&plc_discrete[0],1);*/
        }
        
    }
    close(file_d1);
    
    file_d2=open(DI_2,O_RDWR);
    if(file_d2)
    {
        cnt = read(file_d2,buff,sizeof(buff));  
        #if DEBUG==20
        printf("cnt2_len=%d\n",cnt);
        if(cnt)
        {
            printf(" %c\n",buff[0]);
        }     
        #endif
        
        
        if((memcmp(&buff[0],"0",1)==0) && (plc_discrete[1]==1))
        {
            plc_discrete[1]=0;
            /*send_IEC104_I_frame((char *)&plc_discrete[1],1);*/
        }
        else if((memcmp(&buff[0],"1",1)==0) && (plc_discrete[1]==0))
        {
            plc_discrete[1]=1;
            /*send_IEC104_I_frame((char *)&plc_discrete[1],1);*/
        }
        
    }
    close(file_d2);
    
    file_d3=open(DI_3,O_RDWR);
    if(file_d3)
    {
        cnt = read(file_d3,buff,sizeof(buff)); 
        #if DEBUG==20
        printf("cnt3_len=%d\n",cnt);
        if(cnt)
        {
            printf(" %c\n",buff[0]);
        }
        #endif
        
        
        if((memcmp(&buff[0],"0",1)==0) && (plc_discrete[2]==1))
        {
            plc_discrete[2]=0;
            /*send_IEC104_I_frame((char *)&plc_discrete[2],1);*/
        }
        else if((memcmp(&buff[0],"1",1)==0) && (plc_discrete[2]==0))
        {
            plc_discrete[2]=1;
            /*send_IEC104_I_frame((char *)&plc_discrete[2],1);*/
        }
        
    }
    close(file_d3);
    
    file_d4=open(DI_4,O_RDWR);
    if(file_d4)
    {
        cnt=read(file_d4,buff,sizeof(buff)); 
        #if DEBUG==20
        printf("cnt4_len=%d\n",cnt);
        if(cnt)
        {
            printf(" %c",buff[0]);
        }
        #endif
        
        
        if((memcmp(&buff[0],"0",1)==0) && (plc_discrete[3]==1))
        {
            plc_discrete[3]=0;
            /*send_IEC104_I_frame((char *)&plc_discrete[3],1);*/
        }
        else if((memcmp(&buff[0],"1",1)==0) && (plc_discrete[3]==0))
        {
            plc_discrete[3]=1;
            /*send_IEC104_I_frame((char *)&plc_discrete[3],1);*/
        }
        
    }
    close(file_d4);
    
    #if DEBUG==0
    int i;
    printf("plc_discrete: ");
    for(i=0;i<4;i++)
    {
        printf("%x  ",plc_discrete[i]);
    }
    printf("\n");
    #endif
    
}

void proc_timer()
{
    int i,j,sub_id;
    fd_set fdsr;
    
    proc_timer_run=1;
	
    while(proc_timer_run)
    {
        #if DEBUG==10
        printf("proc_timer: 1s\n");
        #endif
        
        sleep(1);
        
        

		sem_wait(&sem);     /*  ???  */
		get_di_register();
        
        for(j=0;j<BACKLOG;j++)
        {
            #if DEBUG==1
            printf("t2=%u\n",client_tab[j].t2);
            #endif
            if(client_tab[j].fd)
            {
                if(client_tab[j].t1>0)
                {
                    client_tab[j].t1--;
                    if(client_tab[j].t1==0)
                    {
                        #if DEBUG==0
                        printf("client %u t1  timeout,close tcp\n",j);
                        #endif

						close(client_tab[j].fd);
						FD_CLR(client_tab[j].fd, &fdsr);
                        client_tab[j].fd = 0;
                        conn_amount_104--;
                        
                    }
                }
                if(client_tab[j].t2>0)
                {
                    client_tab[j].t2--;
                    #if DEBUG==0
                    printf("t2=%u,t2--\n",client_tab[j].t2);
                    #endif
                    if(client_tab[j].t2==0)
                    {
                        #if DEBUG==0
                        printf("client %u t2  timeout,send S frame\n",j);
                        #endif
                        
                        send_s_frame(j);
                    }
                }
                if(client_tab[j].t3>0)
                {
                    client_tab[j].t3--;
                    if(client_tab[j].t3==0)
                    {
                        #if DEBUG==0
                        printf("client %u t3  timeout\n",j);
                        #endif
                    }
                }
            }
        }
        
        for(i=0;i<RS_PORT_NUM;i++)

        {
            if((rs_tab[i].flag==1) && (rs_tab[i].t10>0) && (rs_tab[i].fd>0))
            {
                #if DEBUG==10
                printf("rs_tab[%u] poll_intval\n",i);
                #endif
                rs_tab[i].t10--;
                if(rs_tab[i].t10==0)
                {
                    send_rs_frame(i);
                    
                }
            }
            
            if((rs_tab[i].flag==1) && (rs_tab[i].t5>0) && (rs_tab[i].fd>0))
            {
                 #if DEBUG==10
                printf("rs_tab[%u] expire\n",i);
                #endif
                rs_tab[i].t5--;
                if(rs_tab[i].t5==0)
                {
                    sub_id=rs_tab[i].sub_id;
                    #if DEBUG==0
                    printf("port %d, device %d is not avalable\n",i,rs_tab[i].tab[sub_id].addr);
                    #endif
                    
                    rs_tab[i].buf_len=0;
                    
                }
            }
            
         }

		for(j=0;j<MODBUSTCP_NUM;j++)
        {
            #if DEBUG==20
            printf("modbustcp t10=%u\n",modbus_tcpget_tab[j].t10);
            #endif
            if(modbus_tcpget_tab[j].flag==0)
            {
                continue;
            }
            if(modbus_tcpget_tab[j].t10>0)
            {
                
				 modbus_tcpget_tab[j].t10--;
                 if(modbus_tcpget_tab[j].t10==0)
                 {
                        #if DEBUG==0
                        printf("modbus server %u t10  timeout,polling modbus server\n",j);
                        #endif

						poll_modbustcp(j);
                        
                }
			}

			if((modbus_tcpget_tab[j].t20>0) && (modbus_tcpget_tab[j].fd>0))
			{

				modbus_tcpget_tab[j].t20--;
				if(modbus_tcpget_tab[j].t20==0)
                {
                        #if DEBUG==0
                        printf("server %u t20 timeout,close tcp\n",j);
                        #endif

						close(modbus_tcpget_tab[j].fd);
						FD_CLR(modbus_tcpget_tab[j].fd, &fdsr);
                        modbus_tcpget_tab[j].fd = 0;
                        
                        
                }
			}

		}
                
        
		sem_post(&sem);
		/*sleep(1);*/
    }/* while(1) */

    #if DEBUG==0
    printf("proc_timer quir now ++++++++++++++++++\n");
    #endif
}


void init_iec104_entry(int j)
{
   
        client_tab[j].fd=0;
		client_tab[j].buf_len=0;
		client_tab[j].client_state=IEC104_INIT;
		memset(client_tab[j].tcp_buf,0,IEC104_FRAME_MAX_SIZE);
		client_tab[j].t1=0;
		client_tab[j].t2=0;
		client_tab[j].t3=0;
		client_tab[j].win_k=12;
		client_tab[j].win_w=1;
		client_tab[j].vr=0;
		client_tab[j].vs=0;
		memset(client_tab[j].iec104_frame,0,IEC104_FRAME_MAX_SIZE);
		
}

void init_modbus_entry(int j)
{
   
        modbus_tab[j].fd=0;
		modbus_tab[j].buf_len=0;
		memset(modbus_tab[j].tcp_buf,0,IEC104_FRAME_MAX_SIZE);
		memset(modbus_tab[j].modbus_frame,0,IEC104_FRAME_MAX_SIZE);
		
}

void init_iec104_par()
{
    
    iec104_par.t1=15;
    iec104_par.t2=6;
    iec104_par.t3=20;
	iec104_par.win_k=12;
	iec104_par.win_w=1;
}



void init_modbus_tcpget_entry(int j)
{

    modbus_tcpget_tab[j].flag=0;
	memset(modbus_tcpget_tab[j].ip_addr,0,16);
	modbus_tcpget_tab[j].tcp_port=MODBUSPORT;
	modbus_tcpget_tab[j].t10=0;
	modbus_tcpget_tab[j].t20=0;
	
	modbus_tcpget_tab[j].poll_intval=0;
	modbus_tcpget_tab[j].expire=0;

	modbus_tcpget_tab[j].fd=0;
	modbus_tcpget_tab[j].buf_len=0;
	memset(modbus_tcpget_tab[j].tcp_buf,0,IEC104_FRAME_MAX_SIZE);
	memset(modbus_tcpget_tab[j].modbus_frame,0,IEC104_FRAME_MAX_SIZE);

	modbus_tcpget_tab[j].dev_id=0;
    modbus_tcpget_tab[j].data_type=READ_HOLDING_REGISTER;
	modbus_tcpget_tab[j].plc_ind=0;
	modbus_tcpget_tab[j].reg_cnt=0;
	modbus_tcpget_tab[j].start_addr=0;
	
		
}


static void init_DO_register(void)
{   
    
    file_d5=open(DO_1, O_RDWR); 
    file_d6=open(DO_2, O_RDWR); 
    file_d7=open(DO_3, O_RDWR); 
    file_d8=open(DO_4, O_RDWR);    
    
}

void get_sys_par(void)
{
	int i,j,fd;
	
	#if DEBUG==0
	printf("get_sys_par!!!\n");
	#endif

	data_source=sysInfoP->data_source;

	for(i=0;i<RS_PORT_NUM;i++)
	{
	    m_rs=&(data_source.md_rs[i]);
	    
	    if((rs_tab[i].flag==1) && (m_rs->flag==0))
	    {
	        #if DEBUG==0
	        printf("close rs %d\n",i);
	        #endif
	        if(rs_tab[i].fd)
	        {
	            close(rs_tab[i].fd);
	        }
	    }
	    else if((rs_tab[i].flag==0) && (m_rs->flag==1))
	    {
	        #if DEBUG==0
	        printf("open rs %d\n",i);
	        #endif
	        fd = init_dev(i);   
  
		    if (fd<0)
		    {
			    printf("open serials port failed\n");
			    exit(-1);    /* ??????? */
		    }
	    }
	    
		rs_tab[i].flag=m_rs->flag;
		/*rs_tab[i].rs_id=m_rs->rs_id;*//* 0,1,2,3*/
		rs_tab[i].poll_int=m_rs->poll_intval/1000;
		rs_tab[i].expire=m_rs->expire/1000;
		rs_tab[i].protocol=m_rs->protocol;
		
		#if DEBUG==0
		printf("poll_int=%u,expire=%u,protocol=%d\n",rs_tab[i].poll_int,rs_tab[i].expire,m_rs->protocol);
		#endif
        
		if(rs_tab[i].flag==0)
		{
			continue;
		}	
		
		rs_tab[i].t10=rs_tab[i].poll_int;
		rs_tab[i].t5=0;

		for(j=0;j<CHANNEL_NUM;j++)
		{
		    rs_tab[i].tab[j].flag=m_rs->md[j].flag;
		    
			if((m_rs->md[j]).flag==0)
			{
				continue;
			}
			
			
			
			rs_tab[i].tab[j].addr=(m_rs->md[j]).dev_id;
			rs_tab[i].tab[j].data_type=(m_rs->md[j]).data_type;
			rs_tab[i].tab[j].plc_ind=(m_rs->md[j]).plc_ind-DIS_NUM;
			rs_tab[i].tab[j].reg_cnt=(m_rs->md[j]).reg_cnt;
			rs_tab[i].tab[j].start_addr=(m_rs->md[j]).start_addr;
		
			rs_tab[i].tab[j].speed=(m_rs->md[j]).speed;
			rs_tab[i].tab[j].databit=(m_rs->md[j]).databit;
			rs_tab[i].tab[j].stopbit=(m_rs->md[j]).stopbit;
			rs_tab[i].tab[j].parity=(m_rs->md[j]).parity;
			rs_tab[i].tab[j].streamctrl=(m_rs->md[j]).streamctrl;
			
			
			#if DEBUG==0
			printf("i=%d,j=%d,flag=%d\n",i,j,rs_tab[i].tab[j].flag);
			printf("addr=%d, data_type=%d,plc_id=%d, reg_cnt=%d, start_addr=%d\n",
			    rs_tab[i].tab[j].addr,rs_tab[i].tab[j].data_type,rs_tab[i].tab[j].plc_ind,
			    rs_tab[i].tab[j].reg_cnt,rs_tab[i].tab[j].start_addr);
			printf("speed=%u,databit=%u,stopbin=%u,parity=%u,streamctrl=%u\n",
			     rs_tab[i].tab[j].speed, rs_tab[i].tab[j].databit, rs_tab[i].tab[j].stopbit,
			     rs_tab[i].tab[j].parity,rs_tab[i].tab[j].streamctrl);
			#endif
			
		}
		    
	}

	for(i=0;i<MODBUSTCP_NUM;i++)
	{
	    m_tcp=&(data_source.md_tcp[i]);

		modbus_tcpget_tab[i].flag=m_tcp->flag;
	    strcpy(modbus_tcpget_tab[i].ip_addr,m_tcp->ip_addr);
		modbus_tcpget_tab[i].tcp_port=m_tcp->tcp_port;
		modbus_tcpget_tab[i].poll_intval=m_tcp->poll_intval;
		modbus_tcpget_tab[i].expire=m_tcp->expire;

		
		modbus_tcpget_tab[i].dev_id=m_tcp->dev_id;
		modbus_tcpget_tab[i].data_type=m_tcp->data_type;
		modbus_tcpget_tab[i].plc_ind=m_tcp->plc_ind;
		modbus_tcpget_tab[i].reg_cnt=m_tcp->reg_cnt;
		modbus_tcpget_tab[i].start_addr=m_tcp->start_addr;
		
		if(modbus_tcpget_tab[i].flag)
		{
		    modbus_tcpget_tab[i].t10=modbus_tcpget_tab[i].poll_intval;
		    modbus_tcpget_tab[i].t20=0;
		}
		
		#if DEBUG==10
		printf("modbustcp=%d\n",i);
		printf("flag=%u,ip_addr=%s,tcp_port=%u,poll_inter=%u,expire=%u\n",
		    modbus_tcpget_tab[i].flag,modbus_tcpget_tab[i].ip_addr,modbus_tcpget_tab[i].tcp_port,
		    modbus_tcpget_tab[i].poll_intval,modbus_tcpget_tab[i].expire);
		#endif
	
	}
	
	
	#if 0
	/******************test for modbus_tcpget********************************/
	
	modbus_tcpget_tab[0].flag=0;
	strcpy(modbus_tcpget_tab[0].ip_addr,"192.168.1.53");
	modbus_tcpget_tab[0].tcp_port=502;
	
	modbus_tcpget_tab[0].dev_id=1;
	modbus_tcpget_tab[0].data_type=READ_DISCRETE;
	modbus_tcpget_tab[0].plc_ind=4;
	modbus_tcpget_tab[0].reg_cnt=4;
	modbus_tcpget_tab[0].start_addr=0;
	
	modbus_tcpget_tab[0].poll_intval=10;
	modbus_tcpget_tab[0].expire=3;
	
	if(modbus_tcpget_tab[0].flag)
	{
	    modbus_tcpget_tab[0].t10=modbus_tcpget_tab[0].poll_intval;
	    modbus_tcpget_tab[0].t20=0;
	}
	
	/******************end for modbus_tcpget*********************************/
	
	/******************test for rs******************************************/
	int k;
		rs_tab[2].flag=1;
		rs_tab[2].protocol=PROTOCOL_MODBUS;   	
		
		/*rs_tab[2].rs_id=2;*/
		rs_tab[2].poll_int=10;
		rs_tab[2].expire=3;

		k=0;		
		rs_tab[2].tab[k].flag=1;
		rs_tab[2].tab[k].addr=1;
		rs_tab[2].tab[k].data_type=READ_HOLDING_REGISTER;
		rs_tab[2].tab[k].plc_ind=0;
		rs_tab[2].tab[k].reg_cnt=2;
		rs_tab[2].tab[k].start_addr=0;
			
		rs_tab[2].tab[k].speed=9600;
		rs_tab[2].tab[k].parity=0;
		rs_tab[2].tab[k].stopbit=1;
		rs_tab[2].tab[k].databit=8;
		rs_tab[2].tab[k].streamctrl=0;	

	
		fd = init_dev(2);   
  
		if (fd<0)
		{
			printf("open serials port failed\n");
			exit(-1);    
		}
		
		if(rs_tab[2].flag)
		{
		    rs_tab[2].t10=rs_tab[2].poll_int;
		    rs_tab[2].t5=0;
		}

	/************************endoftest**************************************/
	
	#endif
	
}

int pthread_plc(void)
{
    int sock_fd_104,sock_fd_modbus, new_fd;  // listen on sock_fd, new connection on new_fd
    struct sockaddr_in server_addr;    // server address information
    struct sockaddr_in client_addr; // connector's address information
    socklen_t sin_size;
    int yes = 1;
    char buf[BUF_SIZE];
    int ret;
    int i,j,k;
    fd_set fdsr;
    
    struct timeval tv;
    
    pthread_t   pid;
    int err;
 
 
    /*int fd;*/
    
    char ptr[BUF_SIZE];
    int len;
    
    /*plcLogPrintf(1,plcLogFile,"pthread_plc starting....\n");*/


	#if DEBUG==0
	printf("PLC SERVER START ##################################\n");
	#endif
	
	/* below init local table  */
    init_RS_port();  
	init_DO_register();
	for(j=0;j<BACKLOG;j++)
	{
		init_iec104_entry(j);
		init_modbus_entry(j);
	}
	for(j=0;j<MODBUSTCP_NUM;j++)
	{
		init_modbus_tcpget_entry(j);
	}

	/* below get parameter */
	get_sys_par();    
    
    init_iec104_par();

    if ((sock_fd_104 = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    if (setsockopt(sock_fd_104, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }
    
    server_addr.sin_family = AF_INET;         // host byte order
    server_addr.sin_port = htons(IEC104PORT);     // short, network byte order
    server_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
    memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));

    if (bind(sock_fd_104, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sock_fd_104, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    /*printf("listen port %d\n", IEC104PORT);*/

    conn_amount_104 = 0;
    sin_size = sizeof(client_addr);
    /*maxsock = sock_fd;*/
    

    printf("sock_fd=%u\n",sock_fd_104);
    
    if(sock_fd_104>maxsock)
    {
        maxsock=sock_fd_104;
    }
    /* below set modbustcp */
    if ((sock_fd_modbus = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    if (setsockopt(sock_fd_modbus, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }
    
    server_addr.sin_family = AF_INET;         // host byte order
    server_addr.sin_port = htons(MODBUSPORT);     // short, network byte order
    server_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
    memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));

    if (bind(sock_fd_modbus, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sock_fd_modbus, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    /*printf("listen port %d\n", MODBUSPORT);*/

    conn_amount_modbus = 0;
    sin_size = sizeof(client_addr);
    /*maxsock = sock_fd;*/
    
    printf("sock_fd_modbus=%u\n",sock_fd_modbus);
    
    if(sock_fd_modbus>maxsock)
    {
        maxsock=sock_fd_modbus;
    }

	// initialize file descriptor set
	/*FD_ZERO(&fdsr);
    FD_SET(sock_fd, &fdsr);
    FD_SET(fd, &fdsr);*/
    
    /* create a timer thread */

	err=sem_init(&sem,0,1);

    if(err != 0)
    {
        printf("create sem fail: %s\n", strerror(err));
        exit(-1);
    }
    
    err=pthread_create(&pid,NULL,(void *)&proc_timer,NULL);
    if(err!=0)
    {
        printf("pthread_create error\n");
        exit(1);
    }
    
    while ((sysInfoP->data_source).plcRun) 
    {
        /*initialize file descriptor set*/
            FD_ZERO(&fdsr);
            FD_SET(sock_fd_104, &fdsr);
            FD_SET(sock_fd_modbus, &fdsr);
            
            // timeout setting
            tv.tv_sec = 5;
            tv.tv_usec = 0;
     
			/*sem_wait(&sem);*/
			
			for(i=0;i<RS_PORT_NUM;i++)
			{
			    if((rs_tab[i].flag) && (rs_tab[i].fd))
			    {
			        #if DEBUG==30
			        printf("rs_tab[%d].fd=%u\n",i,rs_tab[i].fd);
			        #endif
			        FD_SET(rs_tab[i].fd , &fdsr);
			    }
			}
            
            // add active connection to fd set
            for (i = 0; i < BACKLOG; i++) 
            {
                if (client_tab[i].fd != 0) 
                {
                    #if DEBUG==0
                    printf("104: fd=%u\n",client_tab[i].fd);
                    #endif
                    
                    FD_SET(client_tab[i].fd , &fdsr);
                    /*if(FD_ISSET(client_tab[i].fd , &fdsr))
                    {
                        printf("104 in set fd: %u\n",client_tab[i].fd);
                    }*/
                }
            }
            
            for (i = 0; i < BACKLOG; i++) 
            {
                if (modbus_tab[i].fd != 0) 
                {
                    #if DEBUG==0
                    printf("modbus fd=%u\n",modbus_tab[i].fd);
                    #endif
                    
                    FD_SET(modbus_tab[i].fd , &fdsr);
                    /*if(FD_ISSET(modbus_tab[i].fd , &fdsr))
                    {
                        printf("in set fd: %u\n",modbus_tab[i].fd);
                    }*/
                }
            }

			for (i = 0; i < MODBUSTCP_NUM; i++) 
            {
                if (modbus_tcpget_tab[i].fd != 0) 
                {
                    #if DEBUG==0
                    printf("modbustcp fd=%u\n",modbus_tcpget_tab[i].fd);
                    #endif
                    
                    FD_SET(modbus_tcpget_tab[i].fd , &fdsr);
                    /*if(FD_ISSET(modbus_tab[i].fd , &fdsr))
                    {
                        printf("in set fd: %u\n",modbus_tab[i].fd);
                    }*/
                }

            }
            
            
        #if DEBUG==20
        printf("waiting for message......\n");
        #endif
        
                               
            ret = select(maxsock + 1, &fdsr, NULL, NULL, &tv);
            
        #if DEBUG==10
        printf("selected ret=%d\n",ret);
        #endif
        
                              
            if (ret < 0) 
            {
                perror("select error");
                break;
            } 
            else if (ret == 0) 
            {
                /*printf("timeout!!!\n");*/

				/*sem_post(&sem);*/
                continue;
            }

        #if DEBUG==10
        printf("selected 0 ret=%d\n",ret);
        #endif
            sem_wait(&sem); 
        
            /* check every serial port fd in the set */
            for(k=0;k<RS_PORT_NUM;k++)
            {
                if ( (rs_tab[k].fd>0) && FD_ISSET(rs_tab[k].fd, &fdsr) ) 
                {
                    len = read(rs_tab[k].fd,ptr,BUF_SIZE);    
                
                    if(len<=0)
                    {
						#if DEBUG==0
                        printf("serial %d port read error,len=%u\n",k,len);
						#endif
                    }
                    else
                    {
                        ptr[len] = 0;   
						
						#if DEBUG==0
						printf("!!Readed %d data:\n",len);     
						

                      	for(i=0;i<len;i++)
                      	{
                      	    printf("%x  ",ptr[i]);
                      	} 
                      	printf("\n");
						#endif
                      	
						process_RS_data(k,ptr,len);
						
                    }
                    break;
                }
                
                
            }
            
        #if DEBUG==10
        printf("selected 1 ret=%d\n",ret);
        #endif
        
        
            // check every fd in the set
            for (i = 0; i < BACKLOG; i++) 
            {
                if ( (client_tab[i].fd>0) && FD_ISSET(client_tab[i].fd, &fdsr) ) 
                {
                    ret = recv(client_tab[i].fd, buf, sizeof(buf), 0);

                    if (ret <= 0) 
                    {        // client close
                        printf("receive error ,client[%d] close\n", i);
                        close(client_tab[i].fd);
                        FD_CLR(client_tab[i].fd, &fdsr);
                        client_tab[i].fd = 0;
                        conn_amount_104--;
                    } 
                    else 
                    {        // receive data
                        /*if (ret < BUF_SIZE)
                        {
                            memset(&buf[ret], '\0', 1);       
                        }*/
                        
                        process_tcp104_stream(i,buf,ret);                        
                
                    }
                }
            }
            
        #if DEBUG==10
        printf("selected 2 ret=%d\n",ret);
        #endif
        
            for (i = 0; i < BACKLOG; i++) 
            {
                if ( (modbus_tab[i].fd>0) && FD_ISSET(modbus_tab[i].fd, &fdsr) ) 
                {
                    ret = recv(modbus_tab[i].fd, buf, sizeof(buf), 0);

                    if (ret <= 0) 
                    {        // client close
                        printf("receive error ,client[%d] close\n", i);
                        close(modbus_tab[i].fd);
                        FD_CLR(modbus_tab[i].fd, &fdsr);
                        modbus_tab[i].fd = 0;
                        conn_amount_modbus--;
                    } 
                    else 
                    {        // receive data
                        /*if (ret < BUF_SIZE)
                        {
                            memset(&buf[ret], '\0', 1);       
                        }*/
                        
                        process_tcpmodbus_stream(i,buf,ret);                        
                
                    }
                }
            }

        #if DEBUG==10
        printf("selected 3 ret=%d\n",ret);
        #endif
        

			for (i = 0; i < MODBUSTCP_NUM; i++) 
            {
                if ( (modbus_tcpget_tab[i].fd>0) && FD_ISSET(modbus_tcpget_tab[i].fd, &fdsr) ) 
                {
                    ret = recv(modbus_tcpget_tab[i].fd, buf, sizeof(buf), 0);

                    if (ret <= 0) 
                    {        // client close
                        printf("receive error ,modbustcp[%d] close\n", i);
                        close(modbus_tcpget_tab[i].fd);
                        FD_CLR(modbus_tcpget_tab[i].fd, &fdsr);
                        modbus_tcpget_tab[i].fd = 0;
                    } 
                    else 
                    {        // receive data
                        /*if (ret < BUF_SIZE)
                        {
                            memset(&buf[ret], '\0', 1);       
                        }*/
                        
                        process_tcpget_modbus(i,buf,ret);                        
                
                    }
                }
            }

        #if DEBUG==10
        printf("selected 4 ret=%d\n",ret);
        #endif
        
            // check whether a new connection comes
            if (FD_ISSET(sock_fd_104, &fdsr)) 
            {
                new_fd = accept(sock_fd_104, (struct sockaddr *)&client_addr, &sin_size);
                if (new_fd <= 0) 
                {
                    perror("accept");
					sem_post(&sem);
                    continue;
                }

                // add to fd queue
                if (conn_amount_104 < BACKLOG) 
                {
                    conn_amount_104++;
					#if DEBUG==10
                    printf("new connection client[%d] %s:%u\n", conn_amount_104,
                            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
					#endif

                    for(j=0;j<BACKLOG;j++)
                    {
                        if(client_tab[j].fd==0)
                        {
							init_iec104_entry(j);
                            client_tab[j].fd=new_fd;
						    FD_SET(new_fd , &fdsr);
						    /*printf("new_fd=%u\n",new_fd);*/
                            (client_tab[j].client_addr).sin_addr=client_addr.sin_addr;
                            (client_tab[j].client_addr).sin_port=client_addr.sin_port;
                            break;
                        }
                    }
                                    
                    if (new_fd > maxsock)
                        maxsock = new_fd;
                }
                else 
                {
                    printf("max connections arrive, exit\n");
                    send(new_fd, "bye", 4, 0);
				
                    close(new_fd);
                    break;
                }
            }
            
            if (FD_ISSET(sock_fd_modbus, &fdsr)) 
            {
                new_fd = accept(sock_fd_modbus, (struct sockaddr *)&client_addr, &sin_size);
                if (new_fd <= 0) 
                {
                    perror("accept");
					sem_post(&sem);
                    continue;
                }

                // add to fd queue
                if (conn_amount_modbus < BACKLOG) 
                {
                    conn_amount_modbus++;
					#if DEBUG==10
                    printf("new connection client[%d] %s:%u\n", conn_amount_modbus,
                            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
					#endif
                    for(j=0;j<BACKLOG;j++)
                    {
                        if(modbus_tab[j].fd==0)
                        {
							init_modbus_entry(j);
                            modbus_tab[j].fd=new_fd;
						    FD_SET(new_fd , &fdsr);
						    printf("new_fd=%u\n",new_fd);
                            (modbus_tab[j].client_addr).sin_addr=client_addr.sin_addr;
                            (modbus_tab[j].client_addr).sin_port=client_addr.sin_port;
                            break;
                        }
                    }
                                    
                    if (new_fd > maxsock)
                        maxsock = new_fd;
                }
                else 
                {
                    printf("max connections arrive, exit\n");
                    send(new_fd, "bye", 4, 0);
				
                    close(new_fd);
                    break;
                }
            }
			sem_post(&sem);
            showclient();
           
    }

	#if DEBUG==0
	printf("PLC SERVER STARTing quit+++++++++++++++++++++++++++++++++++\n");
	#endif

	proc_timer_run=0;
	
    pthread_join(pid,NULL);

    // close other connections
    for (i = 0; i < BACKLOG; i++) 
	{
        if (client_tab[i].fd != 0)
		{
            close(client_tab[i].fd);
            client_tab[i].fd=0;
        }
    }
    for (i = 0; i < BACKLOG; i++) 
	{
        if (modbus_tab[i].fd != 0)
		{
            close(modbus_tab[i].fd);
            client_tab[i].fd=0;
        }
    }
	for (i = 0; i < MODBUSTCP_NUM; i++) 
	{
        if (modbus_tcpget_tab[i].fd != 0)
		{
            close(modbus_tcpget_tab[i].fd);
            client_tab[i].fd=0;
        }
    }

	close(sock_fd_104);
	close(sock_fd_modbus);
	for(i=0;i<RS_PORT_NUM;i++)
	{
	    if(rs_tab[i].fd)
	    {
	        close(rs_tab[i].fd);
	        rs_tab[i].fd=0;
	    }
    }
		#if DEBUG==0
	printf("PLC SERVER now quit+++++++++++++++++++++++++++++++++++\n");
	#endif


    return 0;
}






