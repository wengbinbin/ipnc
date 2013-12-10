
#ifndef _DECODE_PRIV_H_
#define _DECODE_PRIV_H_

#include <decode.h>
#include <mcvip.h>
#include <osa_tsk.h>
#include <osa_mutex.h>
#include <alg_vidDec.h>
#include <osa_cmem.h>
#include <osa_flg.h>

#define DECODE_MIN_IN_BUF_PER_CH  (3)

#define DECODE_THR_PRI       (MCVIP_CAPTURE_THR_PRI_HIGH-4)
#define DECODE_STACK_SIZE    (30*KB)

#define DECODE_CMD_CREATE    (0x0300)
#define DECODE_CMD_RUN       (0x0301)
#define DECODE_CMD_DELETE    (0x0302)

#define DECODE_FLAG_DONE     (0x0001)
#define DECODE_FLAG_ERROR    (0x0002)

typedef struct {
  OSA_TskHndl tskHndl;
  OSA_MbxHndl mbxHndl;
  
  DECODE_CreatePrm createPrm;

  void *algVidDecHndl;

  Uint8 *inBufVirtAddr;
  Uint8 *inBufPhysAddr;
  Uint32 inBufSize;
  Uint32 inDataSize;
  Uint32 inDataOffset;
 
  OSA_PrfHndl prfDec;
  OSA_PrfHndl prfResz;
    
  OSA_FlgHndl flgHndl;
  
  Uint32 curDecBufId;
  Uint16 decBufNum;
  Uint32 decBufSize;
  Uint8  *decBufBaseVirtAddr;
  Uint8  *decBufBasePhysAddr;
  Uint8  *decBufVirtAddr[16];
  Uint8  *decBufPhysAddr[16];
  
  int   frameCount;
    
} DECODE_Ctrl;

extern DECODE_Ctrl gDECODE_ctrl;

#endif

