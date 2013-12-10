
#include <system.h>
#include <capture.h>
#include <display.h>
#include <decode.h>
#include <encode.h>
#include <writer.h>

#ifdef VANLINK_DVR_DM365
#include <avserver.h>//20120105 add by sxh stream
#include <video.h>

	VIDEO_Stream_Ctrl gVIDEO_Stream_ctrl;
#endif
//20120105 add by sxh stream
int UI_rtspStreamStart()
{
  OSA_printf(" MCVIP UI: Starting Streaming Server...\n");
  system("./wis-streamer &");
  OSA_waitMsecs(2000);
  OSA_printf(" MCVIP UI: Starting Streaming Server...DONE\n");
  return 0;
}


int UI_sysStart(int mode, int videoInterface, int videoSystem)
{
  int status;
  CAPTURE_CreatePrm captureCreatePrm;
  DISPLAY_CreatePrm displayCreatePrm;  
  ENCODE_CreatePrm  encodeCreatePrm;
  WRITER_CreatePrm  writerCreatePrm;  
  CAPTURE_Info *pCaptureInfo;
  VIDEO_Stream_CreatePrm videostreamCreatePrm;  //20120105 add by sxh stream
  
  OSA_printf(" MCVIP: Starting. Please wait !!!\n");
  
  captureCreatePrm.videoDecoderMode = mode;
  captureCreatePrm.videoSystem = videoSystem;
  captureCreatePrm.videoIfMode = videoInterface;
  captureCreatePrm.frameSkipMask = 0xFFFFFFFF;
  
  if(mode==MCVIP_VIDEO_DECODER_MODE_8CH_CIF)
    captureCreatePrm.frameSkipMask = 0xAAAAAAAA;  
 
  OSA_printf(" MCVIP: Capture Starting !!!\n"); 
  
  status = CAPTURE_create(&captureCreatePrm);
  if(status!=OSA_SOK) {
    OSA_ERROR("CAPTURE_create()\r\n");
    return status;
  }
  
  displayCreatePrm.displayMode = DISPLAY_MODE_COMPOSITE;
  displayCreatePrm.layoutMode  = DISPLAY_LAYOUT_MODE_LIVE_2x2;

  OSA_printf(" MCVIP: Display Starting !!!\n"); 
      
  status = DISPLAY_create(&displayCreatePrm);
  if(status!=OSA_SOK) {
    OSA_ERROR("DISPLAY_create()\r\n");
    CAPTURE_delete();
    return status;
  }  

  pCaptureInfo = CAPTURE_getInfo();
  
  encodeCreatePrm.numCh = pCaptureInfo->chList.numCh;
  encodeCreatePrm.frameWidth  = pCaptureInfo->chList.info[0].width;
  encodeCreatePrm.frameHeight = pCaptureInfo->chList.info[0].height;
  
  if(encodeCreatePrm.frameWidth==720)
    encodeCreatePrm.frameWidth = 704;
  if(encodeCreatePrm.frameWidth==360)
    encodeCreatePrm.frameWidth = 352;

  OSA_printf(" MCVIP: Encode Starting (ch=%d, %dx%d)!!!\n", 
        encodeCreatePrm.numCh,
        encodeCreatePrm.frameWidth,
        encodeCreatePrm.frameHeight
  ); 
  
  status = ENCODE_create(&encodeCreatePrm);
  if(status!=OSA_SOK) {
    OSA_ERROR("ENCODE_create()\r\n");
    CAPTURE_delete();
    DISPLAY_delete();
    return status;
  }  
  
  OSA_printf(" MCVIP: Writer Starting !!!\n"); 
    
  writerCreatePrm.numCh = encodeCreatePrm.numCh;
  writerCreatePrm.frameWidth = encodeCreatePrm.frameWidth;
  writerCreatePrm.frameHeight = encodeCreatePrm.frameHeight;
      
  status = WRITER_create(&writerCreatePrm);
  if(status!=OSA_SOK) {
    OSA_ERROR("WRITER_create()\r\n");
    CAPTURE_delete();
    DISPLAY_delete();
    ENCODE_delete();
    return status;
  }   

#ifdef VANLINK_DVR_STREAM_ENABLE
	videostreamCreatePrm.numCh = encodeCreatePrm.numCh;
	videostreamCreatePrm.frameWidth = encodeCreatePrm.frameWidth;
	videostreamCreatePrm.frameHeight = encodeCreatePrm.frameHeight;
	 
	status = VIDEO_streamCreate(&videostreamCreatePrm);
	if(status!=OSA_SOK) {
	  OSA_ERROR("VIDEO_STREAM_create()\r\n");
	  CAPTURE_delete();
	  DISPLAY_delete();
	  ENCODE_delete();
	  WRITER_delete();	
	  return status;
	}	  
#endif

  OSA_printf(" MCVIP: Display Enable !!!\n"); 

  status = DISPLAY_start();  
  if(status!=OSA_SOK) {
    OSA_ERROR("DISPLAY_start()\r\n");
    DISPLAY_delete();    
    CAPTURE_delete();
    ENCODE_delete();
    WRITER_delete();   
	#ifdef VANLINK_DVR_STREAM_ENABLE
	VIDEO_streamDelete();
	#endif
    return status;
  }  
  
  OSA_printf(" MCVIP: Capture Enable !!!\n"); 
    
  status = CAPTURE_start();  
  if(status!=OSA_SOK) {
    OSA_ERROR("CAPTURE_start()\r\n");
    DISPLAY_stop();
    DISPLAY_delete();        
    CAPTURE_delete();
    ENCODE_delete();
    WRITER_delete();     
	#ifdef VANLINK_DVR_STREAM_ENABLE
	VIDEO_streamDelete();
	#endif
    return status;
  }  
  
  OSA_printf(" MCVIP: Start DONE !!!\n"); 

  UI_rtspStreamStart();
  OSA_printf(" MCVIP: RTSP Start !!!\n"); 
  return status;
}

int UI_sysStop()
{
  OSA_printf(" MCVIP: Capture Stopping !!!\n");
  CAPTURE_stop();

  OSA_printf(" MCVIP: Display Stopping !!!\n");
  DISPLAY_stop();  

  OSA_printf(" MCVIP: Display Deleting !!!\n");
  DISPLAY_delete();

  OSA_printf(" MCVIP: Capture Deleting !!!\n");
  CAPTURE_delete();
    
  OSA_printf(" MCVIP: Encode Deleting !!!\n");
  ENCODE_delete();

  OSA_printf(" MCVIP: Writer Deleting !!!\n");
  WRITER_delete();
  
  OSA_printf(" MCVIP: Stopped !!!\n");

  return OSA_SOK;
}

int UI_sysPlayFile(char *filename, int width, int height)
{
  int status;
  DECODE_CreatePrm createPrm;
  
  createPrm.codec = ALG_VID_CODEC_H264;
  createPrm.maxWidth = width;
  createPrm.maxHeight = height;
  strcpy(createPrm.filename, filename);
  
  status = DECODE_create(&createPrm);
  if(status==OSA_SOK) {
    DECODE_waitComplete();
  }
  DECODE_delete();
  
  return status;
}

char gUI_menuRunning[] = {
"\r\n =================="
"\r\n MCVIP Running Menu"
"\r\n =================="
"\r\n"
"\r\n 1-8: Page Select"
"\r\n"
"\r\n s: Save frame to file"
"\r\n"
"\r\n a: Layout - 1x1"
"\r\n b: Layout - 2x2"
"\r\n c: Layout - 3x3"
"\r\n d: Layout - 8CH"
"\r\n"
"\r\n i: Print profile info"
"\r\n"
"\r\n w: Toggle Write to file enable/disable"
"\r\n "
"\r\n p: Play file"
"\r\n "
"\r\n 0: Stop and Exit"
"\r\n "
"\r\n Enter Choice : "
};

int UI_menuRunning(int mode, int videoInterface, int videoSystem)
{
  char ch;
  Bool done = FALSE, isFieldMode = FALSE;
  int status, i;
  Bool chSave = FALSE;
  
  status = UI_sysStart(mode, videoInterface, videoSystem);
  if(status!=OSA_SOK)
    return status;
 
  do {
    OSA_printf(gUI_menuRunning);

    ch = SYSTEM_getInput();
    OSA_printf("\r\n");

    switch (ch) {

      case '0':
        done = TRUE;
        break;
        
      case 's':
        CAPTURE_saveFrame();
        break;
        
      case 'a':
        DISPLAY_layoutSetMode(DISPLAY_LAYOUT_MODE_LIVE_1x1);
        CAPTURE_selectPage(0);
        break;
        
      case 'b':
        DISPLAY_layoutSetMode(DISPLAY_LAYOUT_MODE_LIVE_2x2);      
        CAPTURE_selectPage(0);        
        break;
      case 'c':
        DISPLAY_layoutSetMode(DISPLAY_LAYOUT_MODE_LIVE_3x3);      
        CAPTURE_selectPage(0);        
        break;
        
      case 'd':
        DISPLAY_layoutSetMode(DISPLAY_LAYOUT_MODE_LIVE_8CH);      
        CAPTURE_selectPage(0);        
        break;
      
      case 'w':
        chSave = !chSave;
        
        if(chSave) {
          OSA_printf(" MCVIP: File write is ENABLED !!!\n");
        } else {
          OSA_printf(" MCVIP: File write is DISABLED !!!\n");
        }
                
        for(i=0; i<8; i++)
          WRITER_enableChSave(i, chSave);
          
        break;  
        
      case 'i':
        SYSTEM_profileInfoShow();
        break;
        
      case 'p':
        //UI_sysPlayFile("CH00_0000_704x480.bits", 704, 480);
        UI_sysPlayFile("./CH00_0000_352x240.bits", 352, 240);        
        break;
        
      case 'f':
        isFieldMode = !isFieldMode;
        
        DISPLAY_fieldModeEnable(isFieldMode);
        break;
        
      case 'k':
        ENCODE_setChKeyFrameInterval(0, 15);
        break;
        
      default:
        if(ch >='1' && ch <='8')
          CAPTURE_selectPage(ch-'1');
        break;
        
    }
  } while(!done);

  UI_sysStop();
  
  SYSTEM_profileInfoShow();

  return OSA_SOK;
}

char gUI_menuMain[] = {
"\r\n ==========================="
"\r\n MCVIP Main Menu (%s mode)"
"\r\n ==========================="
"\r\n"
"\r\n 1: Start - 2Ch D1 - Port A  (BT656 )"
"\r\n 5: Start - 4Ch CIF          (BT656 )"
"\r\n 9: Start - 8Ch CIF          (BT656 )"
#if 1
"\r\n 2: Start - 2Ch D1 - Port B  (BT656 )"
"\r\n 3: Start - 4Ch D1           (BT656 )"
"\r\n 4: Start - 4Ch Half-D1      (BT656 )"
"\r\n 6: Start - 4Ch D1           (BT1120)"
"\r\n 7: Start - 4Ch Half-D1      (BT1120)"
"\r\n 8: Start - 8Ch Half-D1      (BT656 )"
"\r\n a: Start - 4Ch Half-D1 + D1 (BT656 )"
"\r\n b: Start - 4Ch CIF     + D1 (BT656 )"
"\r\n c: Start - 8Ch CIF     + D1 (BT656 )"
#endif
"\r\n"
"\r\n 0: Exit"
"\r\n "
"\r\n Enter Choice : "
};

int UI_menuMain(int videoSys)
{
  char ch;
  Bool done = FALSE, run;
  int mode=MCVIP_VIDEO_DECODER_MODE_4CH_D1, videoIf=MCVIP_VIDEO_IF_MODE_BT656;
  char modeStr[8];
  
  if(videoSys==MCVIP_VIDEO_SYSTEM_NTSC)
    strcpy(modeStr,"NTSC");
  else
    strcpy(modeStr,"PAL ");  
 
  do {
  
    run = FALSE;  
    
    OSA_printf(gUI_menuMain, modeStr);

    ch = SYSTEM_getInput();
    OSA_printf("\r\n");

    switch (ch) {

      case '0':
        run = FALSE;
        done = TRUE;
        break;
        
      case '1':
        mode = MCVIP_VIDEO_DECODER_MODE_2CH_D1_PORT_A;
        videoIf = MCVIP_VIDEO_IF_MODE_BT656;
        run = TRUE;
        break;
      case '5':
        mode = MCVIP_VIDEO_DECODER_MODE_4CH_CIF;
        videoIf = MCVIP_VIDEO_IF_MODE_BT656;
        run = TRUE;
        break;   
      case '9':
        mode = MCVIP_VIDEO_DECODER_MODE_8CH_CIF;
        videoIf = MCVIP_VIDEO_IF_MODE_BT656;
        run = TRUE;
        break;  
                   
      #if 1
      case '2':
        mode = MCVIP_VIDEO_DECODER_MODE_2CH_D1_PORT_B;
        videoIf = MCVIP_VIDEO_IF_MODE_BT656;
        run = TRUE;
        break;
      case '3':
        mode = MCVIP_VIDEO_DECODER_MODE_4CH_D1;
        videoIf = MCVIP_VIDEO_IF_MODE_BT656;
        run = TRUE;
        break;
      case '4':
        mode = MCVIP_VIDEO_DECODER_MODE_4CH_HALF_D1;
        videoIf = MCVIP_VIDEO_IF_MODE_BT656;
        run = TRUE;
        break;
      
      case '6':
        mode = MCVIP_VIDEO_DECODER_MODE_4CH_D1;
        videoIf = MCVIP_VIDEO_IF_MODE_BT1120;
        run = TRUE;
        break;
      case '7':
        mode = MCVIP_VIDEO_DECODER_MODE_4CH_HALF_D1;
        videoIf = MCVIP_VIDEO_IF_MODE_BT1120;
        run = TRUE;
        break;
      case '8':
        mode = MCVIP_VIDEO_DECODER_MODE_8CH_HALF_D1;
        videoIf = MCVIP_VIDEO_IF_MODE_BT656;
        run = TRUE;
        break;
      
      case 'a':
        mode = MCVIP_VIDEO_DECODER_MODE_4CH_HALF_D1_PLUS_D1;
        videoIf = MCVIP_VIDEO_IF_MODE_BT656;
        run = TRUE;
        break;
      case 'b':
        mode = MCVIP_VIDEO_DECODER_MODE_4CH_CIF_PLUS_D1;
        videoIf = MCVIP_VIDEO_IF_MODE_BT656;
        run = TRUE;
        break;
      case 'c':
        mode = MCVIP_VIDEO_DECODER_MODE_8CH_CIF_PLUS_D1;
        videoIf = MCVIP_VIDEO_IF_MODE_BT656;
        run = TRUE;
        break;
      #endif
    }
    
    if(run) {
      UI_menuRunning(mode, videoIf, videoSys);    
      //done = TRUE;
    }
          
  } while(!done);

  return OSA_SOK;
}

void showUsage(char *str)
{
  OSA_printf(" \n");
  OSA_printf(" Multi Channel Video Input Port (MCVIP) Capture Demo, (c) Texas Instruments 2009\n");
  OSA_printf(" \n");  
  OSA_printf(" USAGE: %s <videoSystem> <i2c Mode>\n", str);
  OSA_printf(" \n");  
  OSA_printf(" videoSystem = NTSC (default)  or PAL \n");
  OSA_printf(" i2c Mode    = WinVCC or DM6467 (default) \n");  
  OSA_printf(" \n");    
}

int main(int argc, char **argv)
{
  int status, videoSys;
  int debug=0;
  
  showUsage(argv[0]);
  
  if(argc > 2) {
    if(strcmp(argv[2], "WinVCC")==0) {
      // run in debug mode, WinVCC controls TVP5158 I2C
      debug=1;
    }
  }
  status = SYSTEM_init(debug);
  if(status!=OSA_SOK)
    return status;

  videoSys = MCVIP_VIDEO_SYSTEM_NTSC;
  
  if(argc > 1) {
    if(strcmp(argv[1], "PAL")==0)
      videoSys = MCVIP_VIDEO_SYSTEM_PAL;  
  }
  
  #if 1
  UI_menuMain(videoSys);
  #else
  UI_sysStart(MCVIP_VIDEO_DECODER_MODE_2CH_D1_PORT_A, MCVIP_VIDEO_IF_MODE_BT656, videoSys);
  
  OSA_waitMsecs(1000*60);
  
  UI_sysStop();
  
  SYSTEM_profileInfoShow();
  #endif
  
  OSA_printf(" MCVIP: System Exiting !!!\n");  
  SYSTEM_exit();
  OSA_printf(" MCVIP: Bye Bye !!!\n");  
  OSA_printf(" \n");

  return 0;
}
