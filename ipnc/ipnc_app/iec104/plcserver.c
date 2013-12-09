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

#include <plc.h>
#include <sys_env_type.h>



#define REG_NUM  24
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

#define  PROTOCOL_MODBUS     1


/***********************************************************************************/

/* move to interface/inc/plc.h */

/***************************************************************************************/


/*#define MODBUS_RESP_LEN     1(地址）+1（0x03)+1（长度=2*n)+2*n 
          MODBUS_EXP_LEN      1(地址）+1（0x83)+1（错误码0x01/0x02/0x03/0x04) */


struct DEV_PAR
{
    unsigned char flag;
    unsigned char addr;
    unsigned char pro;   /* modbus/pelco/ppp */
    unsigned char rate;
    unsigned char parity;
    unsigned char stopbit;
    unsigned char databit;    

};


struct DEV_TAB
{
    
    unsigned char flag;
    unsigned char addr;     /* dev_id */
    /*int protocol;*/    /* sub protocol,e.g. kunlun */
    unsigned char data_type;    /* 0x03: read holding register, 0x04: read input register */
	unsigned char start_addr[2];
	unsigned char reg_cnt;
	unsigned char plc_ind;

	int resp_timer;
	/*unsigned short result[10];*/    /* just for test */
    
};

/*MODBUS_FRAME
{
	char addr;
	char cmd;
	char data[...];
	char crc[2];

};*/


struct RS_PORT_TAB
{
    unsigned char dev_name[12];
    int fd;
    char type;      /* rs232/rs485 */
    int  sub_id;
    /*unsigned char  wait_addr;*/
    int t10;        /* poll interval,default is 2s */
	struct DEV_PAR     par;
    struct DEV_TAB    tab[CHANNEL_NUM];
    int buf_len;
    unsigned char asyn_buf[256];
    
};



struct REG_TAB
{
	unsigned char reg[2];
};

struct REG_TAB    plc_hold_reg[REG_NUM];

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
	unsigned char ip_addr[7];
	int tcp_port;
	int t10;    /* poll_intval */
	int t20;        /* expire */


	int dev_id;
	int data_type;
	int plc_ind;
	int reg_cnt;
	unsigned char start_addr[2];      /* support 8 dev_id */

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
static int de_id;
struct IEC104_PAR   iec104_par;
static sem_t sem;

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

int init_dev(int tab_id)
{ 
        int fd,status;
        unsigned char cmd[17];
        struct termios newtio;        
        
#if DEBUG==10
        printf("init_dev!\n");
#endif
               
        fd = open(rs_tab[tab_id].dev_name, O_RDWR| O_NOCTTY|O_NDELAY);      
        
         if (fd ==-1)
        {
            printf("error open\n");
            perror("open"DEV);
            return -1;
        }
        else    
        {//set to block;
            fcntl(fd, F_SETFL, 0);
        }

        
        
        tcgetattr(fd,&newtio);
        
        
        cfsetispeed(&newtio,B9600);
        cfsetospeed(&newtio,B9600);
        
        newtio.c_cflag |=CLOCAL|CREAD;
                
        newtio.c_cflag &= ~CSIZE; 
        newtio.c_cflag |= CS8;    
        newtio.c_cflag &= ~PARENB;
        newtio.c_cflag &= ~CSTOPB;  
        
        newtio.c_cflag &= ~CRTSCTS;
        newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        newtio.c_oflag  &= ~OPOST;  
                
        tcflush(fd,TCIFLUSH);//clear input buffer
        newtio.c_cc[VTIME] = 100; /* inter-character timer unused */
        newtio.c_cc[VMIN] = 0; /* blocking read until 0 character arrives */
        
       
      tcsetattr(fd,TCSANOW,&newtio);
    
        rs_tab[tab_id].fd=fd;
       
        return fd;
}




void send_I_frame(int tab_id,char *buf,int len)
{
    unsigned char send_buf[IEC104_FRAME_MAX_SIZE];
    
    #if DEBUG==0
    printf("send_iec104_I_frame\n");
    #endif

	if(client_tab[tab_id].client_state==IEC104_INIT)
	{
		return;
	}

	send_buf[0]=IEC104_START_BIT;
    send_buf[1]=IEC104_U_FRAME_LEN-2;
    send_buf[2]=((client_tab[tab_id].vs % 256)<<1) & 0xfe;
    send_buf[3]=client_tab[tab_id].vs /256;
    send_buf[4]=((client_tab[tab_id].vr % 256)<<1) & 0xfe;
    send_buf[5]=client_tab[tab_id].vr /256;
	memcpy(&send_buf[6],buf,len);

    

	if(client_tab[tab_id].win_k)
	{
		client_tab[tab_id].win_k--;
		send(client_tab[tab_id].fd,send_buf,6+len,0);
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
    int i;
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
	int u_action;
	
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
	unsigned char send_buf[256];
	int len;

	
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
    int i,ind,j;
    
    #if DEBUG==0
    printf("get_modbustcp_frame: tab_id=%u\n",tab_id);
    #endif
    
    mbap=*(struct MODBUS_MBAP *)(modbus_tab[tab_id].modbus_frame);
    
    #if DEBUG==0

    printf("send modbus frame,only support read/write holding register\n");
    #endif
    
    memcpy(&send_buf[0],&(modbus_tab[tab_id].modbus_frame[0]),4);
    
    if(modbus_tab[tab_id].modbus_frame[7]==READ_HOLDING_REGISTER)
    {
        plc_id=modbus_tab[tab_id].modbus_frame[9];
        reg_cnt=modbus_tab[tab_id].modbus_frame[11];
        #if DEBUG==0
        printf("plc_id=%d,reg_cnt=%d\n",plc_id,reg_cnt);
        #endif
        send_buf[4]=0;
        send_buf[5]=reg_cnt*2+3;
        send_buf[6]=modbus_tab[tab_id].modbus_frame[6];
        send_buf[7]=READ_HOLDING_REGISTER;
        send_buf[8]=reg_cnt*2;
        ind=0;
        for(i=0;i<reg_cnt;i++)
        {
            send_buf[9+ind]=plc_hold_reg[plc_id+i].reg[0];
            ind++;
            send_buf[9+ind]=plc_hold_reg[plc_id+i].reg[1];
            ind++;
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
            plc_hold_reg[plc_id].reg[0]=modbus_tab[tab_id].modbus_frame[10];
            plc_hold_reg[plc_id].reg[1]=modbus_tab[tab_id].modbus_frame[11];
            
            if((plc_id>=20)  && (plc_id<24))
            {
                #if DEBUG==0
                printf("plc_id=%d,setgio=%d\n",plc_id,plc_hold_reg[plc_id].reg[1]);
                #endif
                
                if(plc_id==20)
                {
                                        
                    if(plc_id,plc_hold_reg[plc_id].reg[1]==1)
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
                                        
                    if(plc_id,plc_hold_reg[plc_id].reg[1]==1)
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
                                        
                    if(plc_id,plc_hold_reg[plc_id].reg[1]==1)
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
                                                            
                    if(plc_id,plc_hold_reg[plc_id].reg[1]==1)
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
    
    
}

void get_modbus_tcp(int tab_id)
{
    int plc_id,reg_len;
    
    #if DEBUG==0
    printf("get_modbustcp_frame_from server: tab_id=%u\n",tab_id);
    #endif
    
	if(modbus_tcpget_tab[tab_id].dev_id!=modbus_tab[tab_id].modbus_frame[6])
	{
#if DEBUG==0
		printf("main data error from device %d\n",modbus_tcpget_tab[tab_id].dev_id);
#endif
		
	}    
    else if(modbus_tcpget_tab[tab_id].modbus_frame[7]==READ_HOLDING_REGISTER)
    {
        plc_id=modbus_tcpget_tab[tab_id].plc_ind;
		reg_len=modbus_tcpget_tab[tab_id].modbus_frame[5];

		memcpy(&plc_hold_reg[plc_id],&(modbus_tcpget_tab[tab_id].modbus_frame[9]),reg_len-3);

        
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

/*void init_RS485_port()
{
    int k;
    
    for(k=0;k<CHANNEL_NUM;k++)      
    {
        device_chain[k].chan_no=k;
        device_chain[k].protocol=PRO_KUNLUN;
        device_chain[k].addr=k+1;
        device_chain[k].cmd[0]=device_chain[k].addr;
        
        
    }
     
    
}*/

void init_RS_port()
{
    int i,k;
    
    for(i=0;i<RS_PORT_NUM;i++)
    {
        rs_tab[i].fd=0;
        rs_tab[i].sub_id=-1;
        /*rs_tab[i].wait_addr=0;*/
        rs_tab[i].t10=0;
        rs_tab[i].buf_len=0;
        memset(rs_tab[i].asyn_buf,0,256);

		rs_tab[i].par.pro=PROTOCOL_MODBUS;
        
        for(k=0;k<CHANNEL_NUM;k++)
        {
            rs_tab[i].tab[k].flag=0;
           
            rs_tab[i].tab[k].addr=0;
			rs_tab[i].tab[k].data_type=READ_HOLDING_REGISTER;
            rs_tab[i].tab[k].plc_ind=0;
			rs_tab[i].tab[k].reg_cnt=0;
			rs_tab[i].tab[k].start_addr[0]=0;
			rs_tab[i].tab[k].start_addr[1]=0;
            
            
        }
        
    }
    
    memcpy(rs_tab[0].dev_name,"/dev/ttyXX0",11);
    memcpy(rs_tab[1].dev_name,"/dev/ttyXX1",11);
    memcpy(rs_tab[2].dev_name,"/dev/ttyXM0",11);
    /*memcpy(rs_tab[3].dev_name,"/dev/ttyXM1",11);*/
    
    rs_tab[0].type=RS232;
    rs_tab[1].type=RS232;
    rs_tab[2].type=RS485;
    /*rs_tab[3].type=RS485;*/


    
    
    /* just for test */
    rs_tab[2].t10=2;   
    for(k=0;k<1;k++)      
    {
        rs_tab[2].tab[k].flag=1;
       
        rs_tab[2].tab[k].addr=k+1;
        rs_tab[2].tab[k].data_type=READ_HOLDING_REGISTER;
        rs_tab[2].tab[k].plc_ind=0;
		rs_tab[2].tab[k].reg_cnt=2;
		rs_tab[2].tab[k].start_addr[0]=0;
		rs_tab[2].tab[k].start_addr[1]=0;
        
        
    }
     
    
}

void proc_sub_data(int tab_id,int sub_id,char *ptr,int len)
{
    int i;
	int plc_id;
	
    #if DEBUG==0
    printf("\n!!! proc_sub_data: ");
    for(i=0;i<len;i++)
    {
        printf("%0x  ",ptr[i]);
    }
    printf("\n");
    #endif
    
    
    if(ptr[0]!=rs_tab[tab_id].tab[sub_id].addr)
    {
        #if DEBUG==0
        printf("\nerror addr,return\n");
        #endif
        return;
    }
    
    switch(rs_tab[tab_id].par.pro)
      	{
      	    case PROTOCOL_MODBUS:
      	        #if DEBUG==0
      	        printf("protocol_modbus\n");
      	        #endif
      	        
      	        if((ptr[1] & 0x7f)!=rs_tab[tab_id].tab[sub_id].data_type) 
				{
					return;
				}
				else if( ((ptr[1] & 0x80)==0) && (ptr[1]==READ_HOLDING_REGISTER))        /* read holding register */
				{     	 
					plc_id=rs_tab[tab_id].tab[sub_id].plc_ind;

					memcpy(&plc_hold_reg[plc_id],&ptr[3],ptr[2]);



					#if DEBUG==0
					printf("\n!!! plc_read holding register: ");
					for(i=0;i<REG_NUM;i++)
					{
						printf("%0x %0x ",plc_hold_reg[i].reg[0],plc_hold_reg[i].reg[1]);
					}
					printf("\n");
					#endif

      	                 	              
              	    /*if(ptr[3]!=0xff)           
              	    {
              	        
              	        rs_tab[tab_id].tab[sub_id].result[0]=((ptr[3]<<8)+ptr[4])/10;   	             	            	        
              	        printf("wendu: %u\n",rs_tab[tab_id].tab[sub_id].result[0]);

              	        
              	        
              	    }   
              	    else  
              	    {
              	        printf("below zero\n");
              	        rs_tab[tab_id].tab[sub_id].result[0]=(0xff-ptr[4])/10;
              	        printf("wendu: %u\n",rs_tab[tab_id].tab[sub_id].result[0]);
              	    }
              	    rs_tab[tab_id].tab[sub_id].result[1]=((ptr[5]<<8)+ptr[6])/10;
              	    printf("shidu: %u%\n",rs_tab[tab_id].tab[sub_id].result[1]);*/

              	}
              	else if(ptr[1]==0x83)
              	{
              	    printf("server wrong\n");
              	}
              	else
              	{
              	    printf("wrong input\n");
              	}
          	    
          	    break;
          	    
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
                    proc_sub_data(tab_id,0,ptr,5); 
                                      
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
                    proc_sub_data(tab_id,0,ptr,5+frame_len);                         
                                    
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
        else
        {
            memcpy(&rs_tab[tab_id].asyn_buf[ind],ptr,256-ind);
            rs_tab[tab_id].buf_len=256;
        }
                        
        if(rs_tab[tab_id].buf_len==256)
        {
            proc_sub_data(tab_id,0,rs_tab[tab_id].asyn_buf,256);
        }
        else
        {
            if(rs_tab[tab_id].buf_len>=3)
            {
                if(rs_tab[tab_id].asyn_buf[1] & 0x80)
                {
                    if(rs_tab[tab_id].buf_len>=5)
                    {
                        proc_sub_data(tab_id,0,rs_tab[tab_id].asyn_buf,5);
                        rs_tab[tab_id].buf_len=0;
                                       
                    }
                                   
                }
                else
                {
                    frame_len=rs_tab[tab_id].asyn_buf[2];
                    if(rs_tab[tab_id].buf_len>=5+frame_len)
                    {
                        proc_sub_data(tab_id,0,rs_tab[tab_id].asyn_buf,5+frame_len);
                        rs_tab[tab_id].buf_len=0;
                                        
                    }
                                    
                }
            }
        }
    }
     
}

void process_RS_data(int tab_id,char *ptr,int len)
{
    int sub_id;
    
    
    
    if(rs_tab[tab_id].type==RS232)
    {
        if(rs_tab[tab_id].tab[0].flag)
        {
            if(rs_tab[tab_id].par.pro==PROTOCOL_MODBUS)
            {
                #if DEBUG==0
                printf("process_modbus_buf\n");
                #endif      
                
                check_modbus_frame(tab_id,ptr,len);             
               
            }
        }
            
    }
    else
    {
        
        sub_id=rs_tab[tab_id].sub_id;
        if(rs_tab[tab_id].tab[sub_id].flag)
        {
            if(rs_tab[tab_id].par.pro==PROTOCOL_MODBUS)
            {
                #if DEBUG==0
                printf("process_modbus_buf\n");
                #endif      
                
              
                check_modbus_frame(tab_id,ptr,len);             
               
            }
        }
            
    }
    
    
}

void send_s_frame(int tab_id)
{
    unsigned char send_buf[6];
    
    #if DEBUG==0
    printf("send_iec104_s_frame,vr=%u\n",client_tab[tab_id].vr);
    printf("1 % 256=%u\n",(client_tab[tab_id].vr) % 256);
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
    }
    else
    {
        sub_id=0;
    }
    #if DEBUG==0
    printf("sub_id=%u\n",sub_id);
    #endif

	
    switch(rs_tab[i].par.pro)
    {
        case PROTOCOL_MODBUS:  
			cmd[0]=rs_tab[i].tab[sub_id].addr;
            cmd[1]=rs_tab[i].tab[sub_id].data_type;
            cmd[2]=rs_tab[i].tab[sub_id].start_addr[0];
            cmd[3]=rs_tab[i].tab[sub_id].start_addr[1];
            cmd[4]=0x00;
            cmd[5]=rs_tab[i].tab[sub_id].reg_cnt;
            fcs=getCRC16(cmd,6);
            cmd[6]=(fcs>>8) & 0xff;
            cmd[7]=fcs & 0x00ff;
            send_len=8;
            break;
    }
         
    #if DEBUG==0
    printf("send:  "); 
    for(j=0;j<send_len;j++)
    {
        printf("%x ",cmd[j]);
    } 
    printf("\n");
    #endif  
             
    result = write(rs_tab[i].fd,cmd,send_len);      
                        
    send_len=0;
                   

   /* if (result < 0)
    {
        printf("Write command:failed\n");

        close(rs_tab[i].fd);

        return;

    }*/  
    
    rs_tab[i].t10=2;
    /*rs_tab[i].wait_addr=cmd[0];*/
     
            
}

void poll_modbustcp(int tab_id)
{
	int new_fd;  // listen on sock_fd, new connection on new_fd
    struct sockaddr_in server_addr;    // server address information
    struct sockaddr_in client_addr; // connector's address information
    socklen_t sin_size;
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

	cmd[0]=0x00;
	cmd[1]=0x01;
	cmd[2]=0x00;
	cmd[3]=0x00;
	cmd[4]=0x00;
	cmd[5]=0x06;
	cmd[6]=modbus_tcpget_tab[tab_id].dev_id;
	cmd[7]=modbus_tcpget_tab[tab_id].data_type;
	cmd[8]=modbus_tcpget_tab[tab_id].start_addr[0];
	cmd[9]=modbus_tcpget_tab[tab_id].start_addr[1];
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

	modbus_tcpget_tab[tab_id].t10=10;
    modbus_tcpget_tab[tab_id].t20=5;


}

void get_di_register(void)
{
    int cnt,i;
    char buff[10];
    
    file_d1=open(DI_1,O_RDWR);
    if(file_d1)
    {
        /*cnt=fread(buff,1,sizeof(buff),file_d1);*/
        cnt = read(file_d1,buff,sizeof(buff));  
        #if DEBUG==0
        printf("cnt1=%d\n",cnt);
        if(cnt)
        {
            printf(" %c\n",buff[0]);
        }
        #endif
        
        plc_hold_reg[16].reg[0]=0;
        if(memcmp(&buff[0],"0",1)==0)
        {
            (plc_hold_reg[16]).reg[1]=0;
        }
        else
        {
            (plc_hold_reg[16]).reg[1]=1;
        }
        
    }
    close(file_d1);
    
    file_d2=open(DI_2,O_RDWR);
    if(file_d2)
    {
        cnt = read(file_d2,buff,sizeof(buff));  
        #if DEBUG==0
        printf("cnt2=%d\n",cnt);
        if(cnt)
        {
            printf(" %c\n",buff[0]);
        }     
        #endif
        
        plc_hold_reg[17].reg[0]=0;
        if(memcmp(&buff[0],"0",1)==0)
        {
            (plc_hold_reg[17]).reg[1]=0;
        }
        else
        {
            (plc_hold_reg[17]).reg[1]=1;
        }
        
    }
    close(file_d2);
    
    file_d3=open(DI_3,O_RDWR);
    if(file_d3)
    {
        cnt = read(file_d3,buff,sizeof(buff)); 
        #if DEBUG==0
        printf("cnt3=%d\n",cnt);
        if(cnt)
        {
            printf(" %c\n",buff[0]);
        }
        #endif
        
        plc_hold_reg[18].reg[0]=0;
        if(memcmp(&buff[0],"0",1)==0)
        {
            (plc_hold_reg[18]).reg[1]=0;
        }
        else
        {
            (plc_hold_reg[18]).reg[1]=1;
        }
        
    }
    close(file_d3);
    
    file_d4=open(DI_4,O_RDWR);
    if(file_d4)
    {
        cnt=read(file_d4,buff,sizeof(buff)); 
        #if DEBUG==0
        printf("cnt4=%d\n",cnt);
        if(cnt)
        {
            printf(" %c",buff[0]);
        }
        #endif
        
        plc_hold_reg[19].reg[0]=0;
        if(memcmp(&buff[0],"0",1)==0)
        {
            (plc_hold_reg[19]).reg[1]=0;
        }
        else
        {
            (plc_hold_reg[19]).reg[1]=1;
        }
        
    }
    close(file_d4);
    
    #if DEBUG==0
    printf("plc_hold_reg: ");
    for(i=0;i<REG_NUM;i++)
    {
        printf("%x %x    ",plc_hold_reg[i].reg[0],plc_hold_reg[i].reg[1]);
    }
    printf("\n");
    #endif
    
}

void proc_timer()
{
    int i,j,result,sub_id;
    fd_set fdsr;
    
    
    while(1)
    {
        #if DEBUG==1
        printf("proc_timer: 1s\n");
        #endif
        
        get_di_register();

		/*sem_wait(&sem);*/       /*  ???  */
        
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
            if(rs_tab[i].t10>0)
            {
                #if DEBUG==10
                printf("rs_tab[%u] timer\n",i);
                #endif
                rs_tab[i].t10--;
                if(rs_tab[i].t10==0)
                {
                    send_rs_frame(i);
                    
                }
            }
         }

		for(j=0;j<MODBUSTCP_NUM;j++)
        {
            #if DEBUG==20
            printf("modbustcp t10=%u\n",modbus_tcpget_tab[j].t10);
            #endif
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

			if(modbus_tcpget_tab[j].t20>0)
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
		sleep(1);
    }/* while(1) */
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

/*
struct  MODBUSTCP_GET
{
    
    int flag;    
	unsigned char ip_addr[7];
	int tcp_port;
	int t10;    
	int t20;        



	int dev_id;
	int data_type;
	int plc_ind;
	int reg_cnt;
	unsigned char start_addr[2];      

	int fd;    
    int buf_len;
	unsigned char tcp_buf[IEC104_FRAME_MAX_SIZE];
	unsigned char modbus_frame[IEC104_FRAME_MAX_SIZE];    

};
*/

void init_modbus_tcpget_entry(int j)
{

    modbus_tcpget_tab[j].flag=0;
	memset(modbus_tcpget_tab[j].ip_addr,0,7);
	modbus_tcpget_tab[j].tcp_port=MODBUSPORT;
	modbus_tcpget_tab[j].t10=0;
	modbus_tcpget_tab[j].t20=0;

	modbus_tcpget_tab[j].fd=0;
	modbus_tcpget_tab[j].buf_len=0;
	memset(modbus_tcpget_tab[j].tcp_buf,0,IEC104_FRAME_MAX_SIZE);
	memset(modbus_tcpget_tab[j].modbus_frame,0,IEC104_FRAME_MAX_SIZE);

	modbus_tcpget_tab[j].dev_id=0;
    modbus_tcpget_tab[j].data_type=READ_HOLDING_REGISTER;
	modbus_tcpget_tab[j].plc_ind=0;
	modbus_tcpget_tab[j].reg_cnt=0;
	modbus_tcpget_tab[j].start_addr[0]=0;
	modbus_tcpget_tab[j].start_addr[1]=0;


	/*  set some par for test */

		
}


static void init_DO_register(void)
{   
    
    file_d5=open(DO_1, O_RDWR); 
    file_d6=open(DO_2, O_RDWR); 
    file_d7=open(DO_3, O_RDWR); 
    file_d8=open(DO_4, O_RDWR);    
    
}

static int plcMain(void)
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
    int maxsock;
    struct timeval tv;
    
    pthread_t   pid;
    int err;
 
 
 /*********************************************************************   
    just for test by wangwei,test 485_1
 **********************************************************************/   
    int fd,count;
    
    char ptr[BUF_SIZE];
    int len;
    char cmd[30];
  
    
    
    init_RS_port();     /* just for test,waiting for par */ 
    
    fd = init_dev(2);   /* init serial port device,just for test,waiting for par */
  
    if (fd<0)
    {
        printf("open serials port failed\n");
        exit(-1);
    }

    maxsock = fd;

#if DEBUG==10
    printf("fd=%u\n",fd);
#endif
  /**************************************************************/
 
    
    init_DO_register();
    
            
    for(j=0;j<BACKLOG;j++)
	{
		init_iec104_entry(j);
		init_modbus_entry(j);
	}

    init_iec104_par();


	for(j=0;j<MODBUSTCP_NUM;j++)
	{
		init_modbus_tcpget_entry(j);
	}
    

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
    
    while (1) 
    {
        
        /*initialize file descriptor set*/
            FD_ZERO(&fdsr);
            FD_SET(sock_fd_104, &fdsr);
            FD_SET(sock_fd_modbus, &fdsr);
            FD_SET(fd, &fdsr);
            // timeout setting
            tv.tv_sec = 10;
            tv.tv_usec = 0;
     
			sem_wait(&sem);
            
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
            
            
                               
            ret = select(maxsock + 1, &fdsr, NULL, NULL, &tv);
            
            
            
            if (ret < 0) 
            {
                perror("select error");
                break;
            } 
            else if (ret == 0) 
            {
                printf("timeout\n");

				sem_post(&sem);
                continue;
            }

                       
            /* check every serial port fd in the set */
            for(k=0;k<RS_PORT_NUM;k++)
            {
                if ( (rs_tab[k].fd>0) && FD_ISSET(rs_tab[k].fd, &fdsr) ) 
                {
                    len = read(rs_tab[k].fd,ptr,BUF_SIZE);    
                
                    if(len<=0)
                    {
						#if DEBUG==10
                        printf("serial port read error\n");
						#endif
                    }
                    else
                    {
                        ptr[len] = 0;   
						
						#if DEBUG==10
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
    

    // close other connections
    for (i = 0; i < BACKLOG; i++) 
	{
        if (client_tab[i].fd != 0)
		{
            close(client_tab[i].fd);
        }
    }
    for (i = 0; i < BACKLOG; i++) 
	{
        if (modbus_tab[i].fd != 0)
		{
            close(modbus_tab[i].fd);
        }
    }
	for (i = 0; i < MODBUSTCP_NUM; i++) 
	{
        if (modbus_tcpget_tab[i].fd != 0)
		{
            close(modbus_tcpget_tab[i].fd);
        }
    }
	
    pthread_join(pid,NULL);
	close(sock_fd_104);
	close(sock_fd_modbus);
	close(fd);

    exit(0);
}

int main(int argc,char** argv)
{
    int state;
    
    
}

