
#ifndef _ENCODE_PRIV_H_
#define _ENCODE_PRIV_H_

#include <encode.h>
#include <osa_tsk.h>
#include <osa_mutex.h>
#include <alg_vidEnc.h>
#include <osa_cmem.h>
#include <writer.h>

#define ENCODE_MIN_IN_BUF_PER_CH  (4)

#define ENCODE_THR_PRI       (MCVIP_CAPTURE_THR_PRI_HIGH-3)
#define ENCODE_STACK_SIZE    (30*KB)

#define ENCODE_CMD_CREATE    (0x0300)
#define ENCODE_CMD_RUN       (0x0301)
#define ENCODE_CMD_DELETE    (0x0302)


typedef struct {

  Bool enableEncode;
  Uint32 curBitrate;
  Uint32 newBitrate;
  
  Uint32 curKeyFrameInterval;
  Uint32 newKeyFrameInterval;
  
  void *algVidEncHndl;

  Uint16 bufNum;

  Uint8 *bufVirtAddr;
  Uint8 *bufPhysAddr;
  Uint32 bufSize;
  
  OSA_BufCreate bufCreate;
  OSA_BufHndl   bufHndl;

} ENCODE_ChInfo;

typedef struct {

  OSA_TskHndl tskHndl;
  OSA_MbxHndl mbxHndl;

  OSA_MutexHndl mutexLock;
  
  ENCODE_CreatePrm createPrm;

  ENCODE_ChInfo  chInfo[MCVIP_CHANNELS_MAX];
  
  OSA_PrfHndl prfEnc;

} ENCODE_Ctrl;

extern ENCODE_Ctrl gENCODE_ctrl;

#endif

