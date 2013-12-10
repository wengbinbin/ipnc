
#include <csl_ccdc.h>

CSL_Status CSL_ccdcBufInit(CSL_CcdcHandle hndl, CSL_BufInit * bufInit)
{
  CSL_Status status;

  status = CSL_bufInit(&hndl->dataOutBuf, bufInit);

  hndl->curBufInfo.id = CSL_BUF_ID_INVALID;

  CSL_ccdcSetSdramOutAddr(hndl, (Uint8 *) bufInit->bufAddr[0]);

  return status;
}

CSL_Status CSL_ccdcBufSwitchEnable(CSL_CcdcHandle hndl, Bool32 enable)
{
  hndl->dataOutBufSwitchEnable = enable;

  return CSL_SOK;
}

CSL_Status CSL_ccdcBufGetFull(CSL_CcdcHandle hndl, CSL_BufInfo * buf, Uint32 minBuf, Uint32 timeout)
{
  CSL_Status status;

  status = CSL_bufGetFull(&hndl->dataOutBuf, buf, minBuf, timeout);

  return status;
}

CSL_Status CSL_ccdcBufPutEmpty(CSL_CcdcHandle hndl, CSL_BufInfo * buf)
{
  CSL_Status status;

  status = CSL_bufPutEmpty(&hndl->dataOutBuf, buf);

  return status;
}

CSL_Status CSL_ccdcBufSwitch(CSL_CcdcHandle hndl, Uint32 timestamp, Uint32 count)
{
  CSL_Status status = CSL_SOK;

  if (hndl->dataOutBufSwitchEnable) {
    status = CSL_bufSwitchFull(&hndl->dataOutBuf, &hndl->curBufInfo, 1, timestamp, count);

    if (hndl->curBufInfo.id != CSL_BUF_ID_INVALID) {
      CSL_ccdcSetSdramOutAddr(hndl, (Uint8 *) hndl->curBufInfo.addr);
    }
  }

  return status;
}
