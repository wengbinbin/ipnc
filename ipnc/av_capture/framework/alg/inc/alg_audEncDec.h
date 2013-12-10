
#ifndef _ALG_AUD_ENC_DEC_H_
#define _ALG_AUD_ENC_DEC_H_

#include <alg.h>

typedef struct {

  Uint16 codec;

} ALG_AudEncCreate;

typedef struct {

  Uint16 codec;

} ALG_AudDecCreate;

typedef struct {

  Uint8 *inAddr;
  Uint32 inDataSize;

  Uint8 *outAddr;
  Uint32 outBufMaxSize;

} ALG_AudioRunPrm;

typedef struct {

  Uint32 inDataUsedSize;
  Uint32 outDataSize;

} ALG_AudioRunStatus;

int ALG_audEncDecInit();
int ALG_audEncDecExit();

void *ALG_audEncCreate(ALG_AudEncCreate *create);
int ALG_audEncRun(void *hndl, ALG_AudioRunPrm *prm, ALG_AudioRunStatus *status);
int ALG_audEncDelete(void *hndl);

void *ALG_audDecCreate(ALG_AudDecCreate *create);
int ALG_audDecRun(void *hndl, ALG_AudioRunPrm *prm, ALG_AudioRunStatus *status);
int ALG_audDecDelete(void *hndl);


#endif

