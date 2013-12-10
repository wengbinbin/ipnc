
#include <mcvip_priv.h>
#include <csl_hndl.h>
#include <osa_cmem.h>

int MCVIP_isifSetParams(MCVIP_Hndl *hndl);

int MCVIP_v4l2Create(MCVIP_Hndl *hndl)
{
  int i, status;
  Uint32 bufSize;
  
  CSL_ccdcEnable(&gCSL_ccdcHndl, FALSE);
  
  MCVIP_tvp5158GetInfo(hndl);
  
#ifdef VANLINK_DVR_DM365_DEBUG
	OSA_printf(" MCVIP_v4l2Create: Begin!!!!\n");
#endif
  bufSize = hndl->v4l2FrameInfo.offsetH * hndl->v4l2FrameInfo.offsetV * 2;

//    OSA_printf(" MCVIP_V4L2: Alloc info: numBuf=%d, bufSize=%d, allocSize=%d\n", 
//	      MCVIP_V4L2_INPUT_BUF_MAX,
//    	  bufSize,
//      	bufSize*MCVIP_V4L2_INPUT_BUF_MAX
//      );  
#ifdef VANLINK_DVR_DM365_DEBUG
	  OSA_printf(" MCVIP_v4l2Create: for(i=0; i<MCVIP_V4L2_INPUT_BUF_MAX; i++) !!!!\n");
#endif

  for(i=0; i<MCVIP_V4L2_INPUT_BUF_MAX; i++) {
//    hndl->v4l2CaptureBufVirtAddr[i] = OSA_cmemAllocCached(bufSize, 32);
    hndl->v4l2CaptureBufVirtAddr[i] = OSA_cmemAlloc(bufSize, 32);
    if(hndl->v4l2CaptureBufVirtAddr[i]==NULL) {
      OSA_ERROR("OSA_cmemAlloc()\n");         
      goto error_exit;
    }
  #ifdef VANLINK_DVR_DM365_DEBUG
	  OSA_printf(" MCVIP_v4l2Create: for(i=0; i<MCVIP_V4L2_INPUT_BUF_MAX; i++) Done!!!!\n");
#endif  
    hndl->v4l2CaptureBufPhysAddr[i] = OSA_cmemGetPhysAddr(hndl->v4l2CaptureBufVirtAddr[i]);
    if(hndl->v4l2CaptureBufPhysAddr[i]==NULL) {
      OSA_ERROR("OSA_cmemGetPhysAddr()\n");        
      goto error_exit;
    }
  #ifdef VANLINK_DVR_DM365_DEBUG
		  OSA_printf(" MCVIP_v4l2Create: hndl->v4l2CaptureBufPhysAddr[i] = OSA_cmemGetPhysAddr(hndl->v4l2CaptureBufVirtAddr[i]);!!!!\n");
#endif  

    #ifdef MCVIP_CLEAR_BUF_DURING_INIT    
    {
      MCVIP_DmaPrm dmaPrm;    
      
      // clear buffer with blank data
      dmaPrm.srcPhysAddr = NULL;
      dmaPrm.srcVirtAddr = NULL;    
      dmaPrm.dstPhysAddr = hndl->v4l2CaptureBufPhysAddr[i];
      dmaPrm.dstVirtAddr = hndl->v4l2CaptureBufVirtAddr[i];
      dmaPrm.srcOffsetH  = 0;   
      dmaPrm.srcOffsetV  = 0;  
      dmaPrm.dstOffsetH  = hndl->v4l2FrameInfo.offsetH;   
      dmaPrm.dstOffsetV  = hndl->v4l2FrameInfo.offsetV;   
      dmaPrm.copyWidth   = hndl->v4l2FrameInfo.offsetH;   
      dmaPrm.copyHeight  = hndl->v4l2FrameInfo.offsetV;   
      dmaPrm.fillValueY  = 0x00800080;
      dmaPrm.fillValueC  = 0x00000000;    
   #ifdef VANLINK_DVR_DM365_DEBUG
		  OSA_printf(" MCVIP_v4l2Create: MCVIP_dmaRun Begin;!!!!\n");
#endif       
      MCVIP_dmaRun(&dmaPrm);
    }
    #endif    


  }

  status = MCVIP_isifSetParams(hndl);  
  if(status!=OSA_SOK) {
    OSA_ERROR("DRV_isifSetParams()\n");  
    goto error_exit;
  }
  #ifdef VANLINK_DVR_DM365_DEBUG
  OSA_printf(" MCVIP_v4l2:MCVIP_isifSetParams Done!!!!\n");	  //sk_mem
#endif
  MCVIP_tvp5158Setup(hndl);
  
#ifdef VANLINK_DVR_DM365_DEBUG
  OSA_printf(" MCVIP_v4l2:Create Done!!!!\n");	  //sk_mem
#endif

  return OSA_SOK;
        
error_exit:

  for(i=0; i<MCVIP_V4L2_INPUT_BUF_MAX; i++) {
    if(hndl->v4l2CaptureBufVirtAddr[i])
      OSA_cmemFree(hndl->v4l2CaptureBufVirtAddr[i]);
  }

  return OSA_EFAIL;  
}

int MCVIP_v4l2Delete(MCVIP_Hndl *hndl)
{
  int i;
 
  CSL_ccdcIntEnable(&gCSL_ccdcHndl, CSL_CCDC_INT_TYPE_VD0, FALSE);
  CSL_ccdcIntEnable(&gCSL_ccdcHndl, CSL_CCDC_INT_TYPE_VD1, FALSE);
  CSL_ccdcIntClear(&gCSL_ccdcHndl, CSL_CCDC_INT_TYPE_VD0);
  CSL_ccdcIntClear(&gCSL_ccdcHndl, CSL_CCDC_INT_TYPE_VD1);  
  
  for(i=0; i<MCVIP_V4L2_INPUT_BUF_MAX; i++) {
    if(hndl->v4l2CaptureBufVirtAddr[i])
      OSA_cmemFree(hndl->v4l2CaptureBufVirtAddr[i]);
  }
  
  return OSA_SOK;
}

int MCVIP_v4l2Start(MCVIP_Hndl *hndl,int sampleRate)
{
  int status;
  
  status = CSL_ccdcSdramOutEnable(&gCSL_ccdcHndl, TRUE);
  status |= CSL_ccdcBufSwitchEnable(&gCSL_ccdcHndl, TRUE);
  status |= CSL_ccdcEnable(&gCSL_ccdcHndl, TRUE);
  
  hndl->v4l2CaptureCount=-1;
  
  MCVIP_tvp5158Start(hndl,sampleRate);
  
  return status;
}

int MCVIP_v4l2Stop(MCVIP_Hndl *hndl)
{
  int status;
  
  status = CSL_ccdcSdramOutEnable(&gCSL_ccdcHndl, FALSE);
  status |= CSL_ccdcBufSwitchEnable(&gCSL_ccdcHndl, FALSE);
  
  // wait for setting to take effect
  OSA_waitMsecs(100);
    
  status |= CSL_ccdcEnable(&gCSL_ccdcHndl, FALSE);  
  
  MCVIP_tvp5158Stop(hndl);

  return status;
}

int MCVIP_v4l2GetBuf(MCVIP_Hndl *hndl, MCVIP_V4l2Buf *buf)
{
  int status;
  CSL_BufInfo isifBuf;
  
  status = CSL_ccdcBufGetFull(&gCSL_ccdcHndl, &isifBuf, 1, 1000);
  if(status!=OSA_SOK) {
    #ifdef MCVIP_DEBUG  
    OSA_printf(" MCVIP: Timeout, could not dequeue capture buffer\n");
    #endif
    return status;
  }
  
  if(hndl->v4l2CaptureCount>0) {
    if(isifBuf.count!=(hndl->v4l2CaptureCount+1))
    {
      #ifdef MCVIP_DEBUG
      OSA_printf(" WARNING: MCVIP: %d frame(s) dropped (%d .. %d)\n", (isifBuf.count-hndl->v4l2CaptureCount-1), hndl->v4l2CaptureCount, isifBuf.count);
      #endif
      
      hndl->missedFrameCount++;
    }
  }

  hndl->v4l2CaptureCount = isifBuf.count;
  
  buf->id = isifBuf.id;
  buf->virtAddr = hndl->v4l2CaptureBufVirtAddr[isifBuf.id];
  buf->physAddr = hndl->v4l2CaptureBufPhysAddr[isifBuf.id];  
  //buf->timestamp = 0;
  buf->timestamp = isifBuf.timestamp;
  
  return OSA_SOK;
}

int MCVIP_v4l2PutBuf(MCVIP_Hndl *hndl, MCVIP_V4l2Buf *buf)
{
  CSL_BufInfo isifBuf;
  int status;
  
  if(buf->id < 0 || buf->id >= MCVIP_V4L2_INPUT_BUF_MAX)
      return OSA_EFAIL;
    
  isifBuf.id = buf->id;
  
  status = CSL_ccdcBufPutEmpty(&gCSL_ccdcHndl, &isifBuf);
    
  return status;
}

int MCVIP_v4l2SetColorAdjust(MCVIP_Hndl *hndl, int enableNR, int channel)
{
	return MCVIP_tvp5158setColorAdjust(hndl, enableNR, channel);
}

int MCVIP_v4l2SetBrightness(MCVIP_Hndl *hndl, int channel)
{
	return MCVIP_tvp5158SetBrightness(hndl, channel);
}

int MCVIP_v4l2SetContrast(MCVIP_Hndl *hndl, int channel)
{
	return MCVIP_tvp5158SetContrast(hndl, channel);
}

int MCVIP_v4l2SetSaturation(MCVIP_Hndl *hndl, int channel)
{
	return MCVIP_tvp5158SetSaturation(hndl, channel);
}

int MCVIP_v4l2SetHue(MCVIP_Hndl *hndl, int channel)
{
	return MCVIP_tvp5158SetHue(hndl, channel);
}

int MCVIP_v4l2SetNoiseReduction(MCVIP_Hndl *hndl, int channel, int enable)
{
	return MCVIP_tvp5158SetNoiseReduction(hndl, channel, enable);
}

int MCVIP_v4l2SetHybridChSelect(MCVIP_Hndl *hndl, int channel)
{
	return MCVIP_tvp5158SetHybridChSelect(hndl, channel);
}


int MCVIP_isifSetParams(MCVIP_Hndl *hndl)
{
  int status, i;
  CSL_CcdcHwSetup setup;
  CSL_CcdcInDataConfig       inDataConfig;
  CSL_CcdcMiscConfig         miscConfig;
  CSL_CcdcSdramOutConfig     sdrOutConfig;
  CSL_CcdcVdIntConfig        vdIntConfig;
  CSL_CcdcSyncConfig         syncConfig;
  CSL_CcdcRec656Config       rec656Config;
    
  CSL_IpipeifVpifIsifInConfig isifInConfig;
  CSL_IpipeifH3aConfig        h3aConfig;

  CSL_BufInit bufInit;    

  Uint16 vdInt;
 
  status = CSL_ccdcHwReset(&gCSL_ccdcHndl);
  if(status!=OSA_SOK) {
    OSA_ERROR("CSL_ccdcHwReset()\n");
    return status;
  }

  inDataConfig.hLpfEnable         = FALSE;
  inDataConfig.inDataMsbPosition  = CSL_CCDC_MSB_POSITION_BIT15;
  inDataConfig.inDataType         = CSL_CCDC_IN_DATA_TYPE_YUV8;
  inDataConfig.dataPolarity       = CSL_CCDC_DATA_POLARITY_NO_CHANGE;
  inDataConfig.yPos               = CSL_CCDC_Y_POS_ODD;

  miscConfig.cfaPattern                     = CSL_CCDC_CFA_PATTERN_MOSIAC;
  miscConfig.vdLatchDisable                 = TRUE;
  miscConfig.inverseMsbCout                 = FALSE;
  miscConfig.inverseMsbCin                  = FALSE;
  miscConfig.sdramAddrInitExtTiggerEnable   = FALSE;
  miscConfig.sdramAddrInitTiggerSource      = CSL_CCDC_SDR_ADDR_INIT_DWEN;
  miscConfig.orWenIntExt                    = FALSE;
  miscConfig.fidLatchAtVdDisable            = FALSE;
  miscConfig.ycInSwap                       = FALSE;
  miscConfig.ycOutSwap                      = FALSE;

  sdrOutConfig.outDataShift         = 0;
  sdrOutConfig.outStartH            = 0;
  sdrOutConfig.outStartV0           = 0;
  sdrOutConfig.outStartV1           = 0;
  sdrOutConfig.outWidth             = hndl->v4l2FrameInfo.width*2;
  sdrOutConfig.outHeight            = hndl->v4l2FrameInfo.height;
  sdrOutConfig.culHEven             = 0xFF;
  sdrOutConfig.culHOdd              = 0xFF;
  sdrOutConfig.culV                 = 0xFF;
  sdrOutConfig.outLineOffset        = hndl->v4l2FrameInfo.offsetH*2;
  sdrOutConfig.outAddrDecrement     = FALSE;

  sdrOutConfig.sdramOffsetConfig.fidInv               = FALSE;
  sdrOutConfig.sdramOffsetConfig.fidOffsetOdd         = CSL_CCDC_SDR_OFFSET_PLUS_1LINE;
  sdrOutConfig.sdramOffsetConfig.lineOffsetEvenEven   = CSL_CCDC_SDR_OFFSET_PLUS_1LINE;
  sdrOutConfig.sdramOffsetConfig.lineOffsetOddEven    = CSL_CCDC_SDR_OFFSET_PLUS_1LINE;
  sdrOutConfig.sdramOffsetConfig.lineOffsetEvenOdd    = CSL_CCDC_SDR_OFFSET_PLUS_1LINE;
  sdrOutConfig.sdramOffsetConfig.lineOffsetOddOdd     = CSL_CCDC_SDR_OFFSET_PLUS_1LINE;

  sdrOutConfig.outAddr              = hndl->v4l2CaptureBufPhysAddr[0];
  sdrOutConfig.alawEnable           = FALSE;
  sdrOutConfig.byteSwapEnable       = FALSE;

  sdrOutConfig.packMode             = CSL_CCDC_SDR_OUT_TYPE_8BITS_PER_PIXEL;

  vdInt = ((Uint32)hndl->v4l2FrameInfo.height - (hndl->v4l2FrameInfo.height/30));

  vdIntConfig.vdInt0  = vdInt;
  vdIntConfig.vdInt1  = vdInt;
  vdIntConfig.vdInt2  = vdInt;
  
  syncConfig.interlaceMode = FALSE;
  syncConfig.wenUseEnable  = FALSE;
  syncConfig.fidPolarity   = CSL_CCDC_SIGNAL_POLARITY_POSITIVE;
  syncConfig.hdPolarity    = CSL_CCDC_SIGNAL_POLARITY_POSITIVE;
  syncConfig.vdPolarity    = CSL_CCDC_SIGNAL_POLARITY_NEGATIVE;
  syncConfig.hdVdDir       = CSL_CCDC_SIGNAL_DIR_INPUT;
  syncConfig.fidDir        = CSL_CCDC_SIGNAL_DIR_INPUT;
  syncConfig.hdWidth       = 0;
  syncConfig.vdWidth       = 0;
  syncConfig.pixelsPerLine = 0;
  syncConfig.linesPerFrame = 0;
  
  rec656Config.enable             = TRUE;
  rec656Config.errorCorrectEnable = FALSE;
  rec656Config.dataWidth          = CSL_CCDC_REC656_DATA_WIDTH_8BIT;
  
  setup.enable                    = FALSE;
  setup.sdramOutEnable            = FALSE;
  setup.bypassModuleIfNullConfig  = FALSE;
  setup.inDataConfig              = &inDataConfig;
  setup.syncConfig                = &syncConfig;
  setup.miscConfig                = &miscConfig;
  setup.colPatConfig              = NULL;
  setup.linerizationConfig        = NULL;
  setup.fmtInFrameConfig          = NULL;
  setup.fmtConfig                 = NULL;
  setup.cscConfig                 = NULL;
  setup.clampConfig               = NULL;
  setup.dfcLscOffsetConfig        = NULL;
  setup.dfcConfig                 = NULL;
  setup.lscConfig                 = NULL;
  setup.gainOffsetConfig          = NULL;
  setup.sdramOutSizeConfig        = NULL;
  setup.sdramOutConfig            = &sdrOutConfig;
  setup.dpcmConfig                = NULL;
  setup.rec656Config              = &rec656Config;
  setup.flashConfig               = NULL;
  setup.vdIntConfig               = &vdIntConfig;

  isifInConfig.dpcEnable      = FALSE;
  isifInConfig.dpcThreshold   = 0;
  
  isifInConfig.wenUseEnable   = FALSE;
  isifInConfig.vdPol          = CSL_IPIPEIF_HDPOLARITY_POSITIVE;
  isifInConfig.hdPol          = CSL_IPIPEIF_VDPOLARITY_POSITIVE;

  h3aConfig.pixelDecimationEnable = FALSE;
  h3aConfig.pixelDecimationRatio  = 16;
  h3aConfig.avgFilterEnable       = FALSE;
  h3aConfig.alignHsyncVsync       = FALSE;
  h3aConfig.initReszPosH          = 0;

  status = CSL_ccdcHwSetup(&gCSL_ccdcHndl, &setup);
  if(status!=OSA_SOK) {
    OSA_ERROR("CSL_ccdcHwSetup()\n");
    return status;
  }

  status = CSL_ipipeifSetVpifIsifInConfig(&gCSL_ipipeifHndl, &isifInConfig);
  if(status!=OSA_SOK) {
    OSA_ERROR("CSL_ipipeifSetVpifIsifInConfig()\n");
    return status;
  }

  status = CSL_ipipeifSetH3aConfig(&gCSL_ipipeifHndl, &h3aConfig);
  if(status!=OSA_SOK) {
    OSA_ERROR("CSL_ipipeifSetH3aConfig()\n");
    return status;
  }

  status = CSL_ipipeifSetInputSource1(&gCSL_ipipeifHndl, CSL_IPIPEIF_INPUT_SOURCE_PARALLEL_PORT_RAW);
  if(status!=OSA_SOK) {
    OSA_ERROR("CSL_ipipeifSetInputSource1()\n");
    return status;
  }

  bufInit.numBuf = MCVIP_V4L2_INPUT_BUF_MAX;
  bufInit.curBuf = 0;

  for(i=0; i<bufInit.numBuf; i++) {  
    bufInit.bufAddr[i] = (Uint32)hndl->v4l2CaptureBufPhysAddr[i];  
  }

  status = CSL_ccdcBufInit(&gCSL_ccdcHndl, &bufInit);
  if(status!=OSA_SOK) {
    OSA_ERROR("CSL_ccdcBufInit()\n");  
    return status;
  }  

  gCSL_vpssHndl.isifRegs->FMTLNH = hndl->v4l2FrameInfo.width;
  
  CSL_ccdcIntClear(&gCSL_ccdcHndl, CSL_CCDC_INT_TYPE_VD0);
  CSL_ccdcIntClear(&gCSL_ccdcHndl, CSL_CCDC_INT_TYPE_VD1);  
 
  CSL_ccdcIntEnable(&gCSL_ccdcHndl, CSL_CCDC_INT_TYPE_VD0, TRUE);

  return status;
}

