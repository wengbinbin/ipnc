#include <encode_priv.h>

ENCODE_Ctrl gENCODE_ctrl;

int ENCODE_tskCreate( ENCODE_CreatePrm *prm)
{
  int status, i, k;
  ALG_VidEncCreate vidEncCreatePrm;
  ENCODE_ChInfo *pChInfo;

  status = OSA_mutexCreate(&gENCODE_ctrl.mutexLock);
  if(status!=OSA_SOK) {
    OSA_ERROR("OSA_mutexCreate()\n");
    return status;
  }

  memcpy(&gENCODE_ctrl.createPrm, prm, sizeof(gENCODE_ctrl.createPrm));

  for(i=0; i<prm->numCh; i++) {

    pChInfo = &gENCODE_ctrl.chInfo[i];

    pChInfo->bufNum = ENCODE_MIN_IN_BUF_PER_CH;

    pChInfo->bufSize = (OSA_align(prm->frameWidth, 32)*prm->frameHeight*3)/2;
    pChInfo->bufSize = OSA_align(pChInfo->bufSize, 32);

    #ifdef ENCODE_DEBUG
    OSA_printf(" ENCODE: Allocating memory %dx%d\n", pChInfo->bufSize, pChInfo->bufNum);
    #endif

    pChInfo->bufVirtAddr = OSA_cmemAlloc(pChInfo->bufSize*pChInfo->bufNum, 32);
    pChInfo->bufPhysAddr = OSA_cmemGetPhysAddr(pChInfo->bufVirtAddr);

    if(pChInfo->bufVirtAddr==NULL||pChInfo->bufPhysAddr==NULL) {
      OSA_ERROR("OSA_cmemAlloc()\n");
      goto error_exit;
    }

    pChInfo->bufCreate.numBuf = pChInfo->bufNum;
    for(k=0; k<pChInfo->bufNum; k++) {
      pChInfo->bufCreate.bufPhysAddr[k] = pChInfo->bufPhysAddr + k*pChInfo->bufSize;
      pChInfo->bufCreate.bufVirtAddr[k] = pChInfo->bufVirtAddr + k*pChInfo->bufSize;
    }

    #ifdef ENCODE_DEBUG
    OSA_printf(" ENCODE: Creating buffer\n");
    #endif

    status = OSA_bufCreate(&pChInfo->bufHndl, &pChInfo->bufCreate);
    if(status!=OSA_SOK) {
      OSA_ERROR("OSA_bufCreate()\n");
      goto error_exit;
    }

    if(prm->frameWidth==704)
      pChInfo->curBitrate = 2*1000*1000;
    else
      pChInfo->curBitrate = 1*1000*1000;

    pChInfo->curKeyFrameInterval = 30;

    pChInfo->newBitrate = pChInfo->curBitrate;
    pChInfo->newKeyFrameInterval = pChInfo->curKeyFrameInterval;

    pChInfo->enableEncode = TRUE;

    vidEncCreatePrm.codec           = ALG_VID_CODEC_H264;
    vidEncCreatePrm.dataFormat      = DRV_DATA_FORMAT_YUV420;
    vidEncCreatePrm.width           = prm->frameWidth;
    vidEncCreatePrm.height          = prm->frameHeight;
    vidEncCreatePrm.offsetH         = OSA_align(prm->frameWidth, 32);
    vidEncCreatePrm.offsetV         = prm->frameHeight;
    vidEncCreatePrm.bitrate         = pChInfo->curBitrate;
    vidEncCreatePrm.mode            = ALG_VID_ENC_MODE_STANDARD;
    vidEncCreatePrm.keyFrameInterval= pChInfo->curKeyFrameInterval;
    vidEncCreatePrm.mbMvOutEnable   = FALSE;
    vidEncCreatePrm.qValue          = 75;

    #ifdef ENCODE_DEBUG
    OSA_printf(" ENCODE: Creating ALG_vidEnc %d\n", i);
    #endif

    pChInfo->algVidEncHndl = ALG_vidEncCreate(&vidEncCreatePrm);
    if(pChInfo->algVidEncHndl==NULL) {
      OSA_ERROR("ALG_vidEncCreate(%d)\n", i);
      goto error_exit;
    }
  }

  #ifdef ENCODE_DEBUG
  OSA_printf(" ENCODE: Create DONE\n");
  #endif

  return OSA_SOK;

error_exit:

  OSA_mutexDelete(&gENCODE_ctrl.mutexLock);

  for(i=0; i<gENCODE_ctrl.createPrm.numCh; i++) {

    pChInfo = &gENCODE_ctrl.chInfo[i];

    if(pChInfo->bufVirtAddr) {
      OSA_cmemFree(pChInfo->bufVirtAddr);

      OSA_bufDelete(&pChInfo->bufHndl);
    }

    if(pChInfo->algVidEncHndl)
      ALG_vidEncDelete(pChInfo->algVidEncHndl);
  }

  return OSA_EFAIL;
}

int ENCODE_tskDelete()
{
  int status=OSA_SOK, i;
  ENCODE_ChInfo *pChInfo;

  OSA_mutexDelete(&gENCODE_ctrl.mutexLock);

  for(i=0; i<gENCODE_ctrl.createPrm.numCh; i++) {

    pChInfo = &gENCODE_ctrl.chInfo[i];

    if(pChInfo->bufVirtAddr) {
      OSA_cmemFree(pChInfo->bufVirtAddr);

      OSA_bufDelete(&pChInfo->bufHndl);
    }

    if(pChInfo->algVidEncHndl)
      ALG_vidEncDelete(pChInfo->algVidEncHndl);
  }

  return status;
}

int ENCODE_tskRun(int chId)
{
  int status=OSA_SOK, inBufId, outBufId;
  OSA_BufInfo *pInBuf, *pOutBuf;
  ENCODE_ChInfo *pChInfo;
  ALG_VidEncRunPrm encPrm;
  ALG_VidEncRunStatus encStatus;

  if(chId<0 || chId >= gENCODE_ctrl.createPrm.numCh)
    return OSA_EFAIL;

  pChInfo = &gENCODE_ctrl.chInfo[chId];

  status = OSA_bufGetFull(&pChInfo->bufHndl, &inBufId, OSA_TIMEOUT_FOREVER);

  if(status==OSA_SOK) {

    pInBuf = OSA_bufGetBufInfo(&pChInfo->bufHndl, inBufId);
    OSA_assert(pInBuf!=NULL);

    if(pChInfo->enableEncode) {

      pOutBuf = WRITER_getEmptyBuf(&outBufId, OSA_TIMEOUT_FOREVER);
      if(pOutBuf!=NULL) {

        OSA_mutexLock(&gENCODE_ctrl.mutexLock);

        if(pChInfo->newBitrate!=pChInfo->curBitrate) {
          ALG_vidEncSetBitrate(pChInfo->algVidEncHndl, pChInfo->newBitrate);

          pChInfo->curBitrate = pChInfo->newBitrate;
        }

        if(pChInfo->newKeyFrameInterval!=pChInfo->curKeyFrameInterval) {
          ALG_vidEncSetKeyFrameInterval(pChInfo->algVidEncHndl, pChInfo->newKeyFrameInterval,1);//add by sxh 1

          pChInfo->curKeyFrameInterval = pChInfo->newKeyFrameInterval;
        }

        OSA_mutexUnlock(&gENCODE_ctrl.mutexLock);

        encPrm.inStartX       = 0;
        encPrm.inStartY       = 0;
        encPrm.inAddr         = pInBuf->virtAddr;
        encPrm.outAddr        = pOutBuf->virtAddr;
        encPrm.mbMvInfo       = NULL;
        encPrm.outDataMaxSize = pChInfo->bufSize/2;

        #ifdef ENCODE_DEBUG_RUNNING
        OSA_printf(" ENCODE: Ch %d InBuf %d OutBuf %d\n", chId, inBufId, outBufId);
        #endif

        OSA_prfBegin(&gENCODE_ctrl.prfEnc);

        status = ALG_vidEncRun(pChInfo->algVidEncHndl, &encPrm, &encStatus);

        OSA_prfEnd(&gENCODE_ctrl.prfEnc, 1);

        #ifdef ENCODE_DEBUG_RUNNING
        OSA_printf(" ENCODE: Ch %d DONE (%d bytes, %d, %d msecs)\n",
          chId,
          encStatus.bytesGenerated,
          encStatus.isKeyFrame,
          pInBuf->timestamp/1000
          );
        #endif

        pOutBuf->size = encStatus.bytesGenerated;
        pOutBuf->timestamp = pInBuf->timestamp;
        pOutBuf->count = encStatus.isKeyFrame;

        if(status!=OSA_SOK) {
          OSA_ERROR("ALG_vidEncRun(%d)\n", chId);
        }

        WRITER_putFullBuf(chId, pOutBuf, outBufId);
      }
    }

    OSA_bufPutEmpty(&pChInfo->bufHndl, inBufId);
  }

  return status;
}

int ENCODE_tskMain(struct OSA_TskHndl *pTsk, OSA_MsgHndl *pMsg, Uint32 curState )
{
  int status, chId;
  Bool done = FALSE, ackMsg=FALSE;;
  Uint16 cmd = OSA_msgGetCmd(pMsg);
  ENCODE_CreatePrm *pCreatePrm = (ENCODE_CreatePrm*)OSA_msgGetPrm(pMsg);

  if( cmd != ENCODE_CMD_CREATE || pCreatePrm==NULL) {
    OSA_tskAckOrFreeMsg(pMsg, OSA_EFAIL);
    return OSA_SOK;
  }

  status = ENCODE_tskCreate( pCreatePrm );

  #ifdef ENCODE_DEBUG
  OSA_printf(" ENCODE: Sending ACK (%d)\n", status);
  #endif

  OSA_tskAckOrFreeMsg(pMsg, status);

  if(status != OSA_SOK) {
    OSA_ERROR("Create\n");
    return OSA_SOK;
  }

  #ifdef ENCODE_DEBUG
  OSA_printf(" ENCODE: Waiting MSG\n");
  #endif

  while(!done)
  {
    status = OSA_tskWaitMsg(pTsk, &pMsg);
    if(status != OSA_SOK) {
      done = TRUE;
      break;
    }

    cmd = OSA_msgGetCmd(pMsg);

    switch(cmd) {
      case ENCODE_CMD_RUN:
        chId = (int)OSA_msgGetPrm(pMsg);
        OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
        ENCODE_tskRun(chId);
        break;

      case ENCODE_CMD_DELETE:
        done = TRUE;
        ackMsg = TRUE;
        break;

      default:
        OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
        break;
    }
  }

  ENCODE_tskDelete();
  if(ackMsg)
    OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);

  return OSA_SOK;
}


int ENCODE_sendCmd(Uint16 cmd, void *prm, Uint16 flags )
{
  return OSA_mbxSendMsg(&gENCODE_ctrl.tskHndl.mbxHndl, &gENCODE_ctrl.mbxHndl, cmd, prm, flags);
}

int ENCODE_create(ENCODE_CreatePrm *prm)
{
  int status;

  memset(&gENCODE_ctrl, 0, sizeof(gENCODE_ctrl));

  status = OSA_tskCreate( &gENCODE_ctrl.tskHndl, ENCODE_tskMain, ENCODE_THR_PRI, ENCODE_STACK_SIZE, 0);

  OSA_assertSuccess(status);

  status = OSA_mbxCreate( &gENCODE_ctrl.mbxHndl);

  OSA_assertSuccess(status);

  #ifdef ENCODE_DEBUG
  OSA_printf(" ENCODE: Sending CMD\n");
  #endif

  status = ENCODE_sendCmd(ENCODE_CMD_CREATE, prm, OSA_MBX_WAIT_ACK );

  #ifdef ENCODE_DEBUG
  OSA_printf(" ENCODE: Received ACK\n");
  #endif

  return status;
}

int ENCODE_delete()
{
  int status;

  status = ENCODE_sendCmd(ENCODE_CMD_DELETE, NULL, OSA_MBX_WAIT_ACK );

  status = OSA_tskDelete( &gENCODE_ctrl.tskHndl );

  OSA_assertSuccess(status);

  status = OSA_mbxDelete( &gENCODE_ctrl.mbxHndl);

  OSA_assertSuccess(status);

  return status;
}

OSA_BufInfo *ENCODE_getEmptyBuf(int chId, int *bufId, int timeout)
{
  int status;

  if(chId<0 || chId >= gENCODE_ctrl.createPrm.numCh)
    return NULL;

  status = OSA_bufGetEmpty(&gENCODE_ctrl.chInfo[chId].bufHndl, bufId, timeout);
  if(status!=OSA_SOK)
    return NULL;

  return OSA_bufGetBufInfo(&gENCODE_ctrl.chInfo[chId].bufHndl, *bufId);
}

int ENCODE_putFullBuf(int chId, OSA_BufInfo *buf, int bufId)
{
  if(chId<0 || chId >= gENCODE_ctrl.createPrm.numCh)
    return OSA_EFAIL;

  OSA_bufPutFull(&gENCODE_ctrl.chInfo[chId].bufHndl, bufId);

  ENCODE_sendCmd(ENCODE_CMD_RUN, (void*)chId, 0);

  return OSA_SOK;
}

int ENCODE_enableCh(int chId, Bool enable)
{
  if(chId >= 0 && chId < gENCODE_ctrl.createPrm.numCh) {
    gENCODE_ctrl.chInfo[chId].enableEncode = enable;
  }

  return OSA_SOK;
}

int ENCODE_setChBitrate(int chId, int bitrate)
{
  if(chId >= 0 && chId < gENCODE_ctrl.createPrm.numCh && bitrate > 0) {
    OSA_mutexLock(&gENCODE_ctrl.mutexLock);
    gENCODE_ctrl.chInfo[chId].newBitrate = bitrate;
    OSA_mutexUnlock(&gENCODE_ctrl.mutexLock);
  }

  return OSA_SOK;
}

int ENCODE_setChKeyFrameInterval(int chId, int keyFrameInterval)
{
  if(chId >= 0 && chId < gENCODE_ctrl.createPrm.numCh && keyFrameInterval > 0  && keyFrameInterval <= 100) {
    OSA_mutexLock(&gENCODE_ctrl.mutexLock);
    gENCODE_ctrl.chInfo[chId].newKeyFrameInterval = keyFrameInterval;
    OSA_mutexUnlock(&gENCODE_ctrl.mutexLock);
  }

  return OSA_SOK;
}
