#include <avserver.h>
#include <basket_rec.h>//add by sxh

typedef struct {

  Uint16 startX;
  Uint16 startY;
  Uint16 width;
  Uint16 height;
  Uint16 confidence;
  Uint16 angle;

} VIDEO_SaveFaceInfo;

typedef struct {

  Uint32 magicNum;
  Uint32 numFaces;
  VIDEO_SaveFaceInfo  faceInfo[16];

} VIDEO_SaveFaceList;

#ifdef VANLINK_DVR_DM365
#define FILE_WRITE_BASE_DIR   "./"

int VIDEO_Stream_sendCmd(Uint16 cmd, void *prm, Uint16 flags )
{
  return OSA_mbxSendMsg(&gVIDEO_Stream_ctrl.tskHndl.mbxHndl, &gVIDEO_Stream_ctrl.mbxHndl, cmd, prm, flags);
}
#endif
int VIDEO_streamSaveFaceInfo(FILE *fileHndl, Uint16 streamWidth, Uint16 streamHeight)
{
  static DRV_FaceDetectRunStatus faceStatus;
  static VIDEO_SaveFaceList saveFaceList;
  
  int i, centerX, centerY, size, writtenBytes;  
  
  faceStatus.numFaces=0;
  VIDEO_fdGetFaceStatus(&faceStatus);

  memset(&saveFaceList, 0, sizeof(saveFaceList));
  
  saveFaceList.magicNum = 0xFACEFACE;
  saveFaceList.numFaces = faceStatus.numFaces;
  
  if(saveFaceList.numFaces>16)
    saveFaceList.numFaces = 16;
      
  for(i=0; i<saveFaceList.numFaces; i++) {

    centerX = (faceStatus.info[i].centerX * streamWidth ) / gVIDEO_ctrl.faceDetectStream.fdWidth;
    centerY = (faceStatus.info[i].centerY * streamHeight ) / gVIDEO_ctrl.faceDetectStream.fdHeight;    
    size    = (CSL_FACE_DETECT_GET_SIZE(faceStatus.info[i].sizeConfidence)*streamWidth ) / streamHeight;  

    saveFaceList.faceInfo[i].startX  = centerX - size/2;
    saveFaceList.faceInfo[i].startY  = centerY - size/2;
    
    saveFaceList.faceInfo[i].width   = size;
    saveFaceList.faceInfo[i].height  = size;
    saveFaceList.faceInfo[i].confidence = CSL_FACE_DETECT_GET_CONFIDENCE(faceStatus.info[i].sizeConfidence);
    saveFaceList.faceInfo[i].angle   = faceStatus.info[i].angle;
  }

  writtenBytes = fwrite( &saveFaceList, 1, sizeof(saveFaceList), fileHndl);
  
  if(writtenBytes!=sizeof(saveFaceList)) {
    return OSA_EFAIL;
  }
  
  return OSA_SOK;
}

int VIDEO_streamFileWrite(int streamId, OSA_BufInfo *pBufInfo)
{
  Bool saveToFile;
  int  fileSaveState;
  int  status=OSA_SOK, writtenBytes;
  VIDEO_BufHeader *pInBufHeader;
  VIDEO_EncodeConfig *pEncodeConfig;
  VIDEO_EncodeStream *pEncodeStream;
  
  static char filename[256];  
  static char faceInfoFile[256];
  
  pEncodeConfig = &gAVSERVER_config.encodeConfig[streamId];
  pEncodeStream = &gVIDEO_ctrl.encodeStream[streamId];
  
  pInBufHeader = (VIDEO_BufHeader *)pBufInfo->virtAddr;

  fileSaveState = pEncodeStream->fileSaveState;
  
  saveToFile = pEncodeConfig->fileSaveEnable;
  
  #ifdef AVSERVER_DEBUG_VIDEO_STREAM_THR
    #ifdef AVSERVER_DEBUG_RUNNING
    OSA_printf(" STREAM: Stream %d, saveToFile = %d, fileSaveState = %d, frame size = %d, key frame = %d\n", 
          streamId, saveToFile, fileSaveState, pInBufHeader->encFrameSize, pInBufHeader->encFrameType);
    #endif
  #endif    
  
  if(saveToFile) {
  
    if(fileSaveState==VIDEO_STREAM_FILE_SAVE_STATE_IDLE)
      fileSaveState=VIDEO_STREAM_FILE_SAVE_STATE_OPEN;
  
  } else {

    if(fileSaveState==VIDEO_STREAM_FILE_SAVE_STATE_OPEN)
      fileSaveState=VIDEO_STREAM_FILE_SAVE_STATE_IDLE;
    else
    if(fileSaveState==VIDEO_STREAM_FILE_SAVE_STATE_WRITE)
      fileSaveState=VIDEO_STREAM_FILE_SAVE_STATE_CLOSE;
  }
  
  if(fileSaveState==VIDEO_STREAM_FILE_SAVE_STATE_OPEN) {
    if(pInBufHeader->encFrameType==VIDEO_ENC_FRAME_TYPE_KEY_FRAME) {

      sprintf(filename, "CH%02d_%04d_%dx%d.bits", 
                streamId, 
                pEncodeStream->fileSaveIndex,                
                pEncodeConfig->cropWidth,
                pEncodeConfig->cropHeight                
            );

      sprintf(faceInfoFile, "CH%02d_%04d_%dx%d.faceInf", 
                streamId, 
                pEncodeStream->fileSaveIndex,                
                pEncodeConfig->cropWidth,
                pEncodeConfig->cropHeight                
            );


      #ifdef AVSERVER_DEBUG_VIDEO_STREAM_THR
      OSA_printf(" STREAM: Opening file stream %d, %s\n", streamId, filename);
      #endif  
      
      pEncodeStream->fileSaveHndl = fopen( filename, "wb");
      pEncodeStream->fileFaceInfoHndl = fopen( faceInfoFile, "wb");  
              
      if(   pEncodeStream->fileSaveHndl == NULL 
         || pEncodeStream->fileFaceInfoHndl == NULL 
        ) { 
        #ifdef AVSERVER_DEBUG_VIDEO_STREAM_THR            
        OSA_ERROR("Stream %d open\n", streamId);    
        #endif         
        fileSaveState = VIDEO_STREAM_FILE_SAVE_STATE_IDLE;
        
        fclose(pEncodeStream->fileSaveHndl);
        fclose(pEncodeStream->fileFaceInfoHndl);
        
      } else {
        fileSaveState = VIDEO_STREAM_FILE_SAVE_STATE_WRITE;
        pEncodeStream->fileSaveIndex++;
      }
    }
  }
  
  if(fileSaveState==VIDEO_STREAM_FILE_SAVE_STATE_WRITE) {  
    if(pInBufHeader->encFrameSize>0) {
    
      #ifdef AVSERVER_DEBUG_VIDEO_STREAM_THR
        #ifdef AVSERVER_DEBUG_RUNNING
        OSA_printf(" STREAM: Stream %d writing %d bytes\n", streamId, pInBufHeader->encFrameSize);
        #endif
      #endif    
      
      writtenBytes = fwrite(pBufInfo->virtAddr + VIDEO_BUF_HEADER_SIZE, 
                      1, 
                      pInBufHeader->encFrameSize, 
                      pEncodeStream->fileSaveHndl
                      );
      if(writtenBytes!=pInBufHeader->encFrameSize) {
        #ifdef AVSERVER_DEBUG_VIDEO_STREAM_THR      
        OSA_ERROR("Stream %d write\n", streamId);
        #endif
        pEncodeConfig->fileSaveEnable = FALSE;
        fileSaveState=VIDEO_STREAM_FILE_SAVE_STATE_CLOSE;
      }
      
      if(fileSaveState==VIDEO_STREAM_FILE_SAVE_STATE_WRITE) {
        status = VIDEO_streamSaveFaceInfo(pEncodeStream->fileFaceInfoHndl, pEncodeConfig->cropWidth, pEncodeConfig->cropHeight);
        
        if(status!=OSA_SOK) {
          #ifdef AVSERVER_DEBUG_VIDEO_STREAM_THR      
          OSA_ERROR("Stream %d write\n", streamId);
          #endif
          pEncodeConfig->fileSaveEnable = FALSE;
          fileSaveState=VIDEO_STREAM_FILE_SAVE_STATE_CLOSE;
        }
      }      
    }
  }
  if(fileSaveState==VIDEO_STREAM_FILE_SAVE_STATE_CLOSE) {   
  
    #ifdef AVSERVER_DEBUG_VIDEO_STREAM_THR
    OSA_printf(" STREAM: Closing file stream %d\n", streamId);
    #endif   
    
    fclose(pEncodeStream->fileSaveHndl);
    fclose(pEncodeStream->fileFaceInfoHndl);    
    
    fileSaveState=VIDEO_STREAM_FILE_SAVE_STATE_IDLE;
    
    #ifdef AVSERVER_DEBUG_VIDEO_STREAM_THR
    OSA_printf(" STREAM: Closing file stream ... DONE\n");
    #endif  
  }
    
  pEncodeStream->fileSaveState = fileSaveState; 

  return status;
}

int VIDEO_streamFileClose()
{
  int  i;
  VIDEO_EncodeStream *pEncodeStream;

  for(i=0; i<gAVSERVER_config.numEncodeStream; i++) {  
  
    pEncodeStream = &gVIDEO_ctrl.encodeStream[i];

    if(pEncodeStream->fileSaveState==VIDEO_STREAM_FILE_SAVE_STATE_WRITE) {

      #ifdef AVSERVER_DEBUG_VIDEO_STREAM_THR
      OSA_printf(" STREAM: Closing file stream %d\n", i);
      #endif  

      fclose(pEncodeStream->fileSaveHndl);
      
      #ifdef AVSERVER_DEBUG_VIDEO_STREAM_THR
      OSA_printf(" STREAM: Closing file stream ... DONE\n");
      #endif  
      
    }
      
    pEncodeStream->fileSaveState = VIDEO_STREAM_FILE_SAVE_STATE_IDLE;    
  }

  return OSA_SOK;
}
#ifdef VANLINK_DVR_DM365
int VIDEO_streamTskCreate(VIDEO_Stream_CreatePrm *prm)
{
#else
int VIDEO_streamTskCreate()
{
#endif
#ifdef VANLINK_DVR_DM365
		int status, i;
	  
	  status = OSA_mutexCreate(&gVIDEO_Stream_ctrl.mutexLock);
	  if(status!=OSA_SOK) {
		OSA_ERROR("OSA_mutexCreate()\n");
	  }
	#ifdef VANLINK_DVR_DM365_DEBUG //add by sxh
      OSA_printf("----vl VIDEO_streamTskCreate Begin!!\n");    
	#endif
	  memcpy(&gVIDEO_Stream_ctrl.createPrm, prm, sizeof(gVIDEO_Stream_ctrl.createPrm));
	  
	  gVIDEO_Stream_ctrl.bufNum = prm->numCh*VL_DVR_VIDEO_STREAM_MIN_IN_BUF_PER_CH;
	  
	  gVIDEO_Stream_ctrl.bufSize = (OSA_align(prm->frameWidth, 32)*prm->frameHeight*3)/2;
	  gVIDEO_Stream_ctrl.bufSize = OSA_align(gVIDEO_Stream_ctrl.bufSize, 32);

	  #ifdef VANLINK_DVR_DM365_DEBUG //add by sxh
      OSA_printf("----VIDEO_streamTskCreate:OSA_cmemAlloc  Begin!!\n");  
	  OSA_printf("----VIDEO_streamTskCreate:OSA_cmemAlloc size=%x!!\n",gVIDEO_Stream_ctrl.bufSize*gVIDEO_Stream_ctrl.bufNum);
	  OSA_waitMsecs(20);
	  #endif
	  gVIDEO_Stream_ctrl.bufVirtAddr = OSA_cmemAlloc(gVIDEO_Stream_ctrl.bufSize*gVIDEO_Stream_ctrl.bufNum, 32);
	  gVIDEO_Stream_ctrl.bufPhysAddr = OSA_cmemGetPhysAddr(gVIDEO_Stream_ctrl.bufVirtAddr);
	  #ifdef VANLINK_DVR_DM365_DEBUG //add by sxh
      OSA_printf("----VIDEO_streamTskCreate:OSA_cmemAlloc  Done!!\n"); 
	  OSA_waitMsecs(20);
	  #endif
	  if(gVIDEO_Stream_ctrl.bufVirtAddr==NULL||gVIDEO_Stream_ctrl.bufPhysAddr==NULL) {
		OSA_ERROR("OSA_cmemAlloc()\n");
		goto error_exit;
	  }
	
	  gVIDEO_Stream_ctrl.bufCreate.numBuf = gVIDEO_Stream_ctrl.bufNum;
	  for(i=0; i<gVIDEO_Stream_ctrl.bufNum; i++) {
		gVIDEO_Stream_ctrl.bufCreate.bufPhysAddr[i] = gVIDEO_Stream_ctrl.bufPhysAddr + i*gVIDEO_Stream_ctrl.bufSize;
		gVIDEO_Stream_ctrl.bufCreate.bufVirtAddr[i] = gVIDEO_Stream_ctrl.bufVirtAddr + i*gVIDEO_Stream_ctrl.bufSize;	  
	  }

	  #ifdef VANLINK_DVR_DM365_DEBUG //add by sxh
      OSA_printf("----VIDEO_streamTskCreate:OSA_bufCreate  Begin!!\n");    
	  OSA_waitMsecs(20);
	  #endif
	  status = OSA_bufCreate(&gVIDEO_Stream_ctrl.bufHndl, &gVIDEO_Stream_ctrl.bufCreate);
	  if(status!=OSA_SOK) {
		OSA_ERROR("OSA_bufCreate()\n");
		goto error_exit;
	  }
	  
	  for(i=0; i<prm->numCh; i++) {
		  if(i==0)
			gVIDEO_Stream_ctrl.chInfo[i].enableChSave = FALSE;
		  else
		  	gVIDEO_Stream_ctrl.chInfo[i].enableChSave = FALSE;
		  
	      gVIDEO_Stream_ctrl.chInfo[i].fileSaveState = VIDEO_STREAM_FILE_SAVE_STATE_IDLE;
		  gVIDEO_Stream_ctrl.chInfo[i].fileSaveIndex = 0;
		  gVIDEO_Stream_ctrl.chInfo[i].fileSaveHndl  = NULL;

		  gVIDEO_Stream_ctrl.chInfo[i].enableAudio  = FALSE;
		  gVIDEO_Stream_ctrl.chInfo[i].event_mode   = 0;
		
		if(gVIDEO_Stream_ctrl.chInfo[i].enableChSave)
		{
		
			strncpy(gVIDEO_Stream_ctrl.chInfo[i].camName, "vlkcamara", 10);
			
			//////////////////////////////////////////////////////////////////////////
			if(BKTREC_open(DEFAULT_DISK_MPOINT) == BKT_ERR)
				break;
			//////////////////////////////////////////////////////////////////////////    	
		}
	  }
	#if 0//def VANLINK_DVR_DM365_DEBUG //add by sxh
      OSA_printf("----vl VIDEO_streamSysInit Begin!!\n"); 
	  VIDEO_Stream_enableChSaveEx1(0,1,0,0,"CAM0");
	  VIDEO_Stream_enableChSaveEx1(1,1,0,0,"CAM0");
	  //VIDEO_Stream_enableChSaveEx1(2,1,0,0,"CAM0");
	  //VIDEO_Stream_enableChSaveEx1(3,1,0,0,"CAM0");
	#endif
	  VIDEO_streamSysInit();
    #ifdef VANLINK_DVR_DM365_DEBUG //add by sxh
      OSA_printf("----vl VIDEO_streamSysInit Done!!\n");    
	#endif

	#ifdef VANLINK_DVR_DM365_DEBUG //add by sxh
      OSA_printf("----vl VIDEO_streamTskCreate Done!!\n");    
	#endif
  	  return OSA_SOK;
	  
	error_exit:
	
	  OSA_mutexDelete(&gVIDEO_Stream_ctrl.mutexLock);
	  
	  if(gVIDEO_Stream_ctrl.bufVirtAddr) {
		OSA_cmemFree(gVIDEO_Stream_ctrl.bufVirtAddr);
		
		OSA_bufDelete(&gVIDEO_Stream_ctrl.bufHndl);
	  }
	
	  return OSA_EFAIL; 
#else
	  VIDEO_streamSysInit();
  
  	  return OSA_SOK;
#endif

}

int VIDEO_streamTskDelete()
{
#ifdef VANLINK_DVR_DM365
  	int status;
	
	
	VIDEO_streamFileClose();
  
  	VIDEO_streamSysExit();
    VIDEO_Stream_fileSaveExit();

	OSA_mutexDelete(&gVIDEO_Stream_ctrl.mutexLock);
	
	if(gVIDEO_Stream_ctrl.bufVirtAddr) {
		OSA_cmemFree(gVIDEO_Stream_ctrl.bufVirtAddr);
		OSA_bufDelete(&gVIDEO_Stream_ctrl.bufHndl);
	}
	return status;
#else

  VIDEO_streamFileClose();
  
  VIDEO_streamSysExit();
    
  return OSA_SOK;
 #endif
}

int VIDEO_streamTskRun(int streamId)
{
#ifdef VANLINK_DVR_DM365
  int status=OSA_SOK, inBufId, chId;
	OSA_BufInfo *pInBuf;
	Bool isKeyFrame;
		
	status = OSA_bufGetFull(&gVIDEO_Stream_ctrl.bufHndl, &inBufId, OSA_TIMEOUT_FOREVER);
	
	if(status==OSA_SOK) {
	  
	  pInBuf = OSA_bufGetBufInfo(&gVIDEO_Stream_ctrl.bufHndl, inBufId);
	  OSA_assert(pInBuf!=NULL);
	  
	  chId = pInBuf->flags;
	  isKeyFrame = pInBuf->count;
	  pInBuf->isKeyFrame=pInBuf->count;
	  // frameSize = pInBuf->size
	  // frame addr = pInBuf->virtAddr
	  // timestamp = pInBuf->timestamp
		  
	  if(chId >= 0 && chId < gVIDEO_Stream_ctrl.createPrm.numCh) {
		
		// save to file
		VIDEO_Stream_fileSaveRun(chId, isKeyFrame, pInBuf);
		
#ifdef VANLINK_DVR_DM365_DEBUG
		//OSA_printf(" -------chID=%d-------VIDEO_stream_TSK_Run: VIDEO_streamShmCopy Begin!!!\n",chId); 
		//OSA_waitMsecs(10);
#endif

		// stream over network, ...
		VIDEO_streamShmCopy(chId, pInBuf);

#ifdef VANLINK_DVR_DM365_DEBUG
	  //OSA_printf(" VIDEO_stream_TSK_Run: VIDEO_streamShmCopy Done!!!\n"); 
	  //OSA_waitMsecs(10);
#endif	
	  }  
	  
	  OSA_bufPutEmpty(&gVIDEO_Stream_ctrl.bufHndl, inBufId);
	}
#ifdef VANLINK_DVR_DM365_DEBUG
	  //OSA_printf(" VIDEO_stream_TSK_Run: Done!!!\n"); 
	  //OSA_waitMsecs(10);
#endif		
	return status;

#else

  int status=OSA_EFAIL, inBufId;
  OSA_BufInfo *pInBufInfo;
  VIDEO_BufHeader *pInBufHeader;
  
  pInBufInfo = AVSERVER_bufGetFull( VIDEO_TSK_STREAM, streamId, &inBufId, OSA_TIMEOUT_FOREVER);
  
  OSA_assert(pInBufInfo!=NULL);
  
  if(pInBufInfo!=NULL) {
   
    pInBufHeader  = (VIDEO_BufHeader*)pInBufInfo->virtAddr;
    
    OSA_assert(pInBufHeader!=NULL);    

    #ifdef AVSERVER_DEBUG_VIDEO_STREAM_THR
      #ifdef AVSERVER_DEBUG_RUNNING
      OSA_printf(" STREAM: Stream %d InBuf %d, Frame size %d bytes, Key frame %d\n", streamId, inBufId, pInBufHeader->encFrameSize, pInBufHeader->encFrameType);
      #endif
    #endif  

    if(pInBufHeader->encFrameSize > 0) {
    
      OSA_prfBegin(&gAVSERVER_ctrl.streamPrf);
    
      VIDEO_streamShmCopy(streamId, pInBufInfo);
      VIDEO_streamFileWrite(streamId, pInBufInfo);
      
      OSA_prfEnd(&gAVSERVER_ctrl.streamPrf, 1);      
    }      

    AVSERVER_bufPutEmpty( VIDEO_TSK_STREAM, streamId, inBufId);  
  }
  
   return status;
#endif
}

int VIDEO_streamTskMain(struct OSA_TskHndl *pTsk, OSA_MsgHndl *pMsg, Uint32 curState )
{
#ifdef VANLINK_DVR_DM365

	  int status;
	  Bool done = FALSE, ackMsg=FALSE;;
	  Uint16 cmd = OSA_msgGetCmd(pMsg);
	  VIDEO_Stream_CreatePrm* pCreatePrm = (VIDEO_Stream_CreatePrm*)OSA_msgGetPrm(pMsg);
	  
	  if( cmd != VL_DVR_VIDEO_STREAM_CMD_CREATE|| pCreatePrm==NULL) {
	    OSA_tskAckOrFreeMsg(pMsg, OSA_EFAIL);
	    return OSA_SOK;
	  }
	  
	  status = VIDEO_streamTskCreate( pCreatePrm );
	  
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
	    
	    switch(cmd) {
	      case VL_DVR_VIDEO_STREAM_CMD_RUN:
	        OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);      
	        VIDEO_streamTskRun(0);      //此处参数是没有用的
	        break;
	        
	      case VL_DVR_VIDEO_STREAM_CMD_DELETE:
	        done = TRUE;        
	        ackMsg = TRUE;
	        break;   
	                   
	      default:   
	        OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
	        break;              
	    }
	  }

	  VIDEO_streamTskDelete();
	  if(ackMsg)
	    OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);          

	  return OSA_SOK;
#else

  int status, streamId;
  Bool done=FALSE, ackMsg = FALSE;
  Uint16 cmd = OSA_msgGetCmd(pMsg);
  
  #ifdef AVSERVER_DEBUG_VIDEO_STREAM_THR
  OSA_printf(" STREAM: Recevied CMD = 0x%04x\n", cmd);
  #endif  
  
  if(cmd!=AVSERVER_CMD_CREATE) {
    OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
    return OSA_SOK;
  }

  #ifdef AVSERVER_DEBUG_VIDEO_STREAM_THR
  OSA_printf(" STREAM: Create...\n");
  #endif  
  
  status = VIDEO_streamTskCreate();
  
  OSA_tskAckOrFreeMsg(pMsg, status);  
  
  if(status!=OSA_SOK) {
    OSA_ERROR("VIDEO_streamTskCreate()\n");
    return status;
  }

  #ifdef AVSERVER_DEBUG_VIDEO_STREAM_THR
  OSA_printf(" STREAM: Create...DONE\n");
  #endif  

  while(!done) {
    
    status = OSA_tskWaitMsg(pTsk, &pMsg);

    if(status!=OSA_SOK) 
      break;

    cmd = OSA_msgGetCmd(pMsg);

    switch(cmd) {
      case AVSERVER_CMD_DELETE:
        done = TRUE;
        ackMsg = TRUE;
        break;

      case AVSERVER_CMD_NEW_DATA:
        streamId = (int)OSA_msgGetPrm(pMsg);
        
        OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
        
        VIDEO_streamTskRun(streamId);
        
        break;
      default:
      
        #ifdef AVSERVER_DEBUG_VIDEO_STREAM_THR
        OSA_printf(" STREAM: Unknown CMD = 0x%04x\n", cmd);
        #endif  
      
        OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);      
        break;
    }
  }
 
  #ifdef AVSERVER_DEBUG_VIDEO_STREAM_THR
  OSA_printf(" STREAM: Delete...\n");
  #endif  
    
  VIDEO_streamTskDelete();
  
  if(ackMsg)
    OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);

  #ifdef AVSERVER_DEBUG_VIDEO_STREAM_THR
  OSA_printf(" STREAM: Delete...DONE\n");
  #endif  

  return OSA_SOK;
#endif

}

#ifdef VANLINK_DVR_DM365
int VIDEO_streamCreate(VIDEO_Stream_CreatePrm *prm)
{
#else
int VIDEO_streamCreate()
{
#endif

#ifdef VANLINK_DVR_DM365
	int status;
	
	memset(&gVIDEO_Stream_ctrl, 0, sizeof(gVIDEO_Stream_ctrl));
	gVIDEO_Stream_ctrl.capMode = 0;
	status = OSA_tskCreate( &gVIDEO_Stream_ctrl.tskHndl, VIDEO_streamTskMain, VL_DVR_VIDEO_STREAM_THR_PRI, VL_DVR_VIDEO_STREAM_STACK_SIZE, 0);
	
	OSA_assertSuccess(status);
	  
	status = OSA_mbxCreate( &gVIDEO_Stream_ctrl.mbxHndl);  
	
	OSA_assertSuccess(status);	
	
	status = VIDEO_Stream_sendCmd(VL_DVR_VIDEO_STREAM_CMD_CREATE, prm, OSA_MBX_WAIT_ACK );
	
	return status;	
	
#else
  int status;
  
  status = OSA_tskCreate( &gVIDEO_ctrl.streamTsk, VIDEO_streamTskMain, VIDEO_STREAM_THR_PRI, VIDEO_STREAM_STACK_SIZE*2, 0);
  if(status!=OSA_SOK) {
    OSA_ERROR("OSA_tskCreate()\n");
    return status;
  }
  
  return status;
#endif
}


int VIDEO_streamDelete()
{
#ifdef VANLINK_DVR_DM365
	int status;
	
	status = VIDEO_Stream_sendCmd(VL_DVR_VIDEO_STREAM_CMD_DELETE, NULL, OSA_MBX_WAIT_ACK );
	
	status = OSA_tskDelete( &gVIDEO_Stream_ctrl.tskHndl );
	
	OSA_assertSuccess(status);
	  
	status = OSA_mbxDelete( &gVIDEO_Stream_ctrl.mbxHndl);  
	
	OSA_assertSuccess(status);	
	
	return status;

#else
  int status;
  
  status = OSA_tskDelete( &gVIDEO_ctrl.streamTsk );
  
  return status;

#endif
	
	

  
}
#ifdef VANLINK_DVR_DM365
int VIDEO_Stream_fileSaveRun(int chId, Bool isKeyFrame, OSA_BufInfo *pBufInfo)
{
  Bool saveToFile;
  int  fileSaveState;
  int  status=OSA_SOK, writtenBytes;
  VIDEO_Stream_ChInfo *pChInfo;
  
  static char filename[256];  
    
  pChInfo = &gVIDEO_Stream_ctrl.chInfo[chId];

  fileSaveState = pChInfo->fileSaveState;
  
  saveToFile = pChInfo->enableChSave;
  
 
  
  if(saveToFile) {
  	 #if 1//def VANLINK_DVR_DM365_DEBUG
	  OSA_printf(" WRITER: Ch %d, saveToFile = %d, fileSaveState = %d, frame size = %d, key frame = %d\n", 
	          chId, saveToFile, fileSaveState, pBufInfo->size, isKeyFrame);
	  #endif
  
    if(fileSaveState==VL_DVR_VIDEO_STREAM_FILE_SAVE_STATE_IDLE)
      fileSaveState=VL_DVR_VIDEO_STREAM_FILE_SAVE_STATE_OPEN;
  
  } else {

    if(fileSaveState==VL_DVR_VIDEO_STREAM_FILE_SAVE_STATE_OPEN)
      fileSaveState=VL_DVR_VIDEO_STREAM_FILE_SAVE_STATE_IDLE;
    else
    if(fileSaveState==VL_DVR_VIDEO_STREAM_FILE_SAVE_STATE_WRITE)
      fileSaveState=VL_DVR_VIDEO_STREAM_FILE_SAVE_STATE_CLOSE;
  }
  
  if(fileSaveState==VL_DVR_VIDEO_STREAM_FILE_SAVE_STATE_OPEN) {
    if(isKeyFrame) {
        fileSaveState = VL_DVR_VIDEO_STREAM_FILE_SAVE_STATE_WRITE;
    }
  }
  
  if(fileSaveState==VL_DVR_VIDEO_STREAM_FILE_SAVE_STATE_WRITE) {  
    if(pBufInfo->size>0) {
			
		//TRACE0_WTR("Stream %d writing %d bytes\n", chId, pBufInfo->size);
	
		if(pChInfo->firstKeyframe == FALSE && isKeyFrame)
		{
			pChInfo->firstKeyframe = TRUE;
		}
		
		if(pChInfo->firstKeyframe == TRUE)
		{
			T_VIDEO_REC_PARAM vp;
			
			OSA_prfBegin(&gVIDEO_Stream_ctrl.prfFileWr);
			
			vp.ch 		 = chId;
			vp.framesize = pBufInfo->size;
			vp.framerate = 0;//conv_framerate(gSETPARAM_info.codecSetPrm[chId].frameSkipMask);
			vp.event     = gVIDEO_Stream_ctrl.chInfo[chId].event_mode;
			vp.frametype = isKeyFrame;
			vp.width     = gVIDEO_Stream_ctrl.createPrm.frameWidth;
			vp.height    = gVIDEO_Stream_ctrl.createPrm.frameHeight;
			vp.buff      = pBufInfo->virtAddr;
			vp.camName	 = "vlk01";//gVIDEO_Stream_ctrl.chInfo[chId].camName;
			vp.ts.sec    = pBufInfo->timesec;
			vp.ts.usec   = pBufInfo->timestamp;
			vp.audioONOFF = 0;//pChInfo->enableAudio; // BK - 100119 
			vp.capMode    = gVIDEO_Stream_ctrl.capMode;
			
			//TRACE0_WTR("frameskipmask [%x], framerate :%d, CAPMODE:%d,  CAMNAME:%s\n", gSETPARAM_info.codecSetPrm[chId].frameSkipMask, vp.framerate, vp.captureMode, vp.camName);
			
			if(BKTREC_WriteVideoStream(&vp) == BKT_ERR){ //bk.. except case full basket.caution...091106
				
				pChInfo->enableChSave = FALSE;
				fileSaveState=VL_DVR_VIDEO_STREAM_FILE_SAVE_STATE_CLOSE;
				
				OSA_printf("failed write video Ch %d \n", chId);
			}
			
			OSA_prfEnd(&gVIDEO_Stream_ctrl.prfFileWr, 1);
		}
	}
  }
  if(fileSaveState==VL_DVR_VIDEO_STREAM_FILE_SAVE_STATE_CLOSE) {   
  
    #if 1//def WRITER_DEBUG
    OSA_printf(" rec: Closing file stream %d\n", chId);
	OSA_printf(" rec: Closing file stream %d\n", chId);
    #endif   

    
    fileSaveState=VL_DVR_VIDEO_STREAM_FILE_SAVE_STATE_IDLE;
    pChInfo->firstKeyframe = FALSE;
    #if 1//def WRITER_DEBUG
    OSA_printf(" WRITER: Closing file stream ... DONE\n");
    #endif  
  }
    
  pChInfo->fileSaveState = fileSaveState; 

  return status;
}

int VIDEO_Stream_fileSaveExit()
{
  int  i;
  VIDEO_Stream_ChInfo *pChInfo;

  for(i=0; i<gVIDEO_Stream_ctrl.createPrm.numCh; i++) {  
  
    pChInfo = &gVIDEO_Stream_ctrl.chInfo[i];

    if(pChInfo->fileSaveState==VL_DVR_VIDEO_STREAM_FILE_SAVE_STATE_WRITE) {

      #ifdef WRITER_DEBUG
      OSA_printf(" WRITER: Closing file stream %d\n", i);
      #endif  

      OSA_prfBegin(&gVIDEO_Stream_ctrl.prfFileWr);
      BKTREC_exit(SF_STOP, BKT_REC_CLOSE_BUF_FLUSH);
      OSA_prfEnd(&gVIDEO_Stream_ctrl.prfFileWr, 1);
	 
	
      
      #ifdef WRITER_DEBUG
      OSA_printf(" WRITER: Closing file stream ... DONE\n");
      #endif  
      
    }
      
    pChInfo->fileSaveState = VL_DVR_VIDEO_STREAM_FILE_SAVE_STATE_IDLE;    
	pChInfo->firstKeyframe = FALSE;
  }

  //////////////////////////////////////////////////////////////////////////
	// bk
	OSA_prfBegin(&gVIDEO_Stream_ctrl.prfFileWr);
	BKTREC_exit(SF_STOP, BKT_REC_CLOSE_BUF_FLUSH);
	OSA_prfEnd(&gVIDEO_Stream_ctrl.prfFileWr, 1);
	//////////////////////////////////////////////////////////////////////////
  return OSA_SOK;
}

OSA_BufInfo *VIDEO_Stream_getEmptyBuf(int *bufId, int timeout)
{
  int status;
    
  status = OSA_bufGetEmpty(&gVIDEO_Stream_ctrl.bufHndl, bufId, timeout);
  if(status!=OSA_SOK)
    return NULL;

  return OSA_bufGetBufInfo(&gVIDEO_Stream_ctrl.bufHndl, *bufId);
}

int VIDEO_Stream_putFullBuf(int chId, OSA_BufInfo *buf, int bufId)
{
  buf->flags = chId;
    
  OSA_bufPutFull(&gVIDEO_Stream_ctrl.bufHndl, bufId);
    
  VIDEO_Stream_sendCmd(VL_DVR_VIDEO_STREAM_CMD_RUN, NULL, 0);

  return OSA_SOK;
}

int VIDEO_Stream_enableChSave(int chId, Bool enable)
{
  if(chId >= 0 && chId < gVIDEO_Stream_ctrl.createPrm.numCh) {
    gVIDEO_Stream_ctrl.chInfo[chId].enableChSave = enable;
	gVIDEO_Stream_ctrl.chInfo[chId].event_mode = EVT_CONT;

		//////////////////////////////////////////////////////////////////////////
		if(BKTREC_open(DEFAULT_DISK_MPOINT) == BKT_ERR)
			return 0;
  }

  return OSA_SOK;
}
int VIDEO_Stream_enableChSaveEx(int chId, Bool enable, int enableAudio, int event_mode)
{
  if(chId >= 0 && chId < gVIDEO_Stream_ctrl.createPrm.numCh) {
    gVIDEO_Stream_ctrl.chInfo[chId].enableChSave = enable;

	gVIDEO_Stream_ctrl.chInfo[chId].event_mode  = event_mode+1;
	gVIDEO_Stream_ctrl.chInfo[chId].enableAudio = enableAudio;

		//////////////////////////////////////////////////////////////////////////
		if(BKTREC_open(DEFAULT_DISK_MPOINT) == BKT_ERR)
			return 0;
		//////////////////////////////////////////////////////////////////////////
  }

  return OSA_SOK;
}
int VIDEO_Stream_enableChSaveEx1(int chId, Bool enable, int enableAudio, int event_mode, char *camName)
{
  if(chId >= 0 && chId < gVIDEO_Stream_ctrl.createPrm.numCh) {
    gVIDEO_Stream_ctrl.chInfo[chId].enableChSave = enable;
	
	gVIDEO_Stream_ctrl.chInfo[chId].event_mode  = event_mode+1;
	gVIDEO_Stream_ctrl.chInfo[chId].enableAudio = enableAudio;
	
	if(strlen(camName))
	{
		strncpy(gVIDEO_Stream_ctrl.chInfo[chId].camName, camName, 16);
	}
	
	//////////////////////////////////////////////////////////////////////////
	if(BKTREC_open(DEFAULT_DISK_MPOINT) == BKT_ERR)
		return 0;
	//////////////////////////////////////////////////////////////////////////
  }

  return OSA_SOK;
}

int VIDEO_Stream_disableChSave(int chId, Bool disable)
{
	
	if(chId >= 0 && chId < gVIDEO_Stream_ctrl.createPrm.numCh) 
	{
		gVIDEO_Stream_ctrl.chInfo[chId].enableChSave = disable;
	}
	
	return OSA_SOK;
}

#endif
