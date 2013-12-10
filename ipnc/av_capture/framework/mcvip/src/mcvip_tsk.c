
#include <mcvip_priv.h>
#include <osa_file.h>


TVP5158PATCH Tvp5158patch ;

int MCVIP_tskCreate( MCVIP_Hndl *hndl)
{
  int status, i, chId, numBufPerCh;
      
  if(hndl->createPrm.numBuf > MCVIP_BUF_MAX)
    return OSA_EFAIL;
#ifdef VANLINK_DVR_DM365_DEBUG
  OSA_printf(" MCVIP_tskCreate: Begin!!!!\n");
#endif

  hndl->chList.numCh = MCVIP_getNumCh(hndl->createPrm.videoDecoderMode);

#if DEBUG==0
printf("hndl->chList.numCh=%d\n",hndl->chList.numCh);
#endif
  
  if(hndl->createPrm.numBuf < hndl->chList.numCh*(MCVIP_BUF_PER_CH_MIN - 1) ) {
    OSA_ERROR("Insufficient number of buffers allocated by user, need atleast %d buffers\n", hndl->chList.numCh*2);
    return OSA_EFAIL;
  }
    
  hndl->saveFrame = FALSE;
  hndl->saveFileIndex = 0;
  
  for(i=0; i<MCVIP_TVP5158_MAX_CASCADE; i++) {
    if(gMCVIP_ctrl.i2cHndl[hndl->createPrm.videoInputPort][i].fd >=0 ) {
      hndl->pI2cHndl[i] = &gMCVIP_ctrl.i2cHndl[hndl->createPrm.videoInputPort][i];
    } else {
      hndl->pI2cHndl[i] = NULL;
    }
  }
 #ifdef VANLINK_DVR_DM365_DEBUG
  OSA_printf(" MCVIP_tskCreate: MCVIP_v4l2Create!!!!\n");
#endif
  status = MCVIP_v4l2Create(hndl);  
  if(status!=OSA_SOK) {
    return status;
  }
 #ifdef VANLINK_DVR_DM365_DEBUG
	OSA_printf(" MCVIP_tskCreate: DRV_dmaOpen!!!!\n");
#endif

  status = DRV_dmaOpen(&hndl->dmaDemuxHndl, DRV_DMA_MODE_DEMUX, DRV_DMA_MAX_DEMUX_TRANSFERS);
  if(status!=OSA_SOK)
    return status;
  
  for(i=0; i<hndl->chList.numCh; i++)
  {
    MCVIP_getChInfo(hndl->createPrm.videoDecoderMode, 
                    hndl->createPrm.videoSystem, 
                   &hndl->chList.info[i]);
  }
  
  if(     hndl->createPrm.videoDecoderMode==MCVIP_VIDEO_DECODER_MODE_4CH_HALF_D1_PLUS_D1
      ||  hndl->createPrm.videoDecoderMode==MCVIP_VIDEO_DECODER_MODE_4CH_CIF_PLUS_D1
      ||  hndl->createPrm.videoDecoderMode==MCVIP_VIDEO_DECODER_MODE_8CH_CIF_PLUS_D1
    ) 
  {
    hndl->chList.info[hndl->chList.numCh-1].width = 720;
    
    if(hndl->createPrm.videoSystem==MCVIP_VIDEO_SYSTEM_NTSC)
      hndl->chList.info[hndl->chList.numCh-1].height = 480;
    else
      hndl->chList.info[hndl->chList.numCh-1].height = 576;      
	
	hndl->chList.info[hndl->chList.numCh-1].offsetH = OSA_align(hndl->chList.info[hndl->chList.numCh-1].width, 32);	   
	hndl->chList.info[hndl->chList.numCh-1].offsetV = hndl->chList.info[hndl->chList.numCh-1].height;
  }

  status = OSA_queCreate(&hndl->fullQue, hndl->createPrm.numBuf);
  OSA_assertSuccess(status);

  numBufPerCh = hndl->createPrm.numBuf/hndl->chList.numCh;

  for(chId=0; chId<hndl->chList.numCh; chId++) {    
    status = OSA_queCreate(&hndl->emptyQue[chId], numBufPerCh);
    OSA_assertSuccess(status);  
  }
  
  status = OSA_mutexCreate(&hndl->putBufLock);
  OSA_assertSuccess(status);  

  chId = 0;
  for(i=0; i<hndl->createPrm.numBuf; i++) {
    hndl->bufInfo[i].flags = 0;
    hndl->bufInfo[i].chId = -1;
    hndl->bufInfo[i].physAddr = hndl->createPrm.bufPhysAddr[i];
    hndl->bufInfo[i].virtAddr = hndl->createPrm.bufVirtAddr[i];    
    hndl->bufInfo[i].timestamp = 0;

    if(i>0) {
      if((i%numBufPerCh)==0)
        chId++;
    }
        
    status = OSA_quePut(&hndl->emptyQue[chId], i, OSA_TIMEOUT_FOREVER);
    OSA_assertSuccess(status);      
  
    #ifdef MCVIP_CLEAR_BUF_DURING_INIT    
    {
      MCVIP_DmaPrm dmaPrm;    
      
      // clear buffer with blank data
      dmaPrm.srcPhysAddr = NULL;
      dmaPrm.srcVirtAddr = NULL;    
      dmaPrm.dstPhysAddr = hndl->createPrm.bufPhysAddr[i];
      dmaPrm.dstVirtAddr = hndl->createPrm.bufVirtAddr[i];
      dmaPrm.srcOffsetH  = 0;   
      dmaPrm.srcOffsetV  = 0;  
      dmaPrm.dstOffsetH  = hndl->chList.info[chId].offsetH;   
      dmaPrm.dstOffsetV  = hndl->chList.info[chId].offsetV;   
      dmaPrm.copyWidth   = hndl->chList.info[chId].width  ;   
      dmaPrm.copyHeight  = hndl->chList.info[chId].height ;   
      dmaPrm.fillValueY  = 0x00800080;
      dmaPrm.fillValueC  = 0x00000000;    
      
      MCVIP_dmaRun(&dmaPrm);
    }
    #endif
  }  
 #ifdef VANLINK_DVR_DM365_DEBUG
	  OSA_printf(" MCVIP_tskCreate: MCVIP_demuxInit!!!!\n");
#endif

  MCVIP_demuxInit(hndl);

  pthread_mutex_lock(&Tvp5158patch.mutex) ;
  Tvp5158patch.status  = TRUE ;
  pthread_mutex_unlock(&Tvp5158patch.mutex) ;

  return OSA_SOK;
}

int MCVIP_tskDelete(MCVIP_Hndl *hndl )
{
  int status, i;
  
  OSA_printf(" MCVIP_tskDelete :  Begin! ...\n");

  status = MCVIP_v4l2Delete(hndl);  
  OSA_assertSuccess(status);
 OSA_printf(" MCVIP_v4l2Delete :  Done! ...\n");
  status = OSA_queDelete(&hndl->fullQue);
  OSA_assertSuccess(status);
    
  for(i=0; i<hndl->chList.numCh; i++) {    
    status = OSA_queDelete(&hndl->emptyQue[i]);
    OSA_assertSuccess(status);  
  }
    
  status = OSA_mutexDelete(&hndl->putBufLock);
  OSA_assertSuccess(status);  
  OSA_printf(" DRV_dmaClose :  Begin! ...\n");
  DRV_dmaClose(&hndl->dmaDemuxHndl);
    OSA_printf(" DRV_dmaClose :  Done! ...\n");     
  return status;
}

int MCVIP_tskStart(MCVIP_Hndl *hndl,int sampleRate)
{
  int status;
  
  status = MCVIP_v4l2Start(hndl,sampleRate);
  
  return status;
}

int MCVIP_tskStop(MCVIP_Hndl *hndl)
{
  int status;
  
  status = MCVIP_v4l2Stop(hndl);
  
  return status;
}

int MCVIP_tskSaveFrame( MCVIP_Hndl *hndl, MCVIP_V4l2Buf *pBufInfo)
{
  char filename[20];
  Uint32 fileSize;
  int status;
  
  if(!hndl->saveFrame)
    return 0;
  
  hndl->saveFrame = FALSE;
  
  sprintf(filename, "MCV_%04d.BIN", hndl->saveFileIndex);
  
  fileSize = hndl->v4l2FrameInfo.offsetH * hndl->v4l2FrameInfo.offsetV * 2;
  
  OSA_printf("MCVIP: Saving frame of size %dx%d\n", hndl->v4l2FrameInfo.offsetH, hndl->v4l2FrameInfo.offsetV);
  
  status = OSA_fileWriteFile(filename, pBufInfo->virtAddr, fileSize);

  hndl->saveFileIndex++;
  
  return status;
}


int MCVIP_tskStartRun(MCVIP_Hndl *hndl, OSA_TskHndl *pTsk, OSA_MsgHndl *pMsg, Bool *isDone, Bool *doAckMsg )
{
  int status,sampleRate;
  Bool done = FALSE, ackMsg = FALSE;
  Uint16 cmd;
  MCVIP_V4l2Buf v4l2Buf;

  *isDone = FALSE;
  *doAckMsg = FALSE;
  
  sampleRate = *(int*)OSA_msgGetPrm(pMsg);
  status = MCVIP_tskStart(hndl,sampleRate);
      #ifdef VANLINK_DVR_DM365_DEBUG
    OSA_printf(" ISIF:-----------vl MCVIP_tskStart Done!!! \n");
    #endif   
  OSA_tskAckOrFreeMsg(pMsg, status);  
  
  if(status != OSA_SOK) {
    return status;
  }
  
  OSA_prfReset(&hndl->prfCapture);
  OSA_prfReset(&hndl->prfDemux);
  OSA_prfReset(&hndl->prfDma);
  OSA_prfReset(&hndl->prfQueWait);      
  
  hndl->frameCount = 0;
  hndl->missedFrameCount = 0;

  while(!done) {
    
    OSA_prfBegin(&hndl->prfCapture);
    #ifdef VANLINK_DVR_DM365_DEBUG
   // OSA_printf(" ISIF: Get Buf (%d)\n", OSA_getCurTimeInMsec());
    #endif 
    status = MCVIP_v4l2GetBuf(hndl, &v4l2Buf);
    
    if(status==OSA_SOK) {
    #ifdef VANLINK_DVR_DM365_DEBUG 
   // OSA_printf(" ISIF: Buf %d 0x%08x\n", v4l2Buf.id, (Uint32)v4l2Buf.physAddr);
    #endif
      OSA_prfBegin(&hndl->prfDemux);
      
      status = MCVIP_demuxRun(hndl, &v4l2Buf);
      
      OSA_prfEnd(&hndl->prfDemux, 1);    
      
      MCVIP_tskSaveFrame(hndl, &v4l2Buf);
      
      MCVIP_v4l2PutBuf(hndl, &v4l2Buf);
      
      OSA_prfEnd(&hndl->prfCapture, 1);    

      if(hndl->frameCount%(30*20)==0) {
        #ifdef MCVIP_PRF_PRINT 
        OSA_printf(" Avg Time (msecs) for Capture: %4.1f, Demux: %4.1f, DMA: %4.1f (%d frames)\n", 
          (float)hndl->prfCapture.totalTime/hndl->prfCapture.count,
          (float)(hndl->prfDemux.totalTime/*-hndl->prfQueWait.totalTime*/)/hndl->prfDemux.count,
          (float)(hndl->prfDma.totalTime)/hndl->prfDma.count,
          hndl->prfCapture.count
          );   
        #endif 
        
        OSA_prfReset(&hndl->prfCapture);
        OSA_prfReset(&hndl->prfDemux);
        OSA_prfReset(&hndl->prfDma);
        OSA_prfReset(&hndl->prfQueWait); 
      }

     if(hndl->frameCount%(30*5)==0 ) {
        int i;
         
        if(hndl->missedFrameCount>5) {
          OSA_printf(" MCVIP: Missed frame threshold reached, trying to recover !!!\n");
          for(i=0; i<MCVIP_V4L2_INPUT_BUF_MAX*2; i++) {
            status = MCVIP_v4l2GetBuf(hndl, &v4l2Buf);
            if(status==OSA_SOK) {
              MCVIP_v4l2PutBuf(hndl, &v4l2Buf);
            }
          }
        }
        hndl->missedFrameCount = 0;                          
     }

      hndl->frameCount++; 
      
      if(status!=OSA_SOK)
        break;   
      
    }        
    
    status = OSA_tskCheckMsg(pTsk, &pMsg);
    if(status==OSA_SOK) {
    
      cmd = OSA_msgGetCmd(pMsg);  
    
      switch(cmd) {
        case MCVIP_CMD_STOP:
          done = TRUE;
          ackMsg = TRUE;
          break;
          
        case MCVIP_CMD_DELETE:
          done = TRUE;
          *isDone = TRUE;
          *doAckMsg = TRUE;
          break;   
                     
        default:   
          OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
          break;              
      }    
    }
  }

  MCVIP_tskStop(hndl); 
  
  OSA_quePut(&hndl->fullQue, 0xFF, OSA_TIMEOUT_FOREVER);         
  
  if(ackMsg)
    OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);          
  
  return status;
}

int MCVIP_tskMain(struct OSA_TskHndl *pTsk, OSA_MsgHndl *pMsg, Uint32 curState )
{
  int status;
  Bool done = FALSE, ackMsg=FALSE;;
  Uint16 cmd = OSA_msgGetCmd(pMsg);
  MCVIP_Hndl *pHndl = (MCVIP_Hndl*)OSA_msgGetPrm(pMsg);
  
  if( cmd != MCVIP_CMD_CREATE || pHndl==NULL) {
    OSA_tskAckOrFreeMsg(pMsg, OSA_EFAIL);
    return OSA_SOK;
  }
  
  status = MCVIP_tskCreate( pHndl );
  
  OSA_tskAckOrFreeMsg(pMsg, status);  
  
  if(status != OSA_SOK)
    return OSA_SOK;
  
  while(!done)
  {
    status = OSA_tskWaitMsg(pTsk, &pMsg);
    if(status != OSA_SOK) {
      done = TRUE;
      break;
    }
      
    cmd = OSA_msgGetCmd(pMsg);  
    
    OSA_printf(" MCVIP_tskMain : cmd=%d ! ...\n",cmd);
    
    switch(cmd) {
      case MCVIP_CMD_START:
        MCVIP_tskStartRun(pHndl, pTsk, pMsg, &done, &ackMsg);      
        break;
        
      case MCVIP_CMD_DELETE:
        done = TRUE;        
        ackMsg = TRUE;
        break;   
                   
      default:   
        OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
        break;              
    }
  }
  OSA_printf(" MCVIP_tskMain : MCVIP_tskDelete Begin! ...\n");

  MCVIP_tskDelete(pHndl);
  if(ackMsg)
    OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);          

  return OSA_SOK;
}

