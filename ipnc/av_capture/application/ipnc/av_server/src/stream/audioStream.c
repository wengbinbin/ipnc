#include <avserver.h>
#include <stream.h>
#include <encode.h>

extern GlobalData gbl;

int AUDIO_streamShmCopy(int streamId, AUDIO_BufInfo *pBufInfo)
{
  int  status=OSA_SOK;
    
 
  status = stream_write(
        pBufInfo->virtAddr, 
        pBufInfo->encFrameSize, 
        AUDIO_FRAME,
        STREAM_AUDIO,
        pBufInfo->timestamp,
        stream_get_handle(),
        streamId
      );
      
  if(status!=OSA_SOK) {
    OSA_ERROR("stream_write(%d, %d, %d, %u)\n",
        pBufInfo->encFrameSize, 
        AUDIO_FRAME,
        STREAM_AUDIO,
        pBufInfo->timestamp
      );      
  }
  
  return status;
}

