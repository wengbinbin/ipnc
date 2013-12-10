#include <csl_rszIoctl.h>

CSL_Status CSL_rszHwControl(CSL_RszHandle hndl, Uint32 cmd, void *prm)
{
  CSL_Status status = CSL_SOK;

  static CSL_RszHwSetup data;
  static CSL_RszInConfig inConfig;
  static CSL_RszInFrameConfig inFrameConfig;
  static CSL_RszClkConfig clkConfig;
  static CSL_RszMinMaxConfig minMaxConfig;
  static CSL_RszOutConfig rszAConfig;
  static CSL_RszOutConfig rszBConfig;

  static CSL_RszOutConfig rszOutConfig;
  static CSL_RszSetOutConfigPrm setOutConfigPrm;
  static CSL_RszOutEnablePrm outEnablePrm;
  static CSL_RszSetOutAddrPrm setOutAddrPrm;
  static CSL_RszGetStatusPrm getStatusPrm;
  static CSL_RszStatus rszStatus;

  static CSL_RszSetOutFrameConfigPrm setOutFrameConfigPrm;
  static CSL_RszOutFrameConfig outFrameConfig;

  static CSL_RszGetOutFrameConfigPrm getOutFrameConfig;
  static CSL_RszDzoomConfig setDzoomConfig;

  static CSL_RszBufInitPrm bufInitPrm;
  static CSL_BufInit bufInit;
  static CSL_RszBufSwitchEnablePrm bufSwitchEnablePrm;
  static CSL_RszBufGetFullPrm bufGetFullPrm;
  static CSL_RszBufPutEmptyPrm bufPutEmptyPrm;
  static CSL_BufInfo buf;
  static CSL_RszBufConfig bufConfig;
  static CSL_RszSetFlipPrm flipPrm;

  switch (cmd) {

  case CSL_RSZ_CMD_HW_SETUP:

    if (status == CSL_SOK)
      status = CSL_copyFromUser(&data, prm, sizeof(data));

    if (status == CSL_SOK) {
      if(data.inConfig) {
        status = CSL_copyFromUser(&inConfig, data.inConfig, sizeof(inConfig));
        
        data.inConfig = &inConfig;
      }
    }

    if (status == CSL_SOK) {
      if(data.inFrameConfig) {
        status = CSL_copyFromUser(&inFrameConfig, data.inFrameConfig, sizeof(inFrameConfig));
        
        data.inFrameConfig = &inFrameConfig;
      }
    }

    if (status == CSL_SOK) {
      if(data.clkConfig) {
        status = CSL_copyFromUser(&clkConfig, data.clkConfig, sizeof(clkConfig));
        
        data.clkConfig = &clkConfig;
      }
    }

    if (status == CSL_SOK) {
    
      if(data.minMaxConfig) {
        status = CSL_copyFromUser(&minMaxConfig, data.minMaxConfig, sizeof(minMaxConfig));
        
        data.minMaxConfig = &minMaxConfig;
      }
    }

    if (status == CSL_SOK) {
    
      if(data.rszAConfig) {
        status = CSL_copyFromUser(&rszAConfig, data.rszAConfig, sizeof(rszAConfig));
        
        data.rszAConfig = &rszAConfig;
      }
    }

    if (status == CSL_SOK) {
      if(data.rszBConfig) {
        status = CSL_copyFromUser(&rszBConfig, data.rszBConfig, sizeof(rszBConfig));
        
        data.rszBConfig = &rszBConfig;
      }
    }

    if (status == CSL_SOK)
      status = CSL_rszHwSetup(hndl, &data);

    break;

  case CSL_RSZ_CMD_HW_RESET:

    status = CSL_rszHwReset(hndl);
    break;

  case CSL_RSZ_CMD_SET_IN_CONFIG:

    status = CSL_copyFromUser(&inConfig, prm, sizeof(inConfig));

    if (status == CSL_SOK)
      status = CSL_rszSetInConfig(hndl, &inConfig);

    break;

  case CSL_RSZ_CMD_SET_IN_FRAME_CONFIG:

    status = CSL_copyFromUser(&inFrameConfig, prm, sizeof(inFrameConfig));

    if (status == CSL_SOK)
      status = CSL_rszSetInFrameConfig(hndl, &inFrameConfig);

    break;

  case CSL_RSZ_CMD_GET_IN_FRAME_CONFIG:

    status = CSL_rszGetInFrameConfig(hndl, &inFrameConfig);

    if (status == CSL_SOK)
      status = CSL_copyToUser(prm, &inFrameConfig, sizeof(inFrameConfig));

    break;

  case CSL_RSZ_CMD_SET_CLK_CONFIG:

    status = CSL_copyFromUser(&clkConfig, prm, sizeof(clkConfig));

    if (status == CSL_SOK)
      status = CSL_rszSetClkConfig(hndl, &clkConfig);

    break;

  case CSL_RSZ_CMD_SET_MINMAX_CONFIG:

    status = CSL_copyFromUser(&minMaxConfig, prm, sizeof(minMaxConfig));

    if (status == CSL_SOK)
      status = CSL_rszSetMinMaxConfig(hndl, &minMaxConfig);

    break;

  case CSL_RSZ_CMD_SET_OUT_CONFIG:

    status = CSL_copyFromUser(&setOutConfigPrm, prm, sizeof(setOutConfigPrm));

    if (status == CSL_SOK) 
      status = CSL_copyFromUser(&rszOutConfig, setOutConfigPrm.data, sizeof(rszOutConfig));

    if (status == CSL_SOK) {
      setOutConfigPrm.data = &rszOutConfig;
    }

    if (status == CSL_SOK)
      status = CSL_rszSetOutConfig(hndl, setOutConfigPrm.rszMod, setOutConfigPrm.data);

    break;

  case CSL_RSZ_CMD_ENABLE:

    status = CSL_rszEnable(hndl, (Bool32) prm);
    break;

  case CSL_RSZ_CMD_ONE_SHOT_ENABLE:

    status = CSL_rszOneShotEnable(hndl, (Bool32) prm);
    break;

  case CSL_RSZ_CMD_OUT_ENABLE:

    status = CSL_copyFromUser(&outEnablePrm, prm, sizeof(outEnablePrm));

    if (status == CSL_SOK)
      status = CSL_rszOutEnable(hndl, outEnablePrm.rszMod, outEnablePrm.enable);

    break;

  case CSL_RSZ_CMD_OUT_ONE_SHOT_ENABLE:

    status = CSL_copyFromUser(&outEnablePrm, prm, sizeof(outEnablePrm));

    if (status == CSL_SOK)
      status = CSL_rszOutOneShotEnable(hndl, outEnablePrm.rszMod, outEnablePrm.enable);

    break;

  case CSL_RSZ_CMD_SET_OUT_ADDR:

    status = CSL_copyFromUser(&setOutAddrPrm, prm, sizeof(setOutAddrPrm));

    if (status == CSL_SOK)
      status = CSL_rszSetOutAddr(hndl, setOutAddrPrm.rszMod, setOutAddrPrm.addr, setOutAddrPrm.yuv420CoutAddr);

    break;

  case CSL_RSZ_CMD_GET_STATUS:

    status = CSL_copyFromUser(&getStatusPrm, prm, sizeof(getStatusPrm));

    if (status == CSL_SOK)
      status = CSL_rszGetStatus(hndl, setOutAddrPrm.rszMod, &rszStatus);

    status = CSL_copyToUser(getStatusPrm.status, &rszStatus, sizeof(rszStatus));

    break;

  case CSL_RSZ_CMD_SET_OUT_FRAME_CONFIG:

    status = CSL_copyFromUser(&setOutFrameConfigPrm, prm, sizeof(setOutFrameConfigPrm));

    if (status == CSL_SOK)
      status = CSL_copyFromUser(&inFrameConfig, setOutFrameConfigPrm.inFrameConfig, sizeof(inFrameConfig));

    if (status == CSL_SOK)
      status = CSL_copyFromUser(&outFrameConfig, setOutFrameConfigPrm.outConfig, sizeof(outFrameConfig));

    if (status == CSL_SOK) {
      setOutFrameConfigPrm.inFrameConfig = &inFrameConfig;
      setOutFrameConfigPrm.outConfig = &outFrameConfig;
    }

    if (status == CSL_SOK)
      status = CSL_rszSetOutFrameConfig(hndl, setOutFrameConfigPrm.rszMod, setOutFrameConfigPrm.inFrameConfig, setOutFrameConfigPrm.outConfig);

    break;

  case CSL_RSZ_CMD_GET_OUT_FRAME_CONFIG:

    status = CSL_copyFromUser(&getOutFrameConfig, prm, sizeof(getOutFrameConfig));

    if (status == CSL_SOK)
      status = CSL_rszGetOutFrameConfig(hndl, getOutFrameConfig.rszMod, &outFrameConfig);

    status = CSL_copyToUser(getOutFrameConfig.outConfig, &outFrameConfig, sizeof(outFrameConfig));

    break;

  case CSL_RSZ_CMD_SET_DZOOM_CONFIG:

    status = CSL_copyFromUser(&setDzoomConfig, prm, sizeof(setDzoomConfig));

    if (status == CSL_SOK)
      status = CSL_rszSetDzoomConfig(hndl, &setDzoomConfig);

    break;

  case CSL_RSZ_CMD_BUF_INIT:

    if (status == CSL_SOK)
      status = CSL_copyFromUser(&bufInitPrm, prm, sizeof(bufInitPrm));

    if (status == CSL_SOK)
      status = CSL_copyFromUser(&bufInit, bufInitPrm.bufInit, sizeof(bufInit));

    if (status == CSL_SOK)
      status = CSL_copyFromUser(&bufConfig, bufInitPrm.bufConfig, sizeof(bufConfig));


    if (status == CSL_SOK) {
      bufInitPrm.bufInit = &bufInit;
      bufInitPrm.bufConfig = &bufConfig;
    }

    if (status == CSL_SOK)
      status = CSL_rszBufInit(hndl, bufInitPrm.rszMod, bufInitPrm.bufInit, bufInitPrm.bufConfig);

    break;

  case CSL_RSZ_CMD_BUF_SWITCH_ENABLE:

    status = CSL_copyFromUser(&bufSwitchEnablePrm, prm, sizeof(bufSwitchEnablePrm));

    if (status == CSL_SOK)
      status = CSL_rszBufSwitchEnable(hndl, bufSwitchEnablePrm.rszMod, bufSwitchEnablePrm.enable);

    break;

  case CSL_RSZ_CMD_BUF_GET_FULL:

    if (status == CSL_SOK)
      status = CSL_copyFromUser(&bufGetFullPrm, prm, sizeof(bufGetFullPrm));

    if (status == CSL_SOK)
      status = CSL_rszBufGetFull(hndl, bufGetFullPrm.rszMod, &buf, bufGetFullPrm.minBuf, bufGetFullPrm.timeout);

    status = CSL_copyToUser(bufGetFullPrm.buf, &buf, sizeof(buf));

    break;

  case CSL_RSZ_CMD_BUF_PUT_EMPTY:

    if (status == CSL_SOK)
      status = CSL_copyFromUser(&bufPutEmptyPrm, prm, sizeof(bufPutEmptyPrm));

    if (status == CSL_SOK)
      status = CSL_copyFromUser(&buf, bufPutEmptyPrm.buf, sizeof(buf));

    if (status == CSL_SOK) {
      bufPutEmptyPrm.buf = &buf;
    }

    if (status == CSL_SOK)
      status = CSL_rszBufPutEmpty(hndl, bufPutEmptyPrm.rszMod, bufPutEmptyPrm.buf);

    break;

  case CSL_RSZ_CMD_INT_ENABLE:

    status = CSL_rszIntEnable(hndl, (Bool32) prm);
    break;

  case CSL_RSZ_CMD_INT_CLEAR:

    status = CSL_rszIntClear(hndl);
    break;

  case CSL_RSZ_CMD_INT_WAIT:

    status = CSL_rszIntWait(hndl);
    break;

  case CSL_RSZ_CMD_SET_FLIP:

    if (status == CSL_SOK)
      status = CSL_copyFromUser(&flipPrm, prm, sizeof(flipPrm));

    if (status == CSL_SOK)
      status = CSL_rszSetFlip(hndl, flipPrm.rszMod, flipPrm.flipH, flipPrm.flipV);
  
    break;

  default:
    status = CSL_EFAIL;
    break;
  }

  return status;
}
