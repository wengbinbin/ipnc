
#define __TASK_SERVER_C__

#define TASK_SERVER_TEST 0

#include<sys/types.h>
#include<sys/stat.h>
#include<errno.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>

int server_state;

#if TASK_SERVER_TEST==0
extern void *server(void *arg);
#else
static void *server(void *arg)
{
    int count;

    printf("server start...\n");
        
    count=0;
    while(server_state)
    {
        printf("server count=%d\n",count++);

        sleep(1);
    }

    printf("server close\n");
    
    return 0;
}
#endif

main(int argc,char** argv)
{
    #if TASK_SERVER_TEST==0
    char *fifo;
    #endif
    
    pthread_t pid;
    int err;
    char buf_r[256];
    int fd;
    int nread;

    #if TASK_SERVER_TEST==0
    fifo=FIFO;
    #else
    if(argc!=2)
    {
        printf("fifoRead <FIFO>\n");
        exit(1);
    }

    fifo=argv[1];
    #endif
    
    if(mkfifo(fifo,O_CREAT|O_EXCL)<0 && errno!=EEXIST)
    {
        printf("cannot create FIFO %s: %s\n",fifo,strerror(errno));

        exit(1);
    }

    #if TASK_SERVER_TEST==1
    printf("start server\n");
    #endif
    
    server_state=1;
    
    err = pthread_create(&pid, NULL,&server, NULL);

    if(err != 0)
    {
        printf("can't create thread: %s\n", strerror(err));
        exit(1);
    }

    #if TASK_SERVER_TEST==1
    printf("server created\n");
    #endif
    
    while(1)
    {
        #if TASK_SERVER_TEST==1
        printf("Preparing for reading bytes....\n");
        #endif

        fd=open(fifo,O_RDONLY,0);

        if(fd==-1)
        {
            printf("cannot open FIFO %s: %s\n",fifo,strerror(errno));
            
            exit(1);
        }

        #if TASK_SERVER_TEST==1
        printf("FIFO opend\n");
        #endif

        nread=read(fd,buf_r,sizeof(buf_r));
        
        if(nread==-1)
        {
            #if TASK_SERVER_TEST==1
            if(errno==EAGAIN)
            {
                printf("no data yet\n");
            }
            #endif

            continue;
        }

        #if TASK_SERVER_TEST==1
        printf("read %s from FIFO\n",buf_r);
        #endif

        if(strcmp(buf_r,"quit")==0)
        {
            break;
        }
        else if(strcmp(buf_r,"restart")==0)
        {
            
            #if TASK_SERVER_TEST==1
            printf("close server\n");
            #endif
            
            server_state=0;

            pthread_join(pid,NULL);

            sleep(3);
            
            #if TASK_SERVER_TEST==1
            printf("start server\n");
            #endif
            
            server_state=1;
            
            err = pthread_create(&pid, NULL,&server, NULL);

            if(err != 0)
            {
                printf("can't create thread: %s\n", strerror(err));
                
                exit(1);
            }

            #if TASK_SERVER_TEST==1
            printf("server created\n");
            #endif
        }

        close(fd);

        sleep(1);
    }

    server_state=0;

    pthread_join(pid,NULL);

    unlink(fifo);

    return 0;
}


