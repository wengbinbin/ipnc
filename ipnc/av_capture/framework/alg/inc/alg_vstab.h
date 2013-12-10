
#ifndef _ALG_VSTAB_H_
#define _ALG_VSTAB_H_

#include <alg.h>
#include <csl_ipipe.h>

//#define ALG_VS_DEBUG

typedef struct {

  Uint16 totalFrameWidth;
  Uint16 totalFrameHeight;

  Uint16 stabFrameWidth;
  Uint16 stabFrameHeight;

  CSL_IpipeBscConfig *pBscConfig;

} ALG_VstabCreate;

typedef struct {

  Uint8 *bscDataVirtAddr;
  Uint8 *bscDataPhysAddr;  

} ALG_VstabRunPrm;

typedef struct {

  Uint16 startX;
  Uint16 startY;

} ALG_VstabStatus;


void *ALG_vstabCreate(ALG_VstabCreate *create);
int ALG_vstabRun(void *hndl, ALG_VstabRunPrm *prm, ALG_VstabStatus *status);
int ALG_vstabDelete(void *hndl);

#endif




