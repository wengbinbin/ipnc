
#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <osa_tsk.h>
#include <osa_buf.h>
#include <osa_mutex.h>
#include <osa_flg.h>
#include <drv_audio.h>

#include <alg_g711.h>


typedef struct {
	void *physAddr;
  void *virtAddr;
  Uint32 timestamp;
  Uint32 encFrameSize;

} AUDIO_BufInfo;



typedef struct {
  OSA_TskHndl 		audioTsk;
  DRV_AudioHndl 	audioHndl;
  int 						streamId;
	short 					*encodedBuffer;
	int							encodeBufSize;
	unsigned short 	*inputBuffer;
	int							inputBufSize;
} AUDIO_Ctrl;

int AUDIO_audioCreate();
int AUDIO_audioDelete();

int AUDIO_streamShmCopy(int streamId, AUDIO_BufInfo *pBufInfo);


#endif  /*  _AUDIO_H_ */

