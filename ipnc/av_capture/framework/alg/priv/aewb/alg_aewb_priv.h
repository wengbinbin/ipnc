
#ifndef _ALG_AEWB_PRIV_H_
#define _ALG_AEWB_PRIV_H_

#include <iae.h>
#include <iawb.h>
#include <aewb_xdm.h>
#include <alg_aewb.h>
#include <alg_aewb_ctrl_priv.h>

typedef struct {

  IAE_InArgs    AE_InArgs;
  IAE_OutArgs   AE_OutArgs;
  IAWB_InArgs   AWB_InArgs;
  IAWB_OutArgs  AWB_OutArgs;

  IALG_Handle   handle_ae;
  IALG_Handle   handle_awb;

  IALG_MemRec   memTab_ae[4];
  IALG_MemRec   memTab_awb[4];

  unsigned char *weight;

  IAEWB_StatMat IAEWB_StatMatdata;

  int vsEnable;
  int vnfDemoCfg;
  int aewbType;
  int aewbVendor;
  int aewbPriority;
  int flgAWBCalcDataUpdate;

} ALG_AewbObj;

extern ALG_AewbObj gALG_aewbObj;

extern IAE_DynamicParams  IAE_DynamicParam;
extern IAWB_DynamicParams IAWB_DynamicParam;

short ALG_aewbSetSensorGain(int gain);
short ALG_aewbSetSensorExposure(int shutter);
short ALG_aewbSetIpipeWb(AWB_PARAM  *pAwb_Data );
short ALG_aewbSetIpipeWb2(AWB_PARAM  *pAwb_Data );
short ALG_aewbSetSensorDcsub(int dcsub);
short ALG_aewbSetSensorBin(int bin);
short ALG_aewbSetRgb2Rgb(RGB2RGB_PARAM  *pRgb2Rgb );
short ALG_aewbSetRgb2Rgb2(RGB2RGB_PARAM  *pRgb2Rgb );
short ALG_aewbSetOtfCorrect( int level );
short ALG_aewbSetEdgeEnhancement(EDGE_PARAM  *pParm );
short ALG_aewbSetContrastBrightness(BRT_CRT_PARAM  *pParm );
short ALG_aewbSetSensorBinSkip(int Is_binning);
short ALG_aewbSetSensor50_60Hz(int Is50Hz);
short ALG_aewbSetSensorFrameRate(int frame_rate_mode);

#endif /* _ALG_AEWB_PRIV_H_ */

