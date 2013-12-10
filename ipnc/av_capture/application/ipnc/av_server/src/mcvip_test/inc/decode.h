
#ifndef _DECODE_H_
#define _DECODE_H_

#include <system.h>
#include <osa_buf.h>

typedef struct {

  char filename[100];
  int codec;
  int maxWidth;
  int maxHeight;

} DECODE_CreatePrm;

int DECODE_create(DECODE_CreatePrm *prm);

int DECODE_waitComplete();

int DECODE_delete();

#endif

