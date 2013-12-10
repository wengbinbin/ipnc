#include <stdlib.h>
#include <string.h>

#include <avserver_ui.h>
#define  YUV_MODE_INTERLACED
#define  YUV_MODE
AVSERVER_UI_Ctrl gAVSERVER_UI_ctrl;

int UI_rtspStreamStart()
{
  OSA_printf(" AVSERVER UI: Starting Streaming Server...\n");
  system("./wis-streamer &");
  OSA_waitMsecs(2000);
  OSA_printf(" AVSERVER UI: Starting Streaming Server...DONE\n");

  return 0;
}

int UI_rtspStreamStop()
{
  OSA_printf(" AVSERVER UI: Stopping Streaming Server...\n");
  system("killall wis-streamer");
  OSA_waitMsecs(3000);
  OSA_printf(" AVSERVER UI: Stopping Streaming Server...DONE\n");

  return 0;
}

int UI_setDone(Bool isDone)
{
  gAVSERVER_UI_ctrl.quit = 1;
  return 0;
}

int UI_isDone()
{
  return gAVSERVER_UI_ctrl.quit;
}

int UI_start()
{
  int status;

  memset(&gAVSERVER_UI_ctrl, 0, sizeof(gAVSERVER_UI_ctrl));

  OSA_attachSignalHandler(SIGINT, UI_signalHandler);

  gAVSERVER_UI_ctrl.mode = gAVSERVER_UI_config.mode;

  OSA_printf("\nAVSERVER UI: Initializing.\n")
  status = AVSERVER_init();
  if(status!=OSA_SOK) {
    OSA_ERROR("AVSERVER_init()\n");
    return status;
  }

  OSA_getHostName(gAVSERVER_UI_ctrl.hostName, sizeof(gAVSERVER_UI_ctrl.hostName));

  UI_setConfig( &gAVSERVER_UI_ctrl.avserverConfig);

  gAVSERVER_UI_ctrl.ldcEnable      = gAVSERVER_UI_ctrl.avserverConfig.captureConfig[0].ldcEnable;
  gAVSERVER_UI_ctrl.tnfEnable      = gAVSERVER_UI_ctrl.avserverConfig.captureConfig[0].tnfEnable;
  gAVSERVER_UI_ctrl.snfEnable      = gAVSERVER_UI_ctrl.avserverConfig.captureConfig[0].snfEnable;
  gAVSERVER_UI_ctrl.vsEnable       = gAVSERVER_UI_ctrl.avserverConfig.captureConfig[0].vsEnable;
  gAVSERVER_UI_ctrl.fdEnable       = gAVSERVER_UI_ctrl.avserverConfig.faceDetectConfig.fdEnable;
  gAVSERVER_UI_ctrl.fileSaveEnable = gAVSERVER_UI_ctrl.avserverConfig.encodeConfig[0].fileSaveEnable;
  gAVSERVER_UI_ctrl.aewbEnable     = gAVSERVER_UI_ctrl.avserverConfig.aewbEnable;
  gAVSERVER_UI_ctrl.afEnable       = gAVSERVER_UI_ctrl.avserverConfig.afEnable;
  gAVSERVER_UI_ctrl.gbceEnable     = gAVSERVER_UI_ctrl.avserverConfig.gbceEnable;
  gAVSERVER_UI_ctrl.aewbType       = gAVSERVER_UI_ctrl.avserverConfig.aewbType;
  gAVSERVER_UI_ctrl.aewbVendor     = gAVSERVER_UI_ctrl.avserverConfig.aewbVendor;
  gAVSERVER_UI_ctrl.winmodeEnable  = gAVSERVER_UI_ctrl.avserverConfig.winmodeEnable;
  gAVSERVER_UI_ctrl.vnfDemoEnable  = gAVSERVER_UI_ctrl.avserverConfig.vnfDemoEnable;
  gAVSERVER_UI_ctrl.swosdEnable    = gAVSERVER_UI_ctrl.avserverConfig.captureConfig[0].swosdConfig.swosdEnable;

  if(gAVSERVER_UI_config.rtspEnable)
    UI_rtspStreamStart();

  status = AVSERVER_start(&gAVSERVER_UI_ctrl.avserverConfig);
  if(status!=OSA_SOK) {
    OSA_ERROR("AVSERVER_start()\n");
    AVSERVER_exit();
    return status;
  }

  return status;
}

int UI_stop()
{
  OSA_printf(" AVSERVER UI: Stoping.\n")
  AVSERVER_stop();

  if(gAVSERVER_UI_config.rtspEnable)
    UI_rtspStreamStop();

  OSA_printf(" AVSERVER UI: Exiting.\n")
  AVSERVER_exit();

  OSA_printf(" AVSERVER UI: Bye Bye !!!\n\n")

  return OSA_SOK;
}

void UI_signalHandler(int signum)
{
  UI_setDone(TRUE);
}

void UI_setConfig(AVSERVER_Config *config)
{
  int i, k, numEncodes, allAdvMode=0;
  int platform = gAVSERVER_UI_config.platform_id;

  numEncodes = gAVSERVER_UI_config.numEncodes;

  memset(config, 0, sizeof(*config));

  config->resizerClkdivN = 50;
  config->displayEnable  = FALSE;
#ifdef YUV_MODE //add by sxh 1117
  config->captureRawInMode  = AVSERVER_CAPTURE_RAW_IN_MODE_DDR_IN;
#else
  config->captureRawInMode	= gAVSERVER_UI_config.saldreEnable?AVSERVER_CAPTURE_RAW_IN_MODE_DDR_IN:AVSERVER_CAPTURE_RAW_IN_MODE_ISIF_IN;
#endif

  config->aewbEnable 	= gAVSERVER_UI_config.aewbEnable;
  config->afEnable 		= gAVSERVER_UI_config.afEnable;
  config->autoFocusVal	= 0;
  config->gbceEnable 	= gAVSERVER_UI_config.gbceEnable;
  config->aewbType   	= gAVSERVER_UI_config.aewbType;
  config->aewbVendor 	= AEWB_NONE;//gAVSERVER_UI_config.aewbVendor; //add by sxh 1115
  config->aewbPriority  = AEWB_FRAMERATE_PRIORITY;
  
  config->fxdROIEnable 					= FALSE;
  config->faceDetectConfig.fdROIEnable 	= FALSE;
  config->newROIconfig  				= FALSE;

  config->snap_config.snapEnable	= FALSE;
  config->snap_config.snapLocation	= FALSE;
  strcpy(config->snap_config.snapName, "snap");

  config->faceDetectConfig.fdTracker			= FALSE;//gAVSERVER_UI_config.fdEnable?TRUE:FALSE;
  config->faceDetectConfig.privacyMaskEnable 	= FALSE;
  config->faceDetectConfig.frIdentify 			= FALSE;
  config->faceDetectConfig.frRegUsr 			= FALSE;
  config->faceDetectConfig.frDelUsr 			= FALSE;

  for(i=0;i<AVSERVER_MAX_STREAMS;i++) {
    config->captureConfig[i].flipH = gAVSERVER_UI_config.flipH;
    config->captureConfig[i].flipV = gAVSERVER_UI_config.flipV;

    config->captureConfig[i].swosdConfig.swosdEnable = gAVSERVER_UI_config.swosdEnable;
    config->captureConfig[i].swosdConfig.swosdType = -1;
    config->captureConfig[i].swosdConfig.swosdLogoStringUpdate = OSD_YES;
    strcpy(config->captureConfig[i].swosdConfig.swosdDispString , "EVALUATION");
    config->captureConfig[i].swosdConfig.swosdDateTimeUpdate = OSD_YES;
  }

  for(i=0;i<numEncodes;i++) {
    config->encodeConfig[i].rateControlType = gAVSERVER_UI_config.codecRateControlType[i];
    config->encodeConfig[i].encodePreset = gAVSERVER_UI_config.codecEncodePreset[i];
  }

  config->vnfDemoEnable	= gAVSERVER_UI_config.vnfDemoEnable;

//  config->captureRawInMode	= AVSERVER_CAPTURE_RAW_IN_MODE_DDR_IN;//add by sxh 1115

  if(platform == PLATFORM_DM368)
  	OSA_printf("\nCONFIGURING AVSERVER FOR DM368 .....\n");
  if(platform == PLATFORM_DM365)
    OSA_printf("\nCONFIGURING AVSERVER FOR DM365 .....\n");

  switch(gAVSERVER_UI_config.mode) {

		case AVSERVER_UI_CAPTURE_MODE_D1:

			#ifdef YUV_MODE_INTERLACED
			config->sensorMode          = gAVSERVER_UI_config.winmodeEnable?DRV_IMGS_SENSOR_MODE_720x480:DRV_IMGS_SENSOR_MODE_720x480;
			#else
 			config->sensorMode			= gAVSERVER_UI_config.winmodeEnable?DRV_IMGS_SENSOR_MODE_1920x1080:DRV_IMGS_SENSOR_MODE_720x480;
			#endif
			
 			//config->sensorMode				= gAVSERVER_UI_config.winmodeEnable?DRV_IMGS_SENSOR_MODE_1920x1080:DRV_IMGS_SENSOR_MODE_720x480;
			config->sensorFps 				= 25;//30;//add by sxh 1121

			config->vstabTskEnable			= gAVSERVER_UI_config.vsEnable;
			config->ldcTskEnable			= gAVSERVER_UI_config.ldcEnable;
			config->vnfTskEnable			= gAVSERVER_UI_config.snfEnable|gAVSERVER_UI_config.tnfEnable;
			config->encryptTskEnable		= FALSE;

			config->captureRawInMode		= AVSERVER_CAPTURE_RAW_IN_MODE_DDR_IN;//AVSERVER_CAPTURE_RAW_IN_MODE_ISIF_IN; //add by sxh 1115AVSERVER_CAPTURE_RAW_IN_MODE_ISIF_IN;
			config->captureSingleResize 	= FALSE;
			config->captureYuvFormat		= DRV_DATA_FORMAT_YUV422;//DRV_DATA_FORMAT_YUV420;//add by sxh 1121

			//config->numCaptureStream		= 2;
			#ifdef YUV_MODE_INTERLACED
			config->numCaptureStream        = 1;
			#else
			config->numCaptureStream		= 2;
			#endif

			if(numEncodes > config->numCaptureStream)
				numEncodes = config->numCaptureStream;

			config->numEncodeStream 		= numEncodes;

			config->faceDetectConfig.captureStreamId = 1;
			config->faceDetectConfig.fdEnable 		 = gAVSERVER_UI_config.fdEnable;

			config->displayConfig.captureStreamId 	 = 0;
			config->displayConfig.width 			 = 640;
			config->displayConfig.height			 = 480;
      		config->displayConfig.expandH		     = TRUE;

			config->audioConfig.captureEnable 		 = FALSE;
			config->audioConfig.samplingRate		 = gAVSERVER_UI_config.audioSampleRate;
			config->audioConfig.codecType 			 = ALG_AUD_CODEC_G711;
			config->audioConfig.fileSaveEnable		 = FALSE;

			i=0;

			k=0;
			config->captureConfig[i].width						= 720;
			//config->captureConfig[i].height 					= 512;
			#ifdef YUV_MODE_INTERLACED
			config->captureConfig[i].height                     = 576;//480;//add by sxh 1121
			#else
			config->captureConfig[i].height 					= 512;
			#endif
			config->captureConfig[i].ldcEnable					= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable					= gAVSERVER_UI_config.snfEnable;
			config->captureConfig[i].tnfEnable					= gAVSERVER_UI_config.tnfEnable;
			config->captureConfig[i].vsEnable 					= gAVSERVER_UI_config.vsEnable;

		    if(numEncodes>0)
			  config->captureConfig[i].numEncodes 				= 1;

			config->captureConfig[i].encodeStreamId[k++]		= 0;
			config->captureConfig[i].frameSkipMask				= 0x3FFFFFFF;
			i++;

			k=0;
			config->captureConfig[i].width						= 288;
			config->captureConfig[i].height 					= 192;
			config->captureConfig[i].ldcEnable					= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable					= (numEncodes>1)?gAVSERVER_UI_config.snfEnable:FALSE;
			config->captureConfig[i].tnfEnable					= (numEncodes>1)?gAVSERVER_UI_config.tnfEnable:FALSE;
			config->captureConfig[i].vsEnable 					= gAVSERVER_UI_config.vsEnable;

			if(numEncodes>1)
				config->captureConfig[i].numEncodes 			= 1;

			config->captureConfig[i].encodeStreamId[k++]		= 1;
			config->captureConfig[i].frameSkipMask				= 0x3FFFFFFF;
			i++;

			i=0;

			config->encodeConfig[i].captureStreamId 		 = 0;
			config->encodeConfig[i].cropWidth 				 = ALIGN_ENCODE(720);
			config->encodeConfig[i].cropHeight				 = ALIGN_ENCODE(576);//ALIGN_ENCODE(480);//add by sxh 1121
			config->encodeConfig[i].frameRateBase			 = 30000;
			config->encodeConfig[i].frameSkipMask 			 = 0x3FFFFFFF;
			config->encodeConfig[i].codecType 				 = gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate			 = gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 			 = FALSE;
			config->encodeConfig[i].fileSaveEnable			 = FALSE;
			config->encodeConfig[i].motionVectorOutputEnable = FALSE;
			config->encodeConfig[i].qValue					 = gAVSERVER_UI_config.codecBitrate[i];
			i++;

			config->encodeConfig[i].captureStreamId 		 = 1;
			config->encodeConfig[i].cropWidth 				 = ALIGN_ENCODE(288);
			config->encodeConfig[i].cropHeight				 = ALIGN_ENCODE(192);
			config->encodeConfig[i].frameRateBase			 = 30000;
			config->encodeConfig[i].frameSkipMask 			 = 0x3FFFFFFF;
			config->encodeConfig[i].codecType 				 = gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate			 = gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 			 = FALSE;
			config->encodeConfig[i].fileSaveEnable			 = FALSE;
			config->encodeConfig[i].motionVectorOutputEnable = FALSE;
			config->encodeConfig[i].qValue					 = gAVSERVER_UI_config.codecBitrate[i];
			i++;

			break;

		case AVSERVER_UI_CAPTURE_MODE_D1_D1:

		  	if(platform==PLATFORM_DM368) {
				if(gAVSERVER_UI_config.flipH||gAVSERVER_UI_config.flipV) {
					if(gAVSERVER_UI_config.vsEnable)
						config->resizerClkdivN = 100;
					else if(gAVSERVER_UI_config.ldcEnable||gAVSERVER_UI_config.snfEnable||gAVSERVER_UI_config.tnfEnable)
						config->resizerClkdivN = 100;
					else
						config->resizerClkdivN = 80;
				}
				else
					config->resizerClkdivN = gAVSERVER_UI_config.vsEnable ? 60:70;
		    }
		    else {
				if(gAVSERVER_UI_config.flipH||gAVSERVER_UI_config.flipV) {
					if(gAVSERVER_UI_config.vsEnable)
						config->resizerClkdivN = 80;
					else if(gAVSERVER_UI_config.ldcEnable)
						config->resizerClkdivN = 90;
					else
						config->resizerClkdivN = 80;
				}
				else
					config->resizerClkdivN = gAVSERVER_UI_config.vsEnable ? 60:70;
			}

 			config->sensorMode				= gAVSERVER_UI_config.winmodeEnable?DRV_IMGS_SENSOR_MODE_720x480:DRV_IMGS_SENSOR_MODE_720x480;
			config->sensorFps 				= 30;

			config->vstabTskEnable			= gAVSERVER_UI_config.vsEnable;
			config->ldcTskEnable			= gAVSERVER_UI_config.ldcEnable;
			config->vnfTskEnable			= gAVSERVER_UI_config.snfEnable|gAVSERVER_UI_config.tnfEnable;
			config->encryptTskEnable		= FALSE;

			config->captureRawInMode		= AVSERVER_CAPTURE_RAW_IN_MODE_DDR_IN;
			config->captureSingleResize 	= FALSE;
			config->captureYuvFormat		= DRV_DATA_FORMAT_YUV420;

			config->numCaptureStream		= 3;

			if(numEncodes > config->numCaptureStream)
				numEncodes = config->numCaptureStream;

			config->numEncodeStream 		= numEncodes;

			config->faceDetectConfig.captureStreamId = 2;
			config->faceDetectConfig.fdEnable 		 = gAVSERVER_UI_config.fdEnable;

			config->displayConfig.captureStreamId 	 = 0;
			config->displayConfig.width 			 = 640;
			config->displayConfig.height			 = 480;
      		config->displayConfig.expandH		     = TRUE;

			config->audioConfig.captureEnable 		 = FALSE;
			config->audioConfig.samplingRate		 = gAVSERVER_UI_config.audioSampleRate;
			config->audioConfig.codecType 			 = ALG_AUD_CODEC_G711;
			config->audioConfig.fileSaveEnable		 = FALSE;

			i=0;

			k=0;
			config->captureConfig[i].width					= 720;
			config->captureConfig[i].height 				= 480;
			config->captureConfig[i].ldcEnable				= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable				= gAVSERVER_UI_config.snfEnable;
			config->captureConfig[i].tnfEnable				= gAVSERVER_UI_config.tnfEnable;
			config->captureConfig[i].vsEnable 				= gAVSERVER_UI_config.vsEnable;

		    if(numEncodes>0)
			  config->captureConfig[i].numEncodes 				= 1;

			config->captureConfig[i].encodeStreamId[k++]= 0;
			config->captureConfig[i].frameSkipMask			= 0x3FFFFFFF;
			i++;

			k=0;
			config->captureConfig[i].width					= 720;
			config->captureConfig[i].height 				= 480;
			if(gAVSERVER_UI_config.demomode)
			{
				config->captureConfig[i].ldcEnable			= FALSE;
				config->captureConfig[i].snfEnable			= FALSE;
				config->captureConfig[i].tnfEnable			= FALSE;
				config->captureConfig[i].vsEnable 			= FALSE;
			}
			else {
				config->captureConfig[i].ldcEnable			= gAVSERVER_UI_config.ldcEnable;
				config->captureConfig[i].snfEnable			= (numEncodes>1)?gAVSERVER_UI_config.snfEnable:FALSE;
				config->captureConfig[i].tnfEnable			= (numEncodes>1)?gAVSERVER_UI_config.tnfEnable:FALSE;
				config->captureConfig[i].vsEnable 			= gAVSERVER_UI_config.vsEnable;
			}

      		if(numEncodes>1)
  		   		config->captureConfig[i].numEncodes 		= 1;

			config->captureConfig[i].encodeStreamId[k++]= 1;
			config->captureConfig[i].frameSkipMask			= 0x3FFFFFFF;
			i++;

			k=0;
			config->captureConfig[i].width					= 288;
			config->captureConfig[i].height 				= 192;
			config->captureConfig[i].ldcEnable				= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable				= (numEncodes>2)?gAVSERVER_UI_config.snfEnable:FALSE;
			config->captureConfig[i].tnfEnable				= (numEncodes>2)?gAVSERVER_UI_config.snfEnable:FALSE;
			config->captureConfig[i].vsEnable 				= gAVSERVER_UI_config.vsEnable;

			if(numEncodes>2)
				config->captureConfig[i].numEncodes 			= 1;

			config->captureConfig[i].encodeStreamId[k++]= 2;
			config->captureConfig[i].frameSkipMask			= 0x3FFFFFFF;
			i++;

			i=0;

			config->encodeConfig[i].captureStreamId 		 = 0;
			config->encodeConfig[i].cropWidth 				 = ALIGN_ENCODE(720);
			config->encodeConfig[i].cropHeight				 = ALIGN_ENCODE(480);
			config->encodeConfig[i].frameRateBase			 = 30000;
			config->encodeConfig[i].frameSkipMask 			 = 0x3FFFFFFF;
			config->encodeConfig[i].codecType 				 = gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate			 = gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 			 = FALSE;
			config->encodeConfig[i].fileSaveEnable			 = FALSE;
			config->encodeConfig[i].motionVectorOutputEnable = FALSE;
			config->encodeConfig[i].qValue					 = gAVSERVER_UI_config.codecBitrate[i];
			i++;

			config->encodeConfig[i].captureStreamId 		 = 1;
			config->encodeConfig[i].cropWidth 				 = ALIGN_ENCODE(720);
			config->encodeConfig[i].cropHeight				 = ALIGN_ENCODE(480);
			config->encodeConfig[i].frameRateBase			 = 30000;
			config->encodeConfig[i].frameSkipMask 			 = 0x3FFFFFFF;
			config->encodeConfig[i].codecType 				 = gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate			 = gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 			 = FALSE;
			config->encodeConfig[i].fileSaveEnable			 = FALSE;
			config->encodeConfig[i].motionVectorOutputEnable = FALSE;
			config->encodeConfig[i].qValue					 = gAVSERVER_UI_config.codecBitrate[i];
			i++;

			config->encodeConfig[i].captureStreamId 		 = 2;
			config->encodeConfig[i].cropWidth 				 = ALIGN_ENCODE(288);
			config->encodeConfig[i].cropHeight				 = ALIGN_ENCODE(192);
			config->encodeConfig[i].frameRateBase			 = 30000;
			config->encodeConfig[i].frameSkipMask 			 = 0x3FFFFFFF;
			config->encodeConfig[i].codecType 				 = gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate			 = gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 			 = FALSE;
			config->encodeConfig[i].fileSaveEnable			 = FALSE;
			config->encodeConfig[i].motionVectorOutputEnable = FALSE;
			config->encodeConfig[i].qValue					 = gAVSERVER_UI_config.codecBitrate[i];
			i++;

			config->streamConfig.mem_layou_set				 = 1;

			break;

	default:
    case AVSERVER_UI_CAPTURE_MODE_720P:
	case AVSERVER_UI_CAPTURE_MODE_720P_720P:
	case AVSERVER_UI_CAPTURE_MODE_720P_720P_30:

      config->sensorMode          = gAVSERVER_UI_config.winmodeEnable?DRV_IMGS_SENSOR_MODE_1920x1080:DRV_IMGS_SENSOR_MODE_1280x720;
      config->sensorFps           = 30;

      config->vstabTskEnable      = gAVSERVER_UI_config.vsEnable;
      config->ldcTskEnable        = gAVSERVER_UI_config.ldcEnable;
      config->vnfTskEnable        = (gAVSERVER_UI_config.snfEnable | gAVSERVER_UI_config.tnfEnable);
      config->encryptTskEnable    = FALSE;

      config->captureRawInMode    = AVSERVER_CAPTURE_RAW_IN_MODE_ISIF_IN;
      config->captureSingleResize = FALSE;
      config->captureYuvFormat    = DRV_DATA_FORMAT_YUV420;

      config->numCaptureStream    = 2;

      if(numEncodes > config->numCaptureStream)
        numEncodes = config->numCaptureStream;

      config->numEncodeStream     = numEncodes;

      config->faceDetectConfig.captureStreamId = 1;
      config->faceDetectConfig.fdEnable        = gAVSERVER_UI_config.fdEnable;

      config->displayConfig.captureStreamId    = 0;
      config->displayConfig.width              = 640;
      config->displayConfig.height             = 480;
      config->displayConfig.expandH		       = TRUE;

      config->audioConfig.captureEnable        = FALSE;
      config->audioConfig.samplingRate         = gAVSERVER_UI_config.audioSampleRate;
      config->audioConfig.codecType            = ALG_AUD_CODEC_G711;
      config->audioConfig.fileSaveEnable       = FALSE;

      i=0;

      k=0;
      config->captureConfig[i].width              = 1280;
      config->captureConfig[i].height             = 736;
      config->captureConfig[i].ldcEnable          = gAVSERVER_UI_config.ldcEnable;
      config->captureConfig[i].snfEnable          = gAVSERVER_UI_config.snfEnable;
      config->captureConfig[i].tnfEnable          = gAVSERVER_UI_config.tnfEnable;
      config->captureConfig[i].vsEnable           = gAVSERVER_UI_config.vsEnable;

      if(numEncodes>0)
        config->captureConfig[i].numEncodes       = ((gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_720P_720P)||(gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_720P_720P_30))?2:1;

      config->captureConfig[i].encodeStreamId[k++]= 0;
      if((gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_720P_720P)||(gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_720P_720P_30))
      	config->captureConfig[i].encodeStreamId[k++]= 1;

      config->captureConfig[i].frameSkipMask      = 0x3FFFFFFF;
      i++;

      k=0;
      config->captureConfig[i].width              = 320;
      config->captureConfig[i].height             = 192;
      config->captureConfig[i].ldcEnable          = gAVSERVER_UI_config.ldcEnable;
	  config->captureConfig[i].snfEnable		  = (numEncodes>1)?gAVSERVER_UI_config.snfEnable:FALSE;
	  config->captureConfig[i].tnfEnable		  = (numEncodes>1)?gAVSERVER_UI_config.tnfEnable:FALSE;
      config->captureConfig[i].vsEnable           = gAVSERVER_UI_config.vsEnable;

      if(numEncodes>1)
        config->captureConfig[i].numEncodes       = ((gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_720P_720P)||(gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_720P_720P_30))?0:1;

      config->captureConfig[i].encodeStreamId[k++]= 1;

      config->captureConfig[i].frameSkipMask      = (gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_720P_720P)?0x1AAAAAAA:0x3FFFFFFF;
      i++;

      i=0;

      config->encodeConfig[i].captureStreamId          	= 0;
      config->encodeConfig[i].cropWidth                	= ALIGN_ENCODE(1280);
      config->encodeConfig[i].cropHeight               	= ALIGN_ENCODE(720);
      config->encodeConfig[i].frameRateBase			   	= 30000;
      config->encodeConfig[i].frameSkipMask            	= 0x3FFFFFFF;
      config->encodeConfig[i].codecType                	= gAVSERVER_UI_config.codecType[i];
      config->encodeConfig[i].codecBitrate             	= gAVSERVER_UI_config.codecBitrate[i];
      config->encodeConfig[i].encryptEnable            	= FALSE;
      config->encodeConfig[i].fileSaveEnable           	= FALSE;
      config->encodeConfig[i].motionVectorOutputEnable 	= FALSE;
      config->encodeConfig[i].qValue                   	= gAVSERVER_UI_config.codecBitrate[i];
      i++;

	  if((gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_720P_720P)||(gAVSERVER_UI_config.mode==AVSERVER_UI_CAPTURE_MODE_720P_720P_30)) {
        config->encodeConfig[i].captureStreamId         = 0;
      	config->encodeConfig[i].cropWidth               = ALIGN_ENCODE(1280);
      	config->encodeConfig[i].cropHeight              = ALIGN_ENCODE(720);
      	config->encodeConfig[i].frameRateBase		 	= 15000;
	  }
	  else {
        config->encodeConfig[i].captureStreamId         = 1;
      	config->encodeConfig[i].cropWidth               = ALIGN_ENCODE(320);
      	config->encodeConfig[i].cropHeight              = ALIGN_ENCODE(192);
      	config->encodeConfig[i].frameRateBase		 	= 30000;
  	  }
      config->encodeConfig[i].frameSkipMask             = 0x3FFFFFFF;
      config->encodeConfig[i].codecType                	= gAVSERVER_UI_config.codecType[i];
      config->encodeConfig[i].codecBitrate             	= gAVSERVER_UI_config.codecBitrate[i];
      config->encodeConfig[i].encryptEnable            	= FALSE;
      config->encodeConfig[i].fileSaveEnable           	= FALSE;
      config->encodeConfig[i].motionVectorOutputEnable 	= FALSE;
      config->encodeConfig[i].qValue                   	= gAVSERVER_UI_config.codecBitrate[i];
      i++;

      break;

    case AVSERVER_UI_CAPTURE_MODE_720P_VGA:
    case AVSERVER_UI_CAPTURE_MODE_720P_VGA_30:

	  if(platform==PLATFORM_DM368)
		config->resizerClkdivN = gAVSERVER_UI_config.vsEnable ? 60:70;
	  else
		config->resizerClkdivN = 50;

      config->sensorMode          = gAVSERVER_UI_config.winmodeEnable?DRV_IMGS_SENSOR_MODE_1280x720:DRV_IMGS_SENSOR_MODE_1280x720;
      config->sensorFps           = 30;

      config->vstabTskEnable      = gAVSERVER_UI_config.vsEnable;
      config->ldcTskEnable        = gAVSERVER_UI_config.ldcEnable;
      config->vnfTskEnable        = gAVSERVER_UI_config.snfEnable|gAVSERVER_UI_config.tnfEnable;
      config->encryptTskEnable    = FALSE;

      config->captureRawInMode    = AVSERVER_CAPTURE_RAW_IN_MODE_DDR_IN;
      config->captureSingleResize = FALSE;
      config->captureYuvFormat    = DRV_DATA_FORMAT_YUV420;

      config->numCaptureStream    = 3;

      if(numEncodes > config->numCaptureStream)
        numEncodes = config->numCaptureStream;

      config->numEncodeStream     = numEncodes;

      config->faceDetectConfig.captureStreamId = 2;
      config->faceDetectConfig.fdEnable        = gAVSERVER_UI_config.fdEnable;

      config->displayConfig.captureStreamId    = 1;
      config->displayConfig.width              = 640;
      config->displayConfig.height             = 480;
	  config->displayConfig.expandH		       = 1;

      config->audioConfig.captureEnable        = FALSE;
      config->audioConfig.samplingRate         = gAVSERVER_UI_config.audioSampleRate;
      config->audioConfig.codecType            = ALG_AUD_CODEC_G711;
      config->audioConfig.fileSaveEnable       = FALSE;

      i=0;

      k=0;
      config->captureConfig[i].width              = 1280;
      config->captureConfig[i].height             = 736;
      config->captureConfig[i].ldcEnable          = gAVSERVER_UI_config.ldcEnable;
      config->captureConfig[i].snfEnable          = gAVSERVER_UI_config.snfEnable;
      config->captureConfig[i].tnfEnable          = gAVSERVER_UI_config.tnfEnable;
      config->captureConfig[i].vsEnable           = gAVSERVER_UI_config.vsEnable;

      if(numEncodes>0)
        config->captureConfig[i].numEncodes         = 1;

      config->captureConfig[i].encodeStreamId[k++]= 0;
      config->captureConfig[i].frameSkipMask      = 0x3FFFFFFF;
      i++;

      k=0;
      config->captureConfig[i].width            = 640;
      config->captureConfig[i].height           = 352;
      config->captureConfig[i].ldcEnable          = gAVSERVER_UI_config.ldcEnable;
      config->captureConfig[i].snfEnable          = (numEncodes>1)?gAVSERVER_UI_config.snfEnable:FALSE;
      config->captureConfig[i].tnfEnable          = (numEncodes>1)?gAVSERVER_UI_config.tnfEnable:FALSE;
      config->captureConfig[i].vsEnable           = gAVSERVER_UI_config.vsEnable;

      if(numEncodes>1)
        config->captureConfig[i].numEncodes       = 1;

      config->captureConfig[i].encodeStreamId[k++]= 1;

      if( gAVSERVER_UI_config.mode == AVSERVER_UI_CAPTURE_MODE_720P_VGA)
	      config->captureConfig[i].frameSkipMask      = 0x1AAAAAAA;
	    else
	    	config->captureConfig[i].frameSkipMask    = 0x3FFFFFFF;

      i++;

      k=0;
      config->captureConfig[i].width              = 320;
      config->captureConfig[i].height             = 192;
      config->captureConfig[i].ldcEnable          = gAVSERVER_UI_config.ldcEnable;
	  config->captureConfig[i].snfEnable		  = (numEncodes>2)?gAVSERVER_UI_config.snfEnable:FALSE;
	  config->captureConfig[i].tnfEnable		  = (numEncodes>2)?gAVSERVER_UI_config.tnfEnable:FALSE;
      config->captureConfig[i].vsEnable           = gAVSERVER_UI_config.vsEnable;

      if(numEncodes>2)
        config->captureConfig[i].numEncodes       = 1;

      config->captureConfig[i].encodeStreamId[k++]= 2;

      if( gAVSERVER_UI_config.mode == AVSERVER_UI_CAPTURE_MODE_720P_VGA)
	      config->captureConfig[i].frameSkipMask      = 0x1AAAAAAA;
	    else
 	      config->captureConfig[i].frameSkipMask      = 0x3FFFFFFF;
      i++;

      i=0;

      config->encodeConfig[i].captureStreamId          = 0;
      config->encodeConfig[i].cropWidth                = ALIGN_ENCODE(1280);
      config->encodeConfig[i].cropHeight               = ALIGN_ENCODE(720);
      config->encodeConfig[i].frameRateBase			   = 30000;
      config->encodeConfig[i].frameSkipMask            = 0x3FFFFFFF;
      config->encodeConfig[i].codecType                = gAVSERVER_UI_config.codecType[i];
      config->encodeConfig[i].codecBitrate             = gAVSERVER_UI_config.codecBitrate[i];
      config->encodeConfig[i].encryptEnable            = FALSE;
      config->encodeConfig[i].fileSaveEnable           = FALSE;
      config->encodeConfig[i].motionVectorOutputEnable = FALSE;
      config->encodeConfig[i].qValue                   = gAVSERVER_UI_config.codecBitrate[i];
      i++;

      config->encodeConfig[i].captureStreamId          = 1;
	  config->encodeConfig[i].cropWidth                = ALIGN_ENCODE(640);
   	  config->encodeConfig[i].cropHeight               = ALIGN_ENCODE(352);
      if( gAVSERVER_UI_config.mode == AVSERVER_UI_CAPTURE_MODE_720P_VGA)
      	config->encodeConfig[i].frameRateBase		   = 15000;
      else
      	config->encodeConfig[i].frameRateBase		   = 30000;
      config->encodeConfig[i].frameSkipMask            = 0x3FFFFFFF;
      config->encodeConfig[i].codecType                = gAVSERVER_UI_config.codecType[i];
      config->encodeConfig[i].codecBitrate             = gAVSERVER_UI_config.codecBitrate[i];
      config->encodeConfig[i].encryptEnable            = FALSE;
      config->encodeConfig[i].fileSaveEnable           = FALSE;
      config->encodeConfig[i].motionVectorOutputEnable = FALSE;
      config->encodeConfig[i].qValue                   = gAVSERVER_UI_config.codecBitrate[i];
      i++;

      config->encodeConfig[i].captureStreamId          = 2;
      config->encodeConfig[i].cropWidth                = ALIGN_ENCODE(320);
      config->encodeConfig[i].cropHeight               = ALIGN_ENCODE(192);
      if( gAVSERVER_UI_config.mode == AVSERVER_UI_CAPTURE_MODE_720P_VGA)
      	config->encodeConfig[i].frameRateBase		 	= 15000;
      else
      	config->encodeConfig[i].frameRateBase		 	= 30000;
      config->encodeConfig[i].frameSkipMask            = 0x3FFFFFFF;
      config->encodeConfig[i].codecType                = gAVSERVER_UI_config.codecType[i];
      config->encodeConfig[i].codecBitrate             = gAVSERVER_UI_config.codecBitrate[i];
      config->encodeConfig[i].encryptEnable            = FALSE;
      config->encodeConfig[i].fileSaveEnable           = FALSE;
      config->encodeConfig[i].motionVectorOutputEnable = FALSE;
      config->encodeConfig[i].qValue                   = gAVSERVER_UI_config.codecBitrate[i];
      i++;

      break;

		case AVSERVER_UI_CAPTURE_MODE_SXGA:

      		gAVSERVER_UI_config.ldcEnable = FALSE;
      		gAVSERVER_UI_config.vsEnable  = FALSE;

			config->sensorMode				= DRV_IMGS_SENSOR_MODE_1600x1200;//gAVSERVER_UI_config.winmodeEnable?DRV_IMGS_SENSOR_MODE_1600x1200:DRV_IMGS_SENSOR_MODE_1280x1024;
			config->sensorFps 				= 30;

			config->vstabTskEnable			= gAVSERVER_UI_config.vsEnable;
			config->ldcTskEnable			= gAVSERVER_UI_config.ldcEnable;
			config->vnfTskEnable			= gAVSERVER_UI_config.snfEnable|gAVSERVER_UI_config.tnfEnable;
			config->encryptTskEnable		= FALSE;

			config->captureRawInMode		= AVSERVER_CAPTURE_RAW_IN_MODE_ISIF_IN;
			config->captureSingleResize 	= FALSE;
			config->captureYuvFormat		= DRV_DATA_FORMAT_YUV420;

			config->numCaptureStream		= 2;

			if(numEncodes > config->numCaptureStream)
				numEncodes = config->numCaptureStream;

			config->numEncodeStream 		= numEncodes;

			config->faceDetectConfig.captureStreamId = 1;
			config->faceDetectConfig.fdEnable 			 = gAVSERVER_UI_config.fdEnable;

			config->displayConfig.captureStreamId 	 = 1;
			config->displayConfig.width 						 = 640;
			config->displayConfig.height						 = 480;
			config->displayConfig.expandH            = TRUE;

			config->audioConfig.captureEnable 			 = FALSE;
			config->audioConfig.samplingRate				 = gAVSERVER_UI_config.audioSampleRate;
			config->audioConfig.codecType 					 = ALG_AUD_CODEC_G711;
			config->audioConfig.fileSaveEnable			 = FALSE;

			i=0;

			k=0;
			config->captureConfig[i].width							= 1280;
			config->captureConfig[i].height 						= 1024;
			config->captureConfig[i].ldcEnable					= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable					= gAVSERVER_UI_config.snfEnable;
			config->captureConfig[i].tnfEnable					= gAVSERVER_UI_config.tnfEnable;
			config->captureConfig[i].vsEnable 					= gAVSERVER_UI_config.vsEnable;

			if(numEncodes>0)
				config->captureConfig[i].numEncodes 				= 1;

			config->captureConfig[i].encodeStreamId[k++]= 0;
			config->captureConfig[i].frameSkipMask			= 0x3FFFFFFF;
			i++;

			k=0;
			config->captureConfig[i].width							= 256;
			config->captureConfig[i].height 						= 192;
			config->captureConfig[i].ldcEnable					= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable					= (numEncodes>1)?gAVSERVER_UI_config.snfEnable:FALSE;
			config->captureConfig[i].tnfEnable					= (numEncodes>1)?gAVSERVER_UI_config.tnfEnable:FALSE;
			config->captureConfig[i].vsEnable 					= gAVSERVER_UI_config.vsEnable;

			if(numEncodes>1)
				config->captureConfig[i].numEncodes 	= 1;
			else
				config->streamConfig.mem_layou_set		= 2;

			config->captureConfig[i].encodeStreamId[k++]= 1;
			config->captureConfig[i].frameSkipMask			= 0x3FFFFFFF;
			i++;

			i=0;

			config->encodeConfig[i].captureStreamId 		 = 0;
			config->encodeConfig[i].cropWidth 				 = ALIGN_ENCODE(1280);
			config->encodeConfig[i].cropHeight				 = ALIGN_ENCODE(1024);
			config->encodeConfig[i].frameRateBase			 = 30000;
			config->encodeConfig[i].frameSkipMask 			 = 0x3FFFFFFF;
			config->encodeConfig[i].codecType 				 = gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate			 = gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 			 = FALSE;
			config->encodeConfig[i].fileSaveEnable			 = FALSE;
			config->encodeConfig[i].motionVectorOutputEnable = FALSE;
			config->encodeConfig[i].qValue					 = gAVSERVER_UI_config.codecBitrate[i];
			i++;

			config->encodeConfig[i].captureStreamId 		 = 1;
			config->encodeConfig[i].cropWidth 				 = ALIGN_ENCODE(256);
			config->encodeConfig[i].cropHeight				 = ALIGN_ENCODE(192);
			config->encodeConfig[i].frameRateBase			 = 30000;
			config->encodeConfig[i].frameSkipMask 			 = 0x3FFFFFFF;
			config->encodeConfig[i].codecType 				 = gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate			 = gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 			 = FALSE;
			config->encodeConfig[i].fileSaveEnable			 = FALSE;
			config->encodeConfig[i].motionVectorOutputEnable = FALSE;
			config->encodeConfig[i].qValue					 = gAVSERVER_UI_config.codecBitrate[i];
			i++;

			break;

		case AVSERVER_UI_CAPTURE_MODE_1080P:

		    gAVSERVER_UI_config.ldcEnable = FALSE;
		    gAVSERVER_UI_config.vsEnable  = FALSE;

			config->sensorMode				= DRV_IMGS_SENSOR_MODE_1920x1080;
			config->sensorFps 				= 30;//(platform == PLATFORM_DM368)?30:15;

			config->vstabTskEnable			= gAVSERVER_UI_config.vsEnable;
			config->ldcTskEnable			= gAVSERVER_UI_config.ldcEnable;
			config->vnfTskEnable			= gAVSERVER_UI_config.snfEnable|gAVSERVER_UI_config.tnfEnable;
			config->encryptTskEnable		= FALSE;

			config->captureRawInMode		= AVSERVER_CAPTURE_RAW_IN_MODE_ISIF_IN;
			config->captureSingleResize = FALSE;
			config->captureYuvFormat		= DRV_DATA_FORMAT_YUV420;

			config->numCaptureStream		= 2;

			if(numEncodes > config->numCaptureStream)
				numEncodes = config->numCaptureStream;

			config->numEncodeStream 		= numEncodes;

			config->faceDetectConfig.captureStreamId = 1;
			config->faceDetectConfig.fdEnable 			 = gAVSERVER_UI_config.fdEnable;

			config->displayConfig.captureStreamId 	 = 1;
			config->displayConfig.width 						 = 640;
			config->displayConfig.height						 = 480;
			config->displayConfig.expandH            = TRUE;

			config->audioConfig.captureEnable 			 = FALSE;
			config->audioConfig.samplingRate				 = gAVSERVER_UI_config.audioSampleRate;
			config->audioConfig.codecType 					 = ALG_AUD_CODEC_G711;
			config->audioConfig.fileSaveEnable			 = FALSE;

			i=0;

			k=0;
			config->captureConfig[i].width							= 1920;
			config->captureConfig[i].height 						= 1080;
			config->captureConfig[i].ldcEnable					= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable					= gAVSERVER_UI_config.snfEnable;
			config->captureConfig[i].tnfEnable					= gAVSERVER_UI_config.tnfEnable;
			config->captureConfig[i].vsEnable 					= gAVSERVER_UI_config.vsEnable;

			if(numEncodes>0)
				config->captureConfig[i].numEncodes 				= 1;

			config->captureConfig[i].encodeStreamId[k++]= 0;
			config->captureConfig[i].frameSkipMask			= 0x3FFFFFFF;
			i++;

			k=0;
			config->captureConfig[i].width			= 320;
			config->captureConfig[i].height 		= 192;
			config->captureConfig[i].ldcEnable		= gAVSERVER_UI_config.ldcEnable;
	  		config->captureConfig[i].snfEnable		= (numEncodes>1)?gAVSERVER_UI_config.snfEnable:FALSE;
	  		config->captureConfig[i].tnfEnable		= (numEncodes>1)?gAVSERVER_UI_config.tnfEnable:FALSE;
			config->captureConfig[i].vsEnable 		= gAVSERVER_UI_config.vsEnable;

			if(numEncodes>1)
				config->captureConfig[i].numEncodes 	= 1;
			else
				config->streamConfig.mem_layou_set		= 2;

			config->captureConfig[i].encodeStreamId[k++]= 1;
			config->captureConfig[i].frameSkipMask			= 0x3FFFFFFF;
			i++;

			i=0;

			config->encodeConfig[i].captureStreamId 				 = 0;
			config->encodeConfig[i].cropWidth 						= 1920;
			config->encodeConfig[i].cropHeight						= 1080;
			config->encodeConfig[i].frameRateBase						 = 30000;
			config->encodeConfig[i].frameSkipMask 					 = 0x3FFFFFFF;
			config->encodeConfig[i].codecType 							 = gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate						 = gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 					 = FALSE;
			config->encodeConfig[i].fileSaveEnable					 = FALSE;
			config->encodeConfig[i].motionVectorOutputEnable = FALSE;
			config->encodeConfig[i].qValue									 = gAVSERVER_UI_config.codecBitrate[i];
			i++;

			config->encodeConfig[i].captureStreamId 				 = 1;
			config->encodeConfig[i].cropWidth 							 = 320;
			config->encodeConfig[i].cropHeight							 = 192;
			config->encodeConfig[i].frameRateBase						 = 30000;
			config->encodeConfig[i].frameSkipMask 					 = 0x3FFFFFFF;
			config->encodeConfig[i].codecType 							 = gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate						 = gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 					 = FALSE;
			config->encodeConfig[i].fileSaveEnable					 = FALSE;
			config->encodeConfig[i].motionVectorOutputEnable = FALSE;
			config->encodeConfig[i].qValue									 = gAVSERVER_UI_config.codecBitrate[i];
			i++;

			break;

		case AVSERVER_UI_CAPTURE_MODE_2_MEGA:

      		gAVSERVER_UI_config.ldcEnable = FALSE;
      		gAVSERVER_UI_config.vsEnable  = FALSE;
      		gAVSERVER_UI_config.snfEnable = FALSE;
      		gAVSERVER_UI_config.tnfEnable = FALSE;

			config->sensorMode					= DRV_IMGS_SENSOR_MODE_1600x1200;
			config->sensorFps 					= 30;//(platform!=PLATFORM_DM368)?24:30;
			config->vstabTskEnable			= gAVSERVER_UI_config.vsEnable;
			config->ldcTskEnable				= gAVSERVER_UI_config.ldcEnable;
			config->vnfTskEnable				= gAVSERVER_UI_config.snfEnable|gAVSERVER_UI_config.tnfEnable;
			config->encryptTskEnable		= FALSE;

			config->captureRawInMode		= AVSERVER_CAPTURE_RAW_IN_MODE_ISIF_IN;
			config->captureSingleResize = FALSE;
			config->captureYuvFormat		= DRV_DATA_FORMAT_YUV420;

			config->numCaptureStream		= 2;

			if(numEncodes > config->numCaptureStream)
				numEncodes = config->numCaptureStream;

			config->numEncodeStream 		= numEncodes;

			config->faceDetectConfig.captureStreamId = 1;
			config->faceDetectConfig.fdEnable 			 = gAVSERVER_UI_config.fdEnable;

			config->displayConfig.captureStreamId 	 = 1;
			config->displayConfig.width 						 = 640;
			config->displayConfig.height						 = 480;
			config->displayConfig.expandH            = TRUE;

			config->audioConfig.captureEnable 			 = FALSE;
			config->audioConfig.samplingRate				 = gAVSERVER_UI_config.audioSampleRate;
			config->audioConfig.codecType 					 = ALG_AUD_CODEC_G711;
			config->audioConfig.fileSaveEnable			 = FALSE;

			i=0;

			k=0;
			config->captureConfig[i].width							= 1600;
			config->captureConfig[i].height 						= 1200;
			config->captureConfig[i].ldcEnable					= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable					= gAVSERVER_UI_config.snfEnable;
			config->captureConfig[i].tnfEnable					= gAVSERVER_UI_config.tnfEnable;
			config->captureConfig[i].vsEnable 					= gAVSERVER_UI_config.vsEnable;

			if(numEncodes>0)
				config->captureConfig[i].numEncodes 				= 1;

			config->captureConfig[i].encodeStreamId[k++]= 0;
			config->captureConfig[i].frameSkipMask			= 0x3FFFFFFF;
			i++;

			k=0;
			config->captureConfig[i].width							= 256;
			config->captureConfig[i].height 						= 192;
			config->captureConfig[i].ldcEnable					= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable					= (numEncodes>1)?gAVSERVER_UI_config.snfEnable:FALSE;
			config->captureConfig[i].tnfEnable					= (numEncodes>1)?gAVSERVER_UI_config.tnfEnable:FALSE;
			config->captureConfig[i].vsEnable 					= gAVSERVER_UI_config.vsEnable;

			if(numEncodes>1)
				config->captureConfig[i].numEncodes 				= 1;

			config->captureConfig[i].encodeStreamId[k++]= 1;
			config->captureConfig[i].frameSkipMask			= 0x3FFFFFFF;
			i++;

			i=0;

			config->encodeConfig[i].captureStreamId 				 = 0;
			config->encodeConfig[i].cropWidth 							 = ALIGN_ENCODE(1600);
			config->encodeConfig[i].cropHeight							 = ALIGN_ENCODE(1200);
			config->encodeConfig[i].frameRateBase						 = 24000;
			config->encodeConfig[i].frameSkipMask 					 = 0x3FFFFFFF;
			config->encodeConfig[i].codecType 							 = gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate						 = gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 					 = FALSE;
			config->encodeConfig[i].fileSaveEnable					 = FALSE;
			config->encodeConfig[i].motionVectorOutputEnable = FALSE;
			config->encodeConfig[i].qValue									 = gAVSERVER_UI_config.codecBitrate[i];
			i++;

			config->encodeConfig[i].captureStreamId 				 = 1;
			config->encodeConfig[i].cropWidth 							 = ALIGN_ENCODE(256);
			config->encodeConfig[i].cropHeight							 = ALIGN_ENCODE(192);
			config->encodeConfig[i].frameRateBase						 = 24000;
			config->encodeConfig[i].frameSkipMask 					 = 0x3FFFFFFF;
			config->encodeConfig[i].codecType 							 = gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate						 = gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 					 = FALSE;
			config->encodeConfig[i].fileSaveEnable					 = FALSE;
			config->encodeConfig[i].motionVectorOutputEnable = FALSE;
			config->encodeConfig[i].qValue									 = gAVSERVER_UI_config.codecBitrate[i];
			i++;

			break;


		case AVSERVER_UI_CAPTURE_MODE_3_MEGA:

		   gAVSERVER_UI_config.ldcEnable = FALSE;
		   gAVSERVER_UI_config.vsEnable  = FALSE;
		   gAVSERVER_UI_config.snfEnable = FALSE;
		   gAVSERVER_UI_config.tnfEnable = FALSE;

			config->sensorMode					= DRV_IMGS_SENSOR_MODE_2048x1536;
			config->sensorFps 					= 15;

			config->vstabTskEnable			= gAVSERVER_UI_config.vsEnable;
			config->ldcTskEnable				= gAVSERVER_UI_config.ldcEnable;
			config->vnfTskEnable				= gAVSERVER_UI_config.snfEnable|gAVSERVER_UI_config.tnfEnable;
			config->encryptTskEnable		= FALSE;

			config->captureRawInMode		= AVSERVER_CAPTURE_RAW_IN_MODE_ISIF_IN;
			config->captureSingleResize = FALSE;
			config->captureYuvFormat		= DRV_DATA_FORMAT_YUV420;

			config->numCaptureStream		= 2;

			if(numEncodes > config->numCaptureStream)
				numEncodes = config->numCaptureStream;

			config->numEncodeStream 		= numEncodes;

			config->faceDetectConfig.captureStreamId = 1;
			config->faceDetectConfig.fdEnable 			 = gAVSERVER_UI_config.fdEnable;

			config->displayConfig.captureStreamId 	 = 1;
			config->displayConfig.width 						 = 640;
			config->displayConfig.height						 = 480;
			config->displayConfig.expandH            = TRUE;

			config->audioConfig.captureEnable 			 = FALSE;
			config->audioConfig.samplingRate				 = gAVSERVER_UI_config.audioSampleRate;
			config->audioConfig.codecType 					 = ALG_AUD_CODEC_G711;
			config->audioConfig.fileSaveEnable			 = FALSE;

			i=0;

			k=0;
			config->captureConfig[i].width							= 2048;
			config->captureConfig[i].height 						= 1536;
			config->captureConfig[i].ldcEnable					= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable					= gAVSERVER_UI_config.snfEnable;
			config->captureConfig[i].tnfEnable					= gAVSERVER_UI_config.tnfEnable;
			config->captureConfig[i].vsEnable 					= gAVSERVER_UI_config.vsEnable;

			if(numEncodes>0)
				config->captureConfig[i].numEncodes 			= 1;

			config->captureConfig[i].encodeStreamId[k++]= 0;
			config->captureConfig[i].frameSkipMask			= 0x3FFFFFFF;
			i++;

			k=0;
			config->captureConfig[i].width							= 256;
			config->captureConfig[i].height 						= 192;
			config->captureConfig[i].ldcEnable					= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable					= (numEncodes>1)?gAVSERVER_UI_config.snfEnable:FALSE;
			config->captureConfig[i].tnfEnable					= (numEncodes>1)?gAVSERVER_UI_config.tnfEnable:FALSE;
			config->captureConfig[i].vsEnable 					= gAVSERVER_UI_config.vsEnable;

			if(numEncodes>1)
				config->captureConfig[i].numEncodes 				= 1;

			config->captureConfig[i].encodeStreamId[k++]= 1;
			config->captureConfig[i].frameSkipMask			= 0x3FFFFFFF;
			i++;

			i=0;

			config->encodeConfig[i].captureStreamId 				 = 0;
			config->encodeConfig[i].cropWidth 							 = ALIGN_ENCODE(2048);
			config->encodeConfig[i].cropHeight							 = ALIGN_ENCODE(1536);
			config->encodeConfig[i].frameRateBase						 = 15000;
			config->encodeConfig[i].frameSkipMask 					 = 0x3FFFFFFF;
			config->encodeConfig[i].codecType 							 = gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate						 = gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 					 = FALSE;
			config->encodeConfig[i].fileSaveEnable					 = FALSE;
			config->encodeConfig[i].motionVectorOutputEnable = FALSE;
			config->encodeConfig[i].qValue									 = gAVSERVER_UI_config.codecBitrate[i];
			i++;

			config->encodeConfig[i].captureStreamId 				 = 1;
			config->encodeConfig[i].cropWidth 							 = ALIGN_ENCODE(256);
			config->encodeConfig[i].cropHeight							 = ALIGN_ENCODE(192);
			config->encodeConfig[i].frameRateBase						 = 15000;
			config->encodeConfig[i].frameSkipMask 					 = 0x3FFFFFFF;
			config->encodeConfig[i].codecType 							 = gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate						 = gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 					 = FALSE;
			config->encodeConfig[i].fileSaveEnable					 = FALSE;
			config->encodeConfig[i].motionVectorOutputEnable = FALSE;
			config->encodeConfig[i].qValue									 = gAVSERVER_UI_config.codecBitrate[i];
			i++;

			break;

		case AVSERVER_UI_CAPTURE_MODE_5_MEGA:

		   gAVSERVER_UI_config.ldcEnable = FALSE;
		   gAVSERVER_UI_config.vsEnable  = FALSE;
		   gAVSERVER_UI_config.snfEnable = FALSE;
		   gAVSERVER_UI_config.tnfEnable = FALSE;

			config->resizerClkdivN				= 70;
			config->sensorMode					= DRV_IMGS_SENSOR_MODE_2592x1920;
			config->sensorFps 					= 10;

			config->vstabTskEnable				= gAVSERVER_UI_config.vsEnable;
			config->ldcTskEnable				= gAVSERVER_UI_config.ldcEnable;
			config->vnfTskEnable				= gAVSERVER_UI_config.snfEnable|gAVSERVER_UI_config.tnfEnable;
			config->encryptTskEnable			= FALSE;

			config->captureRawInMode			= AVSERVER_CAPTURE_RAW_IN_MODE_DDR_IN;
			config->captureSingleResize 		= TRUE;
			config->captureYuvFormat			= DRV_DATA_FORMAT_YUV420;

	  		config->numCaptureStream		  	= 1;

			if(numEncodes > config->numCaptureStream)
				numEncodes = config->numCaptureStream;

			config->numEncodeStream 					= numEncodes;

			config->faceDetectConfig.captureStreamId 	= 1;
			config->faceDetectConfig.fdEnable 		 	= FALSE;
			config->faceDetectConfig.fdTracker 		 	= FALSE;

	  		config->displayConfig.captureStreamId    	= 0;
			config->displayConfig.width 			 	= 640;
			config->displayConfig.height			 	= 480;
			config->displayConfig.expandH            	= TRUE;

			config->audioConfig.captureEnable 			= FALSE;
			config->audioConfig.samplingRate			= 8000;
			config->audioConfig.codecType 				= ALG_AUD_CODEC_G711;
			config->audioConfig.fileSaveEnable			= FALSE;

			i=0;

			k=0;
			config->captureConfig[i].width				= 2592;
			config->captureConfig[i].height 			= 1920;
			config->captureConfig[i].ldcEnable			= gAVSERVER_UI_config.ldcEnable;
			config->captureConfig[i].snfEnable			= gAVSERVER_UI_config.snfEnable;
			config->captureConfig[i].tnfEnable			= gAVSERVER_UI_config.tnfEnable;
			config->captureConfig[i].vsEnable 			= gAVSERVER_UI_config.vsEnable;

			if(numEncodes>0)
				config->captureConfig[i].numEncodes 	= 1;

			config->captureConfig[i].encodeStreamId[k++]= 0;
			config->captureConfig[i].frameSkipMask			= 0x3FFFFFFF;

			i=0;
			config->encodeConfig[i].captureStreamId 			= 0;
			config->encodeConfig[i].cropWidth 					= ALIGN_ENCODE(2592);
			config->encodeConfig[i].cropHeight					= ALIGN_ENCODE(1920);
			config->encodeConfig[i].frameRateBase				= 5000;
			config->encodeConfig[i].frameSkipMask 				= 0x3FFFFFFF;
			config->encodeConfig[i].codecType 					= gAVSERVER_UI_config.codecType[i];
			config->encodeConfig[i].codecBitrate				= gAVSERVER_UI_config.codecBitrate[i];
			config->encodeConfig[i].encryptEnable 				= FALSE;
			config->encodeConfig[i].fileSaveEnable				= FALSE;
			config->encodeConfig[i].motionVectorOutputEnable 	= FALSE;
			config->encodeConfig[i].qValue					 	= gAVSERVER_UI_config.codecBitrate[i];

			break;
  }

  if(config->vstabTskEnable)
    config->sensorMode |= DRV_IMGS_SENSOR_MODE_VSTAB;

}


