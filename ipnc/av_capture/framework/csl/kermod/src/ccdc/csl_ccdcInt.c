
#include <csl_ccdc.h>
#include <csl_sysDrv.h>

CSL_Status CSL_ccdcBufSwitch(CSL_CcdcHandle hndl, Uint32 timestamp, Uint32 count);
CSL_Status CSL_rszBufSwitch(CSL_RszHandle hndl, Uint8 rszMod, Uint32 timestamp, Uint32 count);
CSL_Status CSL_h3aBufSwitch(CSL_H3aHandle hndl, Uint8 h3aModId, Uint32 timestamp, Uint32 count);

CSL_IntRet_t CSL_ccdcVd0Isr(int intId, void *prm, void *reserved)
{
  CSL_CcdcHandle hndl = (CSL_CcdcHandle) prm;
  Uint32  timestamp;

  timestamp = CSL_sysGetCurTime();
  hndl->count++;

  CSL_ccdcBufSwitch(hndl, timestamp, hndl->count);
  CSL_rszBufSwitch(&gCSL_drvRszObj, CSL_RSZ_A, timestamp, hndl->count);
  CSL_rszBufSwitch(&gCSL_drvRszObj, CSL_RSZ_B, timestamp, hndl->count);
  CSL_h3aBufSwitch(&gCSL_drvH3aObj, CSL_H3A_MOD_AEWB, timestamp, hndl->count);
  CSL_h3aBufSwitch(&gCSL_drvH3aObj, CSL_H3A_MOD_AF, timestamp, hndl->count);
  
  return CSL_INT_RET_SOK;
}

CSL_IntRet_t CSL_ccdcVd1Isr(int intId, void *prm, void *reserved)
{

  return CSL_INT_RET_SOK;
}

CSL_IntRet_t CSL_ccdcIsr(int intId, void *prm, void *reserved)
{
  CSL_CcdcHandle hndl = &gCSL_drvCcdcObj;
  int     intType;

  if (intId == CSL_SYS_INT_CCDC_VD0)
    intType = CSL_CCDC_INT_TYPE_VD0;
  else if (intId == CSL_SYS_INT_CCDC_VD1)
    intType = CSL_CCDC_INT_TYPE_VD1;
  else
    return CSL_INT_RET_SOK;

  if (intType == CSL_CCDC_INT_TYPE_VD0) {
    CSL_ccdcVd0Isr(intId, prm, reserved);
  } else if (intType == CSL_CCDC_INT_TYPE_VD1) {
    CSL_ccdcVd1Isr(intId, prm, reserved);
  }

  CSL_sysFlagSet(&hndl->intFlag[intType]);

  return CSL_INT_RET_SOK;
}

CSL_Status CSL_ccdcIntEnable(CSL_CcdcHandle hndl, Uint8 intType, Bool32 enable)
{
  CSL_Status status;
  int     intId;

  if (hndl == NULL)
    return CSL_EFAIL;

  if (intType == CSL_CCDC_INT_TYPE_VD0)
    intId = CSL_SYS_INT_CCDC_VD0;
  else if (intType == CSL_CCDC_INT_TYPE_VD1)
    intId = CSL_SYS_INT_CCDC_VD1;
  else
    return CSL_EFAIL;

  status = CSL_sysIntEnable(intId, enable);

  return status;
}

CSL_Status CSL_ccdcIntClear(CSL_CcdcHandle hndl, Uint8 intType)
{
  CSL_Status status;
  int     intId;
  
  if (hndl == NULL)
    return CSL_EFAIL;

  if (intType == CSL_CCDC_INT_TYPE_VD0)
    intId = CSL_SYS_INT_CCDC_VD0;
  else if (intType == CSL_CCDC_INT_TYPE_VD1)
    intId = CSL_SYS_INT_CCDC_VD1;
  else
    return CSL_EFAIL;
    
  CSL_sysIntClear(intId);    

  status = CSL_sysFlagClear(&hndl->intFlag[intType]);

  return status;
}

CSL_Status CSL_ccdcIntWait(CSL_CcdcHandle hndl, Uint8 intType, Uint32 numIntWait)
{
  CSL_Status status = CSL_SOK;

  if (hndl == NULL)
    return CSL_EFAIL;

  if (intType >= CSL_CCDC_INT_TYPE_MAX)
    return CSL_EFAIL;

  while (numIntWait) {
    status = CSL_sysFlagWait(&hndl->intFlag[intType], CSL_SYS_TIMEOUT_FOREVER);
    if (status != CSL_SOK)
      break;
    numIntWait--;
  }

  return status;
}
