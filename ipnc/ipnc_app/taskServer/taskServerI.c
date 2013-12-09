
#include<sys/types.h>
#include<sys/stat.h>
#include<errno.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define TASK_SERVER_TEST 0

#if TASK_SERVER_TEST==0
int taskCommand(char *fifo,char *cmd)
#else
int main(int argc,char** argv)
#endif
{
    #if TASK_SERVER_TEST==1
    char *fifo;
    char *cmd;
    #endif
    
    int fd;
    char w_buf[256];
    int nwrite;

    #if TASK_SERVER_TEST==1
    if(argc!=3)
    {
        printf("fifoWrite <FIFO> <string>\n");
        exit(1);
    }

    fifo=argv[1];
    cmd=argv[2];
    #endif
    
    fd=open(fifo,O_WRONLY|O_NONBLOCK,0);

    if(fd==-1)
    {
        printf("cannot open FIFO %s: %s\n",fifo,strerror(errno));

        return 1;
    }

    nwrite=write(fd,cmd,strlen(w_buf)+1);
    if(nwrite==-1)
    {
        #if TASK_SERVER_TEST==1
        if(errno==EAGAIN)
        {
            printf("The FIFO has not been read yet. Please try later\n");
        }
        #endif

        return 1;
    }
    else
    {
        #if TASK_SERVER_TEST==1
        printf("write %s to the FIFO\n",cmd);
        #endif

        return 0;
    }
}

