
/* Standard Linux headers */
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <signal.h>
#include <setjmp.h>
#include <termios.h>
#include <ptzctrl.h>
#include "file_msg_drv.h"
#include "sys_env_type.h"
#define PTZCTRLPORT 18884
volatile int PtzThreadQuit;

static int sockfd;

int ptzGetCmdCtrl(int ptzModelNum, int ctrlCmd, int npreset, int targetID, char *pBuf);
//modify by wbb add a parameter npreset
unsigned int ptzGetBaudRate(unsigned int nBaud);
unsigned int ptzGetDataBit(unsigned int nBit);

void ptzDRX500CheckSum(char *pBuf);
void ptzPelcoCheckSum(char *pBuf);

//***************************SIGNAL PROCESS************************
typedef void ptz_Sigfunc(int);
static sigjmp_buf jmpbuf;
static volatile sig_atomic_t can_jump;
ptz_Sigfunc *ptz_vlSignalInstall(int signo, ptz_Sigfunc *func)
{
    struct sigaction act, oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (signo == SIGALRM)
    {
        #ifdef SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT; // SunOS 4.x
        #endif
    }
    else
    {
        #ifdef SA_RESTART
        act.sa_flags |= SA_RESTART; // SVR4, 4.4BSD
        #endif
    }
    if (sigaction(signo, &act, &oact) < 0)
    {
        return (SIG_ERR);
    }
    return (oact.sa_handler);
}

void ptz_vl_Sigterm(int dummy)
{
    printf("ptz:caught SIGTERM: shutting down\n");
    pthread_exit(0);
}

void ptz_vl_Sigint(int dummy)
{
    /*
    if(can_jump==0){
    return;
    }
    can_jump=0;*/

    close(sockfd);
    
    PtzThreadQuit = 1;

    alarm(1);
    //siglongjmp(jmpbuf,1);
}

void ptz_vl_Sigsegv(int dummy)
{
    printf("ptz:caught SIGSEGV: shutting down\n");
    pthread_exit(0);
}

void ptz_vl_Sigkill(int dummy)
{
    printf("ptz:caught SIGKILL: shutting down\n");
    pthread_exit(0);
}

void ptz_SetupSignalRoutine()
{
    ptz_vlSignalInstall(SIGINT, ptz_vl_Sigint);
    ptz_vlSignalInstall(SIGTERM, ptz_vl_Sigterm);
    ptz_vlSignalInstall(SIGSEGV, ptz_vl_Sigsegv);
    ptz_vlSignalInstall(SIGALRM, ptz_vl_Sigint);
    ptz_vlSignalInstall(SIGKILL, ptz_vl_Sigkill);
}

//***************************SIGNAL PROCESS************************

int ptzGetCmdCtrl(int ptzModelNum, int ctrlCmd, int npreset, int targetID, char *pBuf)
{
    int ctrlSize = 0;

    switch (ptzModelNum)
    {
        case PTZCTRL_DRX_500:
            ctrlSize = 11;
            pBuf[0] = 0x55;
            pBuf[2] = targetID;
            pBuf[9] = 0xaa;
            switch (ctrlCmd)
            {
            case PAN_RIGHT:
                pBuf[4] = 0x02;
                pBuf[5] = 0xB0;
                break;
            case PAN_LEFT:
                pBuf[4] = 0x04;
                pBuf[5] = 0xB0;
                break;
            case TILT_UP:
                pBuf[4] = 0x08;
                pBuf[6] = 0xBF;
                break;
            case TILT_DOWN:
                pBuf[4] = 0x10;
                pBuf[6] = 0xBF;
                break;
            case ZOOM_IN:
                pBuf[4] = 0x20;
                pBuf[7] = 0x05;
                break;
            case ZOOM_OUT:
                pBuf[4] = 0x40;
                pBuf[7] = 0x05;
                break;
            case FOCUS_OUT:
                pBuf[3] = 0x01;
                pBuf[7] = 0x05;
                break;
            case FOCUS_IN:
                pBuf[3] = 0x02;
                pBuf[7] = 0x05;
                break;

            }

            ptzDRX500CheckSum(pBuf);

            break;
        case PTZCTRL_PELCO_D:

            ctrlSize = 7;
            pBuf[0] = 0xFF;
            pBuf[1] = targetID;
            switch (ctrlCmd)
            {
            case PAN_RIGHT:
                pBuf[3] = 0x02;
                pBuf[4] = 0x1B;
                break;
            case PAN_LEFT:
                pBuf[3] = 0x04;
                pBuf[4] = 0x1B;
                break;
            case TILT_UP:
                pBuf[3] = 0x08;
                pBuf[5] = 0x1B;
                break;
            case TILT_DOWN:
                pBuf[3] = 0x10;
                pBuf[5] = 0x1B;
                break;
            case ZOOM_IN:
                pBuf[3] = 0x20;
                break;
            case ZOOM_OUT:
                pBuf[3] = 0x40;
                break;
            case FOCUS_OUT:
                pBuf[3] = 0x80;
                break;
            case FOCUS_IN:
                pBuf[2] = 0x01;
                break;
            case PTZ_ROUND:
                pBuf[3] = 0x07;
                pBuf[5] = 0x63;
                break;
            case RIGHT_UP:
                pBuf[3] = 0x0A;
                pBuf[4] = 0x1B;
                pBuf[5] = 0x1B;
                break;
            case RIGHT_DOWN:
                pBuf[3] = 0x12;
                pBuf[4] = 0x1B;
                pBuf[5] = 0x1B;
                break;
            case LEFT_UP:
                pBuf[3] = 0x0C;
                pBuf[4] = 0x1B;
                pBuf[5] = 0x1B;
                break;
            case LEFT_DOWN:
                pBuf[3] = 0x14;
                pBuf[4] = 0x1B;
                pBuf[5] = 0x1B;
                break;
            case LIGHTON:
                pBuf[3] = 0x09;
                pBuf[5] = 0x02;
                break;
            case LIGHTOFF:
                pBuf[3] = 0x0B;
                pBuf[5] = 0x02;
                break;
            case RAINON:
                pBuf[3] = 0x09;
                pBuf[5] = 0x01;
                break;
            case RAINOFF:
                pBuf[3] = 0x0B;
                pBuf[5] = 0x02;
                break;
            case APERTURE_OUT:
                pBuf[2] = 0x04;
                break;
            case APERTURE_IN:
                pBuf[2] = 0x02;
                break;
            case PRESET:
                pBuf[3]=0x03;
                pBuf[5]=0x00+npreset;
                break;
            case TURNTOPRESET:
                pBuf[3]=0x07;
                pBuf[5]=0x00+npreset;
                break;
//**************add by wbb***********//
/*
            case PRESET1:
                pBuf[3]=0x03;
                pBuf[5]=0x01;
                break;
             case TURNTOPRESET1:
                pBuf[3]=0x07;
                pBuf[5]=0x01;
                break;
            case PRESET2:
                pBuf[3]=0x03;
                pBuf[5]=0x02;
                break;
             case TURNTOPRESET2:
                pBuf[3]=0x07;
                pBuf[5]=0x02;
                break;
             case PRESET3:
                pBuf[3]=0x03;
                pBuf[5]=0x03;
                break;
             case TURNTOPRESET3:
                pBuf[3]=0x07;
                pBuf[5]=0x03;
                break; 
            case PRESET4:
                pBuf[3]=0x03;
                pBuf[5]=0x04;
                break;
             case TURNTOPRESET4:
                pBuf[3]=0x07;
                pBuf[5]=0x04;
                break;
             case PRESET5:
                pBuf[3]=0x03;
                pBuf[5]=0x05;
                break;
             case TURNTOPRESET5:
                pBuf[3]=0x07;
                pBuf[5]=0x05;
                break;  
                */

 //**************add by wbb end***********//               
            }

 
            //**************add by wbb begin******************//
 /*        
            if  (ctrlCmd/10==2)
            {
                pBuf[3]=0x03;
                pBuf[5]=0x00+ctrlCmd%10;
            }
             if  (ctrlCmd/10==3)
            {
                pBuf[3]=0x07;
                pBuf[5]=0x00+ctrlCmd%10;
            }
*/
            //**************add by wbb end******************//   
            ptzPelcoCheckSum(pBuf);
            break;
    }

    return ctrlSize;
}

void ptzDRX500CheckSum(char *pBuf)
{
    int nSum = 0;
    int nNum = 0;
    int nCs = 0;

    nSum = pBuf[0];
    nSum += pBuf[1];
    nSum += pBuf[2];
    nSum += pBuf[3];
    nSum += pBuf[4];
    nSum += pBuf[5];
    nSum += pBuf[6];
    nSum += pBuf[7];
    nSum += pBuf[8];
    nSum += pBuf[9];

    nNum = 0x2020 - nSum;

    if (nNum &1)
    {
        nCs += 1;
    }
    if (nNum &2)
    {
        nCs += 2;
    }
    if (nNum &4)
    {
        nCs += 4;
    }
    if (nNum &8)
    {
        nCs += 8;
    }
    if (nNum &16)
    {
        nCs += 16;
    }
    if (nNum &32)
    {
        nCs += 32;
    }
    if (nNum &64)
    {
        nCs += 64;
    }
    if (nNum &128)
    {
        nCs += 128;
    }

    pBuf[10] = nCs;
}

void ptzPelcoCheckSum(char *pBuf)
{
    unsigned char bySum = 0;
    bySum = pBuf[1];
    bySum += pBuf[2];
    bySum += pBuf[3];
    bySum += pBuf[4];
    bySum += pBuf[5];

    pBuf[6] = bySum;
}

unsigned int ptzGetBaudRate(unsigned int nBaud)
{
    unsigned int nBaudRate;

    switch (nBaud)
    {
        case 1200:
            #if DEBUG==1
            printf("===set baudrate B1200 ===\n");
            #endif
            
            nBaudRate = B1200;
            break;
        case 1800:
            #if DEBUG==1
            printf("===set baudrate B1800 ===\n");
            #endif

            nBaudRate = B1800;
            break;
        case 2400:
            #if DEBUG==1
            printf("===set baudrate B2400 ===\n");
            #endif

            nBaudRate = B2400;
            break;
        case 4800:
            #if DEBUG==1
            printf("===set baudrate B4800 ===\n");
            #endif
            
            nBaudRate = B4800;
            break;
        case 9600:
            #if DEBUG==1
            printf("===set baudrate B9600 ===\n");
            #endif

            nBaudRate = B9600;
            break;
        case 19200:
            #if DEBUG==1
            printf("===set baudrate B19200 ===\n");
            #endif

            nBaudRate = B19200;
            break;
        case 38400:
            #if DEBUG==1
            printf("===set baudrate B38400 ===\n");
            #endif

            nBaudRate = B38400;
            break;
        case 57600:
            #if DEBUG==1
            printf("===set baudrate B57600 ===\n");
            #endif
            
            nBaudRate = B57600;
            break;
        case 115200:
            #if DEBUG==1
            printf("===set baudrate B115200 ===\n");
            #endif
            
            nBaudRate = B115200;
            break;
        default:
            #if DEBUG==1
            printf("===set baudrate DEFAULT B9600 ===\n");
            #endif
            
            nBaudRate = B9600;
            break;
    }

    return nBaudRate;
}

unsigned int ptzGetDataBit(unsigned int nBit)
{
    unsigned int nDataBit;

    switch (nBit)
    {
        case 5:
            #if DEBUG==1
            printf("===set Databit CS5 ===\n");
            #endif
            
            nDataBit = CS5;
            break;
        case 6:
            #if DEBUG==1
            printf("===set Databit CS6 ===\n");
            #endif
            
            nDataBit = CS6;
            break;
        case 7:
            #if DEBUG==1
            printf("===set Databit CS7 ===\n");
            #endif
            
            nDataBit = CS7;
            break;
        case 8:
            #if DEBUG==1
            printf("===set Databit CS8 ===\n");
            #endif
            
            nDataBit = CS8;
            break;
        default:
            #if DEBUG==1
            printf("===set Databit DEFAULT CS8 ===\n");
            #endif
            
            nDataBit = CS8;
            break;
    }

    return nDataBit;
}

static char *DevNameTab[]=
{
    "/dev/ttyXX0",
    "/dev/ttyXX1",
    "/dev/ttyXM0",
    "/dev/ttyXM1"
};

int PTZCTRL_setInternalCtrl(int Chl, int ctrlCmd,int npreset)
{
    int devNo;
    int fd;
    struct termios oldtio, newtio;
    char ptzBuf[64];
    PTZ_serialInfo g_serialInfo;
    int ptzCtrlSize;

    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    devNo=pSysInfo->ptz_channel[Chl].dev_no;
    
    if(devNo==DEV_NO_NONE)
    {
        #if DEBUG==1
        printf("ERROR devnum=%d\n",devNo);
        #endif
        
        return -1;
    }

    
    fd=open(DevNameTab[pSysInfo->ptz_channel[Chl].dev_no], O_RDWR | O_NOCTTY | O_NONBLOCK);

    if(fd<0)
    {
        #if DEBUG==1
        printf("ERROR open %s fail\n",DevNameTab[pSysInfo->ptz_channel[Chl].dev_no]);
        #endif
        
        return -1;
    }
    
    g_serialInfo.devaddr = pSysInfo->ptz_channel[Chl].devaddr;
    g_serialInfo.ptzDatabit = pSysInfo->ptz_channel[Chl].databit;
    g_serialInfo.ptzParitybit = pSysInfo->ptz_channel[Chl].parity;
    g_serialInfo.ptzStopbit = pSysInfo->ptz_channel[Chl].stopbit;
    g_serialInfo.ptzBaudrate = pSysInfo->ptz_channel[Chl].speed;

  
    tcgetattr(fd, &oldtio);

    bzero(&newtio, sizeof(newtio));

    newtio.c_cflag = CLOCAL | CREAD;

    newtio.c_cflag |= ptzGetBaudRate((unsigned int)g_serialInfo.ptzBaudrate);
    newtio.c_cflag |= ptzGetDataBit((unsigned int)g_serialInfo.ptzDatabit);

    if (g_serialInfo.ptzParitybit == PTZ_PARITY_EVEN)
    {
        #if DEBUG==1
        printf("===set paritybit EVEN===\n");
        #endif

        newtio.c_cflag |= PARENB;
    }
    else if (g_serialInfo.ptzParitybit == PTZ_PARITY_ODD)
    {
        #if DEBUG==1
        printf("===set paritybit ODD===\n");
        #endif

        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
    }

    if (g_serialInfo.ptzStopbit == 2)
    {
       #if DEBUG==1023
       printf("===set stopbit 2 ===\n");
       #endif

        newtio.c_cflag |= CSTOPB;
        newtio.c_cflag |= CSIZE;
    }

    newtio.c_iflag = IGNPAR | ICRNL;

    newtio.c_oflag = 0;
    newtio.c_lflag = ICANON;

    newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
    newtio.c_cc[VMIN] = 1; /* blocking read until 1 character arrives */

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);

    bzero(ptzBuf, 25);

    ptzCtrlSize = ptzGetCmdCtrl(pSysInfo->ptz_channel[Chl].protocol, ctrlCmd, npreset, g_serialInfo.devaddr, ptzBuf);

    if (ptzCtrlSize > 0)
    {
        #if DEBUG==1023
        {
            int i;
        
            for (i = 0; i < ptzCtrlSize; i++)
            {
                printf("%x ", ptzBuf[i]);
            }
            printf("\n");
        }
        #endif

        write(fd, (void*)ptzBuf, ptzCtrlSize);
        close(fd);
    }

    return 0;
}

#if 0
int PTZCTRL_ptzSendBypass(char *ptzBuf, int devnum, int bufSize)
{
    int sendSize;
    PTZ_serialInfo g_serialInfo;
    struct termios oldtio, newtio;
    int ptzCtrlSize;

    //*********************************get the settings of ptzIdx
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    } g_serialInfo.ptzDatabit = pSysInfo->rs485_config[0][devnum].databit;
    g_serialInfo.ptzParitybit = pSysInfo->rs485_config[0][devnum].parity;
    g_serialInfo.ptzStopbit = pSysInfo->rs485_config[0][devnum].stopbit;
    g_serialInfo.ptzBaudrate = pSysInfo->rs485_config[0][devnum].speed;

    //**********************************
    tcgetattr(serial_fd, &oldtio);

    bzero(&newtio, sizeof(newtio));

    newtio.c_cflag = CLOCAL | CREAD;

    newtio.c_cflag |= ptzGetBaudRate((unsigned short)g_serialInfo.ptzBaudrate);
    newtio.c_cflag |= ptzGetDataBit((unsigned short)g_serialInfo.ptzDatabit);

    //////////// parity bit /////////////////
    if (g_serialInfo.ptzParitybit == INT_PARITY_EVEN)
    {
        #if DEBUG==1
        printf("===set paritybit EVEN===\n");
        #endif
        
        newtio.c_cflag |= PARENB;
    }
    else if (g_serialInfo.ptzParitybit == INT_PARITY_ODD)
    {
        #if DEBUG==1
        printf("===set paritybit ODD===\n");
        #endif
        
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
    }

    //////////// stop bit //////////////
    if (g_serialInfo.ptzStopbit == 2)
    {
        #if DEBUG==1
        printf("===set stopbit 2 ===\n");
        #endif
        
        newtio.c_cflag |= CSTOPB;
        newtio.c_cflag |= CSIZE;
    }

    newtio.c_iflag = IGNPAR | ICRNL;

    newtio.c_oflag = 0;
    newtio.c_lflag = ICANON;

    newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
    newtio.c_cc[VMIN] = 1; /* blocking read until 1 character arrives */

    tcflush(serial_fd, TCIFLUSH);
    tcsetattr(serial_fd, TCSANOW, &newtio);

    if (bufSize > 0)
    {
        ptzCtrlSize = bufSize + 1;

        sendSize = write(serial_fd, (void*)ptzBuf, ptzCtrlSize);
    }

    return sendSize;
}
#endif

void *PTZCtrlThread(void *arg)
{
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int clientaddr_size, retval;
    char DataBuffer[128];
    char *username;
    char *passwd;
    char *channel;
    char *cmd;
    char *numbpreset;
    int Chl;
    int i;
    int npreset;

    PtzThreadQuit = 0;

    ptz_SetupSignalRoutine();

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) ==  - 1)
    {
        perror("socket");
        return  NULL;
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PTZCTRLPORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) ==  - 1)
    {
        perror("bind");
        return NULL;
    }

    while (!PtzThreadQuit)
    {
        /*
        if(sigsetjmp(jmpbuf,1)){
        can_jump=1;
        break;
        }
        can_jump=1;*/
        memset(DataBuffer, 0, sizeof(DataBuffer));
        
            #if DEBUG==1025
            printf("=======receivedata======");
            #endif
        if ((retval = recvfrom(sockfd, DataBuffer, 128, 0, (struct sockaddr*) &client_addr, &clientaddr_size) )> 0)
        {
            #if DEBUG==1025
            printf("=======PTZ data: %s\n", DataBuffer);
            #endif

            username = strtok(DataBuffer, "|");
            if (username == NULL)
            {
                continue;
            } 

            passwd = strtok(NULL, "|");
            if (passwd == NULL)
            {
                continue;
            }

            channel = strtok(NULL, "|");
            if (channel == NULL)
            {
                continue;
            }

            cmd = strtok(NULL, "|");
            if (cmd == NULL)
            {
                continue;
            }
            numbpreset= strtok(NULL, "|");          

            
            #if DEBUG==1026
             printf("@@@@@@@@@@@username=%s, passwod=%s, channel=%s, cmd=%s@@@@@@@@@@@\n",username,passwd,channel,cmd);
            #endif
            
            SysInfo *pSysInfo = GetSysInfo();
            
            if (pSysInfo == NULL)
            {
                return NULL;
            }
            
            for (i = 0; i < ACOUNT_NUM; i++)
            {
                if (strcmp(pSysInfo->acounts[i].user, username) == 0 && strcmp(pSysInfo->acounts[i].password, passwd) == 0)
                {
                    #if DEBUG==1
                    printf("match User i=%d\n",i);
                    #endif
                    
                    break;
                }
            }

            if(i>=ACOUNT_NUM)
            {
                continue;
            }
            
            if (pSysInfo->acounts[i].authority >2)
            {
                continue;
            }

            #if DEBUG==1
            printf("match Authrity\n");
            #endif

            Chl=atoi(channel);

            if(Chl>=MAX_NUM_PTZ_CHL_PTZ)
            {
                continue;
            }

            if (strcmp(cmd, "rotate") == 0)
            //round
            {
                #if DEBUG==1
                printf("rotate begin\n");
                #endif

                PTZCTRL_setInternalCtrl( Chl, PTZ_ROUND,0);
                #if DEBUG==1
                printf("rotate done\n");
                #endif
            }
            else if (strcmp(cmd, "left") == 0)
            //left
            {
                #if DEBUG==1
                printf("left begin\n");
                #endif

                PTZCTRL_setInternalCtrl( Chl, PAN_LEFT,0);
                #if DEBUG==1
                printf("left done\n");
                #endif
            }
            else if (strcmp(cmd, "right") == 0)
            //right
            {
                #if DEBUG==1
                printf("right begin\n");
                #endif

                PTZCTRL_setInternalCtrl( Chl, PAN_RIGHT,0);
                #if DEBUG==1
                printf("right done\n");
                #endif
            }
            else if (strcmp(cmd, "up") == 0)
            //up
            {
                #if DEBUG==1
                printf("up begin\n");
                #endif

                PTZCTRL_setInternalCtrl( Chl, TILT_UP,0);
                #if DEBUG==1
                printf("up done\n");
                #endif
            }
            else if (strcmp(cmd, "down") == 0)
            //down
            {
                PTZCTRL_setInternalCtrl( Chl, TILT_DOWN,0);
            }
            else if (strcmp(cmd, "leftup") == 0)
            //leftup
            {
                PTZCTRL_setInternalCtrl( Chl, LEFT_UP,0);
            }
            else if (strcmp(cmd, "leftdown") == 0)
            //leftdown
            {
                PTZCTRL_setInternalCtrl( Chl, LEFT_DOWN,0);
            }
            else if (strcmp(cmd, "rightup") == 0)
            //rightup
            {
                PTZCTRL_setInternalCtrl( Chl, RIGHT_UP,0);
            }
            else if (strcmp(cmd, "rightdown") == 0)
            //rightdown
            {
                PTZCTRL_setInternalCtrl( Chl, RIGHT_DOWN,0);
            }
            else if (strcmp(cmd, "zoomin") == 0)
            //zoomin
            {
                PTZCTRL_setInternalCtrl( Chl, ZOOM_IN,0);
            }
            else if (strcmp(cmd, "zoomout") == 0)
            //zoomout
            {
                PTZCTRL_setInternalCtrl( Chl, ZOOM_OUT,0);
            }
            else if (strcmp(cmd, "focusin") == 0)
            //focusin
            {
                PTZCTRL_setInternalCtrl( Chl, FOCUS_IN,0);
            }
            else if (strcmp(cmd, "focusout") == 0)
            //focusout
            {
                PTZCTRL_setInternalCtrl( Chl, FOCUS_OUT,0);
            }
            else if (strcmp(cmd, "aperturein") == 0)
            //aperturein
            {
                PTZCTRL_setInternalCtrl( Chl, FOCUS_IN,0);
            }
            else if (strcmp(cmd, "apertureout") == 0)
            //apertureout
            {
                PTZCTRL_setInternalCtrl( Chl, FOCUS_OUT,0);
            }
            else if (strcmp(cmd, "lighton") == 0)
            //lighton
            {
                PTZCTRL_setInternalCtrl( Chl, LIGHTON,0);
            }
            else if (strcmp(cmd, "lightoff") == 0)
            //lightoff
            {
                PTZCTRL_setInternalCtrl( Chl, LIGHTOFF,0);
            }
            else if (strcmp(cmd, "rainon") == 0)
            //rainon
            {
                PTZCTRL_setInternalCtrl( Chl, RAINON,0);
            }
            else if (strcmp(cmd, "rainoff") == 0)
            //rainoff
            {
                PTZCTRL_setInternalCtrl( Chl, RAINOFF,0);
            }
            else if (strcmp(cmd,"preset")==0)
                //preset
            {
                npreset=atoi(numbpreset);
                PTZCTRL_setInternalCtrl(Chl, PRESET,npreset);
                
            }
            else if (strcmp(cmd,"turntopreset")==0)
                //turn to preset
     	    {
                npreset=atoi(numbpreset);
                PTZCTRL_setInternalCtrl(Chl, TURNTOPRESET,npreset);
            }

            //***************ADD BY WBB END*******//
            else /*if (strcmp(cmd, "stop") == 0)*/ /* All other command do stop, mayue 2013.8.20 */
            //stop
            {
                #if DEBUG==1
                printf("stop begin\n");
                #endif

                PTZCTRL_setInternalCtrl( Chl, PTZ_STOP,0);

                #if DEBUG==1
                printf("stop done\n");
                #endif
            }
        }
    }

    close(sockfd);
}
