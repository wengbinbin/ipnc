

#include <capture_priv.h>
#include <osa_cmem.h>
#include <osa_file.h>
#include <display.h>
#include <encode.h>
#include <video.h>

//#define CAPTURE_FRAME_SKIP_INPUT

CAPTURE_Ctrl gCAPTURE_ctrl;
//add by sxh for continued settings
extern VIDEO_Ctrl gVIDEO_ctrl;


int CAPTURE_tskCreate(CAPTURE_CreatePrm *prm)
{
    Uint32 allocMemSize;
    int i, status, curOffset;

    gCAPTURE_ctrl.saveFrame = FALSE;
    gCAPTURE_ctrl.saveFileIndex = 0;

    gCAPTURE_ctrl.memVirtAddr = NULL;
    gCAPTURE_ctrl.memPhysAddr = NULL;

    memset(&gCAPTURE_ctrl.info, 0, sizeof(gCAPTURE_ctrl.info));

    status = OSA_mutexCreate(&gCAPTURE_ctrl.mutexLock);
    if (status != OSA_SOK)
    {
        OSA_ERROR("OSA_mutexCreate()\n");
        return status;
    }

    gCAPTURE_ctrl.info.createPrm.videoInputPort = MCVIP_VIDEO_INPUT_PORT_0;

    if (prm->videoDecoderMode == MCVIP_VIDEO_DECODER_MODE_2CH_D1_PORT_B)
    {
        gCAPTURE_ctrl.info.createPrm.videoInputPort = MCVIP_VIDEO_INPUT_PORT_1;
    }

    gCAPTURE_ctrl.info.createPrm.captureThrPri = (CAPTURE_THR_PRI + 1);
    gCAPTURE_ctrl.info.createPrm.videoDecoderId = MCVIP_VIDEO_DECODER_ID_TVP5158;
    gCAPTURE_ctrl.info.createPrm.videoDecoderMode = prm->videoDecoderMode;
    gCAPTURE_ctrl.info.createPrm.videoIfMode = prm->videoIfMode;
    gCAPTURE_ctrl.info.createPrm.videoSystem = prm->videoSystem;
    gCAPTURE_ctrl.info.createPrm.numBuf = MCVIP_getNumCh(prm->videoDecoderMode) *MCVIP_BUF_PER_CH_MIN;
    gCAPTURE_ctrl.info.createPrm.bufSize = MCVIP_getBufSize(prm->videoDecoderMode, prm->videoSystem);

    allocMemSize = gCAPTURE_ctrl.info.createPrm.numBuf * gCAPTURE_ctrl.info.createPrm.bufSize;

    #if DEBUG==0
    OSA_printf(" vl CAPTURE: Alloc info: numCh=%d numBuf=%d, bufSize=%d, allocSize=%d\n", MCVIP_getNumCh(prm->videoDecoderMode), gCAPTURE_ctrl.info.createPrm.numBuf, gCAPTURE_ctrl.info.createPrm.bufSize, allocMemSize);
    #endif
    
    #if DEBUG==0
    OSA_printf(" vl CAPTURE:Start map virtual addr!!!\n");
    OSA_printf(" vl CAPTURE:Start map virtual addr!!!\n");
    #endif

    gCAPTURE_ctrl.memVirtAddr = OSA_cmemAlloc(allocMemSize, 32);
    if (gCAPTURE_ctrl.memVirtAddr == NULL)
    {
        OSA_ERROR("OSA_cmemAlloc()\r\n");
        return OSA_EFAIL;
    }

    gCAPTURE_ctrl.memPhysAddr = OSA_cmemGetPhysAddr(gCAPTURE_ctrl.memVirtAddr);
    if (gCAPTURE_ctrl.memPhysAddr == NULL)
    {
        OSA_ERROR("OSA_cmemGetPhysAddr()\r\n");
        OSA_cmemFree(gCAPTURE_ctrl.memVirtAddr);
        return OSA_EFAIL;
    }
    
    #ifdef VANLINK_DVR_DM365_DEBUG
    OSA_printf(" vl CAPTURE:Start map virtual addr!!!\n");
    OSA_printf(" vl CAPTURE:Start map virtual addr!!!\n");
    #endif

    curOffset = 0;
    for (i = 0; i < gCAPTURE_ctrl.info.createPrm.numBuf; i++)
    {

        gCAPTURE_ctrl.info.createPrm.bufVirtAddr[i] = gCAPTURE_ctrl.memVirtAddr + curOffset;
        gCAPTURE_ctrl.info.createPrm.bufPhysAddr[i] = gCAPTURE_ctrl.memPhysAddr + curOffset;
        curOffset += gCAPTURE_ctrl.info.createPrm.bufSize;
        #ifdef VANLINK_DVR_DM365_DEBUG
        OSA_printf(" vl CAPTURE: gCAPTURE_ctrl.info.createPrm.numBuf = %d\n", gCAPTURE_ctrl.info.createPrm.numBuf);
        OSA_printf(" vl CAPTURE: PHYS = %x, VIRT = %x\n", (Uint32)gCAPTURE_ctrl.info.createPrm.bufPhysAddr[i], (Uint32)gCAPTURE_ctrl.info.createPrm.bufVirtAddr[i]);
        #endif
    }

    gCAPTURE_ctrl.info.mcvipHndl = MCVIP_create(&gCAPTURE_ctrl.info.createPrm);
    if (gCAPTURE_ctrl.info.mcvipHndl == NULL)
    {
        OSA_ERROR("MCVIP_create()\r\n");
        OSA_cmemFree(gCAPTURE_ctrl.memVirtAddr);
        return OSA_EFAIL;
    }

    status = MCVIP_getChList(gCAPTURE_ctrl.info.mcvipHndl, &gCAPTURE_ctrl.info.chList);
    OSA_assertSuccess(status);

    OSA_assert(gCAPTURE_ctrl.info.chList.numCh < MCVIP_CHANNELS_MAX);

    gCAPTURE_ctrl.reszPrm.inType = DRV_DATA_FORMAT_YUV422 | DRV_DATA_FORMAT_INTERLACED;
    gCAPTURE_ctrl.reszPrm.inPhysAddr = NULL;
    gCAPTURE_ctrl.reszPrm.inStartX = 0;
    gCAPTURE_ctrl.reszPrm.inStartY = 0;
    gCAPTURE_ctrl.reszPrm.inWidth = gCAPTURE_ctrl.info.chList.info[0].width;
    gCAPTURE_ctrl.reszPrm.inHeight = gCAPTURE_ctrl.info.chList.info[0].height - gCAPTURE_ctrl.reszPrm.inStartY;
    gCAPTURE_ctrl.reszPrm.inOffsetH = OSA_align(gCAPTURE_ctrl.reszPrm.inWidth, 32);
    gCAPTURE_ctrl.reszPrm.inOffsetV = gCAPTURE_ctrl.info.chList.info[0].height;
    gCAPTURE_ctrl.reszPrm.enableInvAlaw = FALSE;
    gCAPTURE_ctrl.reszPrm.enableInvDpcm = FALSE;
    
    #if DEBUG==0
    OSA_printf("  gCAPTURE_ctrl.reszPrm.inWidth = %d\n", gCAPTURE_ctrl.reszPrm.inWidth);
    OSA_printf(" gCAPTURE_ctrl.reszPrm.inHeight =%d\n", gCAPTURE_ctrl.reszPrm.inHeight);
    OSA_printf("  gCAPTURE_ctrl.reszPrm.inOffsetH = %d\n", gCAPTURE_ctrl.reszPrm.inOffsetH);
    OSA_printf(" gCAPTURE_ctrl.reszPrm.inOffsetV =%d\n", gCAPTURE_ctrl.reszPrm.inOffsetV);
    #endif

    gCAPTURE_ctrl.reszPrm.pOut[0] = NULL;
    gCAPTURE_ctrl.reszPrm.pOut[1] = NULL;

    gCAPTURE_ctrl.reszOutPrm[0].outType = DRV_DATA_FORMAT_YUV420;
    gCAPTURE_ctrl.reszOutPrm[0].flipH = FALSE;
    gCAPTURE_ctrl.reszOutPrm[0].flipV = FALSE;

    gCAPTURE_ctrl.reszOutPrm[1].outType = DRV_DATA_FORMAT_YUV420;
    gCAPTURE_ctrl.reszOutPrm[1].flipH = FALSE;
    gCAPTURE_ctrl.reszOutPrm[1].flipV = FALSE;

    CAPTURE_setFrameSkipMask(prm->frameSkipMask);

    return OSA_SOK;
}

int CAPTURE_tskDelete()
{
    int status = OSA_SOK;
    OSA_printf(" CAPTURE : MCVIP_delete Begin! ...\n");
    if (gCAPTURE_ctrl.info.mcvipHndl != NULL)
    {
        status = MCVIP_delete(gCAPTURE_ctrl.info.mcvipHndl);
    }
    OSA_printf(" CAPTURE : MCVIP_delete Done ...\n");

    if (gCAPTURE_ctrl.memVirtAddr != NULL)
    {
        status |= OSA_cmemFree(gCAPTURE_ctrl.memVirtAddr);
        gCAPTURE_ctrl.memVirtAddr = NULL;
    }

    return status;
}

int CAPTURE_tskStart()
{
    return MCVIP_start(gCAPTURE_ctrl.info.mcvipHndl, 8000);
}

int CAPTURE_tskStop()
{
    OSA_printf(" CAPTURE : MCVIP Stopping ...\n");
    MCVIP_stop(gCAPTURE_ctrl.info.mcvipHndl);
    OSA_printf(" CAPTURE : MCVIP Stopping ...DONE\n");
    return 0;
}

int CAPTURE_tskSaveFrame(MCVIP_BufInfo *pBufInfo)
{
    char filename[20];
    MCVIP_ChInfo *pChInfo;
    Uint32 fileSize;
    int status;

    sprintf(filename, "IMG_%04d.YUV", gCAPTURE_ctrl.saveFileIndex);

    pChInfo = &gCAPTURE_ctrl.info.chList.info[pBufInfo->chId];
    fileSize = pChInfo->offsetH *pChInfo->offsetV *2;

    gCAPTURE_ctrl.saveFileIndex++;

    status = OSA_fileWriteFile(filename, pBufInfo->virtAddr, fileSize);

    return status;
}

int CAPTURE_tskCalcReszClkDiv(int videoDecoderMode, int winId, int layoutMode)
{
    gCAPTURE_ctrl.reszPrm.clkDivM = 10;
    gCAPTURE_ctrl.reszPrm.clkDivN = 80;

    if (videoDecoderMode == MCVIP_VIDEO_DECODER_MODE_2CH_D1_PORT_A)
    {
        if (layoutMode == DISPLAY_LAYOUT_MODE_LIVE_1x1)
        {
            gCAPTURE_ctrl.reszPrm.clkDivM = 10;
            gCAPTURE_ctrl.reszPrm.clkDivN = 100;
        }
        if (layoutMode == DISPLAY_LAYOUT_MODE_LIVE_2x2)
        {
            gCAPTURE_ctrl.reszPrm.clkDivM = 10;
            gCAPTURE_ctrl.reszPrm.clkDivN = 80;
        }
        if (layoutMode == DISPLAY_LAYOUT_MODE_LIVE_3x3)
        {
            gCAPTURE_ctrl.reszPrm.clkDivM = 10;
            gCAPTURE_ctrl.reszPrm.clkDivN = 80;
        }
        if (layoutMode == DISPLAY_LAYOUT_MODE_LIVE_8CH)
        {
            if (winId == 0)
            {
                gCAPTURE_ctrl.reszPrm.clkDivM = 10;
                gCAPTURE_ctrl.reszPrm.clkDivN = 100;
            }
            else
            {
                gCAPTURE_ctrl.reszPrm.clkDivM = 10;
                gCAPTURE_ctrl.reszPrm.clkDivN = 80;
            }
        }
    }
    if (videoDecoderMode == MCVIP_VIDEO_DECODER_MODE_4CH_CIF)
    {
        if (layoutMode == DISPLAY_LAYOUT_MODE_LIVE_1x1)
        {
            gCAPTURE_ctrl.reszPrm.clkDivM = 10;
            gCAPTURE_ctrl.reszPrm.clkDivN = 160;
        }
        if (layoutMode == DISPLAY_LAYOUT_MODE_LIVE_2x2)
        {
            gCAPTURE_ctrl.reszPrm.clkDivM = 10;
            gCAPTURE_ctrl.reszPrm.clkDivN = 80;
        }
        if (layoutMode == DISPLAY_LAYOUT_MODE_LIVE_3x3)
        {
            gCAPTURE_ctrl.reszPrm.clkDivM = 10;
            gCAPTURE_ctrl.reszPrm.clkDivN = 80;
        }
        if (layoutMode == DISPLAY_LAYOUT_MODE_LIVE_8CH)
        {
            if (winId == 0)
            {
                gCAPTURE_ctrl.reszPrm.clkDivM = 10;
                gCAPTURE_ctrl.reszPrm.clkDivN = 100;
            }
            else
            {
                gCAPTURE_ctrl.reszPrm.clkDivM = 10;
                gCAPTURE_ctrl.reszPrm.clkDivN = 80;
            }
        }
    }
    if (videoDecoderMode == MCVIP_VIDEO_DECODER_MODE_8CH_CIF)
    {
        if (layoutMode == DISPLAY_LAYOUT_MODE_LIVE_1x1)
        {
            if (winId == 0)
            {
                gCAPTURE_ctrl.reszPrm.clkDivM = 10;
                gCAPTURE_ctrl.reszPrm.clkDivN = 140;
            }
            else
            {
                gCAPTURE_ctrl.reszPrm.clkDivM = 10;
                gCAPTURE_ctrl.reszPrm.clkDivN = 50;
            }
        }
        if (layoutMode == DISPLAY_LAYOUT_MODE_LIVE_2x2)
        {
            gCAPTURE_ctrl.reszPrm.clkDivM = 10;
            gCAPTURE_ctrl.reszPrm.clkDivN = 50;
        }
        if (layoutMode == DISPLAY_LAYOUT_MODE_LIVE_3x3)
        {
            gCAPTURE_ctrl.reszPrm.clkDivM = 10;
            gCAPTURE_ctrl.reszPrm.clkDivN = 50;
        }
        if (layoutMode == DISPLAY_LAYOUT_MODE_LIVE_8CH)
        {
            if (winId == 0)
            {
                gCAPTURE_ctrl.reszPrm.clkDivM = 10;
                gCAPTURE_ctrl.reszPrm.clkDivN = 80;
            }
            else
            {
                gCAPTURE_ctrl.reszPrm.clkDivM = 10;
                gCAPTURE_ctrl.reszPrm.clkDivN = 50;
            }
        }
    }
    return OSA_SOK;
}

int CAPTURE_tskProcessFrame(MCVIP_BufInfo *pBufInfo)
{
    int status = OSA_SOK, chId, layoutBufId, encBufId, winId;
    MCVIP_ChInfo *pChInfo;
    DISPLAY_LayoutInfo layoutInfo;
    OSA_BufInfo *pLayoutBuf,  *pEncodeBuf;
    Bool skipFrame;

    chId = pBufInfo->chId;

    // valid channel ID
    pChInfo = &gCAPTURE_ctrl.info.chList.info[chId];

    #ifdef CAPTURE_FRAME_SKIP_INPUT
    skipFrame = TRUE;

    if (gCAPTURE_ctrl.chInfo[chId].frameSkipMask &(1 << (gCAPTURE_ctrl.chInfo[chId].curFrameNum % 30)))
    {
        skipFrame = FALSE;
    }

    gCAPTURE_ctrl.chInfo[chId].curFrameNum++;

    if (skipFrame)
    {
        return OSA_SOK;
    }

    #endif

    DISPLAY_layoutGetInfo(&layoutInfo);

    winId =  - 1;

    if (layoutInfo.layoutMode != DISPLAY_LAYOUT_MODE_PLAYBACK)
    {
        if (gCAPTURE_ctrl.curPage == (chId / layoutInfo.numWin))
        {
            winId = chId % layoutInfo.numWin;
        }
    }

    gCAPTURE_ctrl.reszPrm.pOut[0] = NULL;
    gCAPTURE_ctrl.reszPrm.pOut[1] = NULL;
    pLayoutBuf = NULL;
    pEncodeBuf = NULL;

    gCAPTURE_ctrl.reszPrm.inPhysAddr = pBufInfo->physAddr;
    gCAPTURE_ctrl.reszPrm.inVirtAddr = pBufInfo->virtAddr;

    if (winId >= 0 && winId < layoutInfo.numWin)
    {

        //OSA_printf(" CAPTURE: %d: %dx%d\n", winId, gCAPTURE_ctrl.reszOutPrm[0].outWidth, gCAPTURE_ctrl.reszOutPrm[0].outHeight);

        pLayoutBuf = DISPLAY_layoutGetEmptyWinBuf(winId, &layoutBufId, OSA_TIMEOUT_NONE);

        #ifdef CAPTURE_DEBUG
        if (pLayoutBuf == NULL)
        {
            OSA_printf(" CAPTURE: Layout Buf not free for ch %d, win %d\n", chId, winId);
        }
        #endif

        if (pLayoutBuf != NULL)
        {

            gCAPTURE_ctrl.reszPrm.pOut[0] = &gCAPTURE_ctrl.reszOutPrm[0];

            gCAPTURE_ctrl.reszOutPrm[0].outVirtAddr = pLayoutBuf->virtAddr;
            gCAPTURE_ctrl.reszOutPrm[0].outPhysAddr = pLayoutBuf->physAddr;
            gCAPTURE_ctrl.reszOutPrm[0].outWidth = layoutInfo.winInfo[winId].width;
            gCAPTURE_ctrl.reszOutPrm[0].outHeight = layoutInfo.winInfo[winId].height;
            #ifdef VANLINK_DVR_DM365_DEBUG
            //OSA_printf("  gCAPTURE_ctrl.reszOutPrm[0].outWidth=%d\n",  gCAPTURE_ctrl.reszOutPrm[0].outWidth);
            //OSA_printf("gCAPTURE_ctrl.reszOutPrm[0].outHeight =%d!!!\n",gCAPTURE_ctrl.reszOutPrm[0].outHeight);
            #endif
            gCAPTURE_ctrl.reszOutPrm[0].outOffsetH = OSA_align(gCAPTURE_ctrl.reszOutPrm[0].outWidth, 32);
            gCAPTURE_ctrl.reszOutPrm[0].outOffsetV = gCAPTURE_ctrl.reszOutPrm[0].outHeight;
        }
    }

    skipFrame = FALSE;

    #ifndef CAPTURE_FRAME_SKIP_INPUT
    skipFrame = TRUE;

    if (gCAPTURE_ctrl.chInfo[chId].frameSkipMask &(1 << (gCAPTURE_ctrl.chInfo[chId].curFrameNum % 30)))
    {
        skipFrame = FALSE;
    }

    gCAPTURE_ctrl.chInfo[chId].curFrameNum++;
    #endif

    if (!skipFrame)
    {

        #if 1
        pEncodeBuf = ENCODE_getEmptyBuf(chId, &encBufId, OSA_TIMEOUT_NONE);

        #ifdef CAPTURE_DEBUG
        if (pEncodeBuf == NULL)
        {
            OSA_printf(" CAPTURE: Layout Buf not free for ch %d, win %d\n", chId, winId);
        }
        #endif
    }

    if (pEncodeBuf != NULL)
    {

        gCAPTURE_ctrl.reszPrm.pOut[1] = &gCAPTURE_ctrl.reszOutPrm[1];

        gCAPTURE_ctrl.reszOutPrm[1].outVirtAddr = pEncodeBuf->virtAddr;
        gCAPTURE_ctrl.reszOutPrm[1].outPhysAddr = pEncodeBuf->physAddr;
        gCAPTURE_ctrl.reszOutPrm[1].outWidth = pChInfo->width;
        #ifdef VANLINK_DVR_DM365_DEBUG
        //OSA_printf(" gCAPTURE_ctrl.reszOutPrm[1].outWidth=%d\n",  gCAPTURE_ctrl.reszOutPrm[1].outWidth);
        //OSA_printf("gCAPTURE_ctrl.reszOutPrm[1].outHeight =%d!!!\n",gCAPTURE_ctrl.reszOutPrm[1].outHeight);
        #endif

        if (gCAPTURE_ctrl.reszOutPrm[1].outWidth == 720)
        {
            gCAPTURE_ctrl.reszOutPrm[1].outWidth = 704;
        }
        if (gCAPTURE_ctrl.reszOutPrm[1].outWidth == 360)
        {
            gCAPTURE_ctrl.reszOutPrm[1].outWidth = 352;
        }

        gCAPTURE_ctrl.reszOutPrm[1].outHeight = pChInfo->height;
        gCAPTURE_ctrl.reszOutPrm[1].outOffsetH = OSA_align(gCAPTURE_ctrl.reszOutPrm[1].outWidth, 32);
        gCAPTURE_ctrl.reszOutPrm[1].outOffsetV = gCAPTURE_ctrl.reszOutPrm[1].outHeight;
        #ifdef VANLINK_DVR_DM365_DEBUG
        //OSA_printf("gCAPTURE_ctrl.reszOutPrm[1].outOffsetH=%d\n", gCAPTURE_ctrl.reszOutPrm[1].outOffsetH);
        //OSA_printf("gCAPTURE_ctrl.reszOutPrm[1].outOffsetV=%d\n", gCAPTURE_ctrl.reszOutPrm[1].outOffsetV);
        //OSA_printf("gCAPTURE_ctrl.reszOutPrm[1].outHeight =%d!!!\n",gCAPTURE_ctrl.reszOutPrm[1].outHeight);
        #endif
        pEncodeBuf->timesec = pBufInfo->timesec; // AYK - 1217	
        pEncodeBuf->timestamp = pBufInfo->timestamp;
    }
    #endif

    CAPTURE_tskCalcReszClkDiv(gCAPTURE_ctrl.info.createPrm.videoDecoderMode, winId, layoutInfo.layoutMode);

    #if 0//def VANLINK_DVR_DM365_DEBUG
    OSA_printf(" CAPTURE: %d, %d: RESZ: %d, %d %dx%d (%08x) -> %dx%d (%08x), %dx%d (%08x) (DIV M:N=%d:%d)\n", chId, winId, gCAPTURE_ctrl.reszPrm.inStartX, gCAPTURE_ctrl.reszPrm.inStartY, gCAPTURE_ctrl.reszPrm.inWidth, gCAPTURE_ctrl.reszPrm.inHeight, (Uint32)gCAPTURE_ctrl.reszPrm.inPhysAddr, gCAPTURE_ctrl.reszOutPrm[0].outWidth, gCAPTURE_ctrl.reszOutPrm[0].outHeight, (Uint32)gCAPTURE_ctrl.reszOutPrm[0].outPhysAddr, gCAPTURE_ctrl.reszOutPrm[1].outWidth, gCAPTURE_ctrl.reszOutPrm[1].outHeight, (Uint32)gCAPTURE_ctrl.reszOutPrm[1].outPhysAddr, gCAPTURE_ctrl.reszPrm.clkDivM, gCAPTURE_ctrl.reszPrm.clkDivN);
    #endif

    if (gCAPTURE_ctrl.reszPrm.pOut[1] != NULL || gCAPTURE_ctrl.reszPrm.pOut[0])
    {

        OSA_prfBegin(&gCAPTURE_ctrl.prfResz);

        status = DRV_reszRun(&gCAPTURE_ctrl.reszPrm);

        OSA_prfEnd(&gCAPTURE_ctrl.prfResz, 1);
    }

    if (FALSE)
    {
        //gCAPTURE_ctrl.saveFrame) {//add by sxh saveframe
        OSA_fileWriteFile("YUVIN.BIN", gCAPTURE_ctrl.reszPrm.inVirtAddr, (gCAPTURE_ctrl.reszPrm.inOffsetH *gCAPTURE_ctrl.reszPrm.inOffsetV * 2));

        OSA_fileWriteFile("YUVOUT1.BIN", gCAPTURE_ctrl.reszOutPrm[1].outVirtAddr, (gCAPTURE_ctrl.reszOutPrm[1].outOffsetH *gCAPTURE_ctrl.reszOutPrm[1].outOffsetV *3) / 2);
        OSA_fileWriteFile("YUVOUT0.BIN", gCAPTURE_ctrl.reszOutPrm[0].outVirtAddr, (gCAPTURE_ctrl.reszOutPrm[0].outOffsetH *gCAPTURE_ctrl.reszOutPrm[0].outOffsetV *3) / 2);

        gCAPTURE_ctrl.saveFrame = FALSE;
    }

    if (pLayoutBuf != NULL)
    {
        //VIDEO_swosdRun(VIDEO_TSK_CAPTURE, chId, pLayoutBuf);//add by sxh
        DISPLAY_layoutPutFullWinBuf(winId, pLayoutBuf, layoutBufId, layoutInfo.layoutMode);
    }

    if (pEncodeBuf != NULL)
    {
        VIDEO_swosdRun(VIDEO_TSK_CAPTURE, chId, pEncodeBuf); //add by sxh
        ENCODE_putFullBuf(chId, pEncodeBuf, encBufId);
    }

    return status;
}

int CAPTURE_tskStartRun(OSA_TskHndl *pTsk, OSA_MsgHndl *pMsg, Bool *isDone, Bool *doAckMsg)
{
    int status, bufId, chId;
    Bool done = FALSE, ackMsg = TRUE;
    Uint16 cmd;
    MCVIP_BufInfo *pBufInfo;
    OSA_BufInfo *pOSDBufInfo; //add by sxh osd

    *isDone = FALSE;
    *doAckMsg = FALSE;

    status = CAPTURE_tskStart();

    OSA_tskAckOrFreeMsg(pMsg, status);

    if (status != OSA_SOK)
    {
        return status;
    }

    while (!done)
    {

        bufId =  - 1;

        OSA_prfBegin(&gCAPTURE_ctrl.prfCapture);

        status = MCVIP_getBuf(gCAPTURE_ctrl.info.mcvipHndl, &bufId, OSA_TIMEOUT_FOREVER);
        if (status != OSA_SOK)
        {
            done = TRUE;
            break;
        }

        pBufInfo = MCVIP_getBufInfo(gCAPTURE_ctrl.info.mcvipHndl, bufId);


        if (pBufInfo != NULL)
        {
            // valid buffer
            chId = pBufInfo->chId;

            if (chId >= 0 && chId < gCAPTURE_ctrl.info.chList.numCh)
            {
                 /******************add by sxh osd***********************
                pOSDBufInfo = (OSA_BufInfo*)malloc(sizeof(OSA_BufInfo)); //add by sxh
                pOSDBufInfo->virtAddr = pBufInfo->virtAddr;
                pOSDBufInfo->physAddr = pBufInfo->physAddr;
                pOSDBufInfo->timestamp = pBufInfo->timestamp;

                printf("pOSDBufInfo=%s\n", pOSDBufInfo);
                #if 0//def MCVIP_DEBUG_HANG
                OSA_printf(" VIDEO_swosdRun   chId %d\n", chId);
                #endif
                VIDEO_swosdRun(1, chId, pOSDBufInfo); //add by sxh
                *******************add by sxh osd***********************/
                
                CAPTURE_tskProcessFrame(pBufInfo);

                MCVIP_putBuf(gCAPTURE_ctrl.info.mcvipHndl, bufId, MCVIP_FLAG_ALL_DONE);
            }
        }

        OSA_prfEnd(&gCAPTURE_ctrl.prfCapture, 1);

        status = OSA_tskCheckMsg(pTsk, &pMsg);
        //OSA_printf(" OSA_tskCheckMsg :  status=%d ...\n",status);
        if (status == OSA_SOK)
        {

            cmd = OSA_msgGetCmd(pMsg);
            OSA_printf(" CAPTURE_tskStartRun :  cmd=%d ...\n", cmd);
            switch (cmd)
            {
                case CAPTURE_CMD_STOP:
                    done = TRUE;
                    ackMsg = TRUE;
                    break;

                case CAPTURE_CMD_DELETE:
                    done = TRUE;
                    *isDone = TRUE;
                    *doAckMsg = TRUE;
                    break;

                default:
                    OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
                    break;
            }
        }
    }

    CAPTURE_tskStop();
    if (ackMsg)
    {
        OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
    }

    return status;
}

int CAPTURE_tskMain(struct OSA_TskHndl *pTsk, OSA_MsgHndl *pMsg, Uint32 curState)
{
    int status;
    Bool done = FALSE, ackMsg = FALSE;
    ;
    Uint16 cmd = OSA_msgGetCmd(pMsg);
    CAPTURE_CreatePrm *pCreatePrm = (CAPTURE_CreatePrm*)OSA_msgGetPrm(pMsg);

    OSA_printf("CAPTURE_tskMain Starting\n");

    if (cmd != CAPTURE_CMD_CREATE || pCreatePrm == NULL)
    {
        OSA_tskAckOrFreeMsg(pMsg, OSA_EFAIL);
        return OSA_SOK;
    }

    status = CAPTURE_tskCreate(pCreatePrm);

    OSA_tskAckOrFreeMsg(pMsg, status);

    if (status != OSA_SOK)
    {
        return OSA_SOK;
    }

    while (!done)
    {
        status = OSA_tskWaitMsg(pTsk, &pMsg);
        if (status != OSA_SOK)
        {
            done = TRUE;
            break;
        }

        cmd = OSA_msgGetCmd(pMsg);
        OSA_printf(" CAPTURE_tskMain :  cmd=%d ...\n", cmd);
        
        switch (cmd)
        {
            case CAPTURE_CMD_START:
                CAPTURE_tskStartRun(pTsk, pMsg, &done, &ackMsg);
                break;

            case CAPTURE_CMD_DELETE:
                done = TRUE;
                ackMsg = TRUE;
                break;

            default:
                OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
                break;
        }
    }
    OSA_printf(" CAPTURE_tskMain :  CAPTURE_tskDelete ....\n");

    CAPTURE_tskDelete();
    if (ackMsg)
    {
        OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
    }

    return OSA_SOK;
}


int CAPTURE_sendCmd(Uint16 cmd, void *prm, Uint16 flags)
{
    OSA_printf(" CAPTURE_sendCmd :  gCAPTURE_ctrl.tskHndl.mbxHndl=%x..gCAPTURE_ctrl.mbxHndl=%x..cmd=%x\n", gCAPTURE_ctrl.tskHndl.mbxHndl, gCAPTURE_ctrl.mbxHndl, cmd);
    return OSA_mbxSendMsg(&gCAPTURE_ctrl.tskHndl.mbxHndl, &gCAPTURE_ctrl.mbxHndl, cmd, prm, flags);
    OSA_printf("CAPTURE_sendCmd Done! cmd=%x \n", cmd);
}

int CAPTURE_create(CAPTURE_CreatePrm *prm)
{
    int status;

    memset(&gCAPTURE_ctrl, 0, sizeof(gCAPTURE_ctrl));

    status = OSA_tskCreate(&gCAPTURE_ctrl.tskHndl, CAPTURE_tskMain, CAPTURE_THR_PRI, CAPTURE_STACK_SIZE, 0);
    gVIDEO_ctrl.captureTsk = gCAPTURE_ctrl.tskHndl; //add by sxh

    OSA_assertSuccess(status);

    status = OSA_mbxCreate(&gCAPTURE_ctrl.mbxHndl);

    OSA_assertSuccess(status);

    status = CAPTURE_sendCmd(CAPTURE_CMD_CREATE, prm, OSA_MBX_WAIT_ACK);

    return status;
}

int CAPTURE_delete()
{
    int status;

    status = CAPTURE_sendCmd(CAPTURE_CMD_DELETE, NULL, OSA_MBX_WAIT_ACK);

    status = OSA_tskDelete(&gCAPTURE_ctrl.tskHndl);

    OSA_assertSuccess(status);

    status = OSA_mbxDelete(&gCAPTURE_ctrl.mbxHndl);

    OSA_assertSuccess(status);

    return status;
}

int CAPTURE_start()
{
    return CAPTURE_sendCmd(CAPTURE_CMD_START, NULL, OSA_MBX_WAIT_ACK);
}

int CAPTURE_stop()
{
    OSA_printf(" CAPTURE : Capture Tsk Stopping ...\n");
    CAPTURE_sendCmd(CAPTURE_CMD_STOP, NULL, OSA_MBX_WAIT_ACK);
    OSA_printf(" CAPTURE : Capture Tsk Stopping ...Done!\n");
    return OSA_SOK;
}

int CAPTURE_saveFrame()
{
    gCAPTURE_ctrl.saveFrame = TRUE;

    MCVIP_saveFrame(gCAPTURE_ctrl.info.mcvipHndl);

    return OSA_SOK;
}

CAPTURE_Info *CAPTURE_getInfo()
{
    return  &gCAPTURE_ctrl.info;
}

int CAPTURE_selectPage(int pageNum)
{
    if (pageNum >= 0 && pageNum < gCAPTURE_ctrl.info.chList.numCh)
    {
        gCAPTURE_ctrl.curPage = pageNum;
    }

    return OSA_SOK;
}

int CAPTURE_setFrameSkipMask(Uint32 frameSkipMask)
{
    int i;

    OSA_mutexLock(&gCAPTURE_ctrl.mutexLock);

    for (i = 0; i < gCAPTURE_ctrl.info.chList.numCh; i++)
    {
        gCAPTURE_ctrl.chInfo[i].frameSkipMask = frameSkipMask;
    }

    OSA_mutexUnlock(&gCAPTURE_ctrl.mutexLock);

    return OSA_SOK;
}
