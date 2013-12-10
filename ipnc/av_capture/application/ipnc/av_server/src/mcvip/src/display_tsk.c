

#include <display_priv.h>

DISPLAY_Ctrl gDISPLAY_ctrl;

int DISPLAY_tskCreate(DISPLAY_CreatePrm *prm)
{
    int status;
    DRV_DisplayWinConfig displayConfig;

    gDISPLAY_ctrl.blankDetectThres = 15;

    displayConfig.winId = DRV_DISPLAY_VID_WIN_0;
    displayConfig.numBuf = 5;
    displayConfig.dataFormat = DRV_DATA_FORMAT_YUV420;
    displayConfig.startX = 0;
    displayConfig.startY = 0;
    displayConfig.width = 640;
    displayConfig.height = 480; //add by sxh
    displayConfig.expandH = TRUE; //add by sxh
    displayConfig.expandV = FALSE; //add by sxh
    if (displayConfig.dataFormat == DRV_DATA_FORMAT_YUV422)
    {
        displayConfig.offsetH = OSA_align(displayConfig.width, 16);
    }
    else
    {
        displayConfig.offsetH = OSA_align(displayConfig.width, 32);
    }

    displayConfig.offsetV = displayConfig.height;
    displayConfig.zoomH = 0;
    displayConfig.zoomV = 0;


    status = DRV_displaySetMode(DRV_DISPLAY_MODE_NTSC);
    if (status != OSA_SOK)
    {
        return status;
    }

    gDISPLAY_ctrl.displayBufId = 0;
    #ifdef VANLINK_DVR_DM365_DEBUG
    OSA_printf(" vl DRV_displayOpen start!!!\n");
    #endif
    status = DRV_displayOpen(&gDISPLAY_ctrl.displayHndl, &displayConfig);
    if (status != OSA_SOK)
    {
        return status;
    }

    #ifdef VANLINK_DVR_DM365_DEBUG
    OSA_printf(" vl DRV_displayOpen done!!!\n");
    #endif

    DRV_displayGetBufInfo(&gDISPLAY_ctrl.displayHndl, &gDISPLAY_ctrl.displayInfo);

    gDISPLAY_ctrl.frameInfo.width = gDISPLAY_ctrl.displayInfo.width;
    gDISPLAY_ctrl.frameInfo.height = gDISPLAY_ctrl.displayInfo.height;
    gDISPLAY_ctrl.frameInfo.lineOffsetH = gDISPLAY_ctrl.displayInfo.offsetH;
    gDISPLAY_ctrl.frameInfo.lineOffsetV = gDISPLAY_ctrl.displayInfo.offsetV;

    status = DISPLAY_layoutCreate(prm->layoutMode);

    return status;
}

int DISPLAY_tskDelete()
{
    int status;

    status = DRV_displayClose(&gDISPLAY_ctrl.displayHndl);
    status |= DISPLAY_layoutDelete();

    return status;
}

int DISPLAY_tskStartRun(OSA_TskHndl *pTsk, OSA_MsgHndl *pMsg, Bool *isDone, Bool *doAckMsg)
{
    int status = OSA_SOK;
    Bool done = FALSE, ackMsg = TRUE;
    Uint16 cmd;
    DISPLAY_BufInfo displayBufInfo;
    DISPLAY_LayoutInfo layoutInfo;

    *isDone = FALSE;
    *doAckMsg = FALSE;

    OSA_tskAckOrFreeMsg(pMsg, status);

    if (status != OSA_SOK)
    {
        return status;
    }

    while (!done)
    {

        OSA_prfBegin(&gDISPLAY_ctrl.prfDisplay);

        DISPLAY_layoutGetInfo(&layoutInfo);

        if (layoutInfo.layoutMode == DISPLAY_LAYOUT_MODE_PLAYBACK)
        {

            OSA_waitMsecs(30);

        }
        else
        {

            displayBufInfo.id = gDISPLAY_ctrl.displayBufId;
            displayBufInfo.virtAddr = gDISPLAY_ctrl.displayInfo.virtAddr[displayBufInfo.id];
            displayBufInfo.physAddr = gDISPLAY_ctrl.displayInfo.physAddr[displayBufInfo.id];

            DISPLAY_layoutRun(&displayBufInfo, &layoutInfo);

            DRV_displaySwitchBuf(&gDISPLAY_ctrl.displayHndl, &gDISPLAY_ctrl.displayBufId);
        }

        OSA_prfEnd(&gDISPLAY_ctrl.prfDisplay, 1);

        status = OSA_tskCheckMsg(pTsk, &pMsg);
        if (status == OSA_SOK)
        {

            cmd = OSA_msgGetCmd(pMsg);

            switch (cmd)
            {
                case DISPLAY_CMD_STOP:
                    done = TRUE;
                    ackMsg = TRUE;
                    break;

                case DISPLAY_CMD_DELETE:
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

    if (ackMsg)
    {
        OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
    }

    return status;
}

int DISPLAY_tskMain(struct OSA_TskHndl *pTsk, OSA_MsgHndl *pMsg, Uint32 curState)
{
    int status;
    Bool done = FALSE, ackMsg = FALSE;
    ;
    Uint16 cmd = OSA_msgGetCmd(pMsg);
    DISPLAY_CreatePrm *pCreatePrm = (DISPLAY_CreatePrm*)OSA_msgGetPrm(pMsg);

    if (cmd != DISPLAY_CMD_CREATE || pCreatePrm == NULL)
    {
        OSA_tskAckOrFreeMsg(pMsg, OSA_EFAIL);
        return OSA_SOK;
    }

    status = DISPLAY_tskCreate(pCreatePrm);

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

        switch (cmd)
        {
            case DISPLAY_CMD_START:
                DISPLAY_tskStartRun(pTsk, pMsg, &done, &ackMsg);
                break;

            case DISPLAY_CMD_DELETE:
                done = TRUE;
                ackMsg = TRUE;
                break;

            default:
                OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
                break;
        }
    }

    DISPLAY_tskDelete();
    if (ackMsg)
    {
        OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
    }

    return OSA_SOK;
}


int DISPLAY_sendCmd(Uint16 cmd, void *prm, Uint16 flags)
{
    return OSA_mbxSendMsg(&gDISPLAY_ctrl.tskHndl.mbxHndl, &gDISPLAY_ctrl.mbxHndl, cmd, prm, flags);
}

int DISPLAY_create(DISPLAY_CreatePrm *prm)
{
    int status;

    memset(&gDISPLAY_ctrl, 0, sizeof(gDISPLAY_ctrl));

    status = OSA_tskCreate(&gDISPLAY_ctrl.tskHndl, DISPLAY_tskMain, DISPLAY_THR_PRI, DISPLAY_STACK_SIZE, 0);

    OSA_assertSuccess(status);

    status = OSA_mbxCreate(&gDISPLAY_ctrl.mbxHndl);

    OSA_assertSuccess(status);

    status = DISPLAY_sendCmd(DISPLAY_CMD_CREATE, prm, OSA_MBX_WAIT_ACK);

    return status;
}

int DISPLAY_delete()
{
    int status;

    status = DISPLAY_sendCmd(DISPLAY_CMD_DELETE, NULL, OSA_MBX_WAIT_ACK);

    status = OSA_tskDelete(&gDISPLAY_ctrl.tskHndl);

    OSA_assertSuccess(status);

    status = OSA_mbxDelete(&gDISPLAY_ctrl.mbxHndl);

    OSA_assertSuccess(status);

    return status;
}

int DISPLAY_start()
{
    return DISPLAY_sendCmd(DISPLAY_CMD_START, NULL, OSA_MBX_WAIT_ACK);
}

int DISPLAY_stop()
{
    return DISPLAY_sendCmd(DISPLAY_CMD_STOP, NULL, OSA_MBX_WAIT_ACK);
}

int DISPLAY_fieldModeEnable(Bool enable)
{
    DRV_displaySetFieldModeEnable(&gDISPLAY_ctrl.displayHndl, enable);
    return OSA_SOK;
}
