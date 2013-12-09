
#define __PLC_I_C__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <Msg_Def.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <share_mem.h>
#include <pthread.h>
#include <file_msg_drv.h>
#include <alarm_msg_drv.h>
#include <signal.h>

#include <sys_env_type.h>
#include <system_default.h>

static int qid;

/**
 * @brief Send quit command to file manager.

 * This command will make file manager stop running. After you called this, all
 *  the other process can't get system information because file manager is down.
 */
void SendPlcStartCmd(void)
{
    PLC_MSG_BUF msgbuf;

    #if DEBUG==0
    printf("SendPlcStartCmd\n");
    #endif

    memset(&msgbuf, 0, sizeof(msgbuf));
    msgbuf.Des = MSG_TYPE_MSG1;
    msgbuf.src = 0;
    msgbuf.cmd = PLC_MSG_START;
    msgsnd(qid, &msgbuf, sizeof(msgbuf) - sizeof(long), 0); /*send msg1*/
}

void SendPlcQuitCmd(void)
{
    PLC_MSG_BUF msgbuf;

    #if DEBUG==0
    printf("SendPlcQuitCmd\n");
    #endif

    memset(&msgbuf, 0, sizeof(msgbuf));
    msgbuf.Des = MSG_TYPE_MSG1;
    msgbuf.src = 0;
    msgbuf.cmd = PLC_MSG_QUIT;
    msgsnd(qid, &msgbuf, sizeof(msgbuf) - sizeof(long), 0); /*send msg1*/
}

void SendPlcRestartCmd(void)
{
    PLC_MSG_BUF msgbuf;

    #if DEBUG==0
    printf("SendPlcRestartCmd\n");
    #endif

    memset(&msgbuf, 0, sizeof(msgbuf));
    msgbuf.Des = MSG_TYPE_MSG1;
    msgbuf.src = 0;
    msgbuf.cmd = PLC_MSG_RESTART;
    msgsnd(qid, &msgbuf, sizeof(msgbuf) - sizeof(long), 0); /*send msg1*/
}

int plcInit(void)
{
    #if DEBUG==0
    printf("plcInit\n");
    #endif

    qid = Msg_Init(PLC_MSG_KEY);

    if (qid < 0)
    {
        return  - 1;
    }

    return 0;
}

