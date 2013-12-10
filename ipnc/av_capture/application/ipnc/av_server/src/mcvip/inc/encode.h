
#ifndef _ENCODE_H_
#define _ENCODE_H_

#include <system.h>
#include <capture.h>
#include <osa_buf.h>

typedef struct {

  int numCh;
  int frameWidth;
  int frameHeight;

} ENCODE_CreatePrm;

int ENCODE_create(ENCODE_CreatePrm *prm);
int ENCODE_delete();

OSA_BufInfo *ENCODE_getEmptyBuf(int chId, int *bufId, int timeout);
int ENCODE_putFullBuf(int chId, OSA_BufInfo *buf, int bufId);

int ENCODE_enableCh(int chId, Bool enable);
int ENCODE_setChBitrate(int chId, int bitrate);
int ENCODE_setChKeyFrameInterval(int chId, int keyFrameInterval);

#endif

