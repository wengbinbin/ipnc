
#include <capture_priv.h>
#include <display_priv.h>
#include <decode_priv.h>
#include <encode_priv.h>
#include <writer_priv.h>
//extern AVSERVER_Ctrl   gAVSERVER_ctrl;//add by sxh osd

void SYSTEM_profilePrint(char *name, OSA_PrfHndl *hndl)
{
    #ifdef OSA_PRF_ENABLE
    if (hndl->count == 0 || hndl->totalTime < 10)
    {
        return ;
    }

    OSA_printf(" %s | %14.2f | %10.2f | %10d | %12d |\n", name, (float)hndl->totalTime / hndl->count, (float)(hndl->count *1000) / hndl->totalTime, hndl->totalTime, hndl->count);
    #endif
}

int SYSTEM_profileInfoShow()
{
    OSA_printf("\n");
    OSA_printf(" Module   | Avg Time/Frame | Frame-rate | Total time | Total Frames |\n")SYSTEM_profilePrint("CAPTURE ", &gCAPTURE_ctrl.prfCapture);
    SYSTEM_profilePrint("RESIZE  ", &gCAPTURE_ctrl.prfResz);
    SYSTEM_profilePrint("ENCODE  ", &gENCODE_ctrl.prfEnc);
    SYSTEM_profilePrint("WRITER  ", &gWRITER_ctrl.prfFileWr);
    SYSTEM_profilePrint("DISPLAY ", &gDISPLAY_ctrl.prfDisplay);
    SYSTEM_profilePrint("DMA     ", &gDISPLAY_ctrl.prfDma);
    SYSTEM_profilePrint("DECODE  ", &gDECODE_ctrl.prfDec);
    SYSTEM_profilePrint("DEC:RESZ", &gDECODE_ctrl.prfResz);
    //SYSTEM_profilePrint("OSD0    ", &gAVSERVER_ctrl.swosdPrf[0]);
    OSA_printf("\n");

    return OSA_SOK;
}
