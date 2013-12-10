
#ifndef _WRITER_PRIV_H_
#define _WRITER_PRIV_H_

#include <mcvip.h>
#include <writer.h>
#include <osa_tsk.h>
#include <osa_mutex.h>
#include <osa_cmem.h>

#define WRITER_MIN_IN_BUF_PER_CH  (4)

#define WRITER_THR_PRI       (MCVIP_CAPTURE_THR_PRI_HIGH-5)
#define WRITER_STACK_SIZE    (10*KB)

#define WRITER_CMD_CREATE    (0x0400)
#define WRITER_CMD_RUN       (0x0401)
#define WRITER_CMD_DELETE    (0x0402)

#define WRITER_FILE_SAVE_STATE_IDLE   (0)
#define WRITER_FILE_SAVE_STATE_OPEN   (1)
#define WRITER_FILE_SAVE_STATE_WRITE  (2)  
#define WRITER_FILE_SAVE_STATE_CLOSE  (3) 


typedef struct {

  Bool enableChSave;

  int fileSaveState;
  int fileSaveIndex;
  FILE *fileSaveHndl;

} WRITER_ChInfo;

typedef struct {

  OSA_TskHndl tskHndl;
  OSA_MbxHndl mbxHndl;

  OSA_MutexHndl mutexLock;
  
  WRITER_CreatePrm createPrm;
  
  Uint16 bufNum;
  
  Uint8 *bufVirtAddr;
  Uint8 *bufPhysAddr;
  Uint32 bufSize;
  
  OSA_BufCreate bufCreate;
  OSA_BufHndl   bufHndl;

  WRITER_ChInfo  chInfo[MCVIP_CHANNELS_MAX];
  
  OSA_PrfHndl prfFileWr;

} WRITER_Ctrl;

extern WRITER_Ctrl gWRITER_ctrl;

int WRITER_fileSaveRun(int chId, Bool isKeyFrame, OSA_BufInfo *pBufInfo);
int WRITER_fileSaveExit();

#endif

