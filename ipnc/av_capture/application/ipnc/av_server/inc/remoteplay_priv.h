
#ifndef _REMOTE_PRIV_H_
#define _REMOTE_PRIV_H_

#include <mcvip.h>
#include <remoteplay.h>
#include <osa_tsk.h>
#include <osa_mutex.h>
#include <osa_cmem.h>
#include <basket.h>
#define REMOTEPLAY_MIN_IN_BUF_PER_CH  (1)

#define REMOTEPLAY_THR_PRI       (MCVIP_CAPTURE_THR_PRI_HIGH-8)
#define REMOTEPLAY_STACK_SIZE    (10*KB)

#define REMOTEPLAY_CMD_CREATE    (0x0700)
#define REMOTEPLAY_CMD_RUN       (0x0701)
#define REMOTEPLAY_CMD_DELETE    (0x0702)


typedef struct {

  OSA_TskHndl tskHndl;
  OSA_MbxHndl mbxHndl;

  int channel ;
  int sockchannel ;
  int pause ;
  int stop ;
  int runflag ;
  unsigned long rectime ;
  OSA_MutexHndl mutexLock;
  FILE* fp;
  OSA_PrfHndl prfFileWr;

} REMOTEPLAY_Ctrl;

typedef struct REMOTEPLAY_CLIENT_CTRL{

  REMOTEPLAY_Ctrl remoteplay_ctrl;
  pthread_t 	  client_id;
  unsigned char*  inferFIFOName;
  struct REMOTEPLAY_CLIENT_CTRL* next;
}REMOTEPLAY_CLIENT_Ctrl;

typedef struct REMOTEPLAY_CLIENT_LIST{
	int 					count;
	REMOTEPLAY_CLIENT_Ctrl *head;
	REMOTEPLAY_CLIENT_Ctrl *cur;
}REMOTEPLAY_CLIENT_List,*REMOTEPLAY_CLIENT_List_Ptr;

extern REMOTEPLAY_Ctrl gREMOTEPLAY_ctrl;//add by sxh playback

//add by sxh
#define NTSC176144      0x0003
#define NTSC320240      0x0000
#define NTSC640480      0x0001
#define NTSC720480      0x0002
#define NTSC640240      0x0009
#define NTSC352240      0x000A
#define NTSC720240      0x000B

#define PAL176144       0x0013
#define PAL320288       0x0004
#define PAL352288       0x0005
#define PAL704576       0x0006
#define PAL720576       0x0007
#define PAL360288       0x000C
#define PAL720288       0x000D

#endif

