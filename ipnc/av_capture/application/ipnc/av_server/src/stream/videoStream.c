#include <avserver.h>
#include <stream.h>
#include <encode.h>
#include <video.h>

GlobalData gbl = { 0, 0, 0, 0, 0, NOSTD };
static int IsDrawDateTime  = 0;

int VIDEO_streamSysInit()
{
  int status, cnt;
  STREAM_SET streamSet;

  pthread_mutex_init(&gbl.mutex, NULL);

  memset(&streamSet, 0, sizeof(streamSet));

#ifdef VANLINK_DVR_STREAM_ENABLE //add by sxh
	  streamSet.ImageWidth  = gVIDEO_Stream_ctrl.createPrm.frameWidth;
	  streamSet.ImageHeight = gVIDEO_Stream_ctrl.createPrm.frameHeight;
	  streamSet.Mpeg4Quality = 3000000;
	  streamSet.JpgQuality   = 3000000;


	streamSet.ImageWidth_Ext[STREAM_EXT_JPG] = streamSet.ImageWidth;
	streamSet.ImageHeight_Ext[STREAM_EXT_JPG] = streamSet.ImageHeight;
	  

	streamSet.ImageWidth_Ext[STREAM_EXT_MP4_CIF] = streamSet.ImageWidth;
	streamSet.ImageHeight_Ext[STREAM_EXT_MP4_CIF] = streamSet.ImageHeight;


	  streamSet.Mem_layout	= MEM_LAYOUT_2;
	  
	#ifdef VANLINK_DVR_DM365_DEBUG //add by sxh
			OSA_printf("----vl stream_init begin!!\n");    
	#endif

	  status = stream_init( stream_get_handle(), &streamSet);
	#ifdef VANLINK_DVR_DM365_DEBUG //add by sxh
			OSA_printf("----vl stream_init Done!!\n");    
	#endif
	  if(status!=OSA_SOK)
	    OSA_ERROR("stream_init()\n");

#else
	  streamSet.ImageWidth  = gAVSERVER_config.encodeConfig[0].cropWidth;
	  streamSet.ImageHeight = gAVSERVER_config.encodeConfig[0].cropHeight;
	  streamSet.Mpeg4Quality = gAVSERVER_config.encodeConfig[0].codecBitrate;
	  streamSet.JpgQuality   = gAVSERVER_config.encodeConfig[1].qValue;

	  switch(gAVSERVER_config.encodeConfig[0].codecType) {
	    case ALG_VID_CODEC_H264:
	    case ALG_VID_CODEC_MPEG4:
	      break;
	    case ALG_VID_CODEC_MJPEG:
	      streamSet.ImageWidth_Ext[STREAM_EXT_JPG] = gAVSERVER_config.encodeConfig[0].cropWidth;
	      streamSet.ImageHeight_Ext[STREAM_EXT_JPG] = gAVSERVER_config.encodeConfig[0].cropHeight;
	      break;

	  }

	  if(gAVSERVER_config.numEncodeStream>1) {
	    switch(gAVSERVER_config.encodeConfig[1].codecType) {
	      case ALG_VID_CODEC_H264:
	      case ALG_VID_CODEC_MPEG4:
	        streamSet.ImageWidth_Ext[STREAM_EXT_MP4_CIF] = gAVSERVER_config.encodeConfig[1].cropWidth;
	        streamSet.ImageHeight_Ext[STREAM_EXT_MP4_CIF] = gAVSERVER_config.encodeConfig[1].cropHeight;
	        break;
	      case ALG_VID_CODEC_MJPEG:
	        streamSet.ImageWidth_Ext[STREAM_EXT_JPG] = gAVSERVER_config.encodeConfig[1].cropWidth;
	        streamSet.ImageHeight_Ext[STREAM_EXT_JPG] = gAVSERVER_config.encodeConfig[1].cropHeight;
	        break;

	    }
	  }

	  if(gAVSERVER_config.numEncodeStream>2) {
	    switch(gAVSERVER_config.encodeConfig[2].codecType) {
	      case ALG_VID_CODEC_H264:
	      case ALG_VID_CODEC_MPEG4:
	        streamSet.ImageWidth_Ext[STREAM_EXT_MP4_CIF] = gAVSERVER_config.encodeConfig[2].cropWidth;
	        streamSet.ImageHeight_Ext[STREAM_EXT_MP4_CIF] = gAVSERVER_config.encodeConfig[2].cropHeight;
	        break;
	      case ALG_VID_CODEC_MJPEG:
	        streamSet.ImageWidth_Ext[STREAM_EXT_JPG] = gAVSERVER_config.encodeConfig[2].cropWidth;
	        streamSet.ImageHeight_Ext[STREAM_EXT_JPG] = gAVSERVER_config.encodeConfig[2].cropHeight;
	        break;

	    }
	  }

	#ifdef AVSERVER_DEBUG_VIDEO_STREAM_THR
	  for( cnt = 0; cnt < STREAM_EXT_NUM ; cnt++ )
	  {
	    OSA_printf(" STREAM: Ext %d: %dx%d\n", cnt, streamSet.ImageWidth_Ext[cnt], streamSet.ImageHeight_Ext[cnt]);
	  }
	#endif

	  streamSet.Mem_layout	= gAVSERVER_config.streamConfig.mem_layou_set;

	  status = stream_init( stream_get_handle(), &streamSet);

	  if(status!=OSA_SOK)
	    OSA_ERROR("stream_init()\n");

#endif
  

  return status;
}

int VIDEO_streamSysExit()
{
  stream_end( stream_get_handle() );

  pthread_mutex_destroy(&gbl.mutex);

  return OSA_SOK;
}

int VIDEO_streamShmCopy(int streamId, OSA_BufInfo *pBufInfo)
{


#ifdef VANLINK_DVR_STREAM_ENABLE
	  int  status=OSA_SOK;
	  int frameType, streamType = -1;
	  Uint32 timestamp;
	  int keyframe;
	  struct tm tm1;//timestamp add by sxh
	  switch(streamId){
		case 0:
			streamType = STREAM_H264_1;
			break;
		case 1:
			streamType = STREAM_H264_1;
			break;
		case 2:
			streamType = STREAM_H264_1;
			break;
		case 3:
			streamType = STREAM_H264_1;
			break;
		default:
			break;
	  }
	  
#ifdef VANLINK_DVR_DM365_DEBUG
	 //OSA_printf("----streamId=%d,pBufInfo->isKeyFrame=%d------ VIDEO_streamShmCopy:  Begin!!!\n",streamId,pBufInfo->isKeyFrame); 
	// OSA_waitMsecs(10);
#endif

	if(pBufInfo->isKeyFrame)
		keyframe=0;
	else
		keyframe=1;
	  switch(pBufInfo->isKeyFrame) {
			case VIDEO_ENC_FRAME_TYPE_KEY_FRAME:
				frameType = 1;
				break;
			case VIDEO_ENC_FRAME_TYPE_NON_KEY_FRAME:
				frameType = 2;
				break;
			default:
				frameType = -1;
				break;
	  }

	  //timestamp = pBufInfo->timestamp;
	  localtime_r(&pBufInfo->timesec, &tm1);//timestamp add by sxh
      timestamp = ((tm1.tm_hour*3600 + tm1.tm_min*60 + tm1.tm_sec)*1000 + pBufInfo->timestamp/1000);
#else

	int  status=OSA_SOK;
	VIDEO_BufHeader *pInBufHeader;
	int frameType, streamType = -1;
	Uint32 timestamp;
	
	pInBufHeader = (VIDEO_BufHeader *)pBufInfo->virtAddr;

	switch(gAVSERVER_config.encodeConfig[streamId].codecType)
	  {
	    case ALG_VID_CODEC_H264:
	      if(streamId==0)
	        streamType = STREAM_H264_1;
	      else
	        streamType = STREAM_H264_2;
	      break;
	    case ALG_VID_CODEC_MPEG4:
	      if(streamId==0)
	        streamType = STREAM_MP4;
	      else
	        streamType = STREAM_MP4_EXT;
	      break;
	    case ALG_VID_CODEC_MJPEG:
	      streamType = STREAM_MJPG;
	      break;
	  }

	  switch(pInBufHeader->encFrameType) {
			case VIDEO_ENC_FRAME_TYPE_KEY_FRAME:
				frameType = 1;
				break;
			case VIDEO_ENC_FRAME_TYPE_NON_KEY_FRAME:
				frameType = 2;
				break;
			default:
				frameType = -1;
				break;
	  }

	  timestamp = pInBufHeader->timestamp;

#endif

#ifdef VANLINK_DVR_DM365_DEBUG
		//OSA_printf(" VIDEO_streamShmCopy:  stream_write Begin!!!\n"); 
		//OSA_waitMsecs(10);
#endif

  #if 1
  //if(streamId==0){//add by sxh  only send the first stream
  status = stream_write(
        pBufInfo->virtAddr,
        pBufInfo->size,
        frameType,
        streamType,
        timestamp,
        stream_get_handle(),
        streamId
      );
 // }
#ifdef VANLINK_DVR_DM365_DEBUG
		//  OSA_printf(" VIDEO_streamShmCopy:  stream_write Done!!!\n"); 
		  //OSA_waitMsecs(10);
#endif

  if(status!=OSA_SOK) {
    OSA_ERROR("stream_write(%d, %d, %d, %d)\n",
        pBufInfo->size,
        frameType,
        streamType,
        timestamp
      );
  }
  #endif

  return status;
}

void SetPtzCmd(int Getcmd)
{

}

void SetDrawDateTimeStatus( int IsDraw )
{
	IsDrawDateTime = IsDraw;

	if(IsDrawDateTime > 0)
		AVSERVER_swosdEnableDateTimeStamp(IsDrawDateTime);
	else
		AVSERVER_swosdEnableDateTimeStamp(FALSE);

}

int SetOsdText(char *strText, int nLength)
{
	return AVSERVER_swosdText(strText, nLength);
}

void SetOsdTextEnable(int enable)
{
	AVSERVER_swosdEnableText(enable);
}

void SetHistogramEnable(int enable)
{
	AVSERVER_histEnable(enable);
}

void SetOsdLogoEnable(int enable)
{
	AVSERVER_swosdEnableLogo(enable);
}

void SetROICfgEnable(int value)
{
	if(value==ROI_FD_CFG){
		AVSERVER_fdROIEnable(1);
	}
	else if(value==ROI_CENTER_CFG){
		AVSERVER_fxdROIEnable(1);
	}
	else {
		AVSERVER_fdROIEnable(0);
		AVSERVER_fxdROIEnable(0);
	}
}

/**
 * @brief	Check if Draw DateTime on image
 * @param	none
 * @return	IsDrawDateTime  : 1: draw 0: not draw
 */
int GetDrawDateTimeStatus(void)
{
	return IsDrawDateTime;

}

int SetDispInterface(int type)
{
	AVSERVER_SetDisplay(type);
	return 0;
}

void video_frameRate_setparm( int streamId, unsigned int frameRate )
{
	AVSERVER_setEncFramerate(streamId, frameRate);
}

void jpeg_quality_setparm( int quality )
{
  int streamId;

  for(streamId=0; streamId<gAVSERVER_config.numEncodeStream; streamId++) {
    if(gAVSERVER_config.encodeConfig[streamId].codecType==ALG_VID_CODEC_MJPEG) {
      AVSERVER_setEncBitrate(streamId, quality);
    }
  }
}


void video_bitRate_setparm( int type, int bitrate )
{
  int streamId=type;

  if(gAVSERVER_config.encodeConfig[streamId].codecType!=ALG_VID_CODEC_MJPEG) {
     AVSERVER_setEncBitrate(streamId, bitrate);
  }
  else {
     AVSERVER_setEncBitrate(streamId+1, bitrate);
  }
}
void Motion_setparm(ApproMotionPrm* pMotionPrm)
{
	AVSERVER_setMotion(pMotionPrm->bMotionEnable, pMotionPrm->bMotionCEnale,
		 pMotionPrm->MotionCValue, pMotionPrm->MotionLevel, pMotionPrm->MotionBlock);
}

int VIDEO_streamGetJPGSerialNum(void)
{
	return stream_get_handle()->MemInfo.video_info[VIDOE_INFO_MJPG].cur_serial;

}

void VIDEO_streamSetVNFStatus(unsigned int streamId, unsigned int value)
{
	if(value&FFLAG_SNF)
		AVSERVER_snfEnable(streamId, 1);
	else
		AVSERVER_snfEnable(streamId, 0);

	if(value&FFLAG_TNF)
		AVSERVER_tnfEnable(streamId, 1);
	else
		AVSERVER_tnfEnable(streamId, 0);
}

void VIDEO_streamSetFace( FaceDetectParam *faceParam )
{
	DRV_FaceDetectRunType faceType;

	faceType.type = FACE_T_NO_DETECT;

	/* Disable Face Detect for 5MP */
	if(gAVSERVER_config.sensorMode==DRV_IMGS_SENSOR_MODE_2592x1920)
	{
		AVSERVER_faceDetectEnable(DISABLE);
		AVSERVER_pMaskEnable(DISABLE);
		AVSERVER_faceRecogClear();
 		AVSERVER_faceTrackerEnable(DISABLE);
	}
	else {
		if(faceParam->pmask)
		{
			faceType.type = FACE_T_MASK;
			AVSERVER_pMaskEnable(ENABLE);
		}
		else if(faceParam->fdetect)
		{
			faceType.type = FACE_T_DETECT;
			AVSERVER_faceDetectEnable(ENABLE);
		}
		else {
			AVSERVER_faceDetectEnable(DISABLE);
			AVSERVER_pMaskEnable(DISABLE);
		}

		if(faceParam->frecog == FACE_RECOGIZE ){
			faceType.type = FACE_T_RECOGIZE;
			AVSERVER_faceRecogIdentifyEnable(ENABLE);
		}
		else if(faceParam->frecog == FACE_REGUSER) {
			faceType.type = FACE_T_REGUSER;
			AVSERVER_faceRegUsrEnable(ENABLE);
		}
		else if(faceParam->frecog == FACE_DELUSER){
			faceType.type = FACE_T_DELUSER;
			AVSERVER_faceRegUsrDelete(ENABLE);
		}
		else {
			AVSERVER_faceRecogClear();
		}

 		AVSERVER_faceTrackerEnable(gAVSERVER_config.faceDetectConfig.fdEnable);
	}

	gAVSERVER_config.faceDetectConfig.startX = faceParam->startX;
    gAVSERVER_config.faceDetectConfig.startY = faceParam->startY;
    gAVSERVER_config.faceDetectConfig.width = faceParam->width;
    gAVSERVER_config.faceDetectConfig.height = faceParam->height;
    gAVSERVER_config.faceDetectConfig.fdconflevel = faceParam->fdconflevel;
    gAVSERVER_config.faceDetectConfig.fddirection =	faceParam->fddirection;
    gAVSERVER_config.faceDetectConfig.frconflevel = faceParam->frconflevel;
    gAVSERVER_config.faceDetectConfig.frdatabase = faceParam->frdatabase;
    gAVSERVER_config.faceDetectConfig.maskoption = faceParam->maskoption;

	VIDEO_fdSetFaceType(&faceType);
}

void VIDEO_streamSetDateTimePrm(DateTimePrm* datetimeParam)
{
	int count;

	for(count = 0; count<AVSERVER_MAX_STREAMS;count++) {
		gAVSERVER_config.captureConfig[count].swosdConfig.swosdDateFormat 		= datetimeParam->dateFormat;
		gAVSERVER_config.captureConfig[count].swosdConfig.swosdDatePos 			= datetimeParam->datePos;
		gAVSERVER_config.captureConfig[count].swosdConfig.swosdTimeFormat 		= datetimeParam->timeFormat;
		gAVSERVER_config.captureConfig[count].swosdConfig.swosdTimePos 			= datetimeParam->timePos;
		gAVSERVER_config.captureConfig[count].swosdConfig.swosdDateTimeUpdate 	= OSD_YES;
	}
}

void VIDEO_streamOsdPrm(OSDPrm* osdPrm, int id)
{
	int enable = osdPrm->dateEnable | osdPrm->timeEnable | osdPrm->logoEnable | osdPrm->textEnable | osdPrm->detailedInfo;

	gAVSERVER_config.captureConfig[id].swosdConfig.swosdDateEnable 		= osdPrm->dateEnable;
	gAVSERVER_config.captureConfig[id].swosdConfig.swosdTimeEnable 		= osdPrm->timeEnable;

	gAVSERVER_config.captureConfig[id].swosdConfig.swosdLogoEnable 		= osdPrm->logoEnable;
	gAVSERVER_config.captureConfig[id].swosdConfig.swosdLogoPos 		= osdPrm->logoPos;

	gAVSERVER_config.captureConfig[id].swosdConfig.swosdTextEnable 		= osdPrm->textEnable;
	gAVSERVER_config.captureConfig[id].swosdConfig.swosdTextPos 		= osdPrm->textPos;

	memcpy(gAVSERVER_config.captureConfig[id].swosdConfig.swosdDispString,osdPrm->text,strlen(osdPrm->text));
	gAVSERVER_config.captureConfig[id].swosdConfig.swosdDispString[strlen(osdPrm->text)] = '\0';

	if(osdPrm->logoEnable || osdPrm->textEnable)
		gAVSERVER_config.captureConfig[id].swosdConfig.swosdLogoStringUpdate = OSD_YES;

	gAVSERVER_config.captureConfig[id].swosdConfig.swosdType	= (osdPrm->detailedInfo==1)?SWOSD_DETAIL:SWOSD_BASIC;

	//AVSERVER_swosdEnable(id, enable);
	//AVSERVER_afEnable(osdPrm->detailedInfo);
}

void VIDEO_streamSetOSDEnable(int value)
{
	AVSERVER_swosdEnable(0, (value&1)>>0);
	AVSERVER_swosdEnable(1, (value&2)>>1);
	AVSERVER_swosdEnable(2, (value&4)>>2);
}
void VIDEO_streamMCVIP_SetOSDEnable(int value)
{//value not used here
	int i;
	for(i=0;i<8;i++){
		mcvip_AvServer_swosdEnable(i, TRUE);
	}
}



void VIDEO_aewbSetType(int value)
{
	AVSERVER_aewbSetType(value);
}
void VIDEO_aewbPriority(int value)
{
	AVSERVER_aewbPriority(value);
}
void SetGBCEValue(int enable)
{
	AVSERVER_gbceEnable(enable);
}

int SetSnapName(char *strText, int nLength)
{
	return AVSERVER_snapName(strText, nLength);
}

void SetSnapLocation(int value)
{
	AVSERVER_snapLocation(value);
}

void VIDEO_streamROIPrm(CodecROIPrm* codecROIPrm, int id)
{
	int i =0;

	gAVSERVER_config.encodeConfig[id].numROI 	= codecROIPrm->numROI;

	for(i=0;i<AVSERVER_MAX_STREAMS;i++) {
		gAVSERVER_config.encodeConfig[id].prmROI[i].startx 	= codecROIPrm->roi[i].startx;
		gAVSERVER_config.encodeConfig[id].prmROI[i].starty 	= codecROIPrm->roi[i].starty;
		gAVSERVER_config.encodeConfig[id].prmROI[i].width 	= codecROIPrm->roi[i].width;
		gAVSERVER_config.encodeConfig[id].prmROI[i].height 	= codecROIPrm->roi[i].height;
	}
}

void VIDEO_codecAdvPrm(CodecAdvPrm* codecAdvPrm, int id)
{
	gAVSERVER_config.encodeConfig[id].newCodecPrm	= (Uint16)TRUE;
	gAVSERVER_config.encodeConfig[id].ipRatio 		= (Uint16)codecAdvPrm->ipRatio;
	gAVSERVER_config.encodeConfig[id].fIframe 		= (Uint16)codecAdvPrm->fIframe;
	gAVSERVER_config.encodeConfig[id].qpInit 		= (Uint16)codecAdvPrm->qpInit;
	gAVSERVER_config.encodeConfig[id].qpMin 		= (Uint16)codecAdvPrm->qpMin;
	gAVSERVER_config.encodeConfig[id].qpMax 		= (Uint16)codecAdvPrm->qpMax;
	gAVSERVER_config.encodeConfig[id].encodePreset	= (Uint16)codecAdvPrm->meConfig;
	gAVSERVER_config.encodeConfig[id].packetSize 	= (Uint16)codecAdvPrm->packetSize;
}

void VIDEO_codecReset(int enable)
{
	int count;

	if(enable) {
		for(count = 0; count<AVSERVER_MAX_STREAMS;count++) {
			gAVSERVER_config.encodeConfig[count].resetCodec		= (Uint16)enable;
			gAVSERVER_config.encodeConfig[count].fIframe 		= (Uint16)enable;
		}
	}
}
////////////////////////////////////////////////add by sxh
int VIDEO_streamStartRecChannel(int chSave)
{
    int enable = TRUE ;
	if(chSave >= 0 && chSave < MCVIP_CHANNELS_MAX)
	{
		VIDEO_Stream_enableChSave(chSave, enable);
	}
	
    return TRUE ;
}


int VIDEO_streamStopRecChannel(int chSave)
{
    int disable = FALSE ;
	if(chSave >= 0 && chSave < MCVIP_CHANNELS_MAX){
		
    	VIDEO_Stream_disableChSave(chSave, disable);
    }
    return TRUE ;
}

int VIDEO_streamStartRecChannelEx(int ch,int iAudioEnable,int iEventMode)
{
    int enable = TRUE ;
	if(ch >= 0 && ch < MCVIP_CHANNELS_MAX)
	{
		
		VIDEO_Stream_enableChSaveEx(ch, enable, iAudioEnable, iEventMode);
	}
	
	//printf("@@@@@@@@@ START_REC_CH_EX!! ch:%d status:%d AUDIO:%d EVENT:%d @@@@@@@@@\n", ch, enable, iAudioEnable, iEventMode);	
	
    return TRUE ;
}

int VIDEO_streamStartRecChannelEx1(int ch,int iAudioEnable,int iEventMode, char *camName)
{
    int enable = TRUE ;
	if(ch >= 0 && ch < MCVIP_CHANNELS_MAX)
	{
		
		VIDEO_Stream_enableChSaveEx1(ch, enable, iAudioEnable, iEventMode, camName);
	}
	
	//printf("@@@@@@@@@ START_REC_CH_EX!! ch:%d status:%d AUDIO:%d EVENT:%d CAMNAME:%s @@@@@@@@@\n", ch, enable, iAudioEnable, iEventMode, camName);	
	
    return TRUE ;
}

////////////////////////////////////////////////add by sxh


