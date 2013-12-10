
#ifndef _WRITER_H_
#define _WRITER_H_

#include <system.h>
#include <osa_buf.h>

typedef struct {

  int numCh;
  int frameWidth;
  int frameHeight;

} WRITER_CreatePrm;

int WRITER_create(WRITER_CreatePrm *prm);
int WRITER_delete();

OSA_BufInfo *WRITER_getEmptyBuf(int *bufId, int timeout);
int WRITER_putFullBuf(int chId, OSA_BufInfo *buf, int bufId);

int WRITER_enableChSave(int chId, Bool enable);

#endif

