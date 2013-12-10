
#include <avserver.h>
#include <osa_cmem.h>
#define YUV_MODE
Uint32 FrameSkipMask[AVSERVER_MAX_FRAMERATE+1] =
{
0x00000000,
0x00000001,0x20004000,0x20080200,0x20404040,0x2082082,
0x21084210,0x01111111,0x11111111,0x24892488,0x24924924,
0x24924949,0x2949494A,0x2949A94A,0x29A9A94A,0x2AAAAAAA,
0x2AAAEAAA,0x2EAAEAAA,0x2B5AD6B5,0x35AD6B5B,0x1B6DB6DB,
0x1BDDB6DB,0x1BDDBDDB,0x3BDDBDDB,0x1EF7BDEF,0x1F7DF7DF,
0x1F7FF7DF,0x1F7FF7FF,0x1FFFF7FF,0x1FFFFFFF,0x3FFFFFFF
};

Uint32 AVSERVER_getFrameSkipMask(int fps)
{
#if 0
 Uint32 frameSkipMask;

  if(fps>=30)
    frameSkipMask = 0x3FFFFFFF;
  else
  if(fps>=24)
    frameSkipMask = (0xF<<0) | (0xF<<5) | (0xF<<10) | (0xF<<15) | (0xF<<20) | (0xF<<25);
  else
  if(fps>=20)
    frameSkipMask = (0x1B<<0) | (0x1B<<6) | (0x1B<<12) | (0x1B<<18) | (0x1B<<24);
  else
  if(fps>=15)
    frameSkipMask = 0x15555555;
  else
  if(fps>=12)
    frameSkipMask = (0xA<<0) | (0xA<<5) | (0xA<<10) | (0xA<<15) | (0xA<<20) | (0xA<<25);
  else
  if(fps>=10)
    frameSkipMask = (0x12<<0) | (0x12<<6) | (0x12<<12) | (0x12<<18) | (0x12<<24);
  else
  if(fps>=8)
    frameSkipMask = 0x031111111;
  else
    frameSkipMask = (0x1<<0) | (0x1<<6) | (0x1<<12) | (0x1<<18) | (0x1<<24);

  return frameSkipMask;

#else

  return FrameSkipMask[fps];

#endif
}

int AVSERVER_bufAlloc()
{
  int status=OSA_SOK;
  OSA_BufHndl *pBufHndl;
  OSA_BufInfo *pBufInfo;
  OSA_BufCreate *pBufCreatePrm;
  int bufId, i, k;
  VIDEO_EncodeStream *pEncodeStream;
  VIDEO_CaptureStream *pCaptureStream;
  Uint32 maxbufSize = 0;
  Uint32 bufSize;

  #ifdef AVSERVER_DEBUG_MAIN_THR
  OSA_printf(" AVSERVER MAIN: Allocating buffers ...\n");
  #endif

  for(i=0; i<gAVSERVER_config.numCaptureStream; i++) {

    pCaptureStream = &gVIDEO_ctrl.captureStream[i];

    bufSize = pCaptureStream->captureOutOffsetH * pCaptureStream->captureOutOffsetV;

    if(gAVSERVER_config.captureYuvFormat == DRV_DATA_FORMAT_YUV420)
      bufSize += bufSize/2;
    else
      bufSize += bufSize;

    bufSize += VIDEO_BUF_HEADER_SIZE;

    pBufHndl      = &pCaptureStream->bufLdcIn;
    pBufCreatePrm = &pCaptureStream->bufLdcInCreatePrm;

    #ifdef AVSERVER_DEBUG_MAIN_THR
    OSA_printf(" AVSERVER MAIN: Stream %d: Allocating LDC buffers: %d of size %d bytes\n", i, pBufCreatePrm->numBuf, bufSize);
    #endif

    if(   gAVSERVER_config.captureRawInMode==AVSERVER_CAPTURE_RAW_IN_MODE_ISIF_IN
      &&  pCaptureStream->captureNextTsk == VIDEO_TSK_LDC
      ) {

      for(k=0; k<pBufCreatePrm->numBuf; k++) {

        pBufInfo = DRV_ipipeGetRszBufInfo(i, k);

        pBufCreatePrm->bufVirtAddr[k] = pBufInfo->virtAddr;
        pBufCreatePrm->bufPhysAddr[k] = pBufInfo->physAddr;
      }

    } else {

      for(k=0; k<pBufCreatePrm->numBuf; k++) {
        pBufCreatePrm->bufVirtAddr[k] = OSA_cmemAlloc(bufSize, 32);
        pBufCreatePrm->bufPhysAddr[k] = OSA_cmemGetPhysAddr(pBufCreatePrm->bufVirtAddr[k]);

        if(pBufCreatePrm->bufVirtAddr[k]==NULL||pBufCreatePrm->bufPhysAddr[k]==NULL) {
          OSA_ERROR("OSA_cmemAlloc()\n");
          return OSA_EFAIL;
        }
      }
    }

    if(pBufCreatePrm->numBuf)
      status |= OSA_bufCreate(pBufHndl, pBufCreatePrm);

    pBufHndl      = &pCaptureStream->bufVnfIn;
    pBufCreatePrm = &pCaptureStream->bufVnfInCreatePrm;

    #ifdef AVSERVER_DEBUG_MAIN_THR
    OSA_printf(" AVSERVER MAIN: Stream %d: Allocating VNF buffers: %d of size %d bytes\n", i, pBufCreatePrm->numBuf, bufSize);
    #endif

    if(   gAVSERVER_config.captureRawInMode==AVSERVER_CAPTURE_RAW_IN_MODE_ISIF_IN
      &&  pCaptureStream->captureNextTsk == VIDEO_TSK_VNF
      ) {

      for(k=0; k<pBufCreatePrm->numBuf; k++) {

        pBufInfo = DRV_ipipeGetRszBufInfo(i, k);

        pBufCreatePrm->bufVirtAddr[k] = pBufInfo->virtAddr;
        pBufCreatePrm->bufPhysAddr[k] = pBufInfo->physAddr;
      }

    } else {

      for(k=0; k<pBufCreatePrm->numBuf; k++) {
        pBufCreatePrm->bufVirtAddr[k] = OSA_cmemAlloc(bufSize, 32);
        pBufCreatePrm->bufPhysAddr[k] = OSA_cmemGetPhysAddr(pBufCreatePrm->bufVirtAddr[k]);

        if(pBufCreatePrm->bufVirtAddr[k]==NULL||pBufCreatePrm->bufPhysAddr[k]==NULL) {
          OSA_ERROR("OSA_cmemAlloc()\n");
          return OSA_EFAIL;
        }
      }
    }
    if(pBufCreatePrm->numBuf)
      status |= OSA_bufCreate(pBufHndl, pBufCreatePrm);

    pBufHndl      = &pCaptureStream->bufEncodeIn;
    pBufCreatePrm = &pCaptureStream->bufEncodeInCreatePrm;

    #ifdef AVSERVER_DEBUG_MAIN_THR
    OSA_printf(" AVSERVER MAIN: Stream %d: Allocating Encode IN buffers: %d of size %d bytes\n", i, pBufCreatePrm->numBuf, bufSize);
    #endif

    if(   gAVSERVER_config.captureRawInMode==AVSERVER_CAPTURE_RAW_IN_MODE_ISIF_IN
      &&  pCaptureStream->captureNextTsk == VIDEO_TSK_ENCODE
      ) {

      for(k=0; k<pBufCreatePrm->numBuf; k++) {

        pBufInfo = DRV_ipipeGetRszBufInfo(i, k);

        pBufCreatePrm->bufVirtAddr[k] = pBufInfo->virtAddr;
        pBufCreatePrm->bufPhysAddr[k] = pBufInfo->physAddr;
      }

    } else {

      for(k=0; k<pBufCreatePrm->numBuf; k++) {
        pBufCreatePrm->bufVirtAddr[k] = OSA_cmemAlloc(bufSize, 32);
        pBufCreatePrm->bufPhysAddr[k] = OSA_cmemGetPhysAddr(pBufCreatePrm->bufVirtAddr[k]);

        if(pBufCreatePrm->bufVirtAddr[k]==NULL||pBufCreatePrm->bufPhysAddr[k]==NULL) {
          OSA_ERROR("OSA_cmemAlloc()\n");
          return OSA_EFAIL;
        }
      }
    }
    if(pBufCreatePrm->numBuf)
      status |= OSA_bufCreate(pBufHndl, pBufCreatePrm);

    pBufHndl      = &pCaptureStream->bufResizeIn;
    pBufCreatePrm = &pCaptureStream->bufResizeInCreatePrm;

    #ifdef AVSERVER_DEBUG_MAIN_THR
    OSA_printf(" AVSERVER MAIN: Stream %d: Assigning Resize IN buffers: %d of size %d bytes\n", i, pBufCreatePrm->numBuf, bufSize);
    #endif

    for(k=0; k<pBufCreatePrm->numBuf; k++) {
      pBufCreatePrm->bufVirtAddr[k] = pCaptureStream->bufEncodeInCreatePrm.bufVirtAddr[k];
      pBufCreatePrm->bufPhysAddr[k] = pCaptureStream->bufEncodeInCreatePrm.bufPhysAddr[k];
    }

    if(pBufCreatePrm->numBuf) {
      status |= OSA_bufCreate(pBufHndl, pBufCreatePrm);
    }

    if(pCaptureStream->bufResizeIn.numBuf) {

      #ifdef AVSERVER_DEBUG_MAIN_THR
      OSA_printf(" AVSERVER MAIN: Stream %d: Getting Encode IN buffers\n", i);
      #endif

      while( OSA_bufGetEmpty(&pCaptureStream->bufEncodeIn, &bufId, OSA_TIMEOUT_NONE) == OSA_SOK) {
        #ifdef AVSERVER_DEBUG_MAIN_THR
        OSA_printf(" AVSERVER MAIN: Stream %d: Encode IN buffers %d\n", i, bufId);
        #endif
      }
    }

    if(gAVSERVER_config.captureRawInMode==AVSERVER_CAPTURE_RAW_IN_MODE_ISIF_IN)
    {
      #ifdef AVSERVER_DEBUG_MAIN_THR
      OSA_printf(" AVSERVER MAIN: Stream %d: Getting Capture OUT buffers\n", i);
      #endif

      while( AVSERVER_bufGetEmpty( VIDEO_TSK_CAPTURE, i, &bufId, OSA_TIMEOUT_NONE) != NULL) {
        #ifdef AVSERVER_DEBUG_MAIN_THR
        OSA_printf(" AVSERVER MAIN: Stream %d: Capture OUT buffers %d\n", i, bufId);
        #endif
      }
    }
  }

  bufSize = OSA_align(gVIDEO_ctrl.faceDetectStream.fdOffsetH, 32)*gVIDEO_ctrl.faceDetectStream.fdHeight*2;
  bufSize += VIDEO_BUF_HEADER_SIZE;

  pBufHndl      = &gVIDEO_ctrl.faceDetectStream.bufFdIn;
  pBufCreatePrm = &gVIDEO_ctrl.faceDetectStream.bufFdInCreatePrm;

  #ifdef AVSERVER_DEBUG_MAIN_THR
  OSA_printf(" AVSERVER MAIN: Allocating FD buffers: %d of size %d bytes\n", pBufCreatePrm->numBuf, bufSize);
  #endif

  for(k=0; k<pBufCreatePrm->numBuf; k++) {
    pBufCreatePrm->bufVirtAddr[k] = OSA_cmemAlloc(bufSize, 32);
    pBufCreatePrm->bufPhysAddr[k] = OSA_cmemGetPhysAddr(pBufCreatePrm->bufVirtAddr[k]);

    if(pBufCreatePrm->bufVirtAddr[k]==NULL||pBufCreatePrm->bufPhysAddr[k]==NULL) {
      OSA_ERROR("OSA_cmemAlloc()\n");
      return OSA_EFAIL;
    }
  }

  if(pBufCreatePrm->numBuf)
    status |= OSA_bufCreate(pBufHndl, pBufCreatePrm);

  pBufHndl      = &gVIDEO_ctrl.displayStream.bufDisplayIn;
  pBufCreatePrm = &gVIDEO_ctrl.displayStream.bufDisplayInCreatePrm;

  #ifdef AVSERVER_DEBUG_MAIN_THR
  OSA_printf(" AVSERVER MAIN: Assigning Display buffers: %d of size %d bytes\n", pBufCreatePrm->numBuf, bufSize);
  #endif

  for(k=0; k<pBufCreatePrm->numBuf; k++) {
    pBufCreatePrm->bufVirtAddr[k] = gVIDEO_ctrl.displayStream.displayInfo.virtAddr[k];
    pBufCreatePrm->bufPhysAddr[k] = gVIDEO_ctrl.displayStream.displayInfo.physAddr[k];
  }

  if(pBufCreatePrm->numBuf)
    status |= OSA_bufCreate(pBufHndl, pBufCreatePrm);

  for(i=0; i<gAVSERVER_config.numEncodeStream; i++) {
    pEncodeStream = &gVIDEO_ctrl.encodeStream[i];

    bufSize = gAVSERVER_config.encodeConfig[i].cropWidth*gAVSERVER_config.encodeConfig[i].cropHeight;

    switch(gAVSERVER_config.encodeConfig[i].codecType) {
      default:
      case ALG_VID_CODEC_MJPEG:
        bufSize /= 1;
        break;

      case ALG_VID_CODEC_H264:
      case ALG_VID_CODEC_MPEG4:
        bufSize /= 2;
        break;
    }

    bufSize += VIDEO_BUF_HEADER_SIZE;

    bufSize = OSA_align(bufSize, KB);

    pBufHndl      = &pEncodeStream->bufEncryptIn;
    pBufCreatePrm = &pEncodeStream->bufEncryptInCreatePrm;

    #ifdef AVSERVER_DEBUG_MAIN_THR
    OSA_printf(" AVSERVER MAIN: Stream %d: Allocating Encrypt buffers: %d of size %d bytes\n", i, pBufCreatePrm->numBuf, bufSize);
    #endif

    for(k=0; k<pBufCreatePrm->numBuf; k++) {
      pBufCreatePrm->bufVirtAddr[k] = OSA_cmemAlloc(bufSize, 32);
      pBufCreatePrm->bufPhysAddr[k] = OSA_cmemGetPhysAddr(pBufCreatePrm->bufVirtAddr[k]);

      if(pBufCreatePrm->bufVirtAddr[k]==NULL||pBufCreatePrm->bufPhysAddr[k]==NULL) {
        OSA_ERROR("OSA_cmemAlloc()\n");
        return OSA_EFAIL;
      }
    }

    if(pBufCreatePrm->numBuf)
      status |= OSA_bufCreate(pBufHndl, pBufCreatePrm);

    pBufHndl      = &pEncodeStream->bufStreamIn;
    pBufCreatePrm = &pEncodeStream->bufStreamInCreatePrm;

    #ifdef AVSERVER_DEBUG_MAIN_THR
    OSA_printf(" AVSERVER MAIN: Stream %d: Allocating Stream buffers: %d of size %d bytes\n", i, pBufCreatePrm->numBuf, bufSize);
    #endif

    for(k=0; k<pBufCreatePrm->numBuf; k++) {
      pBufCreatePrm->bufVirtAddr[k] = OSA_cmemAlloc(bufSize, 32);
      pBufCreatePrm->bufPhysAddr[k] = OSA_cmemGetPhysAddr(pBufCreatePrm->bufVirtAddr[k]);

      if(pBufCreatePrm->bufVirtAddr[k]==NULL||pBufCreatePrm->bufPhysAddr[k]==NULL) {
        OSA_ERROR("OSA_cmemAlloc()\n");
        return OSA_EFAIL;
      }
    }

    if(pBufCreatePrm->numBuf)
      status |= OSA_bufCreate(pBufHndl, pBufCreatePrm);
  }

	maxbufSize = 32;
	for(i=0; i<gAVSERVER_config.numEncodeStream; i++)
	{
		pEncodeStream = &gVIDEO_ctrl.encodeStream[i];
		bufSize = ALG_vidEncGetMVdataSize( pEncodeStream->algEncHndl );
		if( maxbufSize < bufSize )
		{
			maxbufSize = bufSize;
		}
	}
	gVIDEO_ctrl.motionStream.maxbufsize = maxbufSize;
	pBufHndl      = &gVIDEO_ctrl.motionStream.bufMotionIn;
  pBufCreatePrm = &gVIDEO_ctrl.motionStream.bufMotionInCreatePrm;
	for(k=0; k<pBufCreatePrm->numBuf; k++)
	{
    pBufCreatePrm->bufVirtAddr[k] = OSA_memAlloc(maxbufSize);
    pBufCreatePrm->bufPhysAddr[k] = 0;

    if(pBufCreatePrm->bufVirtAddr[k]==NULL)
    {
      OSA_ERROR("OSA_cmemAlloc()\n");
      return OSA_EFAIL;
    }
  }
  if(pBufCreatePrm->numBuf)
  	status |= OSA_bufCreate(pBufHndl, pBufCreatePrm);

  if(status!=OSA_SOK)
    OSA_ERROR("\n");

  #ifdef AVSERVER_DEBUG_MAIN_THR
  OSA_printf(" AVSERVER MAIN: Allocating buffers ...DONE\n");
  #endif

  return status;
}

int AVSERVER_bufFree()
{
  int i, k, status=OSA_SOK;
  VIDEO_EncodeStream *pEncodeStream;
  VIDEO_CaptureStream *pCaptureStream;

  for(i=0; i<gAVSERVER_config.numCaptureStream; i++) {

    pCaptureStream = &gVIDEO_ctrl.captureStream[i];

    if(   gAVSERVER_config.captureRawInMode==AVSERVER_CAPTURE_RAW_IN_MODE_ISIF_IN
      &&  pCaptureStream->captureNextTsk == VIDEO_TSK_LDC
      ) {

    } else {
      for(k=0; k<pCaptureStream->bufLdcInCreatePrm.numBuf; k++)
        OSA_cmemFree(pCaptureStream->bufLdcInCreatePrm.bufVirtAddr[k]);
    }

    if(   gAVSERVER_config.captureRawInMode==AVSERVER_CAPTURE_RAW_IN_MODE_ISIF_IN
      &&  pCaptureStream->captureNextTsk == VIDEO_TSK_VNF
      ) {

    } else {
      for(k=0; k<pCaptureStream->bufVnfInCreatePrm.numBuf; k++)
        OSA_cmemFree(pCaptureStream->bufVnfInCreatePrm.bufVirtAddr[k]);
    }

    if(   gAVSERVER_config.captureRawInMode==AVSERVER_CAPTURE_RAW_IN_MODE_ISIF_IN
      &&  pCaptureStream->captureNextTsk == VIDEO_TSK_ENCODE
      ) {

    } else {
      for(k=0; k<pCaptureStream->bufEncodeInCreatePrm.numBuf; k++)
        OSA_cmemFree(pCaptureStream->bufEncodeInCreatePrm.bufVirtAddr[k]);
    }

    if(pCaptureStream->bufLdcInCreatePrm.numBuf)
      OSA_bufDelete(&pCaptureStream->bufLdcIn);

    if(pCaptureStream->bufVnfInCreatePrm.numBuf)
      OSA_bufDelete(&pCaptureStream->bufVnfIn);

    if(pCaptureStream->bufResizeInCreatePrm.numBuf)
      OSA_bufDelete(&pCaptureStream->bufResizeIn);

    if(pCaptureStream->bufEncodeInCreatePrm.numBuf)
      OSA_bufDelete(&pCaptureStream->bufEncodeIn);
  }

  for(k=0; k<gVIDEO_ctrl.faceDetectStream.bufFdInCreatePrm.numBuf; k++)
    OSA_cmemFree(gVIDEO_ctrl.faceDetectStream.bufFdInCreatePrm.bufVirtAddr[k]);

  if(gVIDEO_ctrl.faceDetectStream.bufFdInCreatePrm.numBuf)
    OSA_bufDelete(&gVIDEO_ctrl.faceDetectStream.bufFdIn);

  if(gVIDEO_ctrl.displayStream.bufDisplayInCreatePrm.numBuf)
    OSA_bufDelete(&gVIDEO_ctrl.displayStream.bufDisplayIn);

  for(i=0; i<gAVSERVER_config.numEncodeStream; i++) {
    pEncodeStream = &gVIDEO_ctrl.encodeStream[i];

    for(k=0; k<pEncodeStream->bufEncryptInCreatePrm.numBuf; k++)
      OSA_cmemFree(pEncodeStream->bufEncryptInCreatePrm.bufVirtAddr[k]);

    for(k=0; k<pEncodeStream->bufStreamInCreatePrm.numBuf; k++)
      OSA_cmemFree(pEncodeStream->bufStreamInCreatePrm.bufVirtAddr[k]);

    if(pEncodeStream->bufEncryptInCreatePrm.numBuf)
      OSA_bufDelete(&pEncodeStream->bufEncryptIn);

    if(pEncodeStream->bufStreamInCreatePrm.numBuf)
      OSA_bufDelete(&pEncodeStream->bufStreamIn);
  }

	for(k=0; k<gVIDEO_ctrl.motionStream.bufMotionInCreatePrm.numBuf; k++)
	{
	  if( gVIDEO_ctrl.motionStream.bufMotionInCreatePrm.bufVirtAddr[k])
	  {
			OSA_memFree(gVIDEO_ctrl.motionStream.bufMotionInCreatePrm.bufVirtAddr[k]);
	  }
	}

	if( gVIDEO_ctrl.motionStream.bufMotionInCreatePrm.numBuf )
		OSA_bufDelete(&(gVIDEO_ctrl.motionStream.bufMotionIn));

  return status;
}


int AVSERVER_tskConnectInit()
{
  int i, status;
  VIDEO_EncodeStream *pEncodeStream;
  VIDEO_CaptureStream *pCaptureStream;

  gVIDEO_ctrl.alawEnable = FALSE;

  if(gAVSERVER_config.captureRawInMode==AVSERVER_CAPTURE_RAW_IN_MODE_ISIF_IN)
  {
    gAVSERVER_config.captureSingleResize = FALSE;

    if(gAVSERVER_config.numCaptureStream > 2)
      gAVSERVER_config.numCaptureStream = 2;
  }

  if(gAVSERVER_config.captureRawInMode==AVSERVER_CAPTURE_RAW_IN_MODE_DDR_IN)
    gVIDEO_ctrl.alawEnable = TRUE;

 #ifdef YUV_MODE
   gVIDEO_ctrl.alawEnable = FALSE;
 #endif

  if(gAVSERVER_config.vstabTskEnable)
  {
    gAVSERVER_config.sensorMode |= DRV_IMGS_SENSOR_MODE_VSTAB;
  }

  if(gAVSERVER_config.vnfTskEnable)
  {
    gAVSERVER_config.sensorMode |= DRV_IMGS_SENSOR_MODE_PIXEL_PAD;
  }

  i = gAVSERVER_config.faceDetectConfig.captureStreamId;

  gVIDEO_ctrl.faceDetectStream.fdWidth  = gAVSERVER_config.captureConfig[i].width;
  gVIDEO_ctrl.faceDetectStream.fdHeight = gAVSERVER_config.captureConfig[i].height;
  gVIDEO_ctrl.faceDetectStream.fdOffsetH = 320;

  if(gVIDEO_ctrl.faceDetectStream.fdWidth>gVIDEO_ctrl.faceDetectStream.fdOffsetH) {
    OSA_printf(" *** WARNING  FD: exceeded max input size for FD, disabling FD (current input %dx%d)!!!\n",
      gVIDEO_ctrl.faceDetectStream.fdWidth, gVIDEO_ctrl.faceDetectStream.fdHeight
      );
    gAVSERVER_config.faceDetectConfig.fdEnable = FALSE;
  }

  memset(&gVIDEO_ctrl.faceDetectStream.faceStatus, 0, sizeof(gVIDEO_ctrl.faceDetectStream.faceStatus));
  memset(&gVIDEO_ctrl.vsStream.vsStatus, 0xFF, sizeof(gVIDEO_ctrl.vsStream.vsStatus));
  memset(&gVIDEO_ctrl.aewbStream.aewbStatus, 0, sizeof(gVIDEO_ctrl.aewbStream.aewbStatus));

  for(i=0; i<gAVSERVER_config.numCaptureStream; i++) {
    pCaptureStream = &gVIDEO_ctrl.captureStream[i];

    pCaptureStream->bufLdcInCreatePrm.numBuf = 0;
    pCaptureStream->bufVnfInCreatePrm.numBuf = 0;
    pCaptureStream->bufResizeInCreatePrm.numBuf = 0;

    pCaptureStream->bufEncodeInCreatePrm.numBuf = VIDEO_NUM_BUF;

    if(gAVSERVER_config.captureSingleResize && i > 0) {

    } else {

      if(gAVSERVER_config.ldcTskEnable)
        pCaptureStream->bufLdcInCreatePrm.numBuf = VIDEO_NUM_BUF;

      if(gAVSERVER_config.vnfTskEnable)
        pCaptureStream->bufVnfInCreatePrm.numBuf = VIDEO_NUM_BUF;

      if(gAVSERVER_config.captureSingleResize) {
        pCaptureStream->bufResizeInCreatePrm.numBuf = pCaptureStream->bufEncodeInCreatePrm.numBuf;
      }
    }
  }

  gVIDEO_ctrl.displayStream.bufDisplayInCreatePrm.numBuf = VIDEO_NUM_BUF;

  gVIDEO_ctrl.faceDetectStream.bufFdInCreatePrm.numBuf = VIDEO_NUM_BUF;

  gVIDEO_ctrl.motionStream.bufMotionInCreatePrm.numBuf = VIDEO_NUM_BUF;

  for(i=0; i<gAVSERVER_config.numEncodeStream; i++) {
    pEncodeStream = &gVIDEO_ctrl.encodeStream[i];

    pEncodeStream->fileSaveState = VIDEO_STREAM_FILE_SAVE_STATE_IDLE;
    pEncodeStream->fileSaveIndex = 0;
    pEncodeStream->fileSaveHndl  = NULL;

    pEncodeStream->bufEncryptInCreatePrm.numBuf = 0;

    if(gAVSERVER_config.encryptTskEnable)
      pEncodeStream->bufEncryptInCreatePrm.numBuf = VIDEO_NUM_BUF;

    pEncodeStream->bufStreamInCreatePrm.numBuf = VIDEO_NUM_BUF;
  }

  status = AVSERVER_tskConnectReset();

  return status;
}

int AVSERVER_tskConnectExit()
{
  int status;

  status = AVSERVER_bufFree();

  return status;
}


int AVSERVER_tskConnectReset()
{
  int i, status=OSA_SOK;
  VIDEO_CaptureStream *pCaptureStream;
  VIDEO_CaptureConfig *pCaptureConfig;
  VIDEO_EncodeStream *pEncodeStream;
  VIDEO_EncodeConfig *pEncodeConfig;

  for(i=0; i<gAVSERVER_config.numCaptureStream; i++) {

    pCaptureStream = &gVIDEO_ctrl.captureStream[i];
    pCaptureConfig = &gAVSERVER_config.captureConfig[i];

    pCaptureStream->captureNextTsk = VIDEO_TSK_NONE;
    pCaptureStream->ldcNextTsk = VIDEO_TSK_NONE;
    pCaptureStream->vnfNextTsk = VIDEO_TSK_NONE;

    pCaptureStream->resizeNextTsk = VIDEO_TSK_ENCODE;

    pCaptureStream->displayCopyTsk = VIDEO_TSK_NONE;
    pCaptureStream->faceDetectCopyTsk = VIDEO_TSK_NONE;
    pCaptureStream->swosdRunTsk = VIDEO_TSK_NONE;
    pCaptureStream->vsApplyTsk = VIDEO_TSK_NONE;

    if( gAVSERVER_config.captureSingleResize && i>0 )
    {
      pCaptureStream->swosdRunTsk = VIDEO_TSK_RESIZE;
      pCaptureStream->displayCopyTsk = VIDEO_TSK_RESIZE;
      pCaptureStream->faceDetectCopyTsk = VIDEO_TSK_RESIZE;

    } else
    {

      if(gAVSERVER_config.ldcTskEnable)
        pCaptureStream->captureNextTsk = VIDEO_TSK_LDC;
      else
      if(gAVSERVER_config.vnfTskEnable)
        pCaptureStream->captureNextTsk = VIDEO_TSK_VNF;
      else
      if(gAVSERVER_config.captureSingleResize)
        pCaptureStream->captureNextTsk = VIDEO_TSK_RESIZE;
      else
        pCaptureStream->captureNextTsk = VIDEO_TSK_ENCODE;

      if(gAVSERVER_config.ldcTskEnable) {
        if(gAVSERVER_config.vnfTskEnable)
          pCaptureStream->ldcNextTsk = VIDEO_TSK_VNF;
        else
        if(gAVSERVER_config.captureSingleResize)
          pCaptureStream->ldcNextTsk = VIDEO_TSK_RESIZE;
        else
          pCaptureStream->ldcNextTsk = VIDEO_TSK_ENCODE;
      }

      if(gAVSERVER_config.vnfTskEnable) {
        if(gAVSERVER_config.captureSingleResize)
          pCaptureStream->vnfNextTsk = VIDEO_TSK_RESIZE;
        else
          pCaptureStream->vnfNextTsk = VIDEO_TSK_ENCODE;
      }

      pCaptureStream->swosdRunTsk = VIDEO_TSK_CAPTURE;
      pCaptureStream->displayCopyTsk = VIDEO_TSK_CAPTURE;
      pCaptureStream->faceDetectCopyTsk = VIDEO_TSK_CAPTURE;
      pCaptureStream->vsApplyTsk = VIDEO_TSK_CAPTURE;

      if(gAVSERVER_config.ldcTskEnable) {
        pCaptureStream->swosdRunTsk = VIDEO_TSK_LDC;
        pCaptureStream->displayCopyTsk = VIDEO_TSK_LDC;
        pCaptureStream->faceDetectCopyTsk = VIDEO_TSK_LDC;
        pCaptureStream->vsApplyTsk = VIDEO_TSK_LDC;
      }
      if(gAVSERVER_config.vnfTskEnable) {
        pCaptureStream->vsApplyTsk = VIDEO_TSK_VNF;
        pCaptureStream->swosdRunTsk = VIDEO_TSK_VNF;
        pCaptureStream->displayCopyTsk = VIDEO_TSK_VNF;
        pCaptureStream->faceDetectCopyTsk = VIDEO_TSK_VNF;
      }

    }
  }

  for(i=0; i<gAVSERVER_config.numEncodeStream; i++) {
    pEncodeStream = &gVIDEO_ctrl.encodeStream[i];
    pEncodeConfig = &gAVSERVER_config.encodeConfig[i];

    pEncodeStream->encodeNextTsk = VIDEO_TSK_STREAM;

    if(gAVSERVER_config.encryptTskEnable)
      pEncodeStream->encodeNextTsk = VIDEO_TSK_ENCRYPT;

  }

  return status;
}

int AVSERVER_bufGetNextTskInfo(int tskId, int streamId, OSA_BufHndl **pBufHndl, OSA_TskHndl **pTskHndl)
{
  VIDEO_CaptureStream *pCaptureStream=NULL;
  VIDEO_EncodeStream  *pEncodeStream=NULL;
  int nextTskId;

  *pTskHndl=NULL;
  *pBufHndl=NULL;

  if(streamId<0 || streamId >= AVSERVER_MAX_STREAMS) {
    OSA_ERROR("Incorrect streamId (%d)\n", streamId);
    return OSA_EFAIL;
  }

  if(tskId <=VIDEO_TSK_NONE || tskId >= VIDEO_TSK_STREAM) {
    OSA_ERROR("Incorrect tskId (%d)\n", tskId);
    return OSA_EFAIL;
  }

  if(tskId <= VIDEO_TSK_RESIZE) {
    pCaptureStream = &gVIDEO_ctrl.captureStream[streamId];

    nextTskId = VIDEO_TSK_NONE;

    if(tskId==VIDEO_TSK_CAPTURE) {
      nextTskId = pCaptureStream->captureNextTsk;
    } else
    if(tskId==VIDEO_TSK_LDC) {
      nextTskId = pCaptureStream->ldcNextTsk;
    } else
    if(tskId==VIDEO_TSK_VNF) {
      nextTskId = pCaptureStream->vnfNextTsk;
    } else
    if(tskId==VIDEO_TSK_RESIZE) {
      nextTskId = pCaptureStream->resizeNextTsk;
    }

    if(nextTskId==VIDEO_TSK_NONE) {
      OSA_ERROR("Incorrect nextTskId (%d)\n", nextTskId);
      return OSA_EFAIL;
    }

    if(nextTskId==VIDEO_TSK_LDC) {
      *pBufHndl = &pCaptureStream->bufLdcIn;
      *pTskHndl = &gVIDEO_ctrl.ldcTsk;
    } else
    if(nextTskId==VIDEO_TSK_VNF) {
      *pBufHndl = &pCaptureStream->bufVnfIn;
      *pTskHndl = &gVIDEO_ctrl.vnfTsk;
    } else
    if(nextTskId==VIDEO_TSK_RESIZE) {
      *pBufHndl = &pCaptureStream->bufResizeIn;
      *pTskHndl = &gVIDEO_ctrl.resizeTsk;
    } else
    if(nextTskId==VIDEO_TSK_ENCODE) {
      *pBufHndl = &pCaptureStream->bufEncodeIn;
      *pTskHndl = &gVIDEO_ctrl.encodeTsk;
    } else {
      OSA_ERROR("Incorrect nextTskId (%d)\n", nextTskId);
      return OSA_EFAIL;
    }

  } else {

    pEncodeStream  = &gVIDEO_ctrl.encodeStream[streamId];

    nextTskId = VIDEO_TSK_NONE;

    if(tskId==VIDEO_TSK_ENCODE) {
      nextTskId = pEncodeStream->encodeNextTsk;
    } else
    if(tskId==VIDEO_TSK_ENCRYPT) {
      nextTskId = VIDEO_TSK_STREAM;
    }

    if(nextTskId==VIDEO_TSK_NONE) {
      OSA_ERROR("Incorrect nextTskId (%d)\n", nextTskId);
      return OSA_EFAIL;
    }

    if(nextTskId==VIDEO_TSK_ENCRYPT) {
      *pBufHndl = &pEncodeStream->bufEncryptIn;
      *pTskHndl = &gVIDEO_ctrl.encryptTsk;
    } else
    if(nextTskId==VIDEO_TSK_STREAM) {
      *pBufHndl = &pEncodeStream->bufStreamIn;
      *pTskHndl = &gVIDEO_ctrl.streamTsk;
    } else {
      OSA_ERROR("Incorrect nextTskId (%d)\n", nextTskId);
      return OSA_EFAIL;
    }
  }

  return OSA_SOK;
}

int AVSERVER_bufGetCurTskInfo(int tskId, int streamId, OSA_BufHndl **pBufHndl)
{
  VIDEO_CaptureStream *pCaptureStream=NULL;
  VIDEO_EncodeStream  *pEncodeStream=NULL;

  *pBufHndl=NULL;

  if(streamId<0 || streamId >= AVSERVER_MAX_STREAMS) {
    OSA_ERROR("Incorrect streamId (%d)\n", streamId);
    return OSA_EFAIL;
  }

  if(tskId <= VIDEO_TSK_CAPTURE || tskId > VIDEO_TSK_STREAM) {
    OSA_ERROR("Incorrect tskId (%d)\n", tskId);
    return OSA_EFAIL;
  }

  if(tskId <= VIDEO_TSK_ENCODE) {
    pCaptureStream = &gVIDEO_ctrl.captureStream[streamId];

    if(tskId==VIDEO_TSK_LDC) {
      *pBufHndl = &pCaptureStream->bufLdcIn;
    } else
    if(tskId==VIDEO_TSK_VNF) {
      *pBufHndl = &pCaptureStream->bufVnfIn;
    } else
    if(tskId==VIDEO_TSK_RESIZE) {
      *pBufHndl = &pCaptureStream->bufResizeIn;
    } else
    if(tskId==VIDEO_TSK_ENCODE) {
      *pBufHndl = &pCaptureStream->bufEncodeIn;
    } else {
      OSA_ERROR("Incorrect tskId (%d)\n", tskId);
      return OSA_EFAIL;
    }

  } else {

    pEncodeStream  = &gVIDEO_ctrl.encodeStream[streamId];

    if(tskId==VIDEO_TSK_ENCRYPT) {
      *pBufHndl = &pEncodeStream->bufEncryptIn;
    } else
    if(tskId==VIDEO_TSK_STREAM) {
      *pBufHndl = &pEncodeStream->bufStreamIn;
    } else {
      OSA_ERROR("Incorrect tskId (%d)\n", tskId);
      return OSA_EFAIL;
    }
  }

  return OSA_SOK;
}

OSA_BufInfo *AVSERVER_bufGetEmpty(int tskId, int streamId, int *bufId, int timeout)
{
  OSA_BufHndl *pBufHndl;
  OSA_TskHndl *pTskHndl;
  int status;

  *bufId = -1;

  status = AVSERVER_bufGetNextTskInfo(tskId, streamId, &pBufHndl, &pTskHndl);
  if(status!=OSA_SOK) {
    OSA_ERROR("AVSERVER_bufGetNextTskInfo(%d, %d)\n", tskId, streamId);
    return NULL;
  }

  status = OSA_bufGetEmpty(pBufHndl, bufId, timeout);
  if(status!=OSA_SOK) {
    //OSA_ERROR("OSA_bufGetEmpty(%d, %d, %d)\n", tskId, streamId, *bufId);
    return NULL;
  }

  return OSA_bufGetBufInfo(pBufHndl, *bufId);
}

OSA_BufInfo *AVSERVER_bufPutFull(int tskId, int streamId, int bufId)
{
  OSA_BufHndl *pBufHndl;
  OSA_TskHndl *pTskHndl;
  int status;

  status = AVSERVER_bufGetNextTskInfo(tskId, streamId, &pBufHndl, &pTskHndl);
  if(status!=OSA_SOK) {
    OSA_ERROR("AVSERVER_bufGetNextTskInfo(%d, %d)\n", tskId, streamId);
    return NULL;
  }

  status = OSA_bufPutFull(pBufHndl, bufId);
  if(status!=OSA_SOK) {
    OSA_ERROR("OSA_bufPutFull(%d, %d, %d)\n", tskId, streamId, bufId);
    return NULL;
  }

  status = OSA_tskSendMsg(pTskHndl, NULL, AVSERVER_CMD_NEW_DATA, (void*)streamId, 0);
  if(status!=OSA_SOK) {
    OSA_ERROR("OSA_tskSendMsg(AVSERVER_CMD_NEW_DATA, %d, %d)\n", tskId, streamId);
    return NULL;
  }

  return OSA_bufGetBufInfo(pBufHndl, bufId);
}

OSA_BufInfo *AVSERVER_bufGetFull(int tskId, int streamId, int *bufId, int timeout)
{
  OSA_BufHndl *pBufHndl;
  int status;

  *bufId = -1;

  status = AVSERVER_bufGetCurTskInfo(tskId, streamId, &pBufHndl);
  if(status!=OSA_SOK) {
    OSA_ERROR("AVSERVER_bufGetCurTskInfo(%d, %d)\n", tskId, streamId);
    return NULL;
  }

  status = OSA_bufGetFull(pBufHndl, bufId, timeout);
  if(status!=OSA_SOK) {
    //OSA_ERROR("OSA_bufGetFull(%d, %d, %d)\n", tskId, streamId, *bufId);
    return NULL;
  }

  return OSA_bufGetBufInfo(pBufHndl, *bufId);
}

OSA_BufInfo *AVSERVER_bufPutEmpty(int tskId, int streamId, int bufId)
{
  OSA_BufHndl *pBufHndl;
  int status;

  status = AVSERVER_bufGetCurTskInfo(tskId, streamId, &pBufHndl);
  if(status!=OSA_SOK) {
    OSA_ERROR("AVSERVER_bufGetCurTskInfo(%d, %d)\n", tskId, streamId);
    return NULL;
  }

  status = OSA_bufPutEmpty(pBufHndl, bufId);
  if(status!=OSA_SOK) {
    OSA_ERROR("OSA_bufPutEmpty(%d, %d, %d)\n", tskId, streamId, bufId);
    return NULL;
  }

  return OSA_bufGetBufInfo(pBufHndl, bufId);
}


