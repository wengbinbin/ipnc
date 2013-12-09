
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <Msg_Def.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <share_mem.h>
#include <pthread.h>
#include <file_msg_drv.h>
#include <alarm_msg_drv.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#include <sys_env_type.h>
#include <system_default.h>
#include <sys_info_default.h>

#define INNER_PTHREAD_PLC 0

SysInfo *sysInfoP=NULL;

static int qid;
static int mid;
static int mFileId;
static void *pFileShareMem;

static pthread_t pid_pthread_monitor;

static int progress_plc_run;
static int progress_plc_status;

FILE *plcLogFile;

char *getTime(void)
{
    #define NUM_BUF 8
    #define BUF_SIZE 128
    static int ind=0;
    static char buf[NUM_BUF][BUF_SIZE];
    struct timeval tv;
    struct tm *tmP;
    char time_buf[BUF_SIZE];
    int usec;
    
    gettimeofday(&tv,NULL);

    tmP=localtime(&tv.tv_sec);
        
    ind=(ind+1) % NUM_BUF;

    strftime(time_buf,sizeof(time_buf),"%Y-%m-%d %H:%M:%S",tmP);

    usec=tv.tv_usec;
    
    sprintf(buf[ind],"%s %d",time_buf,usec);

    return buf[ind];
}

/**
 * @brief Initialize message queue.

 * Initialize message queue.
 * @note This API must be used before use any other message driver API.
 * @param msgKey [I ] Key number for message queue and share memory.
 * @return message ID.
 */

int Msg_Init(int msgKey)
{
    int qid;
    key_t key = msgKey;

    qid = msgget(key, 0);

    if (qid < 0)
    {
        qid = msgget(key, IPC_CREAT | 0666);

        #if DEBUG==1
        printf("Creat queue id:%d\n", qid);
        #endif
    }

    #if DEBUG==1
    printf("queue id:%d\n", qid);
    #endif

    return qid;
}

/**
 * @brief Initialize pshared memory driver.

 * Initialize pshared momory driver.
 * @note The memory ID isn't saved to global variable.
 * @return Memory ID
 * @retval -1 Fail to initialize shared memory.
 */
int pShareMemInit(int key)
{
    mid = shmget(key, 0, 0);

    if (mid < 0)
    {
        mid = shmget(key, PROC_MEM_SIZE *MAX_SHARE_PROC, IPC_CREAT | 0660);

        #if DEBUG==1
        printf("Create shared memory id:%d\n", mid);
        #endif
    }

    #if DEBUG==1
    printf("shared memory size %d\n", PROC_MEM_SIZE *MAX_SHARE_PROC);
    #endif

    #if DEBUG==1
    printf("shared memory id:%d\n", mid);
    #endif

    return mid;
}

/**
 * @brief    check magic number
 * @param    fp [I ]file pointer
 * @return    error code : SUCCESS(0) or FAIL(-1)
 */
int check_magic_num(FILE *fp)
{
    int ret;
    unsigned long MagicNum;
    if (fread(&MagicNum, 1, sizeof(MagicNum), fp) != sizeof(MagicNum))
    {
        ret = FAIL;
    }
    else
    {
        if (MagicNum == MAGIC_NUM)
        {
            ret = SUCCESS;
        }
        else
        {
            ret = FAIL;
        }
    }
    return ret;
}

int create_log_file(char *name)
{
    if ((plcLogFile= fopen(name, "w+")) == NULL)
    {
        printf("PLC  Create log file %s fail: %s\n", PLC_LOG_FILE, strerror(errno));

        return FAIL;
    }

    return SUCCESS;
}

int close_log_file(void)
{
    if (( fclose(plcLogFile)) == -1)
    {
        perror("Can't create PLC log file\n");

        return FAIL;
    }

    return SUCCESS;
}

long set_log_file_ptr(void)
{
    long plcLogFileInd;

    #if DEBUG==1
    printf("set_log_file_ptr plcLogFileInd=%ld\n",sysInfoP->data_source.plcLogFileInd);
    #endif
    
    plcLogFileInd=fseek(plcLogFile,sysInfoP->data_source.plcLogFileInd,SEEK_SET);

    if(plcLogFileInd<0)
   {
        printf("PLC Init  Set logFile Pointer fail: %s\n",  strerror(errno));

        return -1;
   }

    sysInfoP->data_source.plcLogFileInd= plcLogFileInd;

    return plcLogFileInd;
}

/**
 * @brief    read SysInfo from system file
 * @param    "void *Buffer" : [OUT]buffer to store SysInfo
 * @return    error code : SUCCESS(0) or FAIL(-1)
 */
int ReadGlobal(void *Buffer)
{
    FILE *fp;
    int ret;

    if ((fp = fopen(SYS_FILE, "rb")) == NULL)
    {
        //printf("ReadGlobal ret == FAIL)\n");
        /* System file not exist */
        ret = FAIL;
    }
    else
    {
        if (check_magic_num(fp) == SUCCESS)
        {
            if (fread(Buffer, 1, SYS_ENV_SIZE, fp) != SYS_ENV_SIZE)
            {
                ret = FAIL;
            }
            else
            {
                ret = SUCCESS;
            }
        }
        else
        {
            ret = FAIL;
        }

        fclose(fp);
    }
    return ret;
}

/**
 * @brief    create system file
 * @param    name [I ]File name to create in nand.
 * @param    Global [I ]Pointer to System information
 * @return    error code : SUCCESS(0) or FAIL(-1)
 */
int create_sys_file(char *name, void *Global)
{
    FILE *fp;
    int ret;
    unsigned long MagicNum = MAGIC_NUM;

    if ((fp = fopen(name, "wb")) == NULL)
    {
        perror("Can't create system file\n");
        ret = FAIL;
    }
    else
    {
        if (fwrite(&MagicNum, 1, sizeof(MagicNum), fp) != sizeof(MagicNum))
        {
            perror("Writing Magic Number fail\n");
            ret = FAIL;
        }
        else
        {
            if (fwrite(Global, 1, SYS_ENV_SIZE, fp) != SYS_ENV_SIZE)
            {
                perror("Writing global fail\n");
                ret = FAIL;
            }
            else
            {
                ret = SUCCESS;
            }
        }
        fclose(fp);
    }
    return ret;
}

/**
 * @brief    file manager initialization
 * @param    ShareMem [O ]Pointer to share memory where system information will
be stored.
 * @return    error code : SUCCESS(0) or FAIL(-1)
 */
int FileMngInit(void *ShareMem)
{
    int ret;

    #if DEBUG==1
    printf("FileMngInit\n");
    #endif

    #if DEBUG==1
    printf("Global value size:%d\n", SYS_ENV_SIZE);
    #endif

    ret = ReadGlobal(ShareMem);
    //printf("FileMngInit ret == %d)\n",FAIL);
    if (ret == FAIL)
    {
        printf("FileMngInit ret == FAIL)\n");
        ret = create_sys_file(SYS_FILE, &SysInfoDefault);
        printf("create_sys_file Done!!!\n");
        if (ret == SUCCESS)
        {
            printf("create_sys_file ret == SUCCESS!!!\n");
            memcpy(ShareMem, &SysInfoDefault, SYS_ENV_SIZE);
        }
    }

    return ret;
}

#if INNER_PTHREAD_PLC==1
static int pthread_plc(void)
{
    int plc_count = 0;

    plcLogPrintf(1,"pthread_plc starting....\n");

    while (sysInfoP->data_source.plcRun)
    //Ñ­»·¶ÁÈ¡Êý\u0178Ý
    {
        plcLogPrintf(1,"pthread_plc started, plc_count=%d.\n", plc_count);

        plc_count++;

        if(plc_count>10)
        {
            plcLogPrintf(1,"pthread_plc stoped\n");

            exit(1);
        }

        sleep(5);
    }

    plcLogPrintf(1,"pthread_plc end\n");

    return 0;
}
#else
extern int pthread_plc(void);
#endif

static void *pthread_monitor(void *arg)
{
    pid_t pid_plc;
    
    while(progress_plc_run)
    {   
        plcLogPrintf(1,"PlcMonitor Starting\n");

        if((pid_plc=fork())==0)
        {
            pthread_plc();

            exit(0);
        }

        plcLogPrintf(1,"plcServer Started\n");

        progress_plc_status=1;

        waitpid(pid_plc,NULL,0);

        progress_plc_status=0;
    }
    
    plcLogPrintf(1,"plcServer Quit\n");

    return NULL;
}

static void start_plc(void)
{
    #if DEBUG==1
    printf("start_plc\n");
    #endif
    
    if(progress_plc_run==1)
    {
        return;
    }

    #if DEBUG==1
    printf("start_plc 0\n");
    #endif
    
    progress_plc_run=1;
    sysInfoP->data_source.plcRun=1;
            
    #if DEBUG==1
    printf("start_plc 1\n");
    #endif
    
    pthread_create(&pid_pthread_monitor, NULL, pthread_monitor, NULL);
}

static void stop_plc(void)
{
    #if DEBUG==1
    printf("stop_plc\n");
    #endif
    
    if(progress_plc_run==0)
    {
        return;
    }
    
    #if DEBUG==1
    printf("stop_plc 0\n");
    #endif
    
    progress_plc_run=0;
    sysInfoP->data_source.plcRun=0;

    #if DEBUG==1
    printf("stop_plc 1\n");
    #endif
    
    pthread_join(pid_pthread_monitor,NULL);
}

/**
 * @brief    thread processing message
 * @param    none
 * @retval    0
 */
static int ProcMsg(PLC_MSG_BUF *pMsg)
{
    #if DEBUG==1
    printf("ProcMsg\n");
    #endif
    
    switch (pMsg->cmd)
    {
        case PLC_MSG_START:
            start_plc();
            break;
        case PLC_MSG_QUIT:
            stop_plc();
            break;
        case PLC_MSG_RESTART:
            stop_plc();
            start_plc();
            break;
    }

    return 1;
}

/**
 * @brief    thread processing message
 * @param    none
 * @retval    0
 */
static void plcMsgProc(void)
{
    PLC_MSG_BUF msgbuf;
    int msg_size, e_count = 0;

    plcLogPrintf(1,"plcMsgProc start\n");

    while (1)
    {
        msg_size = msgrcv(qid, &msgbuf, sizeof(msgbuf) - sizeof(long), MSG_TYPE_MSG1, 0);

        plcLogPrintf(1,"PLC Recieved message:%d from %d\n",msgbuf.cmd, msgbuf.src);
        
        #if DEBUG==1
        printf("PLC Recieved message:%d from %d, plcRun=%d\n", msgbuf.cmd, msgbuf.src,sysInfoP->data_source.plcRun);
        #endif

        if (msg_size < 0)
        {
            plcLogPrintf(1,"Receive msg fail %s\n",strerror(errno));

            if (e_count++ >= 3)
            {
                /* Kill message queue and init angain */
                plcLogPrintf(1,"msgrcv ERROR %s\n",strerror(errno));

                msgctl(qid, IPC_RMID, NULL);

                qid = Msg_Init(PLC_MSG_KEY);
            }
        }
        else if (msgbuf.src == MSG_TYPE_MSG1 || msgbuf.src < 0)
        {
            plcLogPrintf(1,"Got Error message %s\n",strerror(errno));

            sysInfoP->data_source.plcRun = 0;
        }
        else
        {
            e_count = 0;

            if (ProcMsg(&msgbuf) == 1)
             /* Message has been processed */
            {
                #if DEBUG==1
                printf("Message process success\n");
                #endif

                if (msgbuf.src != 0)
                {
                    msgbuf.Des = msgbuf.src;
                    msgbuf.src = MSG_TYPE_MSG1;
                    msgsnd(qid, &msgbuf, sizeof(msgbuf) - sizeof(long), 0);
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    #if DEBUG==1
    printf("plc START......\n");
    #endif

    qid = Msg_Init(PLC_MSG_KEY);
    if (qid < 0)
    {
        printf("PLC Init Msg Queue  %x fail: %s\n", PLC_MSG_KEY, strerror(errno));

        return  - 1;
    }

    mFileId = pShareMemInit(FILE_MSG_KEY);
    if (mFileId < 0)
    {
        printf("PLC Init Share Memory  %x fail: %s\n", FILE_MSG_KEY, strerror(errno));

        return  - 1;
    }

    #if DEBUG==1
    printf("%s:mFileId=%d\n", __func__, mFileId);
    #endif

    pFileShareMem = shmat(mFileId, 0, 0);

    #if DEBUG==1
    printf("%s:Share Memory Addr=%p\n", __func__, pFileShareMem);
    #endif

    if (FileMngInit(pFileShareMem) != 0)
    {
        printf("PLC Init  File Management fail: %s\n",  strerror(errno));

        return  - 1;
    }

    sysInfoP = pFileShareMem;

    #if DEBUG==1
    {
        printf("admin user:=%s, passwd=%s,authority=%d\n", sysInfoP->acounts[0].user, sysInfoP->acounts[0].password, sysInfoP->acounts[0].authority);
    }
    #endif

    create_log_file(PLC_LOG_FILE) ;
    set_log_file_ptr();

    sysInfoP->data_source.plcRun = 1;
    progress_plc_status=0;

    start_plc();

    plcMsgProc();

    return 0;
}

