#include <csl_ipipeifIoctl.h>

CSL_Status CSL_ipipeifHwControl(CSL_IpipeifHandle hndl, Uint32 cmd, void *prm)
{
  CSL_Status status = CSL_SOK;

  static CSL_IpipeifHwSetup data;
  static CSL_IpipeifSdramInConfig sdramInConfig;
  static CSL_IpipeifInSource2Config inSource2Config;
  static CSL_IpipeifVpifIsifInConfig vpifIsifInConfig;
  static CSL_IpipeifDarkFrameConfig darkFrameConfig;
  static CSL_IpipeifH3aConfig h3aConfig;

  static CSL_IpipeifDpcmConfig dpcmConfig;
  static CSL_IpipeifClkConfig clkConfig;
  static CSL_IpipeifSdramInFrameConfig sdramInFrameConfig;

  Bool32    underflowDetect;

  switch (cmd) {

  case CSL_IPIPEIF_CMD_HW_SETUP:

    if (status == CSL_SOK)
      status = CSL_copyFromUser(&data, prm, sizeof(data));

    if (status == CSL_SOK) {
      if(data.sdramInConfig) {
        status = CSL_copyFromUser(&sdramInConfig, data.sdramInConfig, sizeof(sdramInConfig));
        
        data.sdramInConfig = &sdramInConfig;
        
        if(sdramInConfig.dpcmConfig) {
          status = CSL_copyFromUser(&dpcmConfig, sdramInConfig.dpcmConfig, sizeof(dpcmConfig));
          
          sdramInConfig.dpcmConfig = &dpcmConfig;
        }
        
        if(sdramInConfig.clkConfig) {
          status = CSL_copyFromUser(&clkConfig, sdramInConfig.clkConfig, sizeof(clkConfig));
          
          sdramInConfig.clkConfig = &clkConfig;
        }
      }
    }

    if (status == CSL_SOK) {
      if(data.inSource2Config) {
        status = CSL_copyFromUser(&inSource2Config, data.inSource2Config, sizeof(inSource2Config));
        
        data.inSource2Config = &inSource2Config;
      }
    }

    if (status == CSL_SOK) {
      if(data.vpifIsifInConfig) {
        status = CSL_copyFromUser(&vpifIsifInConfig, data.vpifIsifInConfig, sizeof(vpifIsifInConfig));
        
        data.vpifIsifInConfig = &vpifIsifInConfig;
      }
    }

    if (status == CSL_SOK) {
      if(data.darkFrameConfig) {
        status = CSL_copyFromUser(&darkFrameConfig, data.darkFrameConfig, sizeof(darkFrameConfig));
        
        data.darkFrameConfig = &darkFrameConfig;
      }
    }

    if (status == CSL_SOK) {
      if(data.h3aConfig) {
        status = CSL_copyFromUser(&h3aConfig, data.h3aConfig, sizeof(h3aConfig));
        
        data.h3aConfig = &h3aConfig;
      }
    }

    if (status == CSL_SOK)
      status = CSL_ipipeifHwSetup(hndl, &data);

    break;

  case CSL_IPIPEIF_CMD_HW_RESET:

    status = CSL_ipipeifHwReset(hndl);
    break;

  case CSL_IPIPEIF_CMD_SET_SDRAM_IN_CONFIG:

    status = CSL_copyFromUser(&sdramInConfig, prm, sizeof(sdramInConfig));

    if (status == CSL_SOK) {
      if(sdramInConfig.dpcmConfig) {
        status = CSL_copyFromUser(&dpcmConfig, sdramInConfig.dpcmConfig, sizeof(dpcmConfig));
        
        sdramInConfig.dpcmConfig = &dpcmConfig;
      }
    }

    if (status == CSL_SOK) {
      if(sdramInConfig.clkConfig) {
        status = CSL_copyFromUser(&clkConfig, sdramInConfig.clkConfig, sizeof(clkConfig));
        
        sdramInConfig.clkConfig = &clkConfig;
      }
    }

    if (status == CSL_SOK)
      status = CSL_ipipeifSetSdramInConfig(hndl, &sdramInConfig);

    break;

  case CSL_IPIPEIF_CMD_SET_SDRAM_IN_FRAME_CONFIG:

    status = CSL_copyFromUser(&sdramInFrameConfig, prm, sizeof(sdramInFrameConfig));

    if (status == CSL_SOK)
      status = CSL_ipipeifSetSdramInFrameConfig(hndl, &sdramInFrameConfig);

    break;

  case CSL_IPIPEIF_CMD_SET_IN_SOURCE_2_CONFIG:

    status = CSL_copyFromUser(&inSource2Config, prm, sizeof(inSource2Config));

    if (status == CSL_SOK)
      status = CSL_ipipeifSetInSource2Config(hndl, &inSource2Config);

    break;

  case CSL_IPIPEIF_CMD_SET_VPIF_ISIF_IN_CONFIG:

    status = CSL_copyFromUser(&vpifIsifInConfig, prm, sizeof(vpifIsifInConfig));

    if (status == CSL_SOK)
      status = CSL_ipipeifSetVpifIsifInConfig(hndl, &vpifIsifInConfig);

    break;

  case CSL_IPIPEIF_CMD_SET_DARK_FRAME_CONFIG:

    status = CSL_copyFromUser(&darkFrameConfig, prm, sizeof(darkFrameConfig));

    if (status == CSL_SOK)
      status = CSL_ipipeifSetDarkFrameConfig(hndl, &darkFrameConfig);

    break;

  case CSL_IPIPEIF_CMD_SET_H3A_CONFIG:

    status = CSL_copyFromUser(&h3aConfig, prm, sizeof(h3aConfig));

    if (status == CSL_SOK)
      status = CSL_ipipeifSetH3aConfig(hndl, &h3aConfig);

    break;

  case CSL_IPIPEIF_CMD_SET_CLK_CONFIG:

    status = CSL_copyFromUser(&clkConfig, prm, sizeof(clkConfig));

    if (status == CSL_SOK)
      status = CSL_ipipeifSetClkConfig(hndl, &clkConfig);

    break;

  case CSL_IPIPEIF_CMD_SET_DPCM_CONFIG:

    status = CSL_copyFromUser(&dpcmConfig, prm, sizeof(dpcmConfig));

    if (status == CSL_SOK)
      status = CSL_ipipeifSetDpcmConfig(hndl, &dpcmConfig);

    break;

  case CSL_IPIPEIF_CMD_SET_INPUT_SOURCE_1:

    status = CSL_ipipeifSetInputSource1(hndl, (Uint32) prm);
    break;

  case CSL_IPIPEIF_CMD_SET_SDRAM_IN_ADDR:

    status = CSL_ipipeifSetSdramInAddr(hndl, (Uint8 *) prm);
    break;

  case CSL_IPIPEIF_CMD_SDRAM_IN_ENABLE:

    status = CSL_ipipeifSdramInEnable(hndl, (Bool32) prm);
    break;

  case CSL_IPIPEIF_CMD_SDRAM_IN_ONE_SHOT_ENABLE:

    status = CSL_ipipeifSdramInOneShotEnable(hndl, (Bool32) prm);
    break;

  case CSL_IPIPEIF_CMD_UNDERFLOW_DETECT_CLEAR:

    status = CSL_ipipeifUnderflowDetectClear(hndl);
    break;

  case CSL_IPIPEIF_CMD_IS_UNDERFLOW_DETECT:

    status = CSL_ipipeifIsUnderflowDetect(hndl, &underflowDetect);

    if (status == CSL_SOK)
      status = CSL_copyToUser(prm, &underflowDetect, sizeof(underflowDetect));

    break;

  default:
    status = CSL_EFAIL;
    break;
  }

  return status;
}
