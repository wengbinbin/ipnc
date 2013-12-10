

#include <mcvip_priv.h>
#include <drv_tvp5158.h>
 
int MCVIP_tvp5158GetInfo(MCVIP_Hndl *hndl)
{
  strcpy(hndl->v4l2InputToUse, "TVP5158");

  switch(hndl->createPrm.videoDecoderMode) {
    case MCVIP_VIDEO_DECODER_MODE_2CH_D1_PORT_A:
    case MCVIP_VIDEO_DECODER_MODE_2CH_D1_PORT_B:
    case MCVIP_VIDEO_DECODER_MODE_2CH_D1_2CH_CIF:	//sk_cif
      hndl->v4l2StdToUse = TVP5158_STD_NTSC_06;

      hndl->v4l2FrameInfo.width = 728;
      hndl->v4l2FrameInfo.height = 1050;
      if(hndl->createPrm.videoSystem == MCVIP_VIDEO_SYSTEM_PAL)
        hndl->v4l2FrameInfo.height = 1250;  //sk_pal
      break;

    case MCVIP_VIDEO_DECODER_MODE_4CH_D1:
   	case MCVIP_VIDEO_DECODER_MODE_4CH_D1_4CH_CIF_A:	//sk_cif
	case MCVIP_VIDEO_DECODER_MODE_4CH_D1_4CH_CIF_B:    	    	
      if(hndl->createPrm.videoIfMode==MCVIP_VIDEO_IF_MODE_BT656) {
        hndl->v4l2StdToUse = TVP5158_STD_NTSC_12;
      } else {
        hndl->v4l2StdToUse = TVP5158_STD_NTSC_16;
      }

      hndl->v4l2FrameInfo.width = 728;
      hndl->v4l2FrameInfo.height = 2100;
      if(hndl->createPrm.videoSystem == MCVIP_VIDEO_SYSTEM_PAL)
        hndl->v4l2FrameInfo.height = 2500;
      break;

    case MCVIP_VIDEO_DECODER_MODE_4CH_HALF_D1:
      if(hndl->createPrm.videoIfMode==MCVIP_VIDEO_IF_MODE_BT656) {
        hndl->v4l2StdToUse = TVP5158_STD_NTSC_14;
      } else {
        hndl->v4l2StdToUse = TVP5158_STD_NTSC_18;
      }

      hndl->v4l2FrameInfo.width = 368;
      hndl->v4l2FrameInfo.height = 2100;
      if(hndl->createPrm.videoSystem == MCVIP_VIDEO_SYSTEM_PAL)
        hndl->v4l2FrameInfo.height = 2500;
      break;

    case MCVIP_VIDEO_DECODER_MODE_4CH_CIF:
      hndl->v4l2StdToUse = TVP5158_STD_NTSC_15;

      hndl->v4l2FrameInfo.width = 368;
      hndl->v4l2FrameInfo.height = 1052;
      if(hndl->createPrm.videoSystem == MCVIP_VIDEO_SYSTEM_PAL)
        hndl->v4l2FrameInfo.height = 1252;
      break;

    case MCVIP_VIDEO_DECODER_MODE_8CH_HALF_D1:
      hndl->v4l2StdToUse = TVP5158_STD_NTSC_23;

      hndl->v4l2FrameInfo.width = 368;
      hndl->v4l2FrameInfo.height = 4093;
      if(hndl->createPrm.videoSystem == MCVIP_VIDEO_SYSTEM_PAL)
        hndl->v4l2FrameInfo.height = 4093;
      break;

    case MCVIP_VIDEO_DECODER_MODE_8CH_CIF:
      hndl->v4l2StdToUse = TVP5158_STD_NTSC_21;

      hndl->v4l2FrameInfo.width = 368;
      hndl->v4l2FrameInfo.height = 2104;
      if(hndl->createPrm.videoSystem == MCVIP_VIDEO_SYSTEM_PAL)
        hndl->v4l2FrameInfo.height = 2504;
      break;

    case MCVIP_VIDEO_DECODER_MODE_4CH_HALF_D1_PLUS_D1:
      hndl->v4l2StdToUse = TVP5158_STD_NTSC_24;

      hndl->v4l2FrameInfo.width = 368;
      hndl->v4l2FrameInfo.height = 3150;
      if(hndl->createPrm.videoSystem == MCVIP_VIDEO_SYSTEM_PAL)
        hndl->v4l2FrameInfo.height = 3750;
      break;

    case MCVIP_VIDEO_DECODER_MODE_4CH_CIF_PLUS_D1:
      hndl->v4l2StdToUse = TVP5158_STD_NTSC_19;

      hndl->v4l2FrameInfo.width = 368;
      hndl->v4l2FrameInfo.height = 2102;
      if(hndl->createPrm.videoSystem == MCVIP_VIDEO_SYSTEM_PAL)
        hndl->v4l2FrameInfo.height = 2502;
      break;

    case MCVIP_VIDEO_DECODER_MODE_8CH_CIF_PLUS_D1:
      hndl->v4l2StdToUse = TVP5158_STD_NTSC_22;

      hndl->v4l2FrameInfo.width = 368;
      hndl->v4l2FrameInfo.height = 3154;
      if(hndl->createPrm.videoSystem == MCVIP_VIDEO_SYSTEM_PAL)
        hndl->v4l2FrameInfo.height = 3754;
      break;

    default:
      return OSA_EFAIL;
  }

  hndl->v4l2FrameInfo.offsetH = OSA_align(hndl->v4l2FrameInfo.width, 32);
  hndl->v4l2FrameInfo.offsetV = hndl->v4l2FrameInfo.height;

  return OSA_SOK;
}

int MCVIP_tvp5158Start(MCVIP_Hndl *hndl,int sampleRate)
{
  int status=OSA_SOK, i;

  if(hndl->pI2cHndl[0]!=NULL) {

    // if cascade mode, init the second TVP5158
    switch(hndl->createPrm.videoDecoderMode) {

      case MCVIP_VIDEO_DECODER_MODE_8CH_HALF_D1:
      case MCVIP_VIDEO_DECODER_MODE_8CH_CIF:
      case MCVIP_VIDEO_DECODER_MODE_8CH_CIF_PLUS_D1:

        if(hndl->pI2cHndl[1]==NULL)
          return OSA_EFAIL;

        OSA_printf(" MCVIP: TVP5158 init in progress for stage 1\n");

        status = TVP5158_setOfmMode(
                hndl->pI2cHndl[1],
                hndl->createPrm.videoDecoderMode,
                hndl->createPrm.videoIfMode,
                hndl->createPrm.videoSystem,
                1
              );
              
        // AYK - 0731 start
        OSA_printf("TVP5158_setAudioModei:sampleRate:%d\n",sampleRate);

        status = TVP5158_setAudioMode(hndl->pI2cHndl[1], hndl->createPrm.videoDecoderMode,1,sampleRate);
		if(status != OSA_SOK)
		{
		  return status;
		}
        // AYK - 0731 end

        break;
    }

    OSA_printf(" MCVIP: TVP5158 init in progress for stage 0\n");

    status = TVP5158_setOfmMode(
                hndl->pI2cHndl[0],
                hndl->createPrm.videoDecoderMode,
                hndl->createPrm.videoIfMode,
                hndl->createPrm.videoSystem,
                0
              );
#if 0//ndef VANLINK_DM365_DVR_AUDIO_DISABLED
    // AYK - 0731 start
    status = TVP5158_setAudioMode(hndl->pI2cHndl[0], hndl->createPrm.videoDecoderMode,0,sampleRate);
    if(status!=OSA_SOK)
    {
	  return status;
    }
    // AYK - 0731 end
#endif
    OSA_printf(" MCVIP: TVP5158 init done !!!\n");
  }

  return status;
}


int MCVIP_tvp5158Setup(MCVIP_Hndl *hndl)
{
  #ifdef VANLINK_DVR_DM365
	 TVP5158_softReset(hndl->pI2cHndl[0]);
#else

TVP5158_softReset(hndl->pI2cHndl[0]);
TVP5158_softReset(hndl->pI2cHndl[1]);
#endif
/*TVP5158_registerInit(hndl->pI2cHndl[0]);
TVP5158_registerInit(hndl->pI2cHndl[1]);*/

#ifdef VANLINK_DVR_DM365_DEBUG
	OSA_printf(" MCVIP_tvp5158Setup: Done!!!!\n");	//sk_mem
#endif

  return OSA_SOK;
}

int MCVIP_tvp5158GetAVDOuputCtrlStatus01(MCVIP_Hndl *hndl)
{
	int value;
	
	if(hndl->pI2cHndl[0]!=NULL) {
		value = TVP5158_getAVDoutputCtrlStatus01(hndl->pI2cHndl[0]);
	}
	
	return value;
}

int MCVIP_tvp5158GetChLockStatus(MCVIP_Hndl *hndl)
{
  int i, status=OSA_SOK;
  Uint8 value;

  memset(hndl->chLockStatus, 0, sizeof(hndl->chLockStatus));

  if(hndl->pI2cHndl[0]!=NULL) {

    for(i=0; i<4; i++) {

      value = TVP5158_getStatus(hndl->pI2cHndl[0], i);

      if(value & 0x6)
        hndl->chLockStatus[i]=1;
    }

    // if cascade mode, init the second TVP5158
    switch(hndl->createPrm.videoDecoderMode) {

      case MCVIP_VIDEO_DECODER_MODE_8CH_HALF_D1:
      case MCVIP_VIDEO_DECODER_MODE_8CH_CIF:
      case MCVIP_VIDEO_DECODER_MODE_8CH_CIF_PLUS_D1:

        if(hndl->pI2cHndl[1]==NULL)
          return OSA_EFAIL;

        for(i=0; i<4; i++) {

          value = TVP5158_getStatus(hndl->pI2cHndl[1], i);

          if(value & 0x6)
            hndl->chLockStatus[4+i]=1;
        }

        break;
    }

    if(    hndl->createPrm.videoDecoderMode==MCVIP_VIDEO_DECODER_MODE_4CH_CIF_PLUS_D1
        || hndl->createPrm.videoDecoderMode==MCVIP_VIDEO_DECODER_MODE_8CH_CIF_PLUS_D1
        || hndl->createPrm.videoDecoderMode==MCVIP_VIDEO_DECODER_MODE_4CH_HALF_D1_PLUS_D1
      ) {
      hndl->chLockStatus[hndl->chList.numCh-1] = hndl->chLockStatus[0];
    }

  }

  return status;
}


int MCVIP_tvp5158Stop(MCVIP_Hndl *hndl)
{
  int status=OSA_SOK;

  if(hndl->pI2cHndl[0]!=NULL) {

    status = TVP5158_softReset(hndl->pI2cHndl[0]);
    if(status!=OSA_SOK)
      return status;

    // if cascade mode, init the second TVP5158
    switch(hndl->createPrm.videoDecoderMode) {

      case MCVIP_VIDEO_DECODER_MODE_8CH_HALF_D1:
      case MCVIP_VIDEO_DECODER_MODE_8CH_CIF:
      case MCVIP_VIDEO_DECODER_MODE_8CH_CIF_PLUS_D1:

        if(hndl->pI2cHndl[1]==NULL)
          return OSA_EFAIL;

        status = TVP5158_softReset(hndl->pI2cHndl[1]);
        if(status!=OSA_SOK)
          return status;

        break;
    }
    
  }

  return status;
}

MCVIP_tvp5158setColorAdjust(MCVIP_Hndl *hndl, int enableNR, int channel)
{
	int status = OSA_SOK, stage = 0, chNum;

	chNum = channel;
	if(channel >= 4){
		stage = 1;
		chNum = channel-4;
	}
	
	OSA_printf(" MCVIP: TVP5158 set ColorAdjust Ch[%d] Br[%d] Cont[%d] Sat[%d] Hue[%d] NR[%d] for stage %d\r\n", 
				channel,
				hndl->createPrm.videoBrightness[channel],	
				hndl->createPrm.videoContrast[channel],
				hndl->createPrm.videoSaturation[channel],
				hndl->createPrm.videoHue[channel],
				enableNR, stage);

	if(enableNR == 0)
		enableNR = 0x09;
	if(enableNR == 1)
		enableNR = 0x08;

	status = TVP5158_setColorAdjust(hndl->pI2cHndl[stage],
									hndl->createPrm.videoBrightness[channel],
									hndl->createPrm.videoContrast[channel],
									hndl->createPrm.videoSaturation[channel],
									hndl->createPrm.videoHue[channel],
									enableNR, chNum);

	if(status != OSA_SOK)
		return status;

	OSA_printf(" MCVIP: TVP5158 set ColorAdjust done !!!\r\n");

	return status;

	
}

int MCVIP_tvp5158SetBrightness(MCVIP_Hndl *hndl, int channel)
{
  int status=OSA_SOK, chNum;

	if(channel >= 4){
    if(hndl->pI2cHndl[1]==NULL)
      return OSA_EFAIL;

    OSA_printf(" MCVIP: TVP5158 set Brightness[%d] for stage 1\n",hndl->createPrm.videoBrightness[channel]);
            
		chNum = channel-4;
    status = TVP5158_setBrightness(hndl->pI2cHndl[1], hndl->createPrm.videoBrightness[channel], chNum);

    if(status!=OSA_SOK)
      return status;   
	}
	else{
    if(hndl->pI2cHndl[0]==NULL)
      return OSA_EFAIL;

    OSA_printf(" MCVIP: TVP5158 set Brightness[%d] for stage 0\n",hndl->createPrm.videoBrightness[channel]);
            
    status = TVP5158_setBrightness(hndl->pI2cHndl[0], hndl->createPrm.videoBrightness[channel], channel);

    if(status!=OSA_SOK)
      return status;   
  }
  
  OSA_printf(" MCVIP: TVP5158 set Brightness[%d] done !!!\n",hndl->createPrm.videoBrightness[channel]);      

  return status;
}

int MCVIP_tvp5158SetContrast(MCVIP_Hndl *hndl, int channel)
{
  int status=OSA_SOK, chNum;

	if(channel >= 4){
    if(hndl->pI2cHndl[1]==NULL)
      return OSA_EFAIL;

    OSA_printf(" MCVIP: TVP5158 set Contrast[%d] for stage 1\n",hndl->createPrm.videoContrast[channel]);
            
		chNum = channel-4;
    status = TVP5158_setContrast(hndl->pI2cHndl[1], hndl->createPrm.videoContrast[channel], chNum);

    if(status!=OSA_SOK)
      return status;   
	}
	else{
    if(hndl->pI2cHndl[0]==NULL)
      return OSA_EFAIL;

    OSA_printf(" MCVIP: TVP5158 set Contrast[%d] for stage 0\n",hndl->createPrm.videoContrast[channel]);
            
    status = TVP5158_setContrast(hndl->pI2cHndl[0], hndl->createPrm.videoContrast[channel], channel);

    if(status!=OSA_SOK)
      return status;   
  }
  
  OSA_printf(" MCVIP: TVP5158 set Contrast[%d] done !!!\n",hndl->createPrm.videoContrast[channel]);      

  return status;  	
}

int MCVIP_tvp5158SetSaturation(MCVIP_Hndl *hndl, int channel)
{
  int status=OSA_SOK, chNum;

	if(channel >= 4){
    if(hndl->pI2cHndl[1]==NULL)
      return OSA_EFAIL;

    OSA_printf(" MCVIP: TVP5158 set Saturation[%d] for stage 1\n",hndl->createPrm.videoSaturation[channel]);
            
		chNum = channel-4;
    status = TVP5158_setSaturation(hndl->pI2cHndl[1], hndl->createPrm.videoSaturation[channel], chNum);

    if(status!=OSA_SOK)
      return status;   
	}
	else{
    if(hndl->pI2cHndl[0]==NULL)
      return OSA_EFAIL;

    OSA_printf(" MCVIP: TVP5158 set Saturation[%d] for stage 0\n",hndl->createPrm.videoSaturation[channel]);
            
    status = TVP5158_setSaturation(hndl->pI2cHndl[0], hndl->createPrm.videoSaturation[channel], channel);

    if(status!=OSA_SOK)
      return status;   
  }
  
  OSA_printf(" MCVIP: TVP5158 set Saturation[%d] done !!!\r\n",hndl->createPrm.videoSaturation[channel]);      

  return status;  		
}

int MCVIP_tvp5158SetHue(MCVIP_Hndl *hndl, int channel)
{
  int status=OSA_SOK, chNum;

	if(channel >= 4){
    if(hndl->pI2cHndl[1]==NULL)
      return OSA_EFAIL;

    OSA_printf(" MCVIP: TVP5158 set Hue[%d] for stage 1\r\n",hndl->createPrm.videoHue[channel]);
            
		chNum = channel-4;
    status = TVP5158_setHue(hndl->pI2cHndl[1], hndl->createPrm.videoHue[channel], chNum);

    if(status!=OSA_SOK)
      return status;   
	}
	else{
    if(hndl->pI2cHndl[0]==NULL)
      return OSA_EFAIL;

    OSA_printf(" MCVIP: TVP5158 set Hue[%d] for stage 0\r\n",hndl->createPrm.videoHue[channel]);
            
    status = TVP5158_setHue(hndl->pI2cHndl[0], hndl->createPrm.videoHue[channel], channel);

    if(status!=OSA_SOK)
      return status;   
  }
  
  OSA_printf(" MCVIP: TVP5158 set Hue[%d] done !!!\r\n",hndl->createPrm.videoHue[channel]);      

  return status;  	
}

int MCVIP_tvp5158SetHybridChSelect(MCVIP_Hndl *hndl, int channel)
{
	int status=OSA_SOK;
	
	if(hndl->pI2cHndl[0]==NULL)
		return OSA_EFAIL;
		
	status = TVP5158_setHybridChSelect(hndl->pI2cHndl[0], channel);
	
	return status;
}

int MCVIP_tvp5158SetNoiseReduction(MCVIP_Hndl *hndl, int channel, int enable)
{
	int status=OSA_SOK, stage=0, chNum;

	chNum = channel;
	if(channel >= 4){
		stage = 1;
		chNum = channel-4;
	}

	if(hndl->pI2cHndl[stage]==NULL)
		return OSA_EFAIL;

	OSA_printf(" MCVIP: TVP5158 ch[%d] set NoiseReduction[%d] for stage %d\r\n",channel, enable, stage);

	if(enable == 0)
		enable = 0x09;
	
	if(enable == 1)
		enable = 0x08;
	
	status = TVP5158_setNoiseReduction(hndl->pI2cHndl[stage], enable, chNum);

    if(status!=OSA_SOK)
      return status;   	

  	OSA_printf(" MCVIP: TVP5158 ch[%d] set NoiseReduction[%x] done !!\r\n",channel, enable);

  	return status;	
		
}


int MCVIP_tvp5158SetAudVol(MCVIP_Hndl *hndl, Uint8 channel)
{
    int status = OSA_SOK,chNum;

    if(channel >= 4)
    {
	    if(hndl->pI2cHndl[1] == NULL)
	    {
		    return OSA_EFAIL;
		}

	    OSA_printf(" MCVIP: TVP5158 set audio volume[%d] for channel[%d]\n",hndl->createPrm.audVol[channel],channel);

	    chNum = channel - 4;
	    status = TVP5158_setAudVol(hndl->pI2cHndl[1],hndl->createPrm.audVol[channel],chNum);

	    if(status != OSA_SOK)
	    {
	        return OSA_EFAIL;
	    }
	}
	else
	{
	    if(hndl->pI2cHndl[0] == NULL)
	    {
		    return OSA_EFAIL;
		}

	    OSA_printf(" MCVIP: TVP5158 set audio volume[%d] for channel[%d]\n",hndl->createPrm.audVol[channel],channel);

	    status = TVP5158_setAudVol(hndl->pI2cHndl[0],hndl->createPrm.audVol[channel],channel);

	    if(status != OSA_SOK)
	    {
	        return OSA_EFAIL;
	    }
	}
}

int MCVIP_tvp5158SetAudMute(MCVIP_Hndl *hndl, Uint8 channel)
{
	int status = OSA_SOK,chNum;

	if(channel >= 4)
	{
		if(hndl->pI2cHndl[1] == NULL)
		  return OSA_EFAIL;

    chNum = channel - 4;
    status = TVP5158_setAudMute(hndl->pI2cHndl[1],hndl->createPrm.audMute[channel],chNum);
    if(status != OSA_SOK)
        return OSA_EFAIL;
	}
	else
	{
		if(hndl->pI2cHndl[0] == NULL)
		  return OSA_EFAIL;

	    status = TVP5158_setAudMute(hndl->pI2cHndl[0],hndl->createPrm.audMute[channel],channel);
	    if(status != OSA_SOK)
        return OSA_EFAIL;
	}
	
	return status;
}

int MCVIP_tvp5158SetSampleRate(MCVIP_Hndl *hndl,int sampleRate)
{
    int status = OSA_SOK;

    OSA_printf(" MCVIP: TVP5158 set sample rate[%d]\n",sampleRate);

    if((hndl->createPrm.videoDecoderMode == MCVIP_VIDEO_DECODER_MODE_8CH_HALF_D1) ||
       (hndl->createPrm.videoDecoderMode == MCVIP_VIDEO_DECODER_MODE_8CH_CIF)     ||
       (hndl->createPrm.videoDecoderMode == MCVIP_VIDEO_DECODER_MODE_8CH_CIF_PLUS_D1))
    {
	    if(hndl->pI2cHndl[1] == NULL)
	    {
			return OSA_EFAIL;
	    }

	    status = TVP5158_setSampleRate(hndl->pI2cHndl[1],sampleRate);

	    if(status != OSA_SOK)
		{
			return OSA_EFAIL;
	    }
	}

    if(hndl->pI2cHndl[0] == NULL)
    {
        return OSA_EFAIL;
    }
    OSA_printf(" MCVIP D1: TVP5158 set sample rate[%d]\n",sampleRate);

	status = TVP5158_setSampleRate(hndl->pI2cHndl[0],sampleRate);

	if(status != OSA_SOK)
	{
		return OSA_EFAIL;
	}

	return OSA_SOK;
}
