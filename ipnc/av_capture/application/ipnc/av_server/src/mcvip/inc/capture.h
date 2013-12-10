

#ifndef _CAPTURE_H_
#define _CAPTURE_H_

#include <system.h>
#include <mcvip.h>


typedef struct {

  int videoDecoderMode;   
  int videoIfMode;        
  int videoSystem;
  Uint32 frameSkipMask;      

} CAPTURE_CreatePrm;


typedef struct {

  void *mcvipHndl;
  
  MCVIP_ChList chList;
  MCVIP_CreatePrm createPrm;
 
} CAPTURE_Info;

int CAPTURE_create(CAPTURE_CreatePrm *prm);
int CAPTURE_delete();
int CAPTURE_start();
int CAPTURE_stop();

int CAPTURE_saveFrame();

int CAPTURE_printInfo();

int CAPTURE_selectPage(int pageNum);

int CAPTURE_setFrameSkipMask(Uint32 frameSkipMask);

CAPTURE_Info *CAPTURE_getInfo();


#endif 
