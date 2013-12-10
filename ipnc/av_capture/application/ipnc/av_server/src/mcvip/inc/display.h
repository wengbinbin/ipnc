
#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <system.h>
#include <capture.h>
#include <osa_buf.h>

#define DISPLAY_MODE_COMPOSITE    0

#define DISPLAY_COLOR_NO_VIDEO_Y    0x80808080
#define DISPLAY_COLOR_NO_VIDEO_C    0x80808080

#define DISPLAY_LAYOUT_MODE_LIVE_1x1			(0)
#define DISPLAY_LAYOUT_MODE_LIVE_2x2			(1)
#define DISPLAY_LAYOUT_MODE_LIVE_3x3			(2)
#define DISPLAY_LAYOUT_MODE_LIVE_8CH			(4)
#define DISPLAY_LAYOUT_MODE_PLAYBACK      (5)
#define DISPLAY_LAYOUT_MODE_MAX           (6)

#define DISPLAY_LAYOUT_MAX_WIN       (9)

#define DISPLAY_LAYOUT_PLAYBACK_WIN   (DISPLAY_LAYOUT_MAX_WIN-1)

typedef struct {

  int displayMode;   
  int layoutMode;
    
} DISPLAY_CreatePrm;

typedef struct {

  int startX;
  int startY;
  int width;
  int height;

} DISPLAY_LayoutWinInfo;

typedef struct {

  int numWin;
  int layoutMode;
  
  DISPLAY_LayoutWinInfo winInfo[DISPLAY_LAYOUT_MAX_WIN];

} DISPLAY_LayoutInfo;

int DISPLAY_create(DISPLAY_CreatePrm *prm);
int DISPLAY_delete();
int DISPLAY_start();
int DISPLAY_stop();
int DISPLAY_printInfo();
int DISPLAY_fieldModeEnable(Bool enable); 

int DISPLAY_layoutSetMode(int layoutMode);
OSA_BufInfo *DISPLAY_layoutGetEmptyWinBuf(int winId, int *bufId, int timeout);
int DISPLAY_layoutPutFullWinBuf(int winId, OSA_BufInfo *buf, int bufId, int layoutModeUsed);
int DISPLAY_layoutGetInfo(DISPLAY_LayoutInfo *info);

#endif
