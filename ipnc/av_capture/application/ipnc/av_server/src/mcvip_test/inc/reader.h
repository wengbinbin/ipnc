
#ifndef _READER_H_
#define _READER_H_

#include <system.h>

typedef struct {

  char filename[100];
  int codec;
  int frameWidth;
  int frameHeight

} READER_CreatePrm;

int READER_create(READER_CreatePrm *prm);
int READER_delete();

#endif

