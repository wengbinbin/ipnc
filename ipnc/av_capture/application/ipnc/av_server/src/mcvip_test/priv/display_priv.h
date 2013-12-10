

#ifndef _DISPLAY_PRIV_H_
#define _DISPLAY_PRIV_H_

#include <display.h>
#include <drv_display.h>
#include <osa_tsk.h>
#include <osa_mutex.h>

#define DISPLAY_LAYOUT_MAX_WIN_BUF   (4)

#define DISPLAY_THR_PRI       (MCVIP_CAPTURE_THR_PRI_HIGH-2)
#define DISPLAY_STACK_SIZE    (10*KB)

#define DISPLAY_CMD_CREATE    (0x0200)
#define DISPLAY_CMD_START     (0x0201)
#define DISPLAY_CMD_STOP      (0x0202)  
#define DISPLAY_CMD_DELETE    (0x0203)

typedef struct {

  int id;
  Uint8 *virtAddr;
  Uint8 *physAddr;

} DISPLAY_BufInfo;

typedef struct {

  int width;
  int height;
  int lineOffsetV;
  int lineOffsetH;  

} DISPLAY_FrameInfo;

typedef struct {

  OSA_BufCreate bufCreate;
  OSA_BufHndl   bufHndl;
  
  Uint8 *bufVirtAddr;
  Uint8 *bufPhysAddr;
  
  Uint32 bufSize;
  
  int blankCount;
  
  int curFullBufId;
  
} DISPLAY_LayoutWinCtrl;

typedef struct {

  int displayWidth;
  int displayHeight;
  int displayOffsetH;
  int displayOffsetV;  
  int displayStartX;
  int displayStartY;
  
  OSA_MutexHndl mutexLock;
  
  DISPLAY_LayoutWinCtrl winCtrl[DISPLAY_LAYOUT_MAX_WIN];
  
  DISPLAY_LayoutInfo info;
  
} DISPLAY_LayoutCtrl;

typedef struct {

  OSA_TskHndl tskHndl;
  OSA_MbxHndl mbxHndl;
  
  DISPLAY_FrameInfo frameInfo;
  int blankDetectThres;

  DRV_DisplayWinHndl displayHndl;
  int                displayBufId;
  DRV_DisplayBufInfo displayInfo;
  
  OSA_PrfHndl prfDma;
  OSA_PrfHndl prfDisplay;

} DISPLAY_Ctrl;

extern DISPLAY_Ctrl gDISPLAY_ctrl;

int DISPLAY_tskCreate( DISPLAY_CreatePrm *prm);
int DISPLAY_tskDelete( );

int DISPLAY_layoutRun(DISPLAY_BufInfo *buf, DISPLAY_LayoutInfo *layoutInfo);

int DISPLAY_layoutCreate(int layoutMode);
int DISPLAY_layoutDelete();

#endif
