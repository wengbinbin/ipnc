
#include <system.h>
#include <capture.h>
#include <display.h>
#include <decode.h>
#include <encode.h>
#include <writer.h>
#include <remoteplay.h>
//add by sxh 0417
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "mcvipsignal.h"
#ifdef VANLINK_DVR_DM365
#include <avserver.h>//20120105
#include <video.h>
#include <avserver_config.h>

VIDEO_Stream_Ctrl gVIDEO_Stream_ctrl;

AVSERVER_Config gAVSERVER_config; //add by sxh
VIDEO_Ctrl gVIDEO_ctrl;
AUDIO_Ctrl gAUDIO_ctrl;
AVSERVER_Ctrl gAVSERVER_ctrl;

#endif
int vl_Mcvip_Quit = 0;
//***************************SIGNAL PROCESS************************
Sigfunc *vlSignalInstall(int signo, Sigfunc *func)
{
    struct sigaction act, oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (signo == SIGALRM)
    {
        #ifdef SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT; // SunOS 4.x
        #endif
    }
    else
    {
        #ifdef SA_RESTART
        act.sa_flags |= SA_RESTART; // SVR4, 4.4BSD
        #endif
    }
    if (sigaction(signo, &act, &oact) < 0)
    {
        return (SIG_ERR);
    }
    return (oact.sa_handler);
}

void vl_Sigterm(int dummy)
{
    printf("mcvip.out:caught SIGTERM: shutting down\n");
    UI_sysStop();
    exit(1);
}

void vl_Sigint(int dummy)
{
    printf("mcvip.out:caught SIGINT: shutting down\n");
    vl_Mcvip_Quit = 1;
    alarm(1);
}

void vl_Sighup(int dummy)
{
    printf("mcvip.out:caught SIGHUP: shutting down\n");
    UI_sysStop();
    exit(1);
}

void vl_Sigsegv(int dummy)
{
    printf("mcvip.out:caught SIGSEGV: shutting down\n");
    //UI_sysStop();
    system("killall  mcvip.out&");
    sleep(3);
    system("killall  -9 mcvip.out");
    system("killall  boa");
    UI_rtspStreamStop();
    system("./av_capture_load.sh");
    exit(1);
}

void vl_Sigkill(int dummy)
{
    printf("mcvip.out:caught SIGKILL: shutting down\n");
    exit(1);
}

void vl_Sigchld(int dummy)
{
    printf("mcvip.out:caught SIGCHLD!\n");
    wait((int*)0);
}


void SetupSignalRoutine()
{
    vlSignalInstall(SIGHUP, vl_Sighup);
    vlSignalInstall(SIGINT, vl_Sigint);
    vlSignalInstall(SIGTERM, vl_Sigterm);
    vlSignalInstall(SIGSEGV, vl_Sigsegv);
    vlSignalInstall(SIGALRM, vl_Sigint);
    vlSignalInstall(SIGKILL, vl_Sigkill);
    vlSignalInstall(SIGCHLD, vl_Sigchld);
}

//***************************SIGNAL PROCESS************************

//add by sxh
int UI_rtspStreamStart()
{
    OSA_printf(" MCVIP UI: Starting Streaming Server...\n");
    system("./wis-streamer &");
    system("./live555MediaServer &");
    OSA_waitMsecs(2000);
    OSA_printf(" MCVIP UI: Starting Streaming Server...DONE\n");
    return 0;
}

int UI_rtspStreamStop()
{
    OSA_printf(" MCVIP UI:: Stopping Streaming Server...\n");
    system("killall wis-streamer");
    system("killall live555MediaServer");
    OSA_waitMsecs(3000);
    OSA_printf(" MCVIP UI:: Stopping Streaming Server...DONE\n");

    return 0;
}

int UI_sysStart(int mode, int videoInterface, int videoSystem)
{
    int status;
    CAPTURE_CreatePrm captureCreatePrm;
    DISPLAY_CreatePrm displayCreatePrm;
    ENCODE_CreatePrm encodeCreatePrm;
    WRITER_CreatePrm writerCreatePrm;
    VIDEO_Stream_CreatePrm videostreamCreatePrm; //20120105
    VIDEO_SWOSDCreatePram videoswosdCreateprm;
    CAPTURE_Info *pCaptureInfo;
    int i, j;
    SetupSignalRoutine();
    OSA_printf(" MCVIP: Starting. Please wait !!!\n");

    captureCreatePrm.videoDecoderMode = mode;
    captureCreatePrm.videoSystem = videoSystem;
    captureCreatePrm.videoIfMode = videoInterface;
    captureCreatePrm.frameSkipMask = 0xFFFFFFFF;

    if (mode == MCVIP_VIDEO_DECODER_MODE_8CH_CIF)
    {
        captureCreatePrm.frameSkipMask = 0xAAAAAAAA;
    }

    OSA_printf(" MCVIP: Capture Starting !!!\n");

    status = CAPTURE_create(&captureCreatePrm);
    if (status != OSA_SOK)
    {
        OSA_ERROR("CAPTURE_create()\r\n");
        return status;
    }

    displayCreatePrm.displayMode = DISPLAY_MODE_COMPOSITE;
    displayCreatePrm.layoutMode = DISPLAY_LAYOUT_MODE_LIVE_2x2;

    OSA_printf(" MCVIP: Display Starting !!!\n");

    status = DISPLAY_create(&displayCreatePrm);
    if (status != OSA_SOK)
    {
        OSA_ERROR("vl DISPLAY_create()\r\n");
        CAPTURE_delete();
        return status;
    }

    pCaptureInfo = CAPTURE_getInfo();

    //********************************************//add by sxh
    for (i = 0; i < AVSERVER_MAX_STREAMS; i++)
    {
        gVIDEO_ctrl.captureStream[i].captureOutWidth = 360; //pCaptureInfo->chList.info[0].width;
        gVIDEO_ctrl.captureStream[i].captureOutHeight = 240; //pCaptureInfo->chList.info[0].height;

        gVIDEO_ctrl.captureStream[i].captureOutOffsetH = 360; //pCaptureInfo->chList.info[0].offsetH;
        gVIDEO_ctrl.captureStream[i].captureOutOffsetV = 240; //pCaptureInfo->chList.info[0].offsetV;
        gVIDEO_ctrl.captureStream[i].captureCount = 0;
    }

    for (i = 0; i < AVSERVER_MAX_STREAMS; i++)
    {
        gAVSERVER_config.captureConfig[i].swosdConfig.swosdTimeFormat = SWOSD_FMT_24HR;
        gAVSERVER_config.captureConfig[i].swosdConfig.swosdDateFormat = SWOSD_FMT_YMD;
        gAVSERVER_config.encodeConfig[i].codecType = ALG_VID_CODEC_H264;
        gAVSERVER_config.encodeConfig[i].rateControlType == ALG_VID_ENC_RATE_CONTROL_VBR;
        gAVSERVER_config.encodeConfig[i].codecBitrate = 200000;
        gAVSERVER_config.encodeConfig[i].cropWidth = 352; //pCaptureInfo->chList.info[0].width;
        gAVSERVER_config.encodeConfig[i].cropHeight = 240; //pCaptureInfo->chList.info[0].height;
        gAVSERVER_config.captureConfig[i].swosdConfig.swosdLogoStringUpdate = OSD_YES;
        gAVSERVER_config.captureConfig[i].swosdConfig.swosdTextPos = SWOSD_FMT_TOP_RIGHT;
        gAVSERVER_config.captureConfig[i].swosdConfig.swosdLogoPos = SWOSD_FMT_TOP_LEFT;
        //sprintf(gAVSERVER_config.captureConfig[i].swosdConfig.swosdDispString,"%s","VANLINKVANLINK");
        gAVSERVER_config.captureConfig[i].swosdConfig.swosdType = SWOSD_BASIC;
        gAVSERVER_config.captureConfig[i].swosdConfig.swosdDetailEnable = FALSE;
        gAVSERVER_config.captureConfig[i].swosdConfig.swosdLogoEnable = TRUE;
        gAVSERVER_config.captureConfig[i].swosdConfig.swosdTextEnable = TRUE;
        gAVSERVER_config.captureConfig[i].swosdConfig.swosdTimeEnable = FALSE;
        gAVSERVER_config.captureConfig[i].swosdConfig.swosdDateEnable = FALSE;
        gAVSERVER_config.captureConfig[i].swosdConfig.swosdDatePos = SWOSD_FMT_BOTTOM_LEFT;
        gAVSERVER_config.captureConfig[i].swosdConfig.swosdDateTimeUpdate = OSD_NO;
        gAVSERVER_config.captureConfig[i].swosdConfig.swosdEnable = TRUE;
        for (j = 0; j < 8; j++)
        {
            gAVSERVER_config.captureConfig[i].swosdConfig.swosdFREnable[j] = FALSE;
        }
        gAVSERVER_config.captureConfig[i].swosdConfig.swosdHistEnable = FALSE;
        gAVSERVER_config.captureConfig[i].swosdConfig.swosdTimePos = SWOSD_FMT_BOTTOM_RIGHT;
    }

    gAVSERVER_config.numCaptureStream = AVSERVER_MAX_STREAMS;
    gAVSERVER_config.numEncodeStream = AVSERVER_MAX_STREAMS;
    gAVSERVER_config.autoFocusVal = 0;

    //********************************************//add by sxh
    OSA_printf(" MCVIP: VIDEO_swosdCreate Begin!!!\n");
    videoswosdCreateprm.numCh = pCaptureInfo->chList.numCh;
    videoswosdCreateprm.frameWidth = pCaptureInfo->chList.info[0].width;
    videoswosdCreateprm.frameHeight = pCaptureInfo->chList.info[0].height;
    status = VIDEO_swosdCreate(videoswosdCreateprm);
    if (status != OSA_SOK)
    {
        OSA_ERROR("VIDEO_swosdCreate()\r\n");
        CAPTURE_delete();
        DISPLAY_delete();
        ENCODE_delete();
        return status;
    }

    encodeCreatePrm.numCh = pCaptureInfo->chList.numCh;
    encodeCreatePrm.frameWidth = pCaptureInfo->chList.info[0].width;
    encodeCreatePrm.frameHeight = pCaptureInfo->chList.info[0].height;

    if (encodeCreatePrm.frameWidth == 720)
    {
        encodeCreatePrm.frameWidth = 704;
    }
    if (encodeCreatePrm.frameWidth == 360)
    {
        encodeCreatePrm.frameWidth = 352;
    }

    OSA_printf(" MCVIP: Encode Starting (ch=%d, %dx%d)!!!\n", encodeCreatePrm.numCh, encodeCreatePrm.frameWidth, encodeCreatePrm.frameHeight);
    status = ENCODE_create(&encodeCreatePrm);
    if (status != OSA_SOK)
    {
        OSA_ERROR("ENCODE_create()\r\n");
        CAPTURE_delete();
        DISPLAY_delete();
        return status;
    }
    #ifdef VANLINK_DVR_DM365
    OSA_printf(" MCVIP: Encode Create Done!!!\n");
    //OSA_waitMsecs(10);
    #endif


    #if 0
    OSA_printf(" MCVIP: Writer Starting !!!\n");

    writerCreatePrm.numCh = encodeCreatePrm.numCh;
    writerCreatePrm.frameWidth = encodeCreatePrm.frameWidth;
    writerCreatePrm.frameHeight = encodeCreatePrm.frameHeight;

    status = WRITER_create(&writerCreatePrm);
    if (status != OSA_SOK)
    {
        OSA_ERROR("WRITER_create()\r\n");
        CAPTURE_delete();
        DISPLAY_delete();
        ENCODE_delete();
        return status;
    }
    #endif //add by sxh
    #ifdef VANLINK_DVR_DM365
    OSA_printf(" MCVIP: Writer Skipping !!!\n");
    //OSA_waitMsecs(10);
    #endif

    #ifdef VANLINK_DVR_STREAM_ENABLE
    videostreamCreatePrm.numCh = pCaptureInfo->chList.numCh;
    videostreamCreatePrm.frameWidth = pCaptureInfo->chList.info[0].width;
    videostreamCreatePrm.frameHeight = pCaptureInfo->chList.info[0].height;
    #ifdef VANLINK_DVR_DM365
    OSA_printf(" MCVIP: VIDEO_streamCreate Create Begin!!!\n");
    //OSA_waitMsecs(10);
    #endif
    status = VIDEO_streamCreate(&videostreamCreatePrm);
    #ifdef VANLINK_DVR_DM365
    OSA_printf(" MCVIP: VIDEO_streamCreate Create Done!!!\n");
    //OSA_waitMsecs(10);
    #endif
    if (status != OSA_SOK)
    {
        OSA_ERROR("VIDEO_STREAM_create()\r\n");
        CAPTURE_delete();
        DISPLAY_delete();
        ENCODE_delete();
        WRITER_delete();
        return status;
    }
    #endif

    status = OSA_tskSendMsg(&gVIDEO_ctrl.swosdTsk, &gVIDEO_ctrl.captureTsk, AVSERVER_CMD_CREATE, NULL, OSA_MBX_WAIT_ACK);



    OSA_printf(" MCVIP: REMOTEPLAY_create !!!\n");
    status = REMOTEPLAY_create();
    if (status == OSA_SOK)
    {
        OSA_printf("REMOTEPLAY_create()\r\n");
    }
    if (status != OSA_SOK)
    {
        OSA_ERROR("DISPLAY_start()\r\n");
        DISPLAY_delete();
        CAPTURE_delete();
        ENCODE_delete();
        WRITER_delete();
        #ifdef VANLINK_DVR_STREAM_ENABLE
        VIDEO_streamDelete();
        #endif
        REMOTEPLAY_delete();
        return status;
    }


    OSA_printf(" MCVIP: Display Enable !!!\n");

    status = DISPLAY_start();
    if (status != OSA_SOK)
    {
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
    if (status != OSA_SOK)
    {
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

    OSA_printf(" MCVIP: System Exiting !!!\n");

    CAPTURE_stop();
    OSA_printf(" CAPTURE_stop  !!!\n");
    DISPLAY_stop();
    OSA_printf(" DISPLAY_stop  !!!\n");

    VIDEO_swosdDelete();
    VIDEO_streamDelete();
    OSA_printf(" VIDEO_streamDelete !!!\n");
    OSA_printf(" WRITER_delete !!!\n");

    CAPTURE_delete();
    OSA_printf(" CAPTURE_delete!!!\n");
    DISPLAY_delete();
    OSA_printf(" DISPLAY_delete!!!\n");

    ENCODE_delete();
    OSA_printf("ENCODE_delete !!!\n");
    //WRITER_delete();


    REMOTEPLAY_delete();
    UI_rtspStreamStop();
    OSA_printf(" MCVIP: RemotePlayback Delete!!\n");
    SYSTEM_exit();
    OSA_printf(" MCVIP: Bye Bye !!!\n");
    OSA_printf(" \n");

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
    if (status == OSA_SOK)
    {
        DECODE_waitComplete();
    }
    DECODE_delete();

    return status;
}

char gUI_menuRunning[] =
{
    "\r\n ==================""\r\n MCVIP Running Menu""\r\n ==================""\r\n""\r\n 1-8: Page Select""\r\n""\r\n s: Save frame to file""\r\n""\r\n a: Layout - 1x1""\r\n b: Layout - 2x2""\r\n c: Layout - 3x3""\r\n d: Layout - 8CH""\r\n""\r\n i: Print profile info""\r\n""\r\n w: Toggle Write to file enable/disable""\r\n ""\r\n p: Play file""\r\n ""\r\n 0: Stop and Exit""\r\n ""\r\n Enter Choice : "
};

int UI_menuRunning(int mode, int videoInterface, int videoSystem)
{
    char ch;
    Bool done = FALSE, isFieldMode = FALSE;
    int status, i;
    Bool chSave = FALSE;

    #if DEBUG==0
    printf("UI_menuRunning\n");
    #endif

    status = UI_sysStart(mode, videoInterface, videoSystem);
    if (status != OSA_SOK)
    {
        return status;
    }
    #if 0
    do
    {
        OSA_printf(gUI_menuRunning);

        ch = SYSTEM_getInput();
        OSA_printf("\r\n");

        switch (ch)
        {

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

                if (chSave)
                {
                    OSA_printf(" MCVIP: File write is ENABLED !!!\n");
                }
                else
                {
                    OSA_printf(" MCVIP: File write is DISABLED !!!\n");
                }

                for (i = 0; i < 8; i++)
                {
                    WRITER_enableChSave(i, chSave);
                }

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
                if (ch >= '1' && ch <= '8')
                {
                    CAPTURE_selectPage(ch - '1');
                }
                break;

        }
    }
    while (!done);

    UI_sysStop()
        ;
    #endif
    do
    {
        SYSTEM_profileInfoShow();
        //sleep(400);
        //VIDEO_Stream_enableChSaveEx1(0,0,0,0,"CAM0");
        //VIDEO_Stream_enableChSaveEx1(1,0,0,0,"CAM0");
        sleep(30);
        /*
        struct tm tm_ptr;
        time_t the_time;
        unsigned long rectime;

        int year;
        int month;
        int day;
        int hour;
        int minute;
        int second;
        int channel;
        year=1970;
        month=1;
        day=1;
        hour=8;
        minute=1;
        second=0;
        channel=0;
        printf("~~~~~~~~~~~date=%d-%d-%d %d:%d:%d  channel=%d\n",year,month,day,hour,minute,second,channel);
        printf("~~~~~~~~~~~date=%d-%d-%d %d:%d:%d  channel=%d\n",year,month,day,hour,minute,second,channel);
        printf("~~~~~~~~~~~date=%d-%d-%d %d:%d:%d  channel=%d\n",year,month,day,hour,minute,second,channel);
        tm_ptr.tm_year  = year - 1900 ;
        tm_ptr.tm_mon  = month - 1;
        tm_ptr.tm_mday = day ;
        tm_ptr.tm_hour = hour ;
        tm_ptr.tm_min  = minute ;
        tm_ptr.tm_sec  = second ;

        rectime = mktime(&tm_ptr) ;
        REMOTEPLAY_start(channel, rectime) ;
        sleep(500);*/
    }
    while (!vl_Mcvip_Quit);
    UI_sysStop()
        ;
    return OSA_SOK;
}

char gUI_menuMain[] =
{
    "\r\n ==========================="
    "\r\n MCVIP Main Menu (%s mode)"
    "\r\n ==========================="
    "\r\n""\r\n 1: Start - 2Ch D1 - Port A  (BT656 )"
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
    int mode = MCVIP_VIDEO_DECODER_MODE_4CH_D1, videoIf = MCVIP_VIDEO_IF_MODE_BT656;
    char modeStr[8];

    #if DEBUG==0
    printf("UI_menuMain videoSys=%d\n",videoSys);
    #endif

    if (videoSys == MCVIP_VIDEO_SYSTEM_NTSC)
    {
        strcpy(modeStr, "NTSC");
    }
    else
    {
        strcpy(modeStr, "PAL ");
    }

    do
    {

        run = FALSE;

        //OSA_printf(gUI_menuMain, modeStr);

        OSA_printf("UImemMain %s\n",modeStr);

        //ch = SYSTEM_getInput();
        OSA_printf("\r\n");

        switch ('5')
        {

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
                OSA_printf("run on MCVIP_VIDEO_DECODER_MODE_4CH_CI & MCVIP_VIDEO_IF_MODE_BT656\n");
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

        if (run)
        {
            UI_menuRunning(mode, videoIf, videoSys);
            done = TRUE;
        }

    }
    while (!done);

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
    int debug = 0;

    showUsage(argv[0]);

    if (argc > 2)
    {
        if (strcmp(argv[2], "WinVCC") == 0)
        {
            // run in debug mode, WinVCC controls TVP5158 I2C
            debug = 1;
        }
    }
    status = SYSTEM_init(debug);
    if (status != OSA_SOK)
    {
        return status;
    }

    videoSys = MCVIP_VIDEO_SYSTEM_NTSC;

    if (argc > 1)
    {
        if (strcmp(argv[1], "PAL") == 0)
        {
            videoSys = MCVIP_VIDEO_SYSTEM_PAL;
        }
    }

    #if 1
    UI_menuMain(videoSys);
    #else
    UI_sysStart(MCVIP_VIDEO_DECODER_MODE_2CH_D1_PORT_A, MCVIP_VIDEO_IF_MODE_BT656, videoSys);

    OSA_waitMsecs(1000 *60);

    UI_sysStop();

    SYSTEM_profileInfoShow();
    #endif



    return 0;
}
