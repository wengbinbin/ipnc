#include <sys/ioctl.h>
#include <errno.h>
#include <remoteplay_priv.h>

#include <pthread.h>
#include <signal.h>
#include <basket.h>
#include <basket_remoteplayback.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/stat.h>

///////////////////////////////////////
#define REMOTEPLAY_DEBUG

#ifdef REMOTEPLAY_DEBUG
#define TRACE_REMOTEPLAY(msg, args...)  printf("[REMOTEPLAY] - " msg, ##args)
#define TRACEF_REMOTEPLAY(msg, args...) printf("[REMOTEPLAY] - %s:" msg, __FUNCTION__, ##args)
#define TRACEFL_REMOTEPLAY(msg, args...) printf("[REMOTEPLAY] - %s(%d):" msg, __FUNCTION__, __LINE__, ##args)
#define TRACEFLF_REMOTEPLAY(msg, args...) printf("[REMOTEPLAY] - %s(%d):\t%s:" msg, __FILE__, __LINE__, __FUNCTION__, ##args)
#else
#define TRACE_REMOTEPLAY(msg, args...)  ((void)0)
#define TRACEF_REMOTEPLAY(msg, args...) ((void)0)
#define TRACEFL_REMOTEPLAY(msg, args...) ((void)0)
#define TRACEFLF_REMOTEPLAY(msg, args...) ((void)0)
#endif
///////////////////////////////////////


REMOTEPLAY_Ctrl gREMOTEPLAY_ctrl;//add by sxh playback
REMOTEPLAY_CLIENT_List * gRMPClientList;


int RMPClientList_Init(REMOTEPLAY_CLIENT_List_Ptr * clientlist){
	*clientlist=(REMOTEPLAY_CLIENT_List *)malloc(sizeof(REMOTEPLAY_CLIENT_List));
	#ifdef REMOTEPLAY_DEBUG
	TRACE_REMOTEPLAY("RMPClientList_Init 1 clientlist=%d!!\n",*clientlist);
	#endif
	if(*clientlist==NULL){
		#ifdef REMOTEPLAY_DEBUG
        TRACEFLF_REMOTEPLAY("List malloc Fail....\n") ;
		#endif
		return -1;
	}
	#ifdef REMOTEPLAY_DEBUG
	TRACE_REMOTEPLAY("RMPClientList_Init 2 clientlist=%d!!\n",*clientlist);
	#endif
	(*clientlist)->head=(REMOTEPLAY_CLIENT_Ctrl*)malloc(sizeof(REMOTEPLAY_CLIENT_Ctrl));
	if((*clientlist)->head==NULL){
		#ifdef REMOTEPLAY_DEBUG
        TRACEFLF_REMOTEPLAY("list->head malloc Fail....\n") ;
		#endif
		return -1;
	}
	(*clientlist)->head->next=NULL;
	(*clientlist)->count=0;
	(*clientlist)->cur=(*clientlist)->head;
	#ifdef REMOTEPLAY_DEBUG
	TRACE_REMOTEPLAY("RMPClientList_Init 3 clientlist=%d!!\n",*clientlist);
	#endif
	return 0;
}
int RMPClientList_Insert(REMOTEPLAY_CLIENT_List * clientlist,REMOTEPLAY_CLIENT_Ctrl * node){
	#ifdef REMOTEPLAY_DEBUG
	TRACE_REMOTEPLAY("RMPClientList_Insert 0!!clientlist=%d\n",clientlist);
	#endif
	clientlist->count++;
	#ifdef REMOTEPLAY_DEBUG
	TRACE_REMOTEPLAY("RMPClientList_Insert 1!!\n");
	#endif
	clientlist->cur->next=node;
	#ifdef REMOTEPLAY_DEBUG
	TRACE_REMOTEPLAY("RMPClientList_Insert 2!!\n");
	#endif
	clientlist->cur=node;
	#ifdef REMOTEPLAY_DEBUG
	TRACE_REMOTEPLAY("RMPClientList_Insert 3!!\n");
	#endif
	return 0;
}
int RMPClientList_Delete(REMOTEPLAY_CLIENT_List * clientlist,REMOTEPLAY_CLIENT_Ctrl * node){
	int i;
	if(clientlist->count){
		REMOTEPLAY_CLIENT_Ctrl* del;
		REMOTEPLAY_CLIENT_Ctrl* temp=clientlist->head;
		while(temp->next&&temp->next->client_id!=node->client_id)
			temp=temp->next;
		if(temp->next){
			del=temp->next;
			temp->next=del->next;
			free(del);
			clientlist->count--;
		}
		return 0;	
	}
	return 0;
}
REMOTEPLAY_CLIENT_Ctrl* RMPClientList_Search(REMOTEPLAY_CLIENT_List * clientlist,unsigned char* inferFIFIName){
	REMOTEPLAY_CLIENT_Ctrl* temp=clientlist->head;
	while(temp->next&&strcmp(temp->next->inferFIFOName,inferFIFIName))
		temp=temp->next;
	return temp->next;
}
REMOTEPLAY_CLIENT_Ctrl* RMPClientList_Search_byId(REMOTEPLAY_CLIENT_List * clientlist,pthread_t cur_pthreadId){
	REMOTEPLAY_CLIENT_Ctrl* temp=clientlist->head;
	while(temp->next&&temp->next->client_id==cur_pthreadId)
		temp=temp->next;
	return temp->next;
}

int RMPClientList_Destroy(REMOTEPLAY_CLIENT_List * clientlist){
	REMOTEPLAY_CLIENT_Ctrl* temp=clientlist->head->next;
	while(temp){
		clientlist->head->next=temp->next;
		free(temp);
		clientlist->count--;
		temp=clientlist->head->next;
	}
	return 0;		
}
void REMOTEPLAY_SIGPIPEProcess(int signum){
	pthread_t 	  cur_pthreadId = pthread_self();
	#ifdef REMOTEPLAY_DEBUG
	TRACE_REMOTEPLAY("REMOTEPLAY_SIGPIPEProcess cur_pthreadId=%d!!\n",cur_pthreadId);
	#endif
	REMOTEPLAY_CLIENT_Ctrl *clientNode=RMPClientList_Search_byId(gRMPClientList,cur_pthreadId);
	#ifdef REMOTEPLAY_DEBUG
	TRACE_REMOTEPLAY("REMOTEPLAY_SIGPIPEProcess cur_pthreadId=%d!!\n",cur_pthreadId);
	#endif
	//fclose(clientNode->remoteplay_ctrl.fp);
	#ifdef REMOTEPLAY_DEBUG
	TRACE_REMOTEPLAY("REMOTEPLAY_SIGPIPEProcess cur_pthreadId=%d!!\n",cur_pthreadId);
	#endif
	//unlink(clientNode->inferFIFOName);
	RMPClientList_Delete(gRMPClientList,clientNode);
	
	#ifdef REMOTEPLAY_DEBUG
	TRACE_REMOTEPLAY("REMOTEPLAY_SIGPIPEProcess cur_pthreadId=%d!!\n",cur_pthreadId);
	#endif
	pthread_exit(cur_pthreadId);
	clientNode->remoteplay_ctrl.stop=TRUE;
	sleep(5);
	return ;
}

int REMOTEPLAY_tskCreate()
{
  int status ;
  int i;
 
  status = OSA_mutexCreate(&gREMOTEPLAY_ctrl.mutexLock);
  if(status!=OSA_SOK) {
    OSA_ERROR("OSA_mutexCreate()\n");
  }
  
  return OSA_SOK;
  
}

int REMOTEPLAY_tskDelete()
{
  int status=0;
  int i;
  if(gRMPClientList)
  	RMPClientList_Destroy(gRMPClientList);
  OSA_mutexDelete(&gREMOTEPLAY_ctrl.mutexLock);
 
  return status;
}


int REMOTEPLAY_tskRun(REMOTEPLAY_CLIENT_Ctrl* RMPClient_Ctrl)
{
	struct sigaction sigAction;
	/* insure a clean shutdown if user types ctrl-c or close by exception*/
	sigAction.sa_handler = REMOTEPLAY_SIGPIPEProcess;
	sigemptyset(&sigAction.sa_mask);
	sigAction.sa_flags = 0;
	sigaction(SIGPIPE, &sigAction, NULL);	

	

	//pthread_detach(pthread_self());
	#ifdef REMOTEPLAY_DEBUG
	TRACE_REMOTEPLAY("REMOTEPLAY_tskRun thread begin pid=%d!!\n",RMPClient_Ctrl->client_id);
	#endif
    int status=OSA_SOK, channel, resolution, retval, hour, min, sec, pause = 0, i = 0, stop = 0, sendlen = 0, done = 0 ;
    unsigned long rectime, RecordedTimestamp[8] = {0,0,0,0,0,0,0,0}, RecordedOldTimestamp[8] = {0,0,0,0,0,0,0,0},RecordedTimegap = 0,Timestamp[8] = {0,0,0,0,0,0,0,0} , OldTimestamp[8] = {0,0,0,0,0,0,0,0}, Timegap = 0, gap = 0;
    char vbuffer[1024*100], basketOC[8] ;
    unsigned short abuffer[1024*2] ; 
    struct tm tm1;
    struct timeval tv ;

    T_BKTRMP_PARAM dec_param;

    memset(&dec_param, 0, sizeof(dec_param));

    rectime = RMPClient_Ctrl->remoteplay_ctrl.rectime ;
    
    channel = RMPClient_Ctrl->remoteplay_ctrl.channel ;

    T_BKTRMP_CTRL rmpCtrl;
    strcpy(rmpCtrl.target_path, "/dvr/data/sdda");

    dec_param.vbuffer = vbuffer;
    dec_param.abuffer = abuffer;
    dec_param.ch = channel ;
	
	if(access(RMPClient_Ctrl->inferFIFOName, F_OK)==  -1)  
	{  
		umask(0);	  
		if (mkfifo(RMPClient_Ctrl->inferFIFOName, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP))
		{
			perror(RMPClient_Ctrl->inferFIFOName);
			exit(1);
		}	
	}  
	/*打开有名管道*/
	if((RMPClient_Ctrl->remoteplay_ctrl.fp = fopen(RMPClient_Ctrl->inferFIFOName,"wb")) == NULL) {
		perror("open");
		exit(1);
	}
	#ifdef REMOTEPLAY_DEBUG
			TRACE_REMOTEPLAY("BKTRMP_Open Begin!!\n");
	#endif
    if( BKT_ERR == BKTRMP_Open(&rmpCtrl, rectime))
    {
#ifdef REMOTEPLAY_DEBUG
        TRACEFLF_REMOTEPLAY("BASKET OPEN FAIL....\n") ;
#endif
        RMPClient_Ctrl->remoteplay_ctrl.runflag = 0 ;
        RMPClient_Ctrl->remoteplay_ctrl.stop = FALSE ;
        return OSA_EFAIL;
    }
	#ifdef REMOTEPLAY_DEBUG
			TRACE_REMOTEPLAY("BKTRMP_Open Done!!\n");
	#endif
	
    while(!done)
    {
        OSA_mutexLock(&RMPClient_Ctrl->remoteplay_ctrl.mutexLock);
        pause = RMPClient_Ctrl->remoteplay_ctrl.pause ;
        stop =  RMPClient_Ctrl->remoteplay_ctrl.stop ;
        OSA_mutexUnlock(&RMPClient_Ctrl->remoteplay_ctrl.mutexLock);

        if(stop == TRUE)
        {
        	#ifdef REMOTEPLAY_DEBUG
			TRACE_REMOTEPLAY("stop msg received!!!!!\n");
			#endif
            RMPClient_Ctrl->remoteplay_ctrl.stop = FALSE ;
            RMPClient_Ctrl->remoteplay_ctrl.runflag = 0 ;
            break ;
        }

        if(pause == TRUE)
        {
            usleep(1000) ;
            continue ;
        }
        else
        {  
        	#if 0//def REMOTEPLAY_DEBUG
			TRACE_REMOTEPLAY("BKTRMP_ReadNextFrame Begin!!\n");
			#endif
            long fposcur = BKTRMP_ReadNextFrame(&rmpCtrl, &dec_param);
			#if 0 //def REMOTEPLAY_DEBUG
			TRACE_REMOTEPLAY("BKTRMP_ReadNextFrame Done!!\n");
			#endif
            if(fposcur == BKT_ERR)
            {
              
#ifdef REMOTEPLAY_DEBUG
                TRACEFLF_REMOTEPLAY("Remote Playback end........\n") ;
#endif
                RMPClient_Ctrl->remoteplay_ctrl.runflag = 0 ;
                RMPClient_Ctrl->remoteplay_ctrl.stop = FALSE ;
 
                status = -1 ;
                done = 1 ;
            }
            if(dec_param.streamtype == ST_VIDEO)
            {
               	if(dec_param.frameWidth == 704)
               	{
                    if(dec_param.frameHeight == 480)
                    {
                      	resolution = NTSC720480 ;
                    }
                    else
              	    {
                      	resolution = PAL720576 ;
              	    }
              	}
                else
               	{
              	    if(dec_param.frameHeight == 240)
                    {
                        resolution =  NTSC352240 ;
                    } 
                    else
              	    {
                        resolution = PAL360288 ;
                    }
                }

                
               	localtime_r(&dec_param.ts.sec, &tm1);

                RecordedTimestamp[dec_param.ch] = ((tm1.tm_hour*3600 + tm1.tm_min*60 + tm1.tm_sec)*1000 + dec_param.ts.usec/1000) ;

//                if(RecordedOldTimestamp[dec_param.ch] != 0 && dec_param.streamtype == ST_VIDEO)
                if(RecordedOldTimestamp[dec_param.ch] != 0)
                    RecordedTimegap = RecordedTimestamp[dec_param.ch] - RecordedOldTimestamp[dec_param.ch] ;

                RecordedOldTimestamp[dec_param.ch] = RecordedTimestamp[dec_param.ch] ;

                gettimeofday(&tv, NULL) ;
                Timestamp[dec_param.ch] = tv.tv_sec*1000 + tv.tv_usec/1000 ;
                if(OldTimestamp[dec_param.ch] != 0)
                    Timegap = Timestamp[dec_param.ch] - OldTimestamp[dec_param.ch] ;

                OldTimestamp[dec_param.ch] = Timestamp[dec_param.ch] ;
                   
                gap = RecordedTimegap - Timegap ;  
                if(gap < 0 || gap > 2000)
                   gap = 0 ;
//fprintf(stderr, "Timegap = %d, Recorded Timegap = %d\n",Timegap, RecordedTimegap) ;
                usleep(gap*1000) ;               
				
				if(channel==dec_param.ch)//add by sxh
                	retval = RemotePlayback(dec_param.ch, dec_param.streamtype, dec_param.frametype, vbuffer, dec_param.framesize, Timestamp[dec_param.ch], resolution,RMPClient_Ctrl) ;
            }
            else
            {
            	
            	if(channel==dec_param.ch)//add by sxh
                	retval = RemotePlayback(dec_param.ch, dec_param.streamtype, 0, abuffer, dec_param.framesize, Timestamp[dec_param.ch], 0,RMPClient_Ctrl) ;
            }
            if(retval == -1)
            {
                status = retval ;
                break ;
            }
        }

    }
	close(RMPClient_Ctrl->remoteplay_ctrl.fp);
	unlink(RMPClient_Ctrl->inferFIFOName);
    return status;
}


int RemotePlayback(int channel, int streamtype, int vtype, void *buffer, int framesize, unsigned long Timestamp, int resolution,REMOTEPLAY_CLIENT_Ctrl* RMPClient_Ctrl) 
{
   	int retval;
	FILE* fp;
	int n;
	char* tmpBuf=buffer;
	int ret;
	if(streamtype == ST_VIDEO){
		switch(channel){
			case 0:	
				/*
				while(framesize>2000){
					while(1){      
						ret=ioctl(RMPClient_Ctrl->remoteplay_ctrl.fp, FIONREAD, &n);
						#ifdef REMOTEPLAY_DEBUG
						TRACE_REMOTEPLAY("ioctl(RMPClient_Ctrl->remoteplay_ctrl.fp, FIONREAD, &n) ret=%d,n=%d!!\n",ret,n);
						#endif
				        if(n>=2000)     
							break;
  					}
					write(RMPClient_Ctrl->remoteplay_ctrl.fp,tmpBuf,2000);
					framesize-=2000;
					tmpBuf+=2000;
				}
				while(1){      
					ioctl(RMPClient_Ctrl->remoteplay_ctrl.fp, FIONREAD, &n);
			        if(n>=framesize)     
						break;
  				}*/
				//printf("buffer addr=%x\n",buffer);
				ret=fwrite(buffer,1,framesize,RMPClient_Ctrl->remoteplay_ctrl.fp);
				//printf("write channel 0!!!!framesize=%d,RMPClient_Ctrl->inferFIFOName=%s ret=%d\n",framesize,RMPClient_Ctrl->inferFIFOName,ret);
				break;
			case 1:
				ret=fwrite(buffer,1,framesize,RMPClient_Ctrl->remoteplay_ctrl.fp);
				//printf("write channel 1!!!!framesize=%d,RMPClient_Ctrl->inferFIFOName=%s\n ret=%d",framesize,RMPClient_Ctrl->inferFIFOName,ret);
				break;
			case 2:
				ret=fwrite(buffer,1,framesize,RMPClient_Ctrl->remoteplay_ctrl.fp);
				break;
			case 3:
				ret=fwrite(buffer,1,framesize,RMPClient_Ctrl->remoteplay_ctrl.fp);
				break;
		}
	}
    return retval ;
}
 


int REMOTEPLAY_tskMain(struct OSA_TskHndl *pTsk, OSA_MsgHndl *pMsg, Uint32 curState )
{
	int status;
	Bool done = FALSE, ackMsg=FALSE;;
	Uint16 cmd = OSA_msgGetCmd(pMsg);
	REMOTEPLAY_CLIENT_Ctrl *CurClient;
	
	struct sched_param schedParam;
	pthread_attr_t     attr;
	if (pthread_attr_init(&attr)) {
	    status = FALSE ;
	}
	/* Force the thread to use custom scheduling attributes */
	if (pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED)) {
	    status = FALSE ;
	}

	/* Set the thread to be fifo real time scheduled */
	if (pthread_attr_setschedpolicy(&attr, SCHED_FIFO)) {
	    status = FALSE ;
	}

	schedParam.sched_priority = sched_get_priority_max(SCHED_FIFO) - 20;
	if (pthread_attr_setschedparam(&attr, &schedParam))
	{
	    status = FALSE ;
	}
	if( cmd != REMOTEPLAY_CMD_CREATE) {
		OSA_tskAckOrFreeMsg(pMsg, OSA_EFAIL);
		return OSA_SOK;
	}
	status = REMOTEPLAY_tskCreate();
	
	OSA_tskAckOrFreeMsg(pMsg, status);  
	
	if(status != OSA_SOK)
		return OSA_SOK;
	
	while(!done)
	{
		status = OSA_tskWaitMsg(pTsk, &pMsg);
		#ifdef REMOTEPLAY_DEBUG
		TRACE_REMOTEPLAY("OSA_tskWaitMsg Success!!\n");
		#endif
		CurClient=(REMOTEPLAY_CLIENT_Ctrl*)OSA_msgGetPrm(pMsg);
		if(status != OSA_SOK) {
			done = TRUE;
			break;
		}
		
		cmd = OSA_msgGetCmd(pMsg);  
		#ifdef REMOTEPLAY_DEBUG
		TRACE_REMOTEPLAY("OSA_tskWaitMsg Success!! cmd=%d\n",cmd);
		#endif
		switch(cmd) {

		case REMOTEPLAY_CMD_RUN:
			
			OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);      

			#ifdef REMOTEPLAY_DEBUG
			TRACE_REMOTEPLAY("REMOTEPLAY_CMD_RUN!!!\n");
			#endif

			if(pthread_create(&CurClient->client_id, NULL, (void*)REMOTEPLAY_tskRun,(void*)CurClient))
		    {
				#ifdef REMOTEPLAY_DEBUG
				TRACE_REMOTEPLAY("RMPClientList_Init Failed!!\n");
				#endif
				done = TRUE;        
				ackMsg = TRUE;
		    }
			#ifdef REMOTEPLAY_DEBUG
				TRACE_REMOTEPLAY("pthread_create Done!!\n");
			
			#endif
			break;
			
		case REMOTEPLAY_CMD_DELETE:
			done = TRUE;        
			ackMsg = TRUE;
			break;   
			
		default:   
			
			OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
			
			break;              
		}
	}
	
	REMOTEPLAY_tskDelete();
	if(ackMsg)
		OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);          
	
	return OSA_SOK;
}





int REMOTEPLAY_sendCmd(Uint16 cmd, void *prm, Uint16 flags )
{
	#ifdef REMOTEPLAY_DEBUG
	TRACE_REMOTEPLAY("REMOTEPLAY_sendCmd Begin!! cmd=%d\n",cmd);
	#endif
  return OSA_mbxSendMsg(&gREMOTEPLAY_ctrl.tskHndl.mbxHndl, &gREMOTEPLAY_ctrl.mbxHndl, cmd, prm, flags);
}

int REMOTEPLAY_create()
{
	int status;
	int i;

	memset(&gREMOTEPLAY_ctrl, 0, sizeof(gREMOTEPLAY_ctrl));

	status = OSA_tskCreate( &gREMOTEPLAY_ctrl.tskHndl, REMOTEPLAY_tskMain, REMOTEPLAY_THR_PRI, REMOTEPLAY_STACK_SIZE, 0);

	OSA_assertSuccess(status);

	status = OSA_mbxCreate( &gREMOTEPLAY_ctrl.mbxHndl);  

	OSA_assertSuccess(status);  

	status = REMOTEPLAY_sendCmd(REMOTEPLAY_CMD_CREATE, NULL, OSA_MBX_WAIT_ACK );
  	return status;  
}

int REMOTEPLAY_delete()
{
  int status;
  int i;
  char prm[12];
  bzero(prm,12);
 
  status = REMOTEPLAY_sendCmd(REMOTEPLAY_CMD_DELETE,NULL, OSA_MBX_WAIT_ACK );
  
  status = OSA_tskDelete( &gREMOTEPLAY_ctrl.tskHndl );

  OSA_assertSuccess(status);
    
  status = OSA_mbxDelete( &gREMOTEPLAY_ctrl.mbxHndl);  
  
  OSA_assertSuccess(status); 	 
  return status;
}

int REMOTEPLAY_start(int channel, unsigned long rectime,unsigned char* inferFIFOName)
{
    int disable = FALSE ;
    int runflag = 0;

	REMOTEPLAY_CLIENT_Ctrl *clientNode=(REMOTEPLAY_CLIENT_Ctrl *)malloc(sizeof(REMOTEPLAY_CLIENT_Ctrl));
	if(gRMPClientList==NULL){
		if(RMPClientList_Init(&gRMPClientList)<0){
			#ifdef REMOTEPLAY_DEBUG
			TRACE_REMOTEPLAY("RMPClientList_Init Failed!!\n");
			#endif
			return -1;
		}
		#ifdef REMOTEPLAY_DEBUG
		TRACE_REMOTEPLAY("REMOTEPLAY_start:gRMPClientList=%d!!\n",gRMPClientList);
		#endif	
	}
	clientNode->inferFIFOName=(unsigned char*)malloc(strlen(inferFIFOName)+1);
	bzero(clientNode->inferFIFOName,strlen(inferFIFOName)+1);
	sprintf(clientNode->inferFIFOName,inferFIFOName,strlen(inferFIFOName));
	clientNode->remoteplay_ctrl.runflag =0;
	clientNode->next=NULL;
	#ifdef REMOTEPLAY_DEBUG
	TRACE_REMOTEPLAY("REMOTEPLAY_start:clientNode->inferFIFOName=%s,gRMPClientList=%d!!\n",clientNode->inferFIFOName,gRMPClientList);
	#endif	
	OSA_mutexLock(&gREMOTEPLAY_ctrl.mutexLock);
	RMPClientList_Insert(gRMPClientList,clientNode);
    OSA_mutexUnlock(&gREMOTEPLAY_ctrl.mutexLock);
	#ifdef REMOTEPLAY_DEBUG
	TRACE_REMOTEPLAY("REMOTEPLAY_start:RMPClientList_Insert Done!!\n");
	#endif	
    if(clientNode->remoteplay_ctrl.runflag == 0)
    {
        //OSA_mutexLock(&gREMOTEPLAY_ctrl.mutexLock);
        clientNode->remoteplay_ctrl.channel = channel ;
        clientNode->remoteplay_ctrl.rectime = rectime ;
        clientNode->remoteplay_ctrl.runflag = 1 ;
        clientNode->remoteplay_ctrl.stop = disable ;
       // OSA_mutexUnlock(&gREMOTEPLAY_ctrl.mutexLock);
        #ifdef REMOTEPLAY_DEBUG
		TRACE_REMOTEPLAY("REMOTEPLAY_start:REMOTEPLAY_sendCmd Begin!!\n");
		#endif
        return REMOTEPLAY_sendCmd(REMOTEPLAY_CMD_RUN,(void*)clientNode, 0);
    }
    else
    {
        return 0 ;
    }

}

int PLAYBACK_pauseCh(int channel,unsigned char* inferFIFOName)
{
    int enable = TRUE, runflag = 0;
	#ifdef REMOTEPLAY_DEBUG
	TRACE_REMOTEPLAY("PLAYBACK_pauseCh:Begin!!\n");
	#endif
  	REMOTEPLAY_CLIENT_Ctrl *clientNode=RMPClientList_Search(gRMPClientList,inferFIFOName);  
    if(clientNode->remoteplay_ctrl.runflag == 1)
    {
        //OSA_mutexLock(&gREMOTEPLAY_ctrl.mutexLock);
        clientNode->remoteplay_ctrl.pause = enable ;
        //OSA_mutexUnlock(&gREMOTEPLAY_ctrl.mutexLock);
    }
    return OSA_SOK;
}

int PLAYBACK_streamCh(int channel,unsigned char* inferFIFOName)
{
    int disable = FALSE ;
	#ifdef REMOTEPLAY_DEBUG
	TRACE_REMOTEPLAY("PLAYBACK_streamCh:Begin!!\n");
	#endif
    REMOTEPLAY_CLIENT_Ctrl *clientNode=RMPClientList_Search(gRMPClientList,inferFIFOName);
    OSA_mutexLock(&gREMOTEPLAY_ctrl.mutexLock);
    clientNode->remoteplay_ctrl.pause = disable ;
    OSA_mutexUnlock(&gREMOTEPLAY_ctrl.mutexLock);

    return OSA_SOK;
}

int PLAYBACK_stopCh(int channel,unsigned char* inferFIFOName)
{
    int enable = TRUE, runflag = 0 ;
 
    REMOTEPLAY_CLIENT_Ctrl *clientNode=RMPClientList_Search(gRMPClientList,inferFIFOName);
   
    if(clientNode->remoteplay_ctrl.runflag == 1)
    {
        //OSA_mutexLock(&gREMOTEPLAY_ctrl.mutexLock);
        clientNode->remoteplay_ctrl.stop = enable ;
        clientNode->remoteplay_ctrl.runflag = 0;
        //OSA_mutexUnlock(&gREMOTEPLAY_ctrl.mutexLock);
    }
    return OSA_SOK ;
}

int PLAYBACK_status(int channel,unsigned char* inferFIFOName)
{
    int runflag = 0;
	REMOTEPLAY_CLIENT_Ctrl *clientNode=RMPClientList_Search(gRMPClientList,inferFIFOName);

    return clientNode->remoteplay_ctrl.runflag ;
}


	
