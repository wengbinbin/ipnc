

#include <mcvip_priv.h>

//#define TVP5158_DEBUG
#define TVP5158_FORCE_PATCH_DOWNLOAD
#define TVP5158_NO_PATCH_DOWNLOAD

static const Uint8 gTVP5158_patch[] = {
//  #include <tvp5158_patch_v1_00_81.h>
//	#include <tvp5158_patch_v1_00_82.h>
//	#include <tvp5158_patch_v2_00_02.h>
	#include <tvp5158_patch_v2_01_15.h>
};

Uint8 TVP5158_getStatus(DRV_I2cHndl *i2cHndl, Uint8 chNum)
{
  int status;
	Uint8 regAddr[2];
	Uint8 regVal[2];
  Uint32 k;

  if(chNum>=4)
    return 0;
    
  k=0;

  regAddr[k] = 0xFF;
  regVal[k]  = (1<<chNum);
  k++;

  status = DRV_i2cWrite8(i2cHndl, regAddr, regVal, k);
  if(status!=OSA_SOK)
    return 0;

  k=0;
  regAddr[k] = 0x0;
  regVal[k]  = 0;
  k++;

  status = DRV_i2cRead8(i2cHndl, regAddr, regVal, k);
  if(status!=OSA_SOK)
    return 0;

  return regVal[0];
}

Uint8 TVP5158_getStdStatus(DRV_I2cHndl *i2cHndl)
{
  int status;
	Uint8 regAddr[2];
	Uint8 regVal[2];
  Uint32 k;

  k=0;
  regAddr[k] = 0xC;
  regVal[k]  = 0;
  k++;

  status = DRV_i2cRead8(i2cHndl, regAddr, regVal, k);
  if(status!=OSA_SOK)
    return 0;

  return regVal[0];	
}

Uint8 TVP5158_getAVDoutputCtrlStatus01(DRV_I2cHndl * i2cHndl)
{
	int status;
	Uint8 regAddr[2];
	Uint8 regVal[2];
  Uint32 k;
  
  k=0;
  regAddr[k] = 0xB0;
  regVal[k]  = 0;
  k++;

  status = DRV_i2cRead8(i2cHndl, regAddr, regVal, k);
  if(status!=OSA_SOK)
    return 0;

  return regVal[0];	  
}

int TVP5158_selectWrite(DRV_I2cHndl *i2cHndl, Uint8 value)
{
	Uint8 regAddr[1];
	Uint8 regVal[1];
  Uint32 k;

  k=0;

  regAddr[k] = 0xFE;
  regVal[k]  = value;
  k++;

  return DRV_i2cWrite8(i2cHndl, regAddr, regVal, k);
}

int TVP5158_setStdStatus(DRV_I2cHndl *i2cHndl)
{
	int status;
	Uint8 regAddr[2];
	Uint8 regVal[2];
  Uint32 k;
  
  status = TVP5158_selectWrite(i2cHndl, 0xF);
  if(status!=OSA_SOK)
    goto error_exit;	
    
	k=0;
	regAddr[k] = 0xD;
	regVal[k] = 0x01;
	k++;
	    
  status = DRV_i2cWrite8(i2cHndl, regAddr, regVal, k);
  if(status!=OSA_SOK)
    goto error_exit;
    	    
error_exit:
  if(status!=OSA_SOK)
    OSA_ERROR("I2C");

  return status;    
}

int TVP5158_vbusWrite(DRV_I2cHndl *i2cHndl, Uint32 vbus_addr, Uint8 val, Uint8 len)
{
	int k;
	Uint8 regAddr[4];
	Uint8 regVal[4];

  k=0;

  regAddr[k] = 0xE8;
  regVal[k]  = (Uint8)((vbus_addr>>0) & 0xFF);
	k++;

  regAddr[k] = 0xE9;
  regVal[k]  = (Uint8)((vbus_addr>>8) & 0xFF);
	k++;

  regAddr[k] = 0xEA;
  regVal[k]  = (Uint8)((vbus_addr>>16) & 0xFF);
	k++;

  if(len) {
    regAddr[k] = 0xE0;
    regVal[k]  = val;
	  k++;
  }

  return DRV_i2cWrite8(i2cHndl, regAddr, regVal, k);
}

int TVP5158_reset(DRV_I2cHndl *i2cHndl)
{
	int status=OSA_SOK;
	
	int k;
	Uint8 regAddr[2];
	Uint8 regVal[2];

  OSA_waitMsecs(100); 

  status = TVP5158_selectWrite(i2cHndl, 0x1);
  if(status!=OSA_SOK)
    goto error_exit;
  
  k=0;
  
//  regAddr[k] = 0xB2;
//  regVal[k] = 0x20;
  regAddr[k] = 0xBA;
  regVal[k] = 0x01;
  k++;

  status = DRV_i2cWrite8(i2cHndl, regAddr, regVal, k);
  if(status!=OSA_SOK)
    goto error_exit;
    
  OSA_waitMsecs(100);  

  k=0;
  
//  regAddr[k] = 0xB2;
//  regVal[k] = 0x20;
  regAddr[k] = 0xBA;
  regVal[k] = 0x00;
  k++;

  status = DRV_i2cWrite8(i2cHndl, regAddr, regVal, k);
  if(status!=OSA_SOK)
    goto error_exit;
    
  OSA_waitMsecs(200); 
      
error_exit:
  if(status!=OSA_SOK)
    OSA_ERROR("I2C\n");
          
  return status;  
}

int TVP5158_checkChipId(DRV_I2cHndl *i2cHndl)
{
	int status=OSA_SOK, k;
	Uint8 regAddr[2];
	Uint8 regVal[2];
  
  k = 0;

  regAddr[k] = 0x08;
  regVal[k]  = 0;
	k++;

  regAddr[k] = 0x09;
  regVal[k]  = 0;
	k++;

  status = DRV_i2cRead8(i2cHndl, regAddr, regVal, k);
  if(status!=OSA_SOK)
    return status;

  #if 0
  if(regVal[0] != 0x51 || regVal[1] != 0x58 )
    return OSA_EFAIL;
  #endif

  return OSA_SOK;
}

int TVP5158_patchDownload(DRV_I2cHndl *i2cHndl)
{
	int status=OSA_SOK;
	Uint8 regAddr[I2C_TRANSFER_SIZE_MAX];
	Uint8 regVal[8];
  Uint8 vbusStatus;
  Uint32 k, wrSize;
  Uint8 *patchAddr;
  Uint32 patchSize;
  patchAddr = (Uint8*)gTVP5158_patch;
  patchSize = sizeof(gTVP5158_patch);

  status = TVP5158_selectWrite(i2cHndl, 0xF);
  if(status!=OSA_SOK)
    goto error_exit;

  k=0;

  regAddr[k] = 0xE8;
  regVal[k] = 0x60;
  k++;

  regAddr[k] = 0xE9;
  regVal[k] = 0x00;
  k++;

  regAddr[k] = 0xEA;
  regVal[k] = 0xB0;
  k++;

  DRV_i2cWrite8(i2cHndl, regAddr, regVal, k);

  k=0;

  regAddr[k] = 0xE0;
  regVal[k] = 0;
  k++;

  status = DRV_i2cRead8(i2cHndl, regAddr, regVal, k);
  if(status!=OSA_SOK)
    goto error_exit;

  vbusStatus = regVal[0];

  #ifdef TVP5158_NO_PATCH_DOWNLOAD
  {
    OSA_printf(" MCVIP: NO Patch downloaded, using ROM firmware.\n");
    return status;  // patch already running
  }
  #endif

  #ifndef TVP5158_FORCE_PATCH_DOWNLOAD
  if(vbusStatus & 0x2) {
    OSA_printf(" MCVIP: Patch is already running.\n");
    return status;  // patch already running
  }
  #endif
  
  status = TVP5158_selectWrite(i2cHndl, 0xF);
  if(status!=OSA_SOK)
    goto error_exit;

  vbusStatus |= 0x1;

  status = TVP5158_vbusWrite(i2cHndl, 0xB00060, vbusStatus, 1);
  if(status!=OSA_SOK)
    goto error_exit;

  status = TVP5158_vbusWrite(i2cHndl, 0x400000, 0, 0);
  if(status!=OSA_SOK)
    goto error_exit;

  memset(regAddr, 0xE1, sizeof(regAddr));

  while(patchSize) {

    if(patchSize < sizeof(regAddr))
      wrSize = patchSize;
    else
      wrSize = sizeof(regAddr);

    status = DRV_i2cWrite8(i2cHndl, regAddr, patchAddr, wrSize);
    if(status!=OSA_SOK)
      goto error_exit;

    patchAddr += wrSize;
    patchSize -= wrSize;
  }

  vbusStatus |= 0x3;

  status = TVP5158_vbusWrite(i2cHndl, 0xB00060, vbusStatus, 1);
  if(status!=OSA_SOK)
    goto error_exit;

  vbusStatus &= ~(0x1);

  status = TVP5158_vbusWrite(i2cHndl, 0xB00060, vbusStatus, 1);
  if(status!=OSA_SOK)
    goto error_exit;

  OSA_waitMsecs(100);

error_exit:
  if(status!=OSA_SOK)
    OSA_ERROR("I2C");

  return status;
}


int TVP5158_setBrightness(DRV_I2cHndl *i2cHndl, int brightness, int channel)
{
	int status=OSA_SOK;
	Uint8 regAddr[64];
	Uint8 regVal[64];
	Uint32 k;
	Uint8 chVal=1;
	
	if(brightness<0)
	  return 0;
	
	chVal = chVal << channel;
	
  status = TVP5158_selectWrite(i2cHndl, chVal);
  if(status!=OSA_SOK)
    goto error_exit;  
    
	k=0;      
  regAddr[k] = 0x10;
  regVal[k]  = (Uint8)brightness;
  k++;	
	
  #ifdef TVP5158_DEBUG
  OSA_printf(" TVP5158: brightness [Ch=%d value=%d]\n", chVal, brightness);
	#endif

  status = DRV_i2cWrite8(i2cHndl, regAddr, regVal, k);
  if(status!=OSA_SOK)
    goto error_exit;  
    
  OSA_waitMsecs(20); 

error_exit:
  if(status!=OSA_SOK)
    OSA_ERROR("I2C-Brightness");

  return status;    	
}

int TVP5158_setContrast(DRV_I2cHndl *i2cHndl, int contrast, int channel)
{
	int status=OSA_SOK;
	Uint8 regAddr[64];
	Uint8 regVal[64];
	Uint32 k;
	Uint8 chVal=1;
	
	if(contrast<0)
	  return 0;
	
	chVal = chVal << channel;
	
  status = TVP5158_selectWrite(i2cHndl, chVal);
  if(status!=OSA_SOK)
    goto error_exit;  

  k=0;

  regAddr[k] = 0x11;
  regVal[k]  = (Uint8)contrast;
  k++;	

  #ifdef TVP5158_DEBUG
  OSA_printf(" TVP5158: contrast [Ch=%d value=%d]\n", chVal, contrast);
	#endif
	
  status = DRV_i2cWrite8(i2cHndl, regAddr, regVal, k);
  if(status!=OSA_SOK)
    goto error_exit;  
    
  OSA_waitMsecs(20); 

error_exit:
  if(status!=OSA_SOK)
    OSA_ERROR("I2C-Contrast");

  return status;    	
}	

int TVP5158_setSaturation(DRV_I2cHndl *i2cHndl, int saturation, int channel)
{
	int status=OSA_SOK;
	Uint8 regAddr[64];
	Uint8 regVal[64];
	Uint32 k;
	Uint8 chVal=1;
	
	if(saturation<0)
	  return 0;
	
	chVal = chVal << channel;
	
  k=0;

  regAddr[k] = 0xFE;
  regVal[k]  = chVal;
  k++;	
  
  status = DRV_i2cWrite8(i2cHndl, regAddr, regVal, k);
  if(status!=OSA_SOK)
    goto error_exit;  	

  k=0;

  regAddr[k] = 0x13;
  regVal[k]  = (Uint8)saturation;
  k++;	

  #ifdef TVP5158_DEBUG
  OSA_printf(" TVP5158: saturation [Ch=%d value=%d]\n", chVal, saturation);
	#endif

	
  status = DRV_i2cWrite8(i2cHndl, regAddr, regVal, k);
  if(status!=OSA_SOK)
    goto error_exit;  
    
  OSA_waitMsecs(20); 

error_exit:
  if(status!=OSA_SOK)
    OSA_ERROR("I2C-Saturation");

  return status;    	
}

int TVP5158_setHue(DRV_I2cHndl *i2cHndl, int hue, int channel)
{
	int status=OSA_SOK;
	Uint8 regAddr[64];
	Uint8 regVal[64];
	Uint32 k;
	Uint8 chVal=1;

	chVal = chVal << channel;
	
  status = TVP5158_selectWrite(i2cHndl, chVal);
  if(status!=OSA_SOK)
    goto error_exit;  
    
  k=0;

  regAddr[k] = 0x14;
  regVal[k]  = (Uint8)hue;
  k++;	

  #ifdef TVP5158_DEBUG
  OSA_printf(" TVP5158: hue [Ch=%d value=%d]\n", chVal, hue);
	#endif

  status = DRV_i2cWrite8(i2cHndl, regAddr, regVal, k);
  if(status!=OSA_SOK)
    goto error_exit;  
    
  OSA_waitMsecs(20); 

error_exit:
  if(status!=OSA_SOK)
    OSA_ERROR("I2C-Hue");

  return status;    	
}

int TVP5158_setHybridChSelect(DRV_I2cHndl *i2cHndl, int channel)
{
	int status=OSA_SOK;
	Uint8 regAddr[64];
	Uint8 regVal[64];
	Uint32 k;
	
	status = TVP5158_selectWrite(i2cHndl, 0xF);
	if(status!=OSA_SOK)
		goto error_exit;  	
		
	k=0;
	
	regAddr[k] = 0xB5;
	regVal[k]  = (Uint8)channel;
	k++;			
	
	#ifdef TVP5158_DEBUG
	OSA_printf(" TVP5158: HybridChannel [Ch=%d]\n", channel);
	#endif

	status = DRV_i2cWrite8(i2cHndl, regAddr, regVal, k);
	if(status!=OSA_SOK)
		goto error_exit;  
    
	OSA_waitMsecs(20); 

error_exit:
	if(status!=OSA_SOK)
		OSA_ERROR("I2C-HybirdChannel");

	return status;    		
}


int TVP5158_setNoiseReduction(DRV_I2cHndl *i2cHndl, int enable, int channel)
{
	int status=OSA_SOK;
	Uint8 regAddr[2];
	Uint8 regVal[2];
	Uint32 k;
	Uint8 chVal=1;	

	chVal = chVal << channel;

	status = TVP5158_selectWrite(i2cHndl, chVal);

	if(status!=OSA_SOK)
    	goto error_exit;  
    
	k=0;	
	regAddr[k] = 0x5D;
	regVal[k]  = (Uint8)enable;
	k++;

  OSA_printf(" TVP5158: NoiseReduction [Ch=%d value=%x]\n", chVal, enable);

 	status = DRV_i2cWrite8(i2cHndl, regAddr, regVal, k);
 	if(status!=OSA_SOK)
   	goto error_exit;	

	OSA_waitMsecs(20); 

error_exit:
 	if(status!=OSA_SOK)
   	OSA_ERROR("I2C-NoiseReduction");	

 	return status;
}


int	TVP5158_setColorAdjust(DRV_I2cHndl *i2cHndl, int brightness, int contrast, int saturation, int hue, int enableNR, int channel)
{
	//return OSA_SOK;//add by sxh
	int status=OSA_SOK;
	Uint8 regAddr[6];
	Uint8 regVal[6];
	Uint32 k;
	Uint8 reg10, reg11, reg13, reg14, regNR, chVal=1;

	if(saturation<0)
	  return 0;

	if(brightness<0)
	  return 0;

	if(contrast<0)
	  return 0;

	chVal = chVal << channel;

	reg10 = (Uint8)brightness;
	reg11 = (Uint8)contrast;
	reg13 = (Uint8)saturation;
	reg14 = (Uint8)hue;
	regNR = (Uint8)enableNR;
	
  
  status = TVP5158_selectWrite(i2cHndl, chVal);
  if(status!=OSA_SOK)
    goto error_exit;  
      
	k=0;      
	
  regAddr[k] = 0x10;
  regVal[k]  = reg10;
  k++;
  
  regAddr[k] = 0x11;
  regVal[k]  = reg11;
  k++;  
  
  regAddr[k] = 0x13;
  regVal[k]  = reg13;
  k++;    
  
  regAddr[k] = 0x14;
  regVal[k]  = reg14;  
  k++;

  regAddr[k] = 0x5D;
  regVal[k] = regNR;
  k++;
  
  status = DRV_i2cWrite8(i2cHndl, regAddr, regVal, k);
  if(status!=OSA_SOK)
    goto error_exit;  
    
  OSA_waitMsecs(20); 

error_exit:
  if(status!=OSA_SOK)
    OSA_ERROR("I2C");

  return status;
}

//static int gTVP5158_firstInitDone[2] = {FALSE,FALSE};
int TVP5158_setOfmMode(DRV_I2cHndl *i2cHndl, int mode, int videoIfMode, int videoSystem, int cascadeStage)
{
	int status=OSA_SOK;
	Uint8 regAddr[64];
	Uint8 regVal[64];
  Uint32 k;
  Uint8 B0, B1, B2, B3, B4, B5, B6, B7, B8, B9;

  status = TVP5158_checkChipId(i2cHndl);
  if(status!=OSA_SOK) {
    OSA_ERROR("TVP5158_checkChipId()\n");
    return status;
  }
  
  status = TVP5158_reset(i2cHndl);
  if(status!=OSA_SOK) {
    OSA_ERROR("TVP5158_reset()\n");  
    return status;
  }

#ifndef TVP5158_NO_PATCH_DOWNLOAD
  status = TVP5158_patchDownload(i2cHndl);
  if(status!=OSA_SOK) {
    OSA_ERROR("TVP5158_patchDownload()\n");
    return status;
  }
#endif

  B1 = 0x10;
//  B2 = 0x05;
//	B2 = 0x35;
	B2 = 0x25;
  B3 = 0xE4;
  B4 = 0xE4;
  B5 = 0x00;
  B6 = 0x1B;

  if(videoSystem==MCVIP_VIDEO_SYSTEM_PAL)
    B7 = 0x14;
  else
    B7 = 0x04;

  B8 = 0x40;
  B9 = 0x00;

  switch(mode) {

    case MCVIP_VIDEO_DECODER_MODE_2CH_D1_PORT_A:
    case MCVIP_VIDEO_DECODER_MODE_2CH_D1_PORT_B:
    case MCVIP_VIDEO_DECODER_MODE_2CH_D1_2CH_CIF:	//sk_cif
      B0 = 0x90;
      break;

    case MCVIP_VIDEO_DECODER_MODE_4CH_D1:
   	case MCVIP_VIDEO_DECODER_MODE_4CH_D1_4CH_CIF_A:	//sk_cif
	case MCVIP_VIDEO_DECODER_MODE_4CH_D1_4CH_CIF_B:    	    	
//    	B2 = 0x35;
      if(videoIfMode==MCVIP_VIDEO_IF_MODE_BT656)
        B0 = 0xA0;
      else
        B0 = 0xA8;
      break;

    case MCVIP_VIDEO_DECODER_MODE_4CH_HALF_D1:
      if(videoIfMode==MCVIP_VIDEO_IF_MODE_BT656)
        B0 = 0xA2;
      else
        B0 = 0xAA;
      break;

    case MCVIP_VIDEO_DECODER_MODE_4CH_CIF:
      B0 = 0xA3;
      break;

    case MCVIP_VIDEO_DECODER_MODE_8CH_HALF_D1:
      if(cascadeStage==0)
        B0 = 0xB2;
      else
        B0 = 0xB6;
      break;

    case MCVIP_VIDEO_DECODER_MODE_8CH_CIF:
      if(cascadeStage==0)
        B0 = 0xB3;
      else
        B0 = 0xB7;
      break;

    case MCVIP_VIDEO_DECODER_MODE_4CH_HALF_D1_PLUS_D1:
      B0 = 0xE2;
      B8 = 0x60;
      B9 = 0x10;
      break;

    case MCVIP_VIDEO_DECODER_MODE_4CH_CIF_PLUS_D1:
      B0 = 0xE3;
      break;

    case MCVIP_VIDEO_DECODER_MODE_8CH_CIF_PLUS_D1:
      if(cascadeStage==0)
        B0 = 0xF3;
      else
        B0 = 0xF7;
      break;

    default:
      OSA_ERROR("Unsupported mode %d\n", mode);
      return OSA_EFAIL;
  }

  status = TVP5158_selectWrite(i2cHndl, 0xF);
  if(status!=OSA_SOK)
    goto error_exit;

  k=0;
/*
  if((mode == MCVIP_VIDEO_DECODER_MODE_8CH_HALF_D1) || 
    (mode == MCVIP_VIDEO_DECODER_MODE_8CH_CIF)  ||
    (mode == MCVIP_VIDEO_DECODER_MODE_8CH_CIF_PLUS_D1) )
    {
          if(gTVP5158_firstInitDone[1] == FALSE)
          {
                  regAddr[k] = 0xB2;
                  regVal[k]  = 0x05;
                  k++;
                  gTVP5158_firstInitDone[1] = TRUE;
          }
          else
               if(gTVP5158_firstInitDone[0] == FALSE)
              {
                      regAddr[k] = 0xB2;
                      regVal[k]  = B2;
                      k++;
                      gTVP5158_firstInitDone[0] = TRUE;
              }
    }
  else
    {
                   if(gTVP5158_firstInitDone[0] == FALSE)
              {
                      regAddr[k] = 0xB2;
                      regVal[k]  = B2;
                      k++;
                      gTVP5158_firstInitDone[0] = TRUE;
              }
    }
*/
  regAddr[k] = 0xB2;
  regVal[k]  = B2;
  k++;
                      
  regAddr[k] = 0xB0;
  regVal[k]  = B0;
  k++;

//if(videoSystem==MCVIP_VIDEO_SYSTEM_PAL)
//{
    regAddr[k] = 0xB7;
    regVal[k]  = B7;
    k++;
//}
  OSA_waitMsecs(100); 
  status = DRV_i2cWrite8(i2cHndl, regAddr, regVal, k);
  if(status!=OSA_SOK)
    goto error_exit;
  OSA_waitMsecs(100); 

  OSA_waitMsecs(20); 

error_exit:
  if(status!=OSA_SOK)
    OSA_ERROR("I2C");

  return status;
}

int TVP5158_registerInit(DRV_I2cHndl *i2cHndl)
{
	int status=OSA_SOK;
	Uint8 regAddr[64];
	Uint8 regVal[64];
  Uint32 k;
  Uint8 B0, B1, B2, B3, B4, B5, B6, B7, B8, B9;

  status = TVP5158_checkChipId(i2cHndl);
  if(status!=OSA_SOK) {
    OSA_ERROR("TVP5158_checkChipId()\n");
    return status;
  }
  
  status = TVP5158_reset(i2cHndl);
  if(status!=OSA_SOK) {
    OSA_ERROR("TVP5158_reset()\n");  
    return status;
  }

#ifdef TVP5158_NO_PATCH_DOWNLOAD
  status = TVP5158_patchDownload(i2cHndl);
  if(status!=OSA_SOK) {
    OSA_ERROR("TVP5158_patchDownload()\n");
    return status;
  }
#endif

  B0 = 0x90;
  B1 = 0x10;
//  B2 = 0x05;
	B2 = 0x25;
  B3 = 0xE4;
  B4 = 0xE4;
  B5 = 0x00;
  B6 = 0x1B;

  B7 = 0x04;

  B8 = 0x40;
  B9 = 0x00;

  status = TVP5158_selectWrite(i2cHndl, 0xF);
  if(status!=OSA_SOK)
    goto error_exit;

  k=0;

	regAddr[k] = 0x5D; 
  	regVal[k]  = 0x08; //Noise filter enable
  	k++;
  regAddr[k] = 0xB0;
  regVal[k]  = B0;
  k++;

  regAddr[k] = 0xB1;
  regVal[k]  = B1;
  k++;
  OSA_waitMsecs(100); 
  status = DRV_i2cWrite8(i2cHndl, regAddr, regVal, k);
  if(status!=OSA_SOK)
    goto error_exit;
  OSA_waitMsecs(100); 
  status = TVP5158_selectWrite(i2cHndl, 0x1);
  if(status!=OSA_SOK)
    goto error_exit;

  k=0;

  regAddr[k] = 0xB2;
  regVal[k]  = B2;
  k++;

  regAddr[k] = 0xB3;
  regVal[k]  = B3;
  k++;

  regAddr[k] = 0xB4;
  regVal[k]  = B4;
  k++;

  regAddr[k] = 0xB5;
  regVal[k]  = B5;
  k++;

  regAddr[k] = 0xB6;
  regVal[k]  = B6;
  k++;

  regAddr[k] = 0xB7;
  regVal[k]  = B7;
  k++;

  regAddr[k] = 0xC3;
  regVal[k]  = 0xE0;
  k++;

  status = DRV_i2cWrite8(i2cHndl, regAddr, regVal, k);
  if(status!=OSA_SOK)
    goto error_exit;
 
  OSA_waitMsecs(20); 

error_exit:
  if(status!=OSA_SOK)
    OSA_ERROR("I2C");

  return status;
}

#if 0
int TVP5158_softReset(DRV_I2cHndl *i2cHndl)
  {
  	int status=OSA_SOK;
	Uint8 regAddr[64];
	Uint8 regVal[64];
  Uint32 k;
  Uint8 vbusStatus;  

    status = TVP5158_selectWrite(i2cHndl, 0xF);
  if(status!=OSA_SOK)
    goto error_exit;

  k=0;

  vbusStatus = 0x1;

  status = TVP5158_vbusWrite(i2cHndl, 0xB00060, vbusStatus, 1);
  if(status!=OSA_SOK)
    goto error_exit;

  vbusStatus = 0x0;

  status = TVP5158_vbusWrite(i2cHndl, 0xB00060, vbusStatus, 1);
  if(status!=OSA_SOK)
    goto error_exit;

  OSA_waitMsecs(300);
  
error_exit:
  if(status!=OSA_SOK)
    OSA_ERROR("I2C");

  return status;  
}
#endif

int TVP5158_softReset(DRV_I2cHndl *i2cHndl) {
 	int status=OSA_SOK;
	Uint8 regAddr[64];
	Uint8 regVal[64];
  Uint32 k;
  Uint8 vbusStatus;  

  status = TVP5158_selectWrite(i2cHndl, 0xF);
  if(status!=OSA_SOK)
    goto error_exit;

  k=0;

  regAddr[k] = 0xE8;
  regVal[k] = 0x60;
  k++;

  regAddr[k] = 0xE9;
  regVal[k] = 0x00;
  k++;

  regAddr[k] = 0xEA;
  regVal[k] = 0xB0;
  k++;

  DRV_i2cWrite8(i2cHndl, regAddr, regVal, k);

  k=0;

  regAddr[k] = 0xE0;
  regVal[k] = 0;
  k++;

  status = DRV_i2cRead8(i2cHndl, regAddr, regVal, k);
  if(status!=OSA_SOK)
    goto error_exit;

  vbusStatus = regVal[0] | 1;

  status = TVP5158_vbusWrite(i2cHndl, 0xB00060, vbusStatus, 1);
  if(status!=OSA_SOK)
    goto error_exit;

  vbusStatus &= ~(0x1);

  status = TVP5158_vbusWrite(i2cHndl, 0xB00060, vbusStatus, 1);
  if(status!=OSA_SOK)
    goto error_exit;

  OSA_waitMsecs(300);

/*
  // For Audio mute
  k = 0;
  regAddr[k] = 0xC5;
  regVal[k]  = 0x0F;
  k++;

  status = DRV_i2cWrite8(i2cHndl, regAddr, regVal, k);

  if(status != OSA_SOK)
  {
    goto error_exit;
  }
  OSA_waitMsecs(20);  
*/

error_exit:
  if(status!=OSA_SOK)
    OSA_ERROR("I2C");

  return status;
}






// AYK - 0731 start
int TVP5158_setAudioMode(DRV_I2cHndl *i2cHndl, int mode, int cascadeStage,int sampleRate)
{
  int status=OSA_SOK;
  Uint8 regAddr[32];
  Uint8 regVal[32];
  Uint32 k;
  Uint8 C0, C1, C2, C3, C4, C5, C6, C7, C8;

  if (cascadeStage >=2 )
  {
  	status = TVP5158_patchDownload(i2cHndl);
  	if(status!=OSA_SOK)
  	{
	  OSA_ERROR("TVP5158_patchDownload()\n");
      return status;
  	}
  }

  switch(sampleRate)
  {
      case 8000:
	      C0 = 0x02;
		  break;
      case 11250:
	      C0 = 0x03;
		  break;
      case 16000:
	      C0 = 0x00;
		  break;
      case 22500:
	      C0 = 0x01;
		  break;
  }

  C1 = 0x88; // Gain Ch1 and Ch2
  C2 = 0x88; // Gain Ch3 and Ch4
  C4 = 0x02; // Mixer select, TDM_Chan_Number[2:0] 2: 8 channels
  C5 = 0x00; // Audio Mute Control
  C6 = 0x00; // Analog Mixing Ratio Control 1
  C7 = 0x00; // Analog Mixing Ratio Control 2

  switch (cascadeStage)
  {
  	case 0:
      C3 = 0x70; // SD_M disabled, SD_R enabled, TVP5158 Master, DSP mode, 256-fs, 16-bit PCM, SD_R only
  	  C8 = 0x00; // First stage channels 1-4
  	  break;
  	case 1:
  	  C3 = 0x20; // SD_M disabled, SD_R disabled, Master, I2S mode
  	  C8 = 0x01; // Second stage channels 5-8
  	  break;
  	case 2:
  	  C3 = 0x20; // SD_M disabled, SD_R disabled, Master, I2S mode
  	  C8 = 0x02; // Third stage channels 9-12
  	  break;
  	case 3:
  	  C3 = 0x20; // SD_M disabled, SD_R disabled, Master, I2S mode
  	  C8 = 0x03; // Fourth stage channels 13-16
  	  break;
  	default:
      status = -1;
  	  goto error_exit;
  	  break;
  }

  // Audio channel Mute control

  C5 = 0x00;

 /* if((mode == MCVIP_VIDEO_DECODER_MODE_2CH_D1_PORT_A) ||
      (mode == MCVIP_VIDEO_DECODER_MODE_2CH_D1_PORT_B))
  {
      C5 = 0x0C;
   }*/

  k=0;
  regAddr[k] = 0xC0;
  regVal[k]  = C0;
  k++;

  regAddr[k] = 0xC1;
  regVal[k]  = C1;
  k++;

  regAddr[k] = 0xC2;
  regVal[k]  = C2;
  k++;

  regAddr[k] = 0xC3;
  regVal[k]  = C3;
  k++;

  regAddr[k] = 0xC4;
  regVal[k]  = C4;
  k++;

  regAddr[k] = 0xC5;
  regVal[k]  = C5;
  k++;

  regAddr[k] = 0xC6;
  regVal[k]  = C6;
  k++;

  regAddr[k] = 0xC7;
  regVal[k]  = C7;
  k++;

  regAddr[k] = 0xC8;
  regVal[k]  = C8;
  k++;

  status = DRV_i2cWrite8(i2cHndl, regAddr, regVal, k);

  if(status != OSA_SOK)
  {
    goto error_exit;
  }

  OSA_waitMsecs(20);  

  // read C5 reg
  regAddr[0] = 0xC5;

  status = DRV_i2cRead8(i2cHndl, regAddr, regVal, 1);

  if(status != OSA_SOK)
  {
      goto error_exit;
   }

  OSA_waitMsecs(20);  

//  OSA_printf(" MCVIP: C5 = %X\n",regVal[0]);

error_exit:
  if(status!=OSA_SOK)
  {
    OSA_ERROR("I2C Audio setting cascadeStage=%d",cascadeStage);
  }

  return status;
}
// AYK - 0731 end

int TVP5158_setAudVol(DRV_I2cHndl *i2cHndl, Uint8 vol, Uint8 channel)
{
    int status = OSA_SOK;
	Uint8 regAddr[64];
	Uint8 regVal[64];
	Uint8 shiftVal,andVal, chVal=1;

//	chVal = chVal << channel;
//  status = TVP5158_selectWrite(i2cHndl, chVal);
	status = TVP5158_selectWrite(i2cHndl, 1);
  if(status!=OSA_SOK)
    goto error_exit;  	
	
	if((vol < 0x0) || (vol > 0xF))
		vol = 0x8; // Default value(0 db)
	
    switch(channel)
    {
	    case 0:
	        shiftVal = 0;
	        andVal   = 0xF0;
	        regAddr[0] = 0xC1;
	        break;

	    case 1:
	        shiftVal = 4;
	        andVal   = 0x0F;
	        regAddr[0] = 0xC1;
	        break;

	    case 2:
	        shiftVal = 0;
	        andVal   = 0xF0;
	        regAddr[0] = 0xC2;
	        break;

	    case 3:
	        shiftVal = 4;
	        andVal   = 0x0F;
	        regAddr[0] = 0xC2;
	        break;
	}

	// read the aud vol register
	status = DRV_i2cRead8(i2cHndl, regAddr, regVal, 1);
	if(status != OSA_SOK)
		goto error_exit;


	regVal[0] &= andVal;
	regVal[0] |= (vol << shiftVal);

	status = DRV_i2cWrite8(i2cHndl, regAddr, regVal, 1);
	if(status!=OSA_SOK)
		goto error_exit;

error_exit:

    if(status != OSA_SOK)
    {
        OSA_ERROR("I2C-Audio Volume");
    }

  return status;
}

int TVP5158_setSampleRate(DRV_I2cHndl *i2cHndl,int sampleRate)
{
	int status = OSA_SOK;
	Uint8 regAddr[64];
	Uint8 regVal[64];
	Uint8 readRegVal[64];
	Uint8 smplRate;

  status = TVP5158_selectWrite(i2cHndl, 0xF);  
  if(status!=OSA_SOK)
    goto error_exit;  
    
 
	switch(sampleRate)
  {
    default:
    case 16000:
        smplRate = 0;
        break;
    case 22050:
        smplRate = 1;
        break;
    case 8000:
        smplRate = 2;
        break;
    case 11025:
        smplRate = 3;
        break;
	}
	
	regAddr[0] = 0xC0;

// Removing read and then write to the register as the bits 2-7 are not used - Reserved bits

#if 0
	status = DRV_i2cRead8(i2cHndl, regAddr, regVal, 1);
         if(status != OSA_SOK)
		goto error_exit;

	regVal[0] &= 0xFC;
	regVal[0] |= smplRate;
#endif

	    regVal[0] = smplRate;

		//OSA_waitMsecs(10);  
  
        status = DRV_i2cWrite8(i2cHndl, regAddr, regVal, 1);
	 if(status!=OSA_SOK)
		goto error_exit;
        OSA_waitMsecs(20);  
        

error_exit:

    if(status != OSA_SOK)
    {
        OSA_ERROR("I2C-Audio sample rate");
    }

    return status;

}

int TVP5158_setAudMute(DRV_I2cHndl *i2cHndl, int enableMute, Uint8 channel)
{
	int status = OSA_SOK;
	Uint8 regAddr[64];
	Uint8 regVal[64];
	Uint8	setMute = 0;
	
  setMute	= enableMute << channel;
  OSA_printf(" @@@@@@@@@@@@TVP5158: setMute = %X\n",setMute);
  
	regAddr[0] = 0xC5;
	status = DRV_i2cRead8(i2cHndl, regAddr, regVal, 1);
  if(status != OSA_SOK)
		goto error_exit;

	regVal[0] |= setMute;
	
	if(enableMute > 0)
		setMute = 0xF;
	else
		setMute = 0;
		
	regVal[0] = setMute;

	status = TVP5158_selectWrite(i2cHndl, 0xF);
  if(status!=OSA_SOK)
    goto error_exit;  
    	
  status = DRV_i2cWrite8(i2cHndl, regAddr, regVal, 1);
	if(status!=OSA_SOK)
		goto error_exit;
		
	OSA_printf(" @@@@@@@@@@@@TVP5158: C5 = %X\n",regVal[0]);		
	
error_exit:
	if(status != OSA_SOK)
		OSA_ERROR("I2C-Audio MUTE");
    		
	return status;
}
