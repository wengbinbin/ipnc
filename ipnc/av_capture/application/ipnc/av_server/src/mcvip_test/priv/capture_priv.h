

#ifndef _CAPTURE_PRIV_H_
#define _CAPTURE_PRIV_H_

#include <capture.h>
#include <osa_tsk.h>
#include <drv_resz.h>

#define CAPTURE_THR_PRI       (MCVIP_CAPTURE_THR_PRI_HIGH-1)
#define CAPTURE_STACK_SIZE    (10*KB)

#define CAPTURE_CMD_CREATE    (0x0100)
#define CAPTURE_CMD_START     (0x0101)
#define CAPTURE_CMD_STOP      (0x0102)  
#define CAPTURE_CMD_DELETE    (0x0103)

typedef struct {
  
  Uint32 frameSkipMask;
  Uint32 curFrameNum;

} CAPTURE_ChInfo;

typedef struct {

  CAPTURE_Info info;
  OSA_TskHndl tskHndl;
  OSA_MbxHndl mbxHndl;
  
  Bool saveFrame;
  int saveFileIndex;
  Uint8 *memVirtAddr;
  Uint8 *memPhysAddr;
  
  DRV_ReszRunPrm reszPrm;
  DRV_ReszOutPrm reszOutPrm[CSL_RSZ_CH_MAX];
  
  int curPage;
  
  OSA_MutexHndl mutexLock;
  
  CAPTURE_ChInfo chInfo[MCVIP_CHANNELS_MAX];
  
  OSA_PrfHndl prfResz;
  OSA_PrfHndl prfCapture;
  
} CAPTURE_Ctrl;

extern CAPTURE_Ctrl gCAPTURE_ctrl;

int CAPTURE_tskCreate( CAPTURE_CreatePrm *prm);
int CAPTURE_tskDelete( );
int CAPTURE_tskStart( );
int CAPTURE_tskStop( );
int CAPTURE_tskSaveFrame( MCVIP_BufInfo *pBufInfo);

#endif
