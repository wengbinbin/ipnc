#include <decode_priv.h>
#include <osa_file.h>
#include <display_priv.h>
#include <drv_frameCopy.h>
#include <drv_resz.h>

DECODE_Ctrl gDECODE_ctrl;

int DECODE_tskCreate( DECODE_CreatePrm *prm)
{
  int status, k;
  ALG_VidDecCreate vidDecCreatePrm;
  
  memcpy(&gDECODE_ctrl.createPrm, prm, sizeof(gDECODE_ctrl.createPrm));
  
  gDECODE_ctrl.decBufNum  = DECODE_MIN_IN_BUF_PER_CH;
  gDECODE_ctrl.decBufSize = (OSA_align(prm->maxWidth+64, 32)*(prm->maxHeight+64)*3)/2;
  gDECODE_ctrl.decBufSize = OSA_align(gDECODE_ctrl.decBufSize, 32);
   
  gDECODE_ctrl.inBufSize = 5*MB;
  
  #ifdef DECODE_DEBUG
  OSA_printf(" DECODE: Allocating memory %d\n", gDECODE_ctrl.inBufSize);
  #endif
    
  gDECODE_ctrl.inBufVirtAddr = OSA_cmemAlloc(gDECODE_ctrl.inBufSize, 32);
  gDECODE_ctrl.inBufPhysAddr = OSA_cmemGetPhysAddr(gDECODE_ctrl.inBufVirtAddr);
  
  if(gDECODE_ctrl.inBufVirtAddr==NULL||gDECODE_ctrl.inBufPhysAddr==NULL) {
    OSA_ERROR("OSA_cmemAlloc()\n");
    goto error_exit;
  }

  #ifdef DECODE_DEBUG
  OSA_printf(" DECODE: Allocated memory %08x:%08x\n", (Uint32)gDECODE_ctrl.inBufVirtAddr, (Uint32)gDECODE_ctrl.inBufPhysAddr);
  #endif
  
  status = OSA_fileReadFile(prm->filename, gDECODE_ctrl.inBufVirtAddr, gDECODE_ctrl.inBufSize,0, &gDECODE_ctrl.inDataSize);
  if(status!=OSA_SOK)
    goto error_exit;
    
  #ifdef DECODE_DEBUG
  OSA_printf(" DECODE: Allocating memory %dx%d\n", gDECODE_ctrl.decBufSize, gDECODE_ctrl.decBufNum);
  #endif
  
  gDECODE_ctrl.decBufBaseVirtAddr = OSA_cmemAlloc(gDECODE_ctrl.decBufSize*gDECODE_ctrl.decBufNum, 32);
  gDECODE_ctrl.decBufBasePhysAddr = OSA_cmemGetPhysAddr(gDECODE_ctrl.decBufBaseVirtAddr);
  
  if(gDECODE_ctrl.decBufBaseVirtAddr==NULL||gDECODE_ctrl.decBufBasePhysAddr==NULL) {
    OSA_ERROR("OSA_cmemAlloc()\n");
    goto error_exit;
  }

  #ifdef DECODE_DEBUG
  OSA_printf(" DECODE: Allocated memory %08x:%08x\n", (Uint32)gDECODE_ctrl.decBufBaseVirtAddr, (Uint32)gDECODE_ctrl.decBufBasePhysAddr);
  #endif

  for(k=0; k<gDECODE_ctrl.decBufNum; k++) {
    gDECODE_ctrl.decBufPhysAddr[k] = gDECODE_ctrl.decBufBasePhysAddr + k*gDECODE_ctrl.decBufSize;
    gDECODE_ctrl.decBufVirtAddr[k] = gDECODE_ctrl.decBufBaseVirtAddr + k*gDECODE_ctrl.decBufSize;    
  }
  
  vidDecCreatePrm.codec           = prm->codec;          
  vidDecCreatePrm.dataFormat      = DRV_DATA_FORMAT_YUV420;     
  vidDecCreatePrm.maxWidth        = prm->maxWidth;          
  vidDecCreatePrm.maxHeight       = prm->maxHeight;         

  #ifdef DECODE_DEBUG
  OSA_printf(" DECODE: Creating ALG_vidDec\n");
  #endif

  gDECODE_ctrl.algVidDecHndl = ALG_vidDecCreate(&vidDecCreatePrm);
  if(gDECODE_ctrl.algVidDecHndl==NULL) {
    OSA_ERROR("ALG_vidDecCreate()\n");
    goto error_exit;
  }
  
  #ifdef DECODE_DEBUG
  OSA_printf(" DECODE: Create DONE\n");  
  #endif
  
  return OSA_SOK;

error_exit:
    
   if(gDECODE_ctrl.inBufVirtAddr) {
     OSA_cmemFree(gDECODE_ctrl.inBufVirtAddr);
   }
  
   if(gDECODE_ctrl.decBufBaseVirtAddr) {
     OSA_cmemFree(gDECODE_ctrl.decBufBaseVirtAddr);
   }  
   
   if(gDECODE_ctrl.algVidDecHndl)
     ALG_vidDecDelete(gDECODE_ctrl.algVidDecHndl);

  return OSA_EFAIL;  
}

int DECODE_tskDelete()
{
  int status=OSA_SOK;
    
  if(gDECODE_ctrl.inBufVirtAddr) {
    OSA_cmemFree(gDECODE_ctrl.inBufVirtAddr);
  }

  if(gDECODE_ctrl.decBufBaseVirtAddr) {
    OSA_cmemFree(gDECODE_ctrl.decBufBaseVirtAddr);
  }  

  if(gDECODE_ctrl.algVidDecHndl)
    ALG_vidDecDelete(gDECODE_ctrl.algVidDecHndl);
    
  return status;
}

static int DECODE_displayCopy(Uint8 *outVirtAddr, Uint8 *outPhysAddr, ALG_VidDecRunStatus *vidDecStatus) {

  int status;
  DRV_FrameCopyPrm frameCopy;
  Bool doResize;

  DRV_ReszRunPrm prm;
  DRV_ReszOutPrm outPrm[2];
  
  frameCopy.srcPhysAddr = outPhysAddr;
  frameCopy.srcVirtAddr = outVirtAddr;
  frameCopy.dstPhysAddr = gDISPLAY_ctrl.displayInfo.physAddr[gDISPLAY_ctrl.displayBufId];
  frameCopy.dstVirtAddr = gDISPLAY_ctrl.displayInfo.virtAddr[gDISPLAY_ctrl.displayBufId];
  frameCopy.srcOffsetH  = vidDecStatus->outOffsetH;
  frameCopy.srcOffsetV  = vidDecStatus->outOffsetV;
  frameCopy.dstOffsetH  = gDISPLAY_ctrl.displayInfo.offsetH;
  frameCopy.dstOffsetV  = gDISPLAY_ctrl.displayInfo.offsetV;
  frameCopy.copyWidth   = vidDecStatus->frameWidth;
  frameCopy.copyHeight  = vidDecStatus->frameHeight;
  frameCopy.dataFormat  = gDISPLAY_ctrl.displayInfo.dataFormat;
  frameCopy.srcStartX   = vidDecStatus->outStartX;
  frameCopy.srcStartY   = vidDecStatus->outStartY;
  frameCopy.dstStartX   = 0;
  frameCopy.dstStartY   = 0;
  
  doResize = FALSE;
  
  if(   vidDecStatus->frameWidth!=gDISPLAY_ctrl.displayInfo.width
    ||  vidDecStatus->frameHeight!=gDISPLAY_ctrl.displayInfo.height
    )  {
  
    doResize = TRUE;
  }
  
  if(doResize) {
  
    prm.inType = DRV_DATA_FORMAT_YUV420
                |DRV_DATA_FORMAT_INTERLACED
              ;
    prm.inPhysAddr = frameCopy.srcPhysAddr;
    prm.inVirtAddr = frameCopy.srcVirtAddr;
    prm.inStartX = frameCopy.srcStartX;
    prm.inStartY = frameCopy.srcStartY;  
    prm.inWidth  = frameCopy.copyWidth;  
    prm.inHeight = frameCopy.copyHeight;  
    prm.inOffsetH= frameCopy.srcOffsetH;
    prm.inOffsetV= frameCopy.srcOffsetV;  
    prm.enableInvAlaw = FALSE;
    prm.enableInvDpcm = FALSE;  
    prm.clkDivM = 10;
    prm.clkDivN = 80;
    
    prm.pOut[0] = NULL;
    prm.pOut[1] = NULL;

    prm.pOut[0] = &outPrm[0];  
    
    outPrm[0].outType = DRV_DATA_FORMAT_YUV420;
    outPrm[0].flipH = FALSE;  
    outPrm[0].flipV = FALSE;
    outPrm[0].outPhysAddr = frameCopy.dstPhysAddr;
    outPrm[0].outVirtAddr = frameCopy.dstVirtAddr;
    outPrm[0].outWidth = gDISPLAY_ctrl.displayInfo.width;
    outPrm[0].outHeight = gDISPLAY_ctrl.displayInfo.height;
    outPrm[0].outOffsetH = gDISPLAY_ctrl.displayInfo.offsetH;
    outPrm[0].outOffsetV = gDISPLAY_ctrl.displayInfo.offsetV;
  
    OSA_prfBegin(&gDECODE_ctrl.prfResz);
    DRV_reszRun(&prm);
    OSA_prfEnd(&gDECODE_ctrl.prfResz, 1);
      
  } else {
    status = DRV_frameCopy(NULL, &frameCopy);
    if(status!=OSA_SOK) {
      OSA_ERROR("DRV_frameCopy(A)\n");
    }
  }
  DRV_displaySwitchBuf( &gDISPLAY_ctrl.displayHndl, &gDISPLAY_ctrl.displayBufId);
  
  return status;
}

int DECODE_tskRun()
{
  int status=OSA_SOK, whOffset;
  ALG_VidDecRunPrm runPrm;
  ALG_VidDecRunStatus runStatus;
  Uint8 *inAddr;
     
  if(gDECODE_ctrl.inDataOffset >= gDECODE_ctrl.inDataSize) {
    // done
    return OSA_EFAIL;
  }
  
  inAddr = gDECODE_ctrl.inBufVirtAddr + gDECODE_ctrl.inDataOffset;
  
  whOffset = 0;
  if(gDECODE_ctrl.createPrm.codec==ALG_VID_CODEC_H264)
    whOffset = 48;
   
  runPrm.inAddr         = inAddr;
  runPrm.outAddr        = gDECODE_ctrl.decBufVirtAddr[gDECODE_ctrl.curDecBufId];        
  runPrm.inDataSize     = 100*KB;
  runPrm.outOffsetH     = OSA_align(gDECODE_ctrl.createPrm.maxWidth+whOffset, 32);
  runPrm.outOffsetV     = gDECODE_ctrl.createPrm.maxHeight+whOffset;
  runPrm.inputBufId     = gDECODE_ctrl.curDecBufId;
  
  OSA_prfBegin(&gDECODE_ctrl.prfDec);

  #ifdef DECODE_DEBUG_RUNNING
  OSA_printf(" DECODE: %d: %08x (%d, %d) -> %08x (%dx%d, %d)\n", 
    gDECODE_ctrl.frameCount,
    (Uint32)runPrm.inAddr,
    gDECODE_ctrl.inDataOffset,
    runPrm.inDataSize,
    (Uint32)runPrm.outAddr,
    runPrm.outOffsetH,
    runPrm.outOffsetV,
    runPrm.inputBufId
    );
  #endif
  
  status = ALG_vidDecRun(gDECODE_ctrl.algVidDecHndl, &runPrm, &runStatus);

  OSA_prfEnd(&gDECODE_ctrl.prfDec, 1);
  
  #ifdef DECODE_DEBUG_RUNNING
  OSA_printf(" DECODE: %d:%d:%d:%dx%d:%d: %d bytes (%d, %d)\n", 
        runStatus.freeBufId,
        runPrm.inputBufId,   
        runStatus.outputBufId, 
        runStatus.frameWidth, runStatus.frameHeight, 
        runStatus.isKeyFrame,
        runStatus.bytesUsed,
        runStatus.outStartX,
        runStatus.outStartY        
        );
  #endif

  gDECODE_ctrl.frameCount++;
  
  if(status==OSA_SOK) {
    gDECODE_ctrl.inDataOffset += runStatus.bytesUsed;

    #if 1
    if(runStatus.outputBufId >= 0 && runStatus.outputBufId < gDECODE_ctrl.decBufNum) {
      DECODE_displayCopy(
          gDECODE_ctrl.decBufVirtAddr[runStatus.outputBufId], 
          gDECODE_ctrl.decBufPhysAddr[runStatus.outputBufId],
          &runStatus
          );
    }
    #endif
    
    gDECODE_ctrl.curDecBufId = (gDECODE_ctrl.curDecBufId+1)%(gDECODE_ctrl.decBufNum);
  }
  
  return status;
}

int DECODE_tskMain(struct OSA_TskHndl *pTsk, OSA_MsgHndl *pMsg, Uint32 curState )
{
  int status;
  Bool done = FALSE, ackMsg=FALSE;;
  Uint16 cmd = OSA_msgGetCmd(pMsg);
  DECODE_CreatePrm *pCreatePrm = (DECODE_CreatePrm*)OSA_msgGetPrm(pMsg);
  
  if( cmd != DECODE_CMD_CREATE || pCreatePrm==NULL) {
    OSA_tskAckOrFreeMsg(pMsg, OSA_EFAIL);
    return OSA_SOK;
  }
  
  OSA_flgClear(&gDECODE_ctrl.flgHndl, 0xFFFFFFFF);
    
  status = DECODE_tskCreate( pCreatePrm );
  
  #ifdef DECODE_DEBUG
  OSA_printf(" DECODE: Sending ACK (%d)\n", status);  
  #endif  
  
  OSA_tskAckOrFreeMsg(pMsg, status);  
  
  if(status != OSA_SOK) {
    OSA_flgSet(&gDECODE_ctrl.flgHndl, DECODE_FLAG_ERROR);  
    OSA_ERROR("Create\n");
    return OSA_SOK;
  }
  
  DISPLAY_layoutSetMode(DISPLAY_LAYOUT_MODE_PLAYBACK);
  OSA_waitMsecs(40);  

  #ifdef DECODE_DEBUG
  OSA_printf(" DECODE: Running\n");  
  #endif  
  
  while(!done)
  {
    status = DECODE_tskRun();        
    if(status!=OSA_SOK)
      break;
      
    status = OSA_tskCheckMsg(pTsk, &pMsg);
    if(status != OSA_SOK) {
      continue;
    }
      
    cmd = OSA_msgGetCmd(pMsg);  
    
    switch(cmd) {
        
      case DECODE_CMD_DELETE:
        done = TRUE;        
        ackMsg = TRUE;
        break;   
                   
      default:   
        OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
        break;              
    }
  }

  DECODE_tskDelete();
  if(ackMsg)
    OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);          

  DISPLAY_layoutSetMode(DISPLAY_LAYOUT_MODE_LIVE_2x2);
  OSA_waitMsecs(40);  
  
  OSA_flgSet(&gDECODE_ctrl.flgHndl, DECODE_FLAG_DONE);

  return OSA_SOK;
}

int DECODE_waitComplete()
{
  OSA_flgWait(&gDECODE_ctrl.flgHndl, DECODE_FLAG_DONE|DECODE_FLAG_ERROR, OSA_FLG_MODE_OR, OSA_TIMEOUT_FOREVER);
  
  if(OSA_flgIsSet(&gDECODE_ctrl.flgHndl, DECODE_FLAG_DONE))
    return OSA_SOK;

  if(OSA_flgIsSet(&gDECODE_ctrl.flgHndl, DECODE_FLAG_ERROR))
    return OSA_EFAIL;

  return OSA_EFAIL;
}

int DECODE_sendCmd(Uint16 cmd, void *prm, Uint16 flags )
{
  return OSA_mbxSendMsg(&gDECODE_ctrl.tskHndl.mbxHndl, &gDECODE_ctrl.mbxHndl, cmd, prm, flags);
}

int DECODE_create(DECODE_CreatePrm *prm)
{
  int status;
  
  memset(&gDECODE_ctrl, 0, sizeof(gDECODE_ctrl));  
  
  status = OSA_flgCreate(&gDECODE_ctrl.flgHndl, 0);
  OSA_assertSuccess(status);
   
  status = OSA_tskCreate( &gDECODE_ctrl.tskHndl, DECODE_tskMain, DECODE_THR_PRI, DECODE_STACK_SIZE, 0);

  OSA_assertSuccess(status);
    
  status = OSA_mbxCreate( &gDECODE_ctrl.mbxHndl);  
  
  OSA_assertSuccess(status);  

  #ifdef DECODE_DEBUG
  OSA_printf(" DECODE: Sending CMD\n");  
  #endif  

  status = DECODE_sendCmd(DECODE_CMD_CREATE, prm, OSA_MBX_WAIT_ACK );
  
  #ifdef DECODE_DEBUG
  OSA_printf(" DECODE: Received ACK\n");  
  #endif  
  
  return status;  
}

int DECODE_delete()
{
  int status;
  
  status = DECODE_sendCmd(DECODE_CMD_DELETE, NULL, OSA_MBX_WAIT_ACK );
  
  status = OSA_tskDelete( &gDECODE_ctrl.tskHndl );

  OSA_assertSuccess(status);
    
  status = OSA_mbxDelete( &gDECODE_ctrl.mbxHndl);  
  
  OSA_assertSuccess(status);  
  
  status = OSA_flgDelete(&gDECODE_ctrl.flgHndl);
  OSA_assertSuccess(status);
  
  return status;
}


