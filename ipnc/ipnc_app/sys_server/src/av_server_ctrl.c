/** ===========================================================================
 * @file av_server_ctrl.c
 *
 * @path $(IPNCPATH)\sys_adm\system_server
 *
 * @desc
 * .
 * Copyright (c) Appro Photoelectron Inc.  2009
 *
 * Use of this software is controlled by the terms and conditions found
 * in the license agreement under which this software has been supplied
 *
 * =========================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <asm/types.h>
#include <file_msg_drv.h>
#include <av_server_ctrl.h>
#include <ApproDrvMsg.h>
#include <Appro_interface.h>
#include <system_control.h>

#define JPEG_HIGH    50
#define JPEG_NORMAL    40
#define JPEG_LOW    30

#define FLAG_STREAM1    (0x01)
#define FLAG_STREAM2    (0x02)
#define FLAG_STREAM3    (0x04)
int ptzSockfd =  - 1; //add by sxh
#define __E(fmt, args...) fprintf(stderr, "Error: " fmt, ## args)

void SetCodecROIParam(int id);
void SetCodecAdvParam(int id);

static int CheckValueBound(int value, int min, int max)
{
    value = (value < min) ? min : value;
    value = (value > max) ? max : value;

    return value;
}

static void attach_opt(char *dst, char *src)
{
    if (strlen(dst) > 0)
    {
        strcat(dst, " ");
    }
    strcat(dst, src);
}

static int no_advfeature(__u8 mode, __u8 res, __u8 advMode)
{
    if (mode == 2)
    {
        return NO_ADV_FTR;
    }
    else if (mode == 6 && res == 2)
    {
        return (advMode &PREPR_RESET);
    }
    else if (mode == 5 && res == 2)
    {
        return (advMode &PREPR_RESET);
    }
    else if (mode == 0 && (res == 3 || res == 2))
    {
        return (advMode &PREPR_RESET);
    }
    else if (mode == 1 && (res == 3 || res == 2))
    {
        return (advMode &PREPR_RESET);
    }
    else
    {
        return advMode;
    }
}

static int no_binskip(__u8 mode, __u8 res)
{
    if (mode == 2)
    {
        return 1;
    }
    else if (mode == 6 && res == 2)
    {
        return 1;
    }
    else if (mode == 5 && res == 2)
    {
        return 1;
    }
    else if (mode == 0 && (res == 3 || res == 2))
    {
        return 1;
    }
    else if (mode == 1 && (res == 3 || res == 2))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static int no_mirror(__u8 mode, __u8 res, __u8 mirror)
{
    if (mode == 2)
    {
        return 0;
    }
    else if (mode == 6 && res == 2)
    {
        return 0;
    }
    else if (mode == 5 && res == 2)
    {
        return 0;
    }
    else if (mode == 0 && (res == 3 || res == 2))
    {
        return 0;
    }
    else if (mode == 1 && (res == 3 || res == 2))
    {
        return 0;
    }
    else
    {
        return mirror;
    }
}

/**
 * @brief Start stream
 *
 * @param *pConfig Pointer to stream settings
 * @retval 0 Success
 * @retval -1 Fail.
 */
int StartStream(StreamEnv_t *pConfig)
{
    char cmd[512] =
    {
        0
    };
    char cmdopt[256] = "FD OSD";
    char *audio_SR[] =
    {
        "AUDIO 8000", "AUDIO 16000"
    };
    char *codec_tpye[3] =
    {
        "H264", "MPEG4", "JPEG"
    };
    char *aewb_con[] =
    {
        "", "AE", "AWB", "AEWB"
    };
    char *aewb_ven[] =
    {
        "NONE2A", "APPRO2A", "TI2A"
    };
    char *brate_con[] =
    {
        "RCOFF", "VBR", "CBR"
    };
    char *mirror_con[] =
    {
        "", "FLIPH", "FLIPV", "FLIPH FLIPV"
    };
    char *mecfg_con[] =
    {
        "AUTO", "CUSTOM"
    };
    char demoFlg[] = "DEMO";
    char adv_VS[] = "VS";
    char adv_LDC[] = "LDC";
    char adv_SNF[] = "SNF";
    char adv_TNF[] = "TNF";
    char win_mode[] = "WIN";
    char vnf_demo[] = "VNFDEMO";
    char *chip_cfg[2] =
    {
        "DM365", "DM368"
    };
    char *sw_brc1,  *sw_brc2,  *sw_mir,  *sw_aewb;
    char *sw_mecfg1,  *sw_mecfg2,  *sw_mecfg3;
    int chipConfig = CheckCpuSpeed();

    printf("DEMOCFG Value    : %d\n", pConfig->nDemoCfg);
    printf("Videocodecmode    : %d\n", pConfig->nVideocodecmode);
    printf("Videocodecres     : %d\n", pConfig->nVideocodecres);

    sw_brc1 = brate_con[pConfig->nRateControl1];
    sw_brc2 = brate_con[pConfig->nRateControl2];

    sw_mecfg1 = mecfg_con[pConfig->nMEConfig1];
    sw_mecfg2 = mecfg_con[pConfig->nMEConfig2];
    sw_mecfg3 = mecfg_con[pConfig->nMEConfig3];

    fSetChipConfig(chipConfig);
    attach_opt(cmdopt, chip_cfg[chipConfig]);

    if (pConfig->nBinning == WINDOW)
    {
        pConfig->AdvanceMode = 0;
        fSetAdvMode(pConfig->AdvanceMode);
        attach_opt(cmdopt, win_mode);
    }

    if (pConfig->nDemoCfg)
    {
        attach_opt(cmdopt, demoFlg);
    }

    switch (pConfig->nDemoCfg)
    {
        case VNF_DEMO:
            pConfig->nFaceDetect = 0;
            pConfig->nAEWswitch = 1;
            fSetImageAEW(pConfig->nAEWswitch);
            /* Force to Appro 2A */
            pConfig->nBinning = SKIPPING;
            fSetBinning(pConfig->nBinning);
            /* Set skipping to increase Noise Level */
            attach_opt(cmdopt, vnf_demo);
            pConfig->AdvanceMode = FFLAG_SNF | FFLAG_TNF;
            break;

        case VS_DEMO:
            pConfig->nFaceDetect = 0;
            pConfig->AdvanceMode = FFLAG_VS;
            pConfig->nBinning = BINNING;
            break;

        case LDC_DEMO:
            pConfig->nFaceDetect = 0;
            pConfig->AdvanceMode = FFLAG_LDC;
            pConfig->nBinning = BINNING;
            break;

        case FD_DEMO:
            pConfig->AdvanceMode = 0;
            pConfig->nFaceDetect = 1;
            pConfig->nBitrate1 = 4000000;
            pConfig->nBinning = BINNING;
            break;

        case ROI_FD_DEMO:
            pConfig->AdvanceMode = 0;
            pConfig->nFaceDetect = 1;
            pConfig->nBitrate1 = 512000;
            pConfig->nBinning = BINNING;
            break;

        case ROI_CENTER_DEMO:
            pConfig->AdvanceMode = 0;
            pConfig->nFaceDetect = 0;
            pConfig->nBitrate1 = 512000;
            pConfig->nBinning = BINNING;
            break;
    }

    if ((pConfig->nDemoCfg == VNF_DEMO) || (pConfig->nDemoCfg == VS_DEMO) || (pConfig->nDemoCfg == LDC_DEMO))
    {
        fSetAdvMode(pConfig->AdvanceMode);
        fSetFaceDetect(pConfig->nFaceDetect);
        fSetBinning(pConfig->nBinning);
        pConfig->nBitrate1 = 1000000;
        fSetMP41bitrate(pConfig->nBitrate1);
        pConfig->nBitrate2 = 1000000;
        fSetMP42bitrate(pConfig->nBitrate2);
        pConfig->nVideocodecmode = 5;
        fSetVideoCodecMode(5);
        pConfig->nVideocodecres = 1;
        fSetVideoCodecRes(1);
        fSetVideoCodecCombo(2);
        fSetVideoMode(1);
    }
    else if ((pConfig->nDemoCfg == FD_DEMO) || (pConfig->nDemoCfg == ROI_FD_DEMO) || (pConfig->nDemoCfg == ROI_CENTER_DEMO))
    {
        fSetAdvMode(pConfig->AdvanceMode);
        fSetFaceDetect(pConfig->nFaceDetect);
        fSetBinning(pConfig->nBinning);
        fSetMP41bitrate(pConfig->nBitrate1);
        pConfig->nVideocodecmode = 0;
        fSetVideoCodecMode(0);
        pConfig->nVideocodecres = 0;
        fSetVideoCodecRes(0);
        fSetVideoCodecCombo(0);
        fSetVideoMode(0);
    }

    pConfig->AdvanceMode = no_advfeature(pConfig->nVideocodecmode, pConfig->nVideocodecres, pConfig->AdvanceMode);
    fSetAdvMode(pConfig->AdvanceMode);

    if (pConfig->AdvanceMode > 0)
    {
        unsigned char value = pConfig->AdvanceMode;
        if ((value &FFLAG_VS) > 0)
        {
            attach_opt(cmdopt, adv_VS);
        }
        if ((value &FFLAG_LDC) > 0)
        {
            attach_opt(cmdopt, adv_LDC);
        }
        if ((value &FFLAG_SNF) > 0)
        {
            attach_opt(cmdopt, adv_SNF);
        }
        if ((value &FFLAG_TNF) > 0)
        {
            attach_opt(cmdopt, adv_TNF);
        }
    }

    if (pConfig->nMirror > 0)
    {
        pConfig->nMirror = no_mirror(pConfig->nVideocodecmode, pConfig->nVideocodecres, pConfig->nMirror);
        fSetMirror(pConfig->nMirror);
        sw_mir = mirror_con[(pConfig->nMirror)];
        attach_opt(cmdopt, sw_mir);
    }

    if (pConfig->nAEWtype > 0)
    {
        sw_aewb = aewb_con[3]; /* Always force to AEWB mode and change runtime */
        attach_opt(cmdopt, sw_aewb);
    }

    attach_opt(cmdopt, aewb_ven[pConfig->nAEWswitch]);
    attach_opt(cmdopt, audio_SR[pConfig->audioSampleRate]);

    int vlk_tempInteger = 10;
    switch (vlk_tempInteger)
    {
        //pConfig -> nVideocodecmode) {
        //#ifdef VANLINK_DVR_STREAM_ENABLE
        case 10:
            {
                fSetStreamActive(1, 1, 1, 1, 1, 1, 1, 1);
                if (pConfig->nVideocodecres != 0)
                {
                    pConfig->nVideocodecres = 0;
                    fSetVideoCodecRes(0);
                    __E("\nCODEC Resolution Error mode9\n");
                }
                sprintf(cmd, "./mcvip.out &");
                fSetVideoSize(1, 352, 240);
                fSetVideoSize(2, 352, 240);
                fSetVideoSize(3, 352, 240);
                fSetVideoSize(4, 352, 240);

                fSetVideoSize(5, 352, 240);
                fSetVideoSize(6, 352, 240);
                fSetVideoSize(7, 352, 240);
                fSetVideoSize(8, 352, 240);

                fSetStreamConfig(0, 352, 240, codec_tpye[0], H264_1_PORTNUM);
                fSetStreamConfig(1, 352, 240, codec_tpye[0], H264_3_PORTNUM);
                fSetStreamConfig(2, 352, 240, codec_tpye[0], H264_4_PORTNUM);
                fSetStreamConfig(3, 352, 240, codec_tpye[0], H264_5_PORTNUM);

                fSetStreamConfig(4, 352, 240, codec_tpye[0], H264_1_PORTNUM);
                fSetStreamConfig(5, 352, 240, codec_tpye[0], H264_3_PORTNUM);
                fSetStreamConfig(6, 352, 240, codec_tpye[0], H264_4_PORTNUM);

                fSetStreamConfig(7, 352, 240, codec_tpye[0], H264_5_PORTNUM);
                break;
            }
            //#endif
        case 9:
            {
                // Dual MPEG4 + MJPEG
                fSetStreamActive(1, 1, 1, 0, 0, 0, 0, 0);
                if (pConfig->nVideocodecres != 0)
                {
                    pConfig->nVideocodecres = 0;
                    fSetVideoCodecRes(0);
                    __E("\nCODEC Resolution Error mode9\n");
                }
                sprintf(cmd, "./av_server.out %s 720P_VGA_30 MPEG4 %d %s %s MJPEG %d MPEG4 %d %s %s MENUOFF &\n", cmdopt, pConfig->nBitrate1, sw_brc1, sw_mecfg1, pConfig->nJpegQuality, pConfig->nBitrate2, sw_brc2, sw_mecfg3);
                fSetVideoSize(1, 640, 352);
                fSetVideoSize(2, 1280, 720);
                fSetVideoSize(3, 320, 192);
                fSetStreamConfig(0, 1280, 720, codec_tpye[1], MPEG4_1_PORTNUM);
                fSetStreamConfig(1, 640, 352, codec_tpye[2], MJPEG_PORTNUM);
                fSetStreamConfig(2, 320, 192, codec_tpye[1], MPEG4_2_PORTNUM);
                break;
            }
        case 8:
            {
                // Dual H.264 + MJPEG
                fSetStreamActive(1, 0, 0, 0, 1, 1, 0, 0);
                if (pConfig->nVideocodecres != 0)
                {
                    pConfig->nVideocodecres = 0;
                    fSetVideoCodecRes(0);
                    __E("\nCODEC Resolution Error mode8\n");
                }
                sprintf(cmd, "./av_server.out %s 720P_VGA_30 H264 %d %s %s MJPEG %d H264 %d %s %s MENUOFF &\n", cmdopt, pConfig->nBitrate1, sw_brc1, sw_mecfg1, pConfig->nJpegQuality, pConfig->nBitrate2, sw_brc2, sw_mecfg3);
                fSetVideoSize(1, 640, 352);
                fSetVideoSize(5, 1280, 720);
                fSetVideoSize(6, 320, 192);
                fSetStreamConfig(0, 1280, 720, codec_tpye[0], H264_1_PORTNUM);
                fSetStreamConfig(1, 640, 352, codec_tpye[2], MJPEG_PORTNUM);
                fSetStreamConfig(2, 320, 192, codec_tpye[0], H264_2_PORTNUM);
                break;
            }
        case 7:
            {
                // H.264 + MPEG4: Only D1+D1
                fSetStreamActive(0, 0, 1, 0, 1, 0, 0, 0);
                if (pConfig->nVideocodecres != 0)
                {
                    pConfig->nVideocodecres = 0;
                    fSetVideoCodecRes(0);
                    __E("\nCODEC Resolution Error mode7\n");
                }
                sprintf(cmd, "./av_server.out %s D1_D1 H264 %d %s %s MPEG4 %d %s %s MENUOFF &\n", cmdopt, pConfig->nBitrate1, sw_brc1, sw_mecfg1, pConfig->nBitrate2, sw_brc2, sw_mecfg2);
                fSetVideoSize(5, 720, 480);
                fSetVideoSize(3, 720, 480);
                fSetStreamConfig(0, 720, 480, codec_tpye[0], H264_1_PORTNUM);
                fSetStreamConfig(1, 720, 480, codec_tpye[1], MPEG4_2_PORTNUM);
                break;
            }
        case 6:
            {
                // DUAL MPEG4
                fSetStreamActive(0, 1, 1, 0, 0, 0, 0, 0);
                if (pConfig->nVideocodecres == 2)
                {
                    /* limit bitrate to 8M */
                    if (pConfig->nBitrate1 > 8000000)
                    {
                        pConfig->nBitrate1 = 8000000;
                        fSetMP41bitrate(pConfig->nBitrate1);
                    }
                    sprintf(cmd, "./av_server.out %s 1080P MPEG4 %d %s %s MPEG4 %d %s %s MENUOFF &\n", cmdopt, pConfig->nBitrate1, sw_brc1, sw_mecfg1, pConfig->nBitrate2, sw_brc2, sw_mecfg2);
                    fSetVideoSize(2, 1920, 1080);
                    fSetVideoSize(3, 320, 192);
                    fSetStreamConfig(0, 1920, 1080, codec_tpye[1], MPEG4_1_PORTNUM);
                    fSetStreamConfig(1, 320, 192, codec_tpye[1], MPEG4_2_PORTNUM);
                }
                else if (pConfig->nVideocodecres == 1)
                {
                    sprintf(cmd, "./av_server.out %s D1_D1 MPEG4 %d %s %s MPEG4 %d %s %s MENUOFF &\n", cmdopt, pConfig->nBitrate1, sw_brc1, sw_mecfg1, pConfig->nBitrate2, sw_brc2, sw_mecfg2);
                    fSetVideoSize(2, 720, 480);
                    fSetVideoSize(3, 720, 480);
                    fSetStreamConfig(0, 720, 480, codec_tpye[1], MPEG4_1_PORTNUM);
                    fSetStreamConfig(1, 720, 480, codec_tpye[1], MPEG4_2_PORTNUM);
                }
                else
                {
                    if (pConfig->nVideocodecres != 0)
                    {
                        pConfig->nVideocodecres = 0;
                        fSetVideoCodecRes(0);
                        __E("\nCODEC Resolution Error mode6\n");
                    }
                    sprintf(cmd, "./av_server.out %s 720P MPEG4 %d %s %s MPEG4 %d %s %s MENUOFF &\n", cmdopt, pConfig->nBitrate1, sw_brc1, sw_mecfg1, pConfig->nBitrate2, sw_brc2, sw_mecfg2);
                    fSetVideoSize(2, 1280, 720);
                    fSetVideoSize(3, 320, 192);
                    fSetStreamConfig(0, 1280, 720, codec_tpye[1], MPEG4_1_PORTNUM);
                    fSetStreamConfig(1, 320, 192, codec_tpye[1], MPEG4_2_PORTNUM);
                }
                break;
            }
        case 5:
            {
                // DUAL H.264
                fSetStreamActive(0, 0, 0, 0, 1, 1, 0, 0);
                if (pConfig->nVideocodecres == 2)
                {
                    /* limit bitrate to 8M */
                    if (pConfig->nBitrate1 > 8000000)
                    {
                        pConfig->nBitrate1 = 8000000;
                        fSetMP41bitrate(pConfig->nBitrate1);
                    }
                    sprintf(cmd, "./av_server.out %s 1080P H264 %d %s %s H264 %d %s %s MENUOFF &\n", cmdopt, pConfig->nBitrate1, sw_brc1, sw_mecfg1, pConfig->nBitrate2, sw_brc2, sw_mecfg2);
                    fSetVideoSize(5, 1920, 1080);
                    fSetVideoSize(6, 320, 192);
                    fSetStreamConfig(0, 1920, 1080, codec_tpye[0], H264_1_PORTNUM);
                    fSetStreamConfig(1, 320, 192, codec_tpye[0], H264_2_PORTNUM);
                }
                else if (pConfig->nVideocodecres == 1)
                {
                    sprintf(cmd, "./av_server.out %s D1_D1 H264 %d %s %s H264 %d %s %s MENUOFF &\n", cmdopt, pConfig->nBitrate1, sw_brc1, sw_mecfg1, pConfig->nBitrate2, sw_brc2, sw_mecfg2);
                    fSetVideoSize(5, 720, 480);
                    fSetVideoSize(6, 720, 480);
                    fSetStreamConfig(0, 720, 480, codec_tpye[0], H264_1_PORTNUM);
                    fSetStreamConfig(1, 720, 480, codec_tpye[0], H264_2_PORTNUM);
                }
                else
                {
                    if (pConfig->nVideocodecres != 0)
                    {
                        pConfig->nVideocodecres = 0;
                        fSetVideoCodecRes(0);
                        __E("\nCODEC Resolution Error mode5\n");
                    }
                    sprintf(cmd, "./av_server.out %s 720P H264 %d %s %s H264 %d %s %s MENUOFF &\n", cmdopt, pConfig->nBitrate1, sw_brc1, sw_mecfg1, pConfig->nBitrate2, sw_brc2, sw_mecfg2);
                    fSetVideoSize(5, 1280, 720);
                    fSetVideoSize(6, 320, 192);
                    fSetStreamConfig(0, 1280, 720, codec_tpye[0], H264_1_PORTNUM);
                    fSetStreamConfig(1, 320, 192, codec_tpye[0], H264_2_PORTNUM);
                }
                break;
            }
        case 4:
            {
                // MPEG4 + JPEG
                fSetStreamActive(1, 1, 0, 0, 0, 0, 0, 0);
                if (pConfig->nVideocodecres == 2)
                {
                    sprintf(cmd, "./av_server.out %s 720P_720P_30 MPEG4 %d %s %s MJPEG %d MENUOFF &\n", cmdopt, pConfig->nBitrate1, sw_brc1, sw_mecfg1, pConfig->nJpegQuality);
                    fSetVideoSize(1, 1280, 720);
                    fSetVideoSize(2, 1280, 720);
                    fSetStreamConfig(0, 1280, 720, codec_tpye[1], MPEG4_1_PORTNUM);
                    fSetStreamConfig(1, 1280, 720, codec_tpye[2], MJPEG_PORTNUM);
                }
                else if (pConfig->nVideocodecres == 1)
                {
                    sprintf(cmd, "./av_server.out %s D1_D1 MPEG4 %d %s %s MJPEG %d MENUOFF &\n", cmdopt, pConfig->nBitrate1, sw_brc1, sw_mecfg1, pConfig->nJpegQuality);
                    fSetVideoSize(1, 720, 480);
                    fSetVideoSize(2, 720, 480);
                    fSetStreamConfig(0, 720, 480, codec_tpye[1], MPEG4_1_PORTNUM);
                    fSetStreamConfig(1, 720, 480, codec_tpye[2], MJPEG_PORTNUM);
                }
                else
                {
                    if (pConfig->nVideocodecres != 0)
                    {
                        pConfig->nVideocodecres = 0;
                        fSetVideoCodecRes(0);
                        __E("\nCODEC Resolution Error mode4\n");
                    }
                    sprintf(cmd, "./av_server.out %s 720P_VGA_30 MPEG4 %d %s %s MJPEG %d MENUOFF &\n", cmdopt, pConfig->nBitrate1, sw_brc1, sw_mecfg1, pConfig->nJpegQuality);
                    fSetVideoSize(1, 640, 352);
                    fSetVideoSize(2, 1280, 720);
                    fSetStreamConfig(0, 1280, 720, codec_tpye[1], MPEG4_1_PORTNUM);
                    fSetStreamConfig(1, 640, 352, codec_tpye[2], MJPEG_PORTNUM);
                }
                break;
            }
        case 3:
            {
                // H.264 + JPEG
                fSetStreamActive(1, 0, 0, 0, 1, 0, 0, 0);
                if (pConfig->nVideocodecres == 2)
                {
                    sprintf(cmd, "./av_server.out %s 720P_720P_30 H264 %d %s %s MJPEG %d MENUOFF &\n", cmdopt, pConfig->nBitrate1, sw_brc1, sw_mecfg1, pConfig->nJpegQuality);
                    fSetVideoSize(1, 1280, 720);
                    fSetVideoSize(5, 1280, 720);
                    fSetStreamConfig(0, 1280, 720, codec_tpye[0], H264_1_PORTNUM);
                    fSetStreamConfig(1, 1280, 720, codec_tpye[2], MJPEG_PORTNUM);
                }
                else if (pConfig->nVideocodecres == 1)
                {
                    sprintf(cmd, "./av_server.out %s D1_D1 H264 %d %s %s MJPEG %d MENUOFF &\n", cmdopt, pConfig->nBitrate1, sw_brc1, sw_mecfg1, pConfig->nJpegQuality);
                    fSetVideoSize(1, 720, 480);
                    fSetVideoSize(5, 720, 480);
                    fSetStreamConfig(0, 720, 480, codec_tpye[0], H264_1_PORTNUM);
                    fSetStreamConfig(1, 720, 480, codec_tpye[2], MJPEG_PORTNUM);
                }
                else
                {
                    if (pConfig->nVideocodecres != 0)
                    {
                        pConfig->nVideocodecres = 0;
                        fSetVideoCodecRes(0);
                        __E("\nCODEC Resolution Error mode3\n");
                    }
                    sprintf(cmd, "./av_server.out %s 720P_VGA_30 H264 %d %s %s MJPEG %d MENUOFF &\n", cmdopt, pConfig->nBitrate1, sw_brc1, sw_mecfg1, pConfig->nJpegQuality);
                    fSetVideoSize(1, 640, 352);
                    fSetVideoSize(5, 1280, 720);
                    fSetStreamConfig(0, 1280, 720, codec_tpye[0], H264_1_PORTNUM);
                    fSetStreamConfig(1, 640, 352, codec_tpye[2], MJPEG_PORTNUM);
                }
                break;
            }
        case 2:
            {
                //MegaPixel JPEG
                fSetStreamActive(1, 0, 0, 0, 0, 0, 0, 0);
                if (pConfig->nVideocodecres == 2)
                {
                    sprintf(cmd, "./av_server.out %s 5MP MJPEG %d MENUOFF &\n", cmdopt, pConfig->nJpegQuality);
                    fSetVideoSize(1, 2592, 1920);
                    fSetStreamConfig(0, 2592, 1920, codec_tpye[2], MJPEG_PORTNUM);
                }
                else if (pConfig->nVideocodecres == 1)
                {
                    sprintf(cmd, "./av_server.out %s 3MP MJPEG %d MENUOFF &\n", cmdopt, pConfig->nJpegQuality);
                    fSetVideoSize(1, 2048, 1536);
                    fSetStreamConfig(0, 2048, 1536, codec_tpye[2], MJPEG_PORTNUM);
                }
                else
                {
                    if (pConfig->nVideocodecres != 0)
                    {
                        pConfig->nVideocodecres = 0;
                        fSetVideoCodecRes(0);
                        __E("\nCODEC Resolution Error mode2\n");
                    }
                    sprintf(cmd, "./av_server.out %s 2MP MJPEG %d MENUOFF &\n", cmdopt, pConfig->nJpegQuality);
                    fSetVideoSize(1, 1600, 1200);
                    fSetStreamConfig(0, 1600, 1200, codec_tpye[2], MJPEG_PORTNUM);
                }
                break;
            }
        case 1:
            {
                // SINGLE MPEG4
                fSetStreamActive(0, 1, 0, 0, 0, 0, 0, 0);
                if (pConfig->nVideocodecres == 3)
                {
                    sprintf(cmd, "./av_server.out %s 1080P MPEG4 %d %s %s MENUOFF &\n", cmdopt, pConfig->nBitrate1, sw_brc1, sw_mecfg1);
                    fSetVideoSize(2, 1920, 1080);
                    fSetStreamConfig(0, 1920, 1080, codec_tpye[1], MPEG4_1_PORTNUM);
                }
                else if (pConfig->nVideocodecres == 2)
                {
                    sprintf(cmd, "./av_server.out %s SXGA MPEG4 %d %s %s MENUOFF &\n", cmdopt, pConfig->nBitrate1, sw_brc1, sw_mecfg1);
                    fSetVideoSize(2, 1280, 1024);
                    fSetStreamConfig(0, 1280, 1024, codec_tpye[1], MPEG4_1_PORTNUM);
                }
                else if (pConfig->nVideocodecres == 1)
                {
                    sprintf(cmd, "./av_server.out %s D1 MPEG4 %d %s %s MENUOFF &\n", cmdopt, pConfig->nBitrate1, sw_brc1, sw_mecfg1);
                    fSetVideoSize(2, 720, 480);
                    fSetStreamConfig(0, 720, 480, codec_tpye[1], MPEG4_1_PORTNUM);
                }
                else
                {
                    if (pConfig->nVideocodecres != 0)
                    {
                        pConfig->nVideocodecres = 0;
                        fSetVideoCodecRes(0);
                        __E("\nCODEC Resolution Error mode1\n");
                    }
                    sprintf(cmd, "./av_server.out %s 720P MPEG4 %d %s %s MENUOFF &\n", cmdopt, pConfig->nBitrate1, sw_brc1, sw_mecfg1);
                    fSetVideoSize(2, 1280, 720);
                    fSetStreamConfig(0, 1280, 720, codec_tpye[1], MPEG4_1_PORTNUM);
                }
                break;
            }
        case 0:
        default:
            {
                // SINGLE H.264
                if (pConfig->nVideocodecmode != 0)
                {
                    __E("\nCODEC Mode Error\n");
                    pConfig->nVideocodecmode = 0;
                    fSetVideoCodecMode(0);
                }
                fSetStreamActive(0, 0, 0, 0, 1, 0, 0, 0);
                if (pConfig->nVideocodecres == 3)
                {
                    sprintf(cmd, "./av_server.out %s 1080P H264 %d %s %s MENUOFF &\n", cmdopt, pConfig->nBitrate1, sw_brc1, sw_mecfg1);
                    fSetVideoSize(5, 1920, 1080);
                    fSetStreamConfig(0, 1920, 1080, codec_tpye[0], H264_1_PORTNUM);
                }
                else if (pConfig->nVideocodecres == 2)
                {
                    sprintf(cmd, "./av_server.out %s SXGA H264 %d %s %s MENUOFF &\n", cmdopt, pConfig->nBitrate1, sw_brc1, sw_mecfg1);
                    fSetVideoSize(5, 1280, 1024);
                    fSetStreamConfig(0, 1280, 1024, codec_tpye[0], H264_1_PORTNUM);
                }
                else if (pConfig->nVideocodecres == 1)
                {
                    sprintf(cmd, "./av_server.out %s D1 H264 %d %s %s MENUOFF &\n", cmdopt, pConfig->nBitrate1, sw_brc1, sw_mecfg1);
                    fSetVideoSize(5, 720, 480);
                    fSetStreamConfig(0, 720, 480, codec_tpye[0], H264_1_PORTNUM);
                }
                else
                {
                    if (pConfig->nVideocodecres != 0)
                    {
                        pConfig->nVideocodecres = 0;
                        fSetVideoCodecRes(0);
                        __E("\nCODEC Resolution Error mode0\n");
                    }
                    sprintf(cmd, "./av_server.out %s 720P H264 %d %s %s MENUOFF &\n", cmdopt, pConfig->nBitrate1, sw_brc1, sw_mecfg1);
                    fSetVideoSize(5, 1280, 720);
                    fSetStreamConfig(0, 1280, 720, codec_tpye[0], H264_1_PORTNUM);
                }
                break;
            }
    }

    // 20090611: Remove PTZ Support
    fSetPtzSupport(0);

    InitCamPtz(); //add by sxh
    printf(cmd);
    system(cmd);

    return 0;
}

/**
 * @brief Wait AV server ready
 *
 * @param stream_flag Stream flag to tell which stream is available.
 * @retval 0 Success
 * @retval -1 Fail.
 */
int WaitStreamReady(__u8 stream_flag)
{
    #define BOOT_PROC_TIMEOUT_CNT    (300)
    AV_DATA vol_data, vol_data1, vol_data2, vol_data3;
    int count = 0, ret, ret1, ret2, ret3;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    vol_data.serial =  - 1;

    if (stream_flag &FLAG_STREAM1)
    {
        while (count++ < BOOT_PROC_TIMEOUT_CNT)
        {
            if (pSysInfo->lan_config.codectype1 == MJPEG_CODEC)
            {
                ret = GetAVData(AV_OP_GET_MJPEG_SERIAL,  - 1, &vol_data);
            }
            else
            {
                ret = GetAVData(AV_OP_GET_MPEG4_SERIAL,  - 1, &vol_data);
                ret1 = GetAVData(AV_OP_GET_MPEG4_1_SERIAL,  - 1, &vol_data1);
                ret2 = GetAVData(AV_OP_GET_MPEG4_2_SERIAL,  - 1, &vol_data2);
                ret3 = GetAVData(AV_OP_GET_MPEG4_3_SERIAL,  - 1, &vol_data3);
            }
            if ((ret == RET_SUCCESS && vol_data.serial > 10) || (ret1 == RET_SUCCESS && vol_data1.serial > 10) || (ret2 == RET_SUCCESS && vol_data2.serial > 10) || (ret3 == RET_SUCCESS && vol_data3.serial > 10))
            {
                return 0;
            }
            usleep(33333);
        }
    }
    if (stream_flag &FLAG_STREAM2)
    {
        while (count++ < BOOT_PROC_TIMEOUT_CNT)
        {
            if (pSysInfo->lan_config.codectype2 == MJPEG_CODEC)
            {
                ret = GetAVData(AV_OP_GET_MJPEG_SERIAL,  - 1, &vol_data);
            }

            else
            {
                ret = GetAVData(AV_OP_GET_MPEG4_CIF_SERIAL,  - 1, &vol_data);
                ret1 = GetAVData(AV_OP_GET_MPEG4_1_CIF_SERIAL,  - 1, &vol_data1);
                ret2 = GetAVData(AV_OP_GET_MPEG4_2_CIF_SERIAL,  - 1, &vol_data2);
                ret3 = GetAVData(AV_OP_GET_MPEG4_3_CIF_SERIAL,  - 1, &vol_data3);
            }
            if ((ret == RET_SUCCESS && vol_data.serial > 10) || (ret1 == RET_SUCCESS && vol_data1.serial > 10) || (ret2 == RET_SUCCESS && vol_data2.serial > 10) || (ret3 == RET_SUCCESS && vol_data3.serial > 10))
            {
                return 0;
            }
            usleep(33333);
        }
    }
    if (stream_flag &FLAG_STREAM3)
    {
        while (count++ < BOOT_PROC_TIMEOUT_CNT)
        {

            ret = GetAVData(AV_OP_GET_MPEG4_CIF_SERIAL,  - 1, &vol_data);
            ret1 = GetAVData(AV_OP_GET_MPEG4_1_CIF_SERIAL,  - 1, &vol_data1);
            ret2 = GetAVData(AV_OP_GET_MPEG4_2_CIF_SERIAL,  - 1, &vol_data2);
            ret3 = GetAVData(AV_OP_GET_MPEG4_3_CIF_SERIAL,  - 1, &vol_data3);

            if ((ret == RET_SUCCESS && vol_data.serial > 10) || (ret1 == RET_SUCCESS && vol_data1.serial > 10) || (ret2 == RET_SUCCESS && vol_data2.serial > 10) || (ret3 == RET_SUCCESS && vol_data3.serial > 10))
            {
                return 0;
            }
            usleep(33333);
        }
    }
    return  - 1;
}

/**
 * @brief Initial AV server
 *
 * @param *pConfig AV server comunicate setting
 * @retval 0 Success
 * @retval -1 Fail.
 */
int InitAV_Server(StreamEnv_t *pConfig)
{
    int i;

    printf("InitAV_Server Begin\n");
    __u8 stream_flag = 0, nROICfg = ROI_NONE;
    if (ApproDrvInit(SYS_MSG_TYPE))
    {
        __E("ApproDrvInit fail\n");
        return  - 1;
    }
    if (func_get_mem(NULL))
    {
        ApproDrvExit();
        __E("CMEM map fail\n");
        return  - 1;
    }
    //printf("InitAV_Server: SetVideoCodecType\n");
    SetVideoCodecType(pConfig->nVideocodecmode);

    printf("InitAV_Server: nVideocodecmode=%d\n", pConfig->nVideocodecmode);

    if (pConfig->nVideocodecmode > 7)
    {
        stream_flag = (FLAG_STREAM1 | FLAG_STREAM2 | FLAG_STREAM3);
    }
    else if (pConfig->nVideocodecmode > 2)
    {
        stream_flag = (FLAG_STREAM1 | FLAG_STREAM2);
    }
    else
    {
        stream_flag = FLAG_STREAM1;
    }

    printf("InitAV_Server: WaitStreamReady begin!!!\n");

    if (WaitStreamReady(stream_flag) != 0)
    {
        ApproDrvExit();
        __E("WaitStreamReady Fail.\n");
        return  - 1;
    }
    printf("InitAV_Server: WaitStreamReady Done!!! pConfig->nDemoCfg=%d\n", pConfig->nDemoCfg);
    switch (pConfig->nDemoCfg)
    {
        case ROI_FD_DEMO:
            nROICfg = ROI_FD_CFG;
            break;

        case ROI_CENTER_DEMO:
            nROICfg = ROI_CENTER_CFG;
            break;
        default:
            nROICfg = ROI_NONE;
            break;
    }
    printf("SetTvSystem:pConfig -> imagesource=%d!!!\n", pConfig->imagesource);
    //SetTvSystem(pConfig -> imagesource);
    printf("SetImage2AType:pConfig -> nAEWtype=%d!!!\n", pConfig->nAEWtype);
    //SetImage2AType(pConfig -> nAEWtype);
    printf("Set2APriority:pConfig -> nAEWtype=%d!!!\n", pConfig->nAEWtype);
    //Set2APriority(pConfig -> expPriority);
    printf("SetCamDayNight:pConfig -> nDayNight=%d!!!\n", pConfig->nDayNight);
    SetCamDayNight(pConfig->nDayNight);
    //SetWhiteBalance(pConfig -> nWhiteBalance);
    printf("SetBacklight:pConfig -> nBackLight=%d!!!\n", pConfig->nBackLight);
    SetBacklight(pConfig->nBackLight);
    printf("SetBrightness:pConfig -> nBrightness=%d!!!\n", pConfig->nBrightness);
    SetBrightness(pConfig->nBrightness);
    SetContrast(pConfig->nContrast);
    SetSaturation(pConfig->nSaturation);
    SetSharpness(pConfig->nSharpness);
    SetHistEnable(pConfig->histogram);
    printf("EnableGBCE:pConfig -> gbce=%d!!!\n", pConfig->gbce);
    //EnableGBCE(pConfig -> gbce);
    //if(no_binskip(pConfig->nVideocodecmode, pConfig->nVideocodecres) == 0)
    //    SetBinningSkip(pConfig -> nBinning);

    //SetROIConfig(nROICfg);
    //SetDisplayValue(pConfig -> nDisplay);
    //SetFaceDetectPrm();
    /* Set Motion Parameters */
    //SetMotionDetectParam();
    SetTimeDateDetail();
    SetOSDEnable(stream_flag); //add by sxh osd
    SetMCVIP_OSDEnable(stream_flag);

    printf("stream_flag=%d!!!\n", stream_flag);
    if ((stream_flag &FLAG_STREAM1) > 0)
    {
        printf("pConfig -> nFrameRate1=%d!!!\n", pConfig->nFrameRate1);
        SetFramerate1(pConfig->nFrameRate1);
        for (i = 0; i < 8; i++)
        {
            SetOSDDetail(i); //add by sxh osd
        }
        SetCodecAdvParam(0);
        //SetCodecROIParam(0);
    }
    if ((stream_flag &FLAG_STREAM2) > 0)
    {
        printf("pConfig -> nFrameRate1=%d!!!\n", pConfig->nFrameRate1);
        SetFramerate2(pConfig->nFrameRate2);
        SetOSDDetail(1);
        SetCodecAdvParam(1);
        SetCodecROIParam(1);
    }
    if ((stream_flag &FLAG_STREAM3) > 0)
    {
        printf("pConfig -> nFrameRate1=%d!!!\n", pConfig->nFrameRate1);
        SetFramerate3(pConfig->nFrameRate3);
        SetOSDDetail(2);
        SetCodecAdvParam(2);
        SetCodecROIParam(2);
    }

    //SetAudioInVolume(pConfig->audioinvolume);
    //SetAudioOutVolume(pConfig->audiooutvolume);
    //SetAudioParams();

    printf("InitAV_Server: Done\n");

    return 0;
}

/**
 * @brief Set Binning
 *
 * @param value 0:binning ; 1:skipping
 * @return function to set Binning
 */
int SetBinning(unsigned char value) // 0:binning / 1:skipping
{
    int ret = 0;
    unsigned char prev;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (pSysInfo->lan_config.nBinning != value)
    {
        prev = pSysInfo->lan_config.nBinning;
        if ((ret = fSetBinning(value)) == 0)
        {
            if (prev == 2 || value == 2)
            {
                SetRestart();
                return ret;
            }
            else
            {
                if (no_binskip(pSysInfo->lan_config.nVideocodecmode, pSysInfo->lan_config.nVideocodecres) == 0)
                {
                    SetBinningSkip(value);
                }
            }
        }
    }

    return ret;
}

/**
 * @brief Set day or night
 *
 * @param value value of day or night
 * @return function to set day or night
 */
int SetCamDayNight(unsigned char value)
{
    int i, j;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (pSysInfo->lan_config.nDayNight == value)
    {
        return 0;
    }

    if (fSetCamDayNight(value) == 0)
    {
        SetDayNight(value);
        if (value == 0)
        {
            i = pSysInfo->lan_config.nVideocodecmode;
            j = pSysInfo->lan_config.nVideocodecres;
            fSetFramerate1(0);
            SetStreamFramerate(0, pSysInfo->lan_config.chipConfig ? enc_framerate_night_1_dm368[i][j][0]: enc_framerate_night_1_dm365[i][j][0]);
            fSetFramerate2(0);
            SetStreamFramerate(1, pSysInfo->lan_config.chipConfig ? enc_framerate_night_2_dm368[i][j][0]: enc_framerate_night_1_dm365[i][j][0]);
            fSetFramerate3(0);
            SetStreamFramerate(2, pSysInfo->lan_config.chipConfig ? enc_framerate_night_3_dm368[i][j][0]: enc_framerate_night_1_dm365[i][j][0]);
        }
    }
    else
    {
        return  - 1;
    }

    return 0;
}

/**
 * @brief Set MPEG4-1 frame rate
 *
 * @param "unsigned char value": selected frame rate
 * @return function to set MPEG4-1 frame rate
 * @retval 0: success
 * @retval -1: failed to set MPEG4-1 frame rate
 */
int SetFramerate1(unsigned char value)
{
    int i = 0, j = 0, ret = 0;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }
    printf("SetFramerate1: pSysInfo!=NULL\n");
    i = pSysInfo->lan_config.nVideocodecmode;
    j = pSysInfo->lan_config.nVideocodecres;

    ret = fSetFramerate1(value);
    printf("fSetFramerate1: Done\n");
    /*
    if (pSysInfo->lan_config.nDayNight == 0)
    SetStreamFramerate(0, pSysInfo->lan_config.chipConfig ? enc_framerate_night_1_dm368[i][j][value]:enc_framerate_night_1_dm365[i][j][value]);
    else
    SetStreamFramerate(0, pSysInfo->lan_config.chipConfig ? enc_framerate_day_1_dm368[i][j][value]:enc_framerate_day_1_dm365[i][j][value]);
     */
    printf("SetFramerate1: Done\n");
    return ret;
}

int SetFramerate2(unsigned char value)
{
    int i = 0, j = 0, ret = 0;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    i = pSysInfo->lan_config.nVideocodecmode;
    j = pSysInfo->lan_config.nVideocodecres;

    ret = fSetFramerate2(value);

    if (pSysInfo->lan_config.nDayNight == 0)
    {
        SetStreamFramerate(1, pSysInfo->lan_config.chipConfig ? enc_framerate_night_2_dm368[i][j][value]: enc_framerate_night_2_dm365[i][j][value]);
    }
    else
    {
        SetStreamFramerate(1, pSysInfo->lan_config.chipConfig ? enc_framerate_day_2_dm368[i][j][value]: enc_framerate_day_2_dm365[i][j][value]);
    }

    return ret;
}

int SetFramerate3(unsigned char value)
{
    int i = 0, j = 0, ret = 0;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    i = pSysInfo->lan_config.nVideocodecmode;
    j = pSysInfo->lan_config.nVideocodecres;

    ret = fSetFramerate3(value);

    if (pSysInfo->lan_config.nDayNight == 0)
    {
        SetStreamFramerate(2, pSysInfo->lan_config.chipConfig ? enc_framerate_night_3_dm368[i][j][value]: enc_framerate_night_3_dm365[i][j][value]);
    }
    else
    {
        SetStreamFramerate(2, pSysInfo->lan_config.chipConfig ? enc_framerate_day_3_dm368[i][j][value]: enc_framerate_day_3_dm365[i][j][value]);
    }

    return ret;
}

/**
 * @brief Set white balance
 *
 * @param value value of white balance
 * @return function to set white balance
 */
int SetCamWhiteBalance(unsigned char value)
{
    SetWhiteBalance(value);
    return fSetCamWhiteBalance(value);
}

/**
 * @brief Set backlight value
 *
 * @param value backlight value
 * @return function to set backlight value
 */
int SetCamBacklight(unsigned char value)
{
    SetBacklight(value);
    return fSetCamBacklight(value);
}

/**
 * @brief Set brightness
 *
 * @param value brightness value
 * @return function to set brightness value
 */
int SetCamBrightness(unsigned char value)
{
    SetBrightness(value);
    return fSetCamBrightness(value);
}

/**
 * @brief Set contrast
 *
 * @param value contrast value
 * @return function to set contrast value
 */
int SetCamContrast(unsigned char value)
{
    SetContrast(value);
    return fSetCamContrast(value);
}

/**
 * @brief Set saturation
 *
 * @param value saturation value
 * @return function to set saturation value
 */
int SetCamSaturation(unsigned char value)
{
    SetSaturation(value);
    return fSetCamSaturation(value);
}

//add by sxh
int SetCamPtzWithCh(char *cmd, int channel)
{
    //add by sxh
    #if DEBUG==1025
    printf("@@@@@@@@@@@@@@@@@@SetCamPtzWithCh Begin!!!\n");
    #endif

    SysInfo *pSysInfo = GetSysInfo();

    if (pSysInfo == NULL)
    {
        return ;
    }

    char *username;
    char *passwd;
    int i, authority = 2;

    for (i = 0; i < ACOUNT_NUM; i++)
    {
        if (pSysInfo->acounts[i].authority == authority)
        {
            username = pSysInfo->acounts[i].user;
            passwd = pSysInfo->acounts[i].password;
            break;
        }
    }

    #if DEBUG==1025
    printf("@@@@@@@@@@@@@@@@@@SetCamPtzWithCh username=%s,passwd=%s!!!\n", username, passwd);
    #endif

    struct sockaddr_in their_addr; //
    int port;

    /*
    switch(channel){
    case 0:
    port=18884;
    break;
    case 1:
    port=18885;
    break;
    case 2:
    port=18886;
    break;
    case 3:
    port=18887;
    break;
    } */

    port = 18884;
    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(port);
    their_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    bzero(&(their_addr.sin_zero), 8);

    char buf[100];
    bzero(buf, 100);

    sprintf(buf, "%s|%s|%d|%s\0", username, passwd, channel, cmd);

    #if DEBUG==1025
    printf("@@@@@@@@@@@@@@@@@@SetCamPtzWithCh sendto_buf=%s!!!\n", buf);
    #endif

    sendto(ptzSockfd, buf, sizeof(buf) - 1, 0, (struct sockaddr*) &their_addr, sizeof(struct sockaddr));

    return 0;
}

int InitCamPtz()
{
    /*
    char* cmd="./ptz0Server&";
    #ifdef VANLINK_DVR_PTZ_DEBUG
    printf("InitCamPtz:cmd=%s\n",cmd);
    #endif
    system(cmd);
    cmd="./ptz1Server&";
    system(cmd);
    cmd="./ptz2Server&";
    system(cmd);
    cmd="./ptz3Server&";
    system(cmd);
     */
    if ((ptzSockfd = socket(AF_INET, SOCK_DGRAM, 0)) ==  - 1)
    {
        perror("socket");
        return  - 1;
    }
    #ifdef VANLINK_DVR_PTZ_DEBUG
    printf("InitCamPtz:Done\n");
    #endif
    return 0;
}

//#endif

/**
 * @brief Set ptz function
 *
 * @param value means ptz function; ex:zoomin, zoomout and so on
 * @return function to set ptz value
 */
int SetCamPtz(int value)
{
    SetPtz(value);
    return 0;
}



/**
 * @brief Set sharpness
 *
 * @param value sharpness value
 * @return function to set sharpness value
 */
int SetCamSharpness(unsigned char value)
{
    SetSharpness(value);
    return fSetCamSharpness(value);
}

int SetMaxExposureValue(unsigned char value)
{
    //SetMaxExposure(value);
    return fSetMaxExposureValue(value);
}

int SetMaxGainValue(unsigned char value)
{
    //SetMaxGain(value);
    return fSetMaxGainValue(value);
}

/**
 * @brief Set audio status
 *
 * @param value audio status.
 *
 * @return function to set audio status
 */

int audiocount = 0;
#define AUDIO_MAX_VARIABLE (2)

void SetAudioParams(void)
{
    int audioMode;
    int IsProcExist = 0;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return ;
    }

    audiocount = 0;

    audioMode = pSysInfo->audio_config.audiomode;

    if (pSysInfo->audio_config.audioON)
    {
        if (audioMode == 1)
        {
            IsProcExist = system("killall -2 wis-streamer\n");
            if (IsProcExist == 0 /*0 is exist*/)
            {
                sleep(2);
                if (pSysInfo->lan_config.net.multicast_enable == 0)
                {
                    system("./wis-streamer -s&\n");
                }
                else
                {
                    system("./wis-streamer -m -s&\n");
                }
            }
        }
        else
        {
            system("killall -2 wis-streamer\n");
            sleep(2);
            if (pSysInfo->lan_config.net.multicast_enable == 0)
            {
                system("./wis-streamer &\n");
            }
            else
            {
                system("./wis-streamer -m&\n");
            }

            fSetAudioEnable(1);
        }
        if (audioMode == 0)
        {
            fSetAlarmAudioPlay(0);
        }
    }
    else
    {
        IsProcExist = system("killall -2 wis-streamer\n");
        if (IsProcExist == 0 /*0 is exist*/)
        {
            sleep(2);
            if (pSysInfo->lan_config.net.multicast_enable == 0)
            {
                system("./wis-streamer -s&\n");
            }
            else
            {
                system("./wis-streamer -m -s&\n");
            }
        }
        fSetAudioEnable(0);
        fSetAlarmAudioPlay(0);
    }

    return ;
}

int SetAudioON(unsigned char value)
{
    int ret = 0;

    audiocount++;
    ret = fSetAudioON(value);

    if (audiocount == AUDIO_MAX_VARIABLE)
    {
        SetAudioParams();
    }

    return ret;
}

int SetAudioMode(unsigned char value)
{
    int ret = 0;

    audiocount++;
    ret = fSetAudioMode(value);

    if (audiocount == AUDIO_MAX_VARIABLE)
    {
        SetAudioParams();
    }

    return ret;
}

int SetAudioInVolume(unsigned char value)
{

    if (value < 10)
    {
        system("amixer set 'PGA' 0");
    }
    else if (value < 40)
    {
        system("amixer set 'PGA' 1");
    }
    else if (value < 80)
    {
        system("amixer set 'PGA' 2");
    }
    else
    {
        system("amixer set 'PGA' 3");
    }

    return fSetAudioInVolume(value);
}

int SetAudioEncode(unsigned char value)
{
    return fSetAudioEncode(value);
}

int SetAudioSampleRate(unsigned char value)
{
    int ret = 0;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (value != pSysInfo->audio_config.samplerate)
    {
        ret = fSetAudioSampleRate(value);
        if (ret == 0)
        {
            SetRestart();
        }
    }

    return ret;
}

int SetAudioBitrate(unsigned char value)
{
    return fSetAudioBitrate(value);
}

int SetAudioAlarmLevel(unsigned char value)
{
    return fSetAudioAlarmLevel(value);
}

int SetAudioOutVolume(unsigned char value)
{
    char cmd[64];
    int setval = (value *63) / 100;

    sprintf(cmd, "amixer set 'Mono DAC' %d", setval);
    system(cmd);

    return fSetAudioOutVolume(value);
}

int fdetectcount = 0;
#define FD_MAX_VARIABLE (12)

void SetFaceDetectPrm(void)
{
    FaceDetectParam faceParam;
    SysInfo *pSysInfo = GetSysInfo();

    fdetectcount = 0;

    faceParam.fdetect = pSysInfo->face_config.fdetect;
    faceParam.startX = pSysInfo->face_config.startX;
    faceParam.startY = pSysInfo->face_config.startY;
    faceParam.width = pSysInfo->face_config.width;
    faceParam.height = pSysInfo->face_config.height;
    faceParam.fdconflevel = pSysInfo->face_config.fdconflevel;
    faceParam.fddirection = pSysInfo->face_config.fddirection;
    faceParam.frecog = pSysInfo->face_config.frecog;
    faceParam.frconflevel = pSysInfo->face_config.frconflevel;
    faceParam.frdatabase = pSysInfo->face_config.frdatabase;
    faceParam.pmask = pSysInfo->face_config.pmask;
    faceParam.maskoption = pSysInfo->face_config.maskoption;

    //printf("FD DEBUG VALUE: %d %d %d %d %d\n" , faceParam.fdetect,
    //    faceParam.startX, faceParam.startY, faceParam.width, faceParam.height);

    SendFaceDetectMsg(&faceParam);
}

int SetFaceDetect(unsigned char value)
{
    int ret = 0;
    fdetectcount++;
    ret = fSetFaceDetect(value);
    if (fdetectcount == FD_MAX_VARIABLE)
    {
        SetFaceDetectPrm();
    }

    return ret;
}

int SetFDX(unsigned int value)
{
    int ret = 0;
    fdetectcount++;
    ret = fSetFDX(value);
    if (fdetectcount == FD_MAX_VARIABLE)
    {
        SetFaceDetectPrm();
    }
    return ret;

}

int SetFDY(unsigned int value)
{
    int ret = 0;
    fdetectcount++;
    ret = fSetFDY(value);
    if (fdetectcount == FD_MAX_VARIABLE)
    {
        SetFaceDetectPrm();
    }
    return ret;
}

int SetFDW(unsigned int value)
{
    int ret = 0;
    fdetectcount++;
    ret = fSetFDW(value);
    if (fdetectcount == FD_MAX_VARIABLE)
    {
        SetFaceDetectPrm();
    }
    return ret;
}

int SetFDH(unsigned int value)
{
    int ret = 0;
    fdetectcount++;
    ret = fSetFDH(value);
    if (fdetectcount == FD_MAX_VARIABLE)
    {
        SetFaceDetectPrm();
    }
    return ret;
}

int SetFDConfLevel(unsigned char value)
{
    int ret = 0;
    fdetectcount++;
    ret = fSetFDConfLevel(value);
    if (fdetectcount == FD_MAX_VARIABLE)
    {
        SetFaceDetectPrm();
    }
    return ret;
}

int SetFDDirection(unsigned char value)
{
    int ret = 0;
    fdetectcount++;
    ret = fSetFDDirection(value);
    if (fdetectcount == FD_MAX_VARIABLE)
    {
        SetFaceDetectPrm();
    }
    return ret;

}

int SetFRecognition(unsigned char value)
{
    int ret = 0;
    fdetectcount++;
    ret = fSetFRecognition(value);
    if (fdetectcount == FD_MAX_VARIABLE)
    {
        SetFaceDetectPrm();
    }
    return ret;
}

int SetFRConfLevel(unsigned char value)
{
    int ret = 0;
    fdetectcount++;
    ret = fSetFRConfLevel(value);
    if (fdetectcount == FD_MAX_VARIABLE)
    {
        SetFaceDetectPrm();
    }
    return ret;
}

int SetFRDatabase(unsigned char value)
{
    int ret = 0;
    fdetectcount++;
    ret = fSetFRDatabase(value);
    if (fdetectcount == FD_MAX_VARIABLE)
    {
        SetFaceDetectPrm();
    }
    return ret;
}

int SetPrivacyMask(unsigned char value)
{
    int ret = 0;
    fdetectcount++;
    ret = fSetPrivacyMask(value);
    if (fdetectcount == FD_MAX_VARIABLE)
    {
        SetFaceDetectPrm();
    }
    return ret;
}

int SetMaskOptions(unsigned char value)
{
    int ret = 0;
    fdetectcount++;
    ret = fSetMaskOptions(value);
    if (fdetectcount == FD_MAX_VARIABLE)
    {
        SetFaceDetectPrm();
    }
    return ret;
}

#define ADVANCED_ALL_VALUES    (4)
int advFeatCount = 0;
int advFeatValue = 0;

int SetAdvMode(unsigned char value)
{
    int ret = 0;
    int nflt_value_new, nflt_value_old, prepr_value_new, prepr_value_old;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    advFeatValue = 0;
    advFeatCount = 0;

    if (value != pSysInfo->lan_config.AdvanceMode)
    {
        nflt_value_new = value &VNF_MASK;
        nflt_value_old = pSysInfo->lan_config.AdvanceMode &VNF_MASK;
        prepr_value_new = value &PREPR_MASK;
        prepr_value_old = pSysInfo->lan_config.AdvanceMode &PREPR_MASK;
        if ((ret = fSetAdvMode(value)) == 0)
        {
            if (prepr_value_new != prepr_value_old)
            {
                SetRestart();
                return ret;
            }
            else if (pSysInfo->lan_config.democfg != VNF_DEMO)
            {
                if ((nflt_value_new == 0) || (nflt_value_old == 0))
                {
                    SetRestart();
                    return ret;
                }
            }
            SetVNFStatus(0, nflt_value_new);
            if (pSysInfo->lan_config.nVideocodecmode > 2)
            {
                SetVNFStatus(1, nflt_value_new);
            }
            if (pSysInfo->lan_config.nVideocodecmode > 7)
            {
                SetVNFStatus(2, nflt_value_new);
            }
        }
    }

    return ret;
}

int SetVstabValue(unsigned char value)
{
    int ret = 0;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    advFeatCount++;

    if (value)
    {
        advFeatValue |= FFLAG_VS;
    }

    if (advFeatCount == ADVANCED_ALL_VALUES)
    {
        SetAdvMode(advFeatValue);
    }

    return ret;
}

int SetLdcValue(unsigned char value)
{
    int ret = 0;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    advFeatCount++;

    if (value)
    {
        advFeatValue |= FFLAG_LDC;
    }

    if (advFeatCount == ADVANCED_ALL_VALUES)
    {
        SetAdvMode(advFeatValue);
    }

    return ret;
}

int SetSnfValue(unsigned char value)
{
    int ret = 0;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    advFeatCount++;

    if (value)
    {
        advFeatValue |= FFLAG_SNF;
    }

    if (advFeatCount == ADVANCED_ALL_VALUES)
    {
        SetAdvMode(advFeatValue);
    }

    return ret;
}

int SetTnfValue(unsigned char value)
{
    int ret = 0;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    advFeatCount++;

    if (value)
    {
        advFeatValue |= FFLAG_TNF;
    }

    if (advFeatCount == ADVANCED_ALL_VALUES)
    {
        SetAdvMode(advFeatValue);
    }

    return ret;
}

int SetDemoCfg(unsigned char value)
{
    int ret = 0;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (value != pSysInfo->lan_config.democfg)
    {
        if ((ret = fSetDemoCfg(value)) == 0)
        {
            if (value == DEMO_NONE)
            {
                 /* reset to default modes */
                SetDefault();
            }
            else
            {
                SetRestart();
            }
        }
    }

    return ret;
}

int SetClickSnapFilename(void *buf, int length)
{
    int ret = fSetClickSnapFilename(buf, length);

    if (ret == 0)
    {
        SetClipSnapName((char*)buf, length);
    }

    return ret;
}

int SetClickSnapStorage(unsigned char value)
{
    int ret = fSetClickSnapStorage(value);

    if (ret == 0)
    {
        SetClipSnapLocation(value);
    }

    return ret;
}

int OSDChangeVal = 0;

void SendOSDChangeMsg(void)
{
    OSDChangeVal++;
    if (OSDChangeVal == OSD_PARAMS_UPDATED)
    {
        SetOSDWindow(OSD_MSG_TRUE);
        OSDChangeVal = 0;
    }

    return ;
}

int SetOSDTextInfo(void *buf, int length)
{
    int ret;
    ret = fSetOSDText(buf, length);
    if (ret == 0)
    {
        SetAvOsdText((char*)buf, length);
    }

    return ret;
}

int SetOSDWin(unsigned char value)
{
    int ret = 0;
    int ivalue = value;
    fprintf(stderr, "\nSetOSDWin: %d\n", value);
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (value != pSysInfo->lan_config.osdwin)
    {
        ret = fSetOSDWin(value);
        SetAvOsdTextEnable(ivalue);
    }

    return 0;
}

int SetHistogram(unsigned char value)
{
    int ret = 0;

    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (value != pSysInfo->lan_config.histogram)
    {
        ret = fSetHistogram(value);
        SetHistEnable(value);
    }
    return 0;

}

int SetGBCE(unsigned char value)
{
    int ret = 0;

    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (value != pSysInfo->lan_config.gbce)
    {
        ret = fSetGBCE(value);
        EnableGBCE(value);
    }
    return 0;

}

int SetExpPriority(unsigned char value)
{
    int ret = 0;

    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (value != pSysInfo->lan_config.expPriority)
    {
        ret = fSetExpPriority(value);
        Set2APriority(value);
    }
    return 0;

}

int SetOSDWinNum(unsigned char value)
{
    #if 0
    int ret = 0;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (value != pSysInfo->lan_config.osdwinnum)
    {
        ret = fSetOSDWinNum(value);
    }

    if (ret == 0)
    {
        SendOSDChangeMsg();
    }

    return ret;
    #else
    int ret = 0;
    int ivalue = value;
    fprintf(stderr, "\nSetOSDWinNum: %d\n", value);
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (value != pSysInfo->lan_config.osdwinnum)
    {
        ret = fSetOSDWinNum(value);
        SetAvOsdLogoEnable(ivalue);
    }
    return ret;
    #endif
}

int SetOSDStream(unsigned char value)
{
    #if 0
    int ret = 0;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (value != pSysInfo->lan_config.osdstream)
    {
        ret = fSetOSDStream(value);
    }

    if (ret == 0)
    {
        SendOSDChangeMsg();
    }

    return ret;
    #else
    fprintf(stderr, "SetOSDWin: %d", value);
    return 0;
    #endif
}

/**
 * @brief Set MPEG4-1 bitrate
 *
 * @param value MPEG4-1 bitrate value
 * @return function to set MPEG4-1 bitrate
 * @retval -1 failed to set MPEG4-1 bitrate
 */

int SetMP41bitrate(unsigned int value)
{
    SysInfo *pSysInfo = GetSysInfo();
    unsigned char nVideocodecmode, nVideocodecres;
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    nVideocodecmode = pSysInfo->lan_config.nVideocodecmode;
    nVideocodecres = pSysInfo->lan_config.nVideocodecres;
    if (nVideocodecres == 2 && (nVideocodecmode == 6 || nVideocodecmode == 5))
    {
        value = CheckValueBound(value, 64000, 8000000);
    }
    else
    {
        value = CheckValueBound(value, 64000, 12000000);
    }

    if (value != pSysInfo->lan_config.nMpeg41bitrate)
    {
        SetEncBitrate(0, value);
        return fSetMP41bitrate(value);
    }
    else
    {
        return 0;
    }
}

/**
 * @brief Set MPEG4-2 bitrate
 *
 * @param value MPEG4-2 bitrate value
 * @return function to set MPEG4-2 bitrate
 * @retval -1 failed to set MPEG4-2 bitrate
 */
int SetMP42bitrate(unsigned int value)
{
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    value = CheckValueBound(value, 64000, 8000000);

    if (value != pSysInfo->lan_config.nMpeg42bitrate)
    {
        SetEncBitrate(1, value);
        return fSetMP42bitrate(value);
    }
    else
    {
        return 0;
    }
}

/**
 * @brief Set motion JPEG quality
 *
 * @param value : QP value
 * @return function to set motion JPEG quality
 */
int SetMJPEGQuality(unsigned char value)
{
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    value = (unsigned char)CheckValueBound((int)value, 5, 98);

    if (value != pSysInfo->lan_config.njpegquality)
    {
        SetJpgQuality(value);
        return fSetJpegQuality(value);
    }
    else
    {
        return 0;
    }
}

int SetMirror(unsigned char value)
{
    int ret = 0;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (value != pSysInfo->lan_config.mirror)
    {
        #if 1
        if ((ret = fSetMirror(value)) == 0)
        {
            SetRestart();
        }
        return ret;
        #else
        SetMirr(value);
        return fSetMirror(value);
        #endif

    }
    return 0;

}

/**
 * @brief Set image AEW
 *
 * @param "unsigned char value": 0: OFF, 1: APPRO
 * @return SUCCESS: 0, FAIL: -1
 */
int SetImageAEW(unsigned char value) /*img2a*/
{
    int ret = 0;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (pSysInfo->lan_config.nAEWswitch != value)
    {
        if ((ret = fSetImageAEW(value)) == 0)
        {
            SetRestart();
        }
    }

    return ret;
}

/**
 * @brief Set image AEW
 *
 * @param "unsigned char value": 0: OFF, 1: APPRO
 * @return SUCCESS: 0, FAIL: -1
 */
int SetImageAEWType(unsigned char value) /*img2a*/
{
    int ret = 0;

    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (pSysInfo->lan_config.nAEWtype != value)
    {
        if ((ret = fSetImageAEWType(value)) == 0)
        {
            SetImage2AType(value);
        }
    }

    return ret;
}

/**
 * @brief Set image source
 *
 * @param value value of image source( NTSC/PAL Select/)
 * @return function to set image source
 */
/* NTSC/PAL Select */
int SetImagesource(unsigned char value)
{
    SetTvSystem(value);
    return fSetImageSource(value);
}

/**
 * @brief Set time stamp status
 * @param value Time stamp status
 */
int SetTStampEnable(unsigned char value)
{
    if (value == 0)
    {
        SetTStamp(value);
        return fSetTStampEnable(value);
    }
    return fSetTStampEnable(value);
}

int SetRateControl1(unsigned char value)
{
    int ret = 0;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (value != pSysInfo->lan_config.nRateControl1)
    {
        if ((ret = fSetRateControl1(value)) == 0)
        {
            SetRestart();
        }
    }

    return ret;
}

int SetRateControl2(unsigned char value)
{
    int ret = 0;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (value != pSysInfo->lan_config.nRateControl2)
    {
        if ((ret = fSetRateControl2(value)) == 0)
        {
            SetRestart();
        }
    }

    return ret;
}

/**
 * @brief Set time stamp format
 * @param value format
 */
int timedatecount = 0;
#define TIMEDATE_MAX_VARIABLE (4)

int SetTimeDateDetail(void)
{
    DateTimePrm datetimeParam;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    timedatecount = 0;

    datetimeParam.dateFormat = pSysInfo->lan_config.dateformat;
    datetimeParam.datePos = pSysInfo->lan_config.dateposition;
    datetimeParam.timeFormat = pSysInfo->lan_config.tstampformat;
    datetimeParam.timePos = pSysInfo->lan_config.timeposition;

    return SetDateTimeFormat(&datetimeParam);
}

int SetRecChEnable(unsigned char value)
{

    int ret = 0;

    ret = fSetChRecEnable(value);
    SetChRecEnableMsg(value);
    return ret;
}

int Avs_GetPbRecDayData(unsigned char *value)
{
    GetPbRecDayDataMsg(value);
}

int Avs_GetPbRecHourData(unsigned char *value)
{
    GetPbRecHourDataMsg(value);
}

int Avs_StartPlaybackCh(unsigned char *value)
{
    StartPlaybackChMsg(value);
}

int Avs_StopPlaybackCh(unsigned char *value)
{
    StopPlaybackChMsg(value);
}

int Avs_PausePlaybackCh(unsigned char *value)
{
    PausePlaybackChMsg(value);
}

int Avs_ResumePlaybackCh(unsigned char *value)
{
    ResumePlaybackChMsg(value);
}



int SetDateFormat(unsigned char value)
{
    int ret = 0;
    timedatecount++;
    ret = fSetDateFormat(value);
    if (timedatecount == TIMEDATE_MAX_VARIABLE)
    {
        SetTimeDateDetail();
    }
    return ret;
}

int SetTStampFormat(unsigned char value)
{
    int ret = 0;
    timedatecount++;
    ret = fSetTStampFormat(value);
    if (timedatecount == TIMEDATE_MAX_VARIABLE)
    {
        SetTimeDateDetail();
    }
    return ret;
}

int SetDatePosition(unsigned char value)
{
    int ret = 0;
    timedatecount++;
    ret = fSetDatePosition(value);
    if (timedatecount == TIMEDATE_MAX_VARIABLE)
    {
        SetTimeDateDetail();
    }
    return ret;
}

int SetTimePosition(unsigned char value)
{
    int ret = 0;
    timedatecount++;
    ret = fSetTimePosition(value);
    if (timedatecount == TIMEDATE_MAX_VARIABLE)
    {
        SetTimeDateDetail();
    }
    return ret;
}

int osdcount[3] =
{
    0, 0, 0
};

#define OSD_MAX_VARIABLE (8)

int SetOSDDetail(int id)
{
    OSDPrm osdPrm;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    osdcount[id] = 0;

    osdPrm.dateEnable = pSysInfo->osd_config[id].dstampenable;
    osdPrm.timeEnable = pSysInfo->osd_config[id].tstampenable;
    osdPrm.logoEnable = pSysInfo->osd_config[id].nLogoEnable;
    osdPrm.logoPos = pSysInfo->osd_config[id].nLogoPosition;
    osdPrm.textEnable = pSysInfo->osd_config[id].nTextEnable;
    osdPrm.textPos = pSysInfo->osd_config[id].nTextPosition;
    strcpy(osdPrm.text, (char*)pSysInfo->osd_config[id].overlaytext);
    osdPrm.detailedInfo = pSysInfo->osd_config[id].nDetailInfo;

    return SetOSDPrmMsg(id, &osdPrm);
}

int SetDateStampEnable1(unsigned char value)
{
    int ret = 0;
    osdcount[0]++;
    ret = fSetDateStampEnable1(value);
    if (osdcount[0] == OSD_MAX_VARIABLE)
    {
        SetOSDDetail(0);
    }
    return ret;
}

int SetDateStampEnable2(unsigned char value)
{
    int ret = 0;
    osdcount[1]++;
    ret = fSetDateStampEnable2(value);
    if (osdcount[1] == OSD_MAX_VARIABLE)
    {
        SetOSDDetail(1);
    }
    return ret;
}

int SetDateStampEnable3(unsigned char value)
{
    int ret = 0;
    osdcount[2]++;
    ret = fSetDateStampEnable3(value);
    if (osdcount[2] == OSD_MAX_VARIABLE)
    {
        SetOSDDetail(2);
    }
    return ret;
}

int SetTimeStampEnable1(unsigned char value)
{
    int ret = 0;
    osdcount[0]++;
    ret = fSetTimeStampEnable1(value);
    if (osdcount[0] == OSD_MAX_VARIABLE)
    {
        SetOSDDetail(0);
    }
    return ret;
}

int SetTimeStampEnable2(unsigned char value)
{
    int ret = 0;
    osdcount[1]++;
    ret = fSetTimeStampEnable2(value);
    if (osdcount[1] == OSD_MAX_VARIABLE)
    {
        SetOSDDetail(1);
    }
    return ret;
}

int SetTimeStampEnable3(unsigned char value)
{
    int ret = 0;
    osdcount[2]++;
    ret = fSetTimeStampEnable3(value);
    if (osdcount[2] == OSD_MAX_VARIABLE)
    {
        SetOSDDetail(2);
    }
    return ret;
}

int SetLogoEnable1(unsigned char value)
{
    int ret = 0;
    osdcount[0]++;
    ret = fSetLogoEnable1(value);
    if (osdcount[0] == OSD_MAX_VARIABLE)
    {
        SetOSDDetail(0);
    }
    return ret;
}

int SetLogoEnable2(unsigned char value)
{
    int ret = 0;
    osdcount[1]++;
    ret = fSetLogoEnable2(value);
    if (osdcount[1] == OSD_MAX_VARIABLE)
    {
        SetOSDDetail(1);
    }
    return ret;
}

int SetLogoEnable3(unsigned char value)
{
    int ret = 0;
    osdcount[2]++;
    ret = fSetLogoEnable3(value);
    if (osdcount[2] == OSD_MAX_VARIABLE)
    {
        SetOSDDetail(2);
    }
    return ret;
}

int SetLogoPosition1(unsigned char value)
{
    int ret = 0;
    osdcount[0]++;
    ret = fSetLogoPosition1(value);
    if (osdcount[0] == OSD_MAX_VARIABLE)
    {
        SetOSDDetail(0);
    }
    return ret;
}

int SetLogoPosition2(unsigned char value)
{
    int ret = 0;
    osdcount[1]++;
    ret = fSetLogoPosition2(value);
    if (osdcount[1] == OSD_MAX_VARIABLE)
    {
        SetOSDDetail(1);
    }
    return ret;
}

int SetLogoPosition3(unsigned char value)
{
    int ret = 0;
    osdcount[2]++;
    ret = fSetLogoPosition3(value);
    if (osdcount[2] == OSD_MAX_VARIABLE)
    {
        SetOSDDetail(2);
    }
    return ret;
}

int SetTextPosition1(unsigned char value)
{
    int ret = 0;
    osdcount[0]++;
    ret = fSetTextPosition1(value);
    if (osdcount[0] == OSD_MAX_VARIABLE)
    {
        SetOSDDetail(0);
    }
    return ret;
}

int SetTextPosition2(unsigned char value)
{
    int ret = 0;
    osdcount[1]++;
    ret = fSetTextPosition2(value);
    if (osdcount[1] == OSD_MAX_VARIABLE)
    {
        SetOSDDetail(1);
    }
    return ret;
}

int SetTextPosition3(unsigned char value)
{
    int ret = 0;
    osdcount[2]++;
    ret = fSetTextPosition3(value);
    if (osdcount[2] == OSD_MAX_VARIABLE)
    {
        SetOSDDetail(2);
    }
    return ret;
}

int SetTextEnable1(unsigned char value)
{
    int ret = 0;
    osdcount[0]++;
    ret = fSetTextEnable1(value);
    if (osdcount[0] == OSD_MAX_VARIABLE)
    {
        SetOSDDetail(0);
    }
    return ret;
}

int SetTextEnable2(unsigned char value)
{
    int ret = 0;
    osdcount[1]++;
    ret = fSetTextEnable2(value);
    if (osdcount[1] == OSD_MAX_VARIABLE)
    {
        SetOSDDetail(1);
    }
    return ret;
}

int SetTextEnable3(unsigned char value)
{
    int ret = 0;
    osdcount[2]++;
    ret = fSetTextEnable3(value);
    if (osdcount[2] == OSD_MAX_VARIABLE)
    {
        SetOSDDetail(2);
    }
    return ret;
}

int SetOverlayText1(void *buf, int length)
{
    int ret = 0;
    osdcount[0]++;
    ret = fSetOverlayText1(buf, length);
    if (osdcount[0] == OSD_MAX_VARIABLE)
    {
        SetOSDDetail(0);
    }
    return ret;
}

int SetOverlayText2(void *buf, int length)
{
    int ret = 0;
    osdcount[1]++;
    ret = fSetOverlayText2(buf, length);
    if (osdcount[1] == OSD_MAX_VARIABLE)
    {
        SetOSDDetail(1);
    }
    return ret;
}

int SetOverlayText3(void *buf, int length)
{
    int ret = 0;
    osdcount[2]++;
    ret = fSetOverlayText3(buf, length);
    if (osdcount[2] == OSD_MAX_VARIABLE)
    {
        SetOSDDetail(2);
    }
    return ret;
}

int SetDetailInfo1(unsigned char value)
{
    int ret = 0;
    osdcount[0]++;
    ret = fSetDetailInfo1(value);
    if (osdcount[0] == OSD_MAX_VARIABLE)
    {
        SetOSDDetail(0);
    }
    return ret;
}

int SetDetailInfo2(unsigned char value)
{
    int ret = 0;
    osdcount[1]++;
    ret = fSetDetailInfo2(value);
    if (osdcount[1] == OSD_MAX_VARIABLE)
    {
        SetOSDDetail(1);
    }
    return ret;
}

int SetDetailInfo3(unsigned char value)
{
    int ret = 0;
    osdcount[2]++;
    ret = fSetDetailInfo3(value);
    if (osdcount[2] == OSD_MAX_VARIABLE)
    {
        SetOSDDetail(2);
    }
    return ret;
}

int SetEncryptVideo(unsigned char value)
{
    int ret = 0;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (value != pSysInfo->lan_config.nEncryptVideo)
    {
        ret = fSetEncryptVideo(value);
    }

    return ret;
}

int SetLocalDisplay(unsigned char value)
{
    int ret = 0;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    SetDisplayValue(value);
    ret = fSetLocalDisplay(value);

    return ret;
}

int codeccount[3] =
{
    0, 0, 0
};

#define CODEC_MAX_VARIABLE (7)

void SetCodecAdvParam(int id)
{
    CodecAdvPrm codecPrm;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return ;
    }

    codeccount[id] = 0;

    codecPrm.ipRatio = pSysInfo->codec_advconfig[id].ipRatio;
    codecPrm.fIframe = pSysInfo->codec_advconfig[id].fIframe;
    codecPrm.qpInit = pSysInfo->codec_advconfig[id].qpInit;
    codecPrm.qpMin = pSysInfo->codec_advconfig[id].qpMin;
    codecPrm.qpMax = pSysInfo->codec_advconfig[id].qpMax;
    codecPrm.meConfig = pSysInfo->codec_advconfig[id].meConfig;
    codecPrm.packetSize = pSysInfo->codec_advconfig[id].packetSize;

    SetCodecAdvPrmMsg(id, &codecPrm);
}

int SetIpratio1(unsigned int value)
{
    int ret = 0, i = 0;

    value = CheckValueBound(value, 1, 30);

    codeccount[i]++;
    ret = fSetIpratio1(value);

    if (codeccount[i] == CODEC_MAX_VARIABLE)
    {
        SetCodecAdvParam(i);
    }

    return ret;
}

int SetIpratio2(unsigned int value)
{
    int ret = 0, i = 1;

    value = CheckValueBound(value, 1, 30);

    codeccount[i]++;
    ret = fSetIpratio2(value);

    if (codeccount[i] == CODEC_MAX_VARIABLE)
    {
        SetCodecAdvParam(i);
    }

    return ret;
}

int SetIpratio3(unsigned int value)
{
    int ret = 0, i = 2;

    value = CheckValueBound(value, 1, 30);

    codeccount[i]++;
    ret = fSetIpratio3(value);

    if (codeccount[i] == CODEC_MAX_VARIABLE)
    {
        SetCodecAdvParam(i);
    }

    return ret;
}

int SetForceIframe1(unsigned char value)
{
    int ret = 0, i = 0;

    codeccount[i]++;
    ret = fSetForceIframe1(value);

    if (codeccount[i] == CODEC_MAX_VARIABLE)
    {
        SetCodecAdvParam(i);
    }

    return ret;
}

int SetForceIframe2(unsigned char value)
{
    int ret = 0, i = 1;

    codeccount[i]++;
    ret = fSetForceIframe2(value);

    if (codeccount[i] == CODEC_MAX_VARIABLE)
    {
        SetCodecAdvParam(i);
    }

    return ret;
}

int SetForceIframe3(unsigned char value)
{
    int ret = 0, i = 2;

    codeccount[i]++;
    ret = fSetForceIframe3(value);

    if (codeccount[i] == CODEC_MAX_VARIABLE)
    {
        SetCodecAdvParam(i);
    }

    return ret;
}

int SetQPInit1(unsigned char value)
{
    int ret = 0, i = 0;

    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (pSysInfo->lan_config.codectype1 == H264_CODEC)
    {
        value = (unsigned char)CheckValueBound((int)value, 1, 51);
    }
    else if (pSysInfo->lan_config.codectype1 == MPEG4_CODEC)
    {
        value = (unsigned char)CheckValueBound((int)value, 1, 31);
    }

    codeccount[i]++;
    ret = fSetQPInit1(value);

    if (codeccount[i] == CODEC_MAX_VARIABLE)
    {
        SetCodecAdvParam(i);
    }

    return ret;
}

int SetQPInit2(unsigned char value)
{
    int ret = 0, i = 0;

    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (pSysInfo->lan_config.codectype2 == H264_CODEC)
    {
        value = (unsigned char)CheckValueBound((int)value, 1, 51);
    }
    else if (pSysInfo->lan_config.codectype2 == MPEG4_CODEC)
    {
        value = (unsigned char)CheckValueBound((int)value, 1, 31);
    }

    codeccount[i]++;
    ret = fSetQPInit2(value);

    if (codeccount[i] == CODEC_MAX_VARIABLE)
    {
        SetCodecAdvParam(i);
    }

    return ret;
}

int SetQPInit3(unsigned char value)
{
    int ret = 0, i = 0;

    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (pSysInfo->lan_config.codectype3 == H264_CODEC)
    {
        value = (unsigned char)CheckValueBound((int)value, 1, 51);
    }
    else if (pSysInfo->lan_config.codectype3 == MPEG4_CODEC)
    {
        value = (unsigned char)CheckValueBound((int)value, 1, 31);
    }

    codeccount[i]++;
    ret = fSetQPInit3(value);

    if (codeccount[i] == CODEC_MAX_VARIABLE)
    {
        SetCodecAdvParam(i);
    }

    return ret;
}

int SetQPMin1(unsigned char value)
{
    int ret = 0, i = 0;

    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (pSysInfo->lan_config.codectype1 == H264_CODEC)
    {
        value = (value > 51) ? 51 : value;
    }
    else if (pSysInfo->lan_config.codectype1 == MPEG4_CODEC)
    {
        value = (value > 31) ? 31 : value;
    }

    value = (value == 0) ? 1 : value;

    codeccount[i]++;
    ret = fSetQPMin1(value);

    if (codeccount[i] == CODEC_MAX_VARIABLE)
    {
        SetCodecAdvParam(i);
    }

    return ret;
}

int SetQPMin2(unsigned char value)
{
    int ret = 0, i = 1;

    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (pSysInfo->lan_config.codectype2 == H264_CODEC)
    {
        value = (value > 51) ? 51 : value;
    }
    else if (pSysInfo->lan_config.codectype2 == MPEG4_CODEC)
    {
        value = (value > 31) ? 31 : value;
    }

    value = (value == 0) ? 1 : value;

    codeccount[i]++;
    ret = fSetQPMin2(value);

    if (codeccount[i] == CODEC_MAX_VARIABLE)
    {
        SetCodecAdvParam(i);
    }

    return ret;
}

int SetQPMin3(unsigned char value)
{
    int ret = 0, i = 2;

    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (pSysInfo->lan_config.codectype3 == H264_CODEC)
    {
        value = (value > 51) ? 51 : value;
    }
    else if (pSysInfo->lan_config.codectype3 == MPEG4_CODEC)
    {
        value = (value > 31) ? 31 : value;
    }

    value = (value == 0) ? 1 : value;

    codeccount[i]++;
    ret = fSetQPMin3(value);

    if (codeccount[i] == CODEC_MAX_VARIABLE)
    {
        SetCodecAdvParam(i);
    }

    return ret;
}

int SetQPMax1(unsigned char value)
{
    int ret = 0, i = 0;

    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (pSysInfo->lan_config.codectype1 == H264_CODEC)
    {
        value = (value > 51) ? 51 : value;
    }
    else if (pSysInfo->lan_config.codectype1 == MPEG4_CODEC)
    {
        value = (value > 31) ? 31 : value;
    }

    value = (value == 0) ? 1 : value;

    codeccount[i]++;
    ret = fSetQPMax1(value);

    if (codeccount[i] == CODEC_MAX_VARIABLE)
    {
        SetCodecAdvParam(i);
    }

    return ret;
}

int SetQPMax2(unsigned char value)
{
    int ret = 0, i = 1;

    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (pSysInfo->lan_config.codectype2 == H264_CODEC)
    {
        value = (value > 51) ? 51 : value;
    }
    else if (pSysInfo->lan_config.codectype2 == MPEG4_CODEC)
    {
        value = (value > 31) ? 31 : value;
    }

    value = (value == 0) ? 1 : value;

    codeccount[i]++;
    ret = fSetQPMax2(value);

    if (codeccount[i] == CODEC_MAX_VARIABLE)
    {
        SetCodecAdvParam(i);
    }

    return ret;
}

int SetQPMax3(unsigned char value)
{
    int ret = 0, i = 2;

    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (pSysInfo->lan_config.codectype3 == H264_CODEC)
    {
        value = (value > 51) ? 51 : value;
    }
    else if (pSysInfo->lan_config.codectype3 == MPEG4_CODEC)
    {
        value = (value > 31) ? 31 : value;
    }

    value = (value == 0) ? 1 : value;

    codeccount[i]++;
    ret = fSetQPMax3(value);

    if (codeccount[i] == CODEC_MAX_VARIABLE)
    {
        SetCodecAdvParam(i);
    }

    return ret;
}

int SetMEConfig1(unsigned char value)
{
    int ret = 0, i = 0;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    codeccount[i]++;

    if (value != pSysInfo->codec_advconfig[i].meConfig)
    {
        if ((ret = fSetMEConfig1(value)) == 0)
        {
            SetRestart();
        }
    }

    if (codeccount[i] == CODEC_MAX_VARIABLE)
    {
        SetCodecAdvParam(i);
    }

    return ret;
}

int SetMEConfig2(unsigned char value)
{
    int ret = 0, i = 1;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    codeccount[i]++;

    if (value != pSysInfo->codec_advconfig[i].meConfig)
    {
        if ((ret = fSetMEConfig2(value)) == 0)
        {
            SetRestart();
        }
    }

    if (codeccount[i] == CODEC_MAX_VARIABLE)
    {
        SetCodecAdvParam(i);
    }

    return ret;
}

int SetMEConfig3(unsigned char value)
{
    int ret = 0, i = 2;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    codeccount[i]++;

    if (value != pSysInfo->codec_advconfig[i].meConfig)
    {
        if ((ret = fSetMEConfig3(value)) == 0)
        {
            SetRestart();
        }
    }

    if (codeccount[i] == CODEC_MAX_VARIABLE)
    {
        SetCodecAdvParam(i);
    }

    return ret;
}

int SetPacketSize1(unsigned char value)
{
    int ret = 0, i = 0;

    codeccount[i]++;
    ret = fSetPacketSize1(value);

    if (codeccount[i] == CODEC_MAX_VARIABLE)
    {
        SetCodecAdvParam(i);
    }

    return ret;
}

int SetPacketSize2(unsigned char value)
{
    int ret = 0, i = 1;

    codeccount[i]++;
    ret = fSetPacketSize2(value);

    if (codeccount[i] == CODEC_MAX_VARIABLE)
    {
        SetCodecAdvParam(i);
    }

    return ret;
}

int SetPacketSize3(unsigned char value)
{
    int ret = 0, i = 2;

    codeccount[i]++;
    ret = fSetPacketSize3(value);

    if (codeccount[i] == CODEC_MAX_VARIABLE)
    {
        SetCodecAdvParam(i);
    }

    return ret;
}

int roicount[3] =
{
    0, 0, 0
};

#define ROI_MAX_VARIABLE (13)

void ResetCodecROIParam(void)
{
    int i = 0, id = 0;

    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return ;
    }

    for (id = 0; id < 3; id++)
    {
        pSysInfo->codec_roiconfig[id].numROI = 0;
        for (i = 0; i < 3; i++)
        {
            pSysInfo->codec_roiconfig[id].roi[i].startx = 0;
            pSysInfo->codec_roiconfig[id].roi[i].starty = 0;
            pSysInfo->codec_roiconfig[id].roi[i].width = 0;
            pSysInfo->codec_roiconfig[id].roi[i].height = 0;
        }
    }
}

void SetCodecROIParam(int id)
{
    int i = 0;
    CodecROIPrm codecROIPrm;
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return ;
    }

    roicount[id] = 0;

    codecROIPrm.numROI = pSysInfo->codec_roiconfig[id].numROI;

    for (i = 0; i < 3; i++)
    {
        codecROIPrm.roi[i].startx = pSysInfo->codec_roiconfig[id].roi[i].startx;
        codecROIPrm.roi[i].starty = pSysInfo->codec_roiconfig[id].roi[i].starty;
        codecROIPrm.roi[i].width = pSysInfo->codec_roiconfig[id].roi[i].width;
        codecROIPrm.roi[i].height = pSysInfo->codec_roiconfig[id].roi[i].height;
    }

    SetCodecROIMsg(id, &codecROIPrm);
}

int SetROIEnable1(unsigned char value)
{
    int ret = 0, i = 0;

    roicount[i]++;
    ret = fSetROIEnable1(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetROIEnable2(unsigned char value)
{
    int ret = 0, i = 1;

    roicount[i]++;
    ret = fSetROIEnable2(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetROIEnable3(unsigned char value)
{
    int ret = 0, i = 2;

    roicount[i]++;
    ret = fSetROIEnable3(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr1X1(unsigned int value)
{
    int ret = 0, i = 0;

    roicount[i]++;
    ret = fSetStr1X1(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr1Y1(unsigned int value)
{
    int ret = 0, i = 0;

    roicount[i]++;
    ret = fSetStr1Y1(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr1W1(unsigned int value)
{
    int ret = 0, i = 0;

    roicount[i]++;
    ret = fSetStr1W1(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr1H1(unsigned int value)
{
    int ret = 0, i = 0;

    roicount[i]++;
    ret = fSetStr1H1(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr1X2(unsigned int value)
{
    int ret = 0, i = 0;

    roicount[i]++;
    ret = fSetStr1X2(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr1Y2(unsigned int value)
{
    int ret = 0, i = 0;

    roicount[i]++;
    ret = fSetStr1Y2(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr1W2(unsigned int value)
{
    int ret = 0, i = 0;

    roicount[i]++;
    ret = fSetStr1W2(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr1H2(unsigned int value)
{
    int ret = 0, i = 0;

    roicount[i]++;
    ret = fSetStr1H2(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr1X3(unsigned int value)
{
    int ret = 0, i = 0;

    roicount[i]++;
    ret = fSetStr1X3(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr1Y3(unsigned int value)
{
    int ret = 0, i = 0;

    roicount[i]++;
    ret = fSetStr1Y3(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr1W3(unsigned int value)
{
    int ret = 0, i = 0;

    roicount[i]++;
    ret = fSetStr1W3(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr1H3(unsigned int value)
{
    int ret = 0, i = 0;

    roicount[i]++;
    ret = fSetStr1H3(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr2X1(unsigned int value)
{
    int ret = 0, i = 1;

    roicount[i]++;
    ret = fSetStr2X1(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr2Y1(unsigned int value)
{
    int ret = 0, i = 1;

    roicount[i]++;
    ret = fSetStr2Y1(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr2W1(unsigned int value)
{
    int ret = 0, i = 1;

    roicount[i]++;
    ret = fSetStr2W1(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr2H1(unsigned int value)
{
    int ret = 0, i = 1;

    roicount[i]++;
    ret = fSetStr2H1(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr2X2(unsigned int value)
{
    int ret = 0, i = 1;

    roicount[i]++;
    ret = fSetStr2X2(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr2Y2(unsigned int value)
{
    int ret = 0, i = 1;

    roicount[i]++;
    ret = fSetStr2Y2(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr2W2(unsigned int value)
{
    int ret = 0, i = 1;

    roicount[i]++;
    ret = fSetStr2W2(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr2H2(unsigned int value)
{
    int ret = 0, i = 1;

    roicount[i]++;
    ret = fSetStr2H2(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr2X3(unsigned int value)
{
    int ret = 0, i = 1;

    roicount[i]++;
    ret = fSetStr2X3(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr2Y3(unsigned int value)
{
    int ret = 0, i = 1;

    roicount[i]++;
    ret = fSetStr2Y3(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr2W3(unsigned int value)
{
    int ret = 0, i = 1;

    roicount[i]++;
    ret = fSetStr2W3(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr2H3(unsigned int value)
{
    int ret = 0, i = 1;

    roicount[i]++;
    ret = fSetStr2H3(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr3X1(unsigned int value)
{
    int ret = 0, i = 2;

    roicount[i]++;
    ret = fSetStr3X1(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr3Y1(unsigned int value)
{
    int ret = 0, i = 2;

    roicount[i]++;
    ret = fSetStr3Y1(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr3W1(unsigned int value)
{
    int ret = 0, i = 2;

    roicount[i]++;
    ret = fSetStr3W1(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr3H1(unsigned int value)
{
    int ret = 0, i = 2;

    roicount[i]++;
    ret = fSetStr3H1(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr3X2(unsigned int value)
{
    int ret = 0, i = 2;

    roicount[i]++;
    ret = fSetStr3X2(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr3Y2(unsigned int value)
{
    int ret = 0, i = 2;

    roicount[i]++;
    ret = fSetStr3Y2(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr3W2(unsigned int value)
{
    int ret = 0, i = 2;

    roicount[i]++;
    ret = fSetStr3W2(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr3H2(unsigned int value)
{
    int ret = 0, i = 2;

    roicount[i]++;
    ret = fSetStr3H2(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr3X3(unsigned int value)
{
    int ret = 0, i = 2;

    roicount[i]++;
    ret = fSetStr3X3(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr3Y3(unsigned int value)
{
    int ret = 0, i = 2;

    roicount[i]++;
    ret = fSetStr3Y3(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr3W3(unsigned int value)
{
    int ret = 0, i = 2;

    roicount[i]++;
    ret = fSetStr3W3(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetStr3H3(unsigned int value)
{
    int ret = 0, i = 2;

    roicount[i]++;
    ret = fSetStr3H3(value);

    if (roicount[i] == ROI_MAX_VARIABLE)
    {
        SetCodecROIParam(i);
    }

    return ret;
}

int SetAlarmEnable(unsigned char value)
{
    return fSetAlarmEnable(value);
}

int Avs_SetExtAlarm(unsigned char value)
{
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (pSysInfo->lan_config.nAlarmEnable == 0)
    {
        value = 0;
    }

    return fSetExtAlarm(value);
}

int Avs_SetDarkBlank(unsigned char value)
{
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (pSysInfo->lan_config.nAlarmEnable == 0)
    {
        value = 0;
    }

    return fSetDarkBlank(value);
}

int Avs_SetAlarmAudioPlay(unsigned char value)
{
    SysInfo *pSysInfo = GetSysInfo();
    if (pSysInfo == NULL)
    {
        return  - 1;
    }

    if (pSysInfo->audio_config.audiomode == 0)
    {
        value = 0;
    }
    if (pSysInfo->audio_config.audioON == 0)
    {
        value = 0;
    }

    return fSetAlarmAudioPlay(value);
}

int Avs_SetAlarmAudioFile(unsigned char value)
{
    return fSetAlarmAudioFile(value);
}

int Avs_SetScheduleRepeatEnable(unsigned char value)
{
    return fSetScheduleRepeatEnable(value);
}

int Avs_SetScheduleNumWeeks(unsigned char value)
{
    return fSetScheduleNumWeeks(value);
}

int Avs_SetScheduleInfiniteEnable(unsigned char value)
{
    return fSetScheduleInfiniteEnable(value);
}

int SetAlarmStorage(unsigned char value)
{
    return fSetAlarmStorage(value);
}

int SetRecordStorage(unsigned char value)
{
    return fSetRecordStorage(value);
}
//add by wbb 2014
int  SetStorageRec(unsigned char value)
{
    return fSetStorageRec(value);
}



