

#ifndef _OSA_BUF_H_
#define _OSA_BUF_H_

#include <osa_que.h>
#include <osa_mutex.h>

#define OSA_BUF_NUM_MAX       (8*8)

#define OSA_BUF_ID_INVALID    (-1)

typedef struct {

  int size;
  int count;
  int flags;
  int timesec ;
  int timestamp;
  int reserved ;
  int 		width;
  int 		height;
  Uint32 	isKeyFrame;
  Uint16 	codecType;
  void *physAddr;
  void *virtAddr;

  int sampleRate; // AYK - 1224
  void *pPrm;     // AYK - 0104
  int audioONOFF; // AYK - 0125 

} OSA_BufInfo;

typedef struct {

  OSA_BufInfo bufInfo[OSA_BUF_NUM_MAX];
  
  OSA_QueHndl emptyQue;
  OSA_QueHndl fullQue;
      
  int numBuf;

} OSA_BufHndl;

typedef struct {

  void *bufPhysAddr[OSA_BUF_NUM_MAX];
  void *bufVirtAddr[OSA_BUF_NUM_MAX];  
  
  int numBuf;

} OSA_BufCreate;

int  OSA_bufCreate(OSA_BufHndl *hndl, OSA_BufCreate *bufInit);
int  OSA_bufDelete(OSA_BufHndl *hndl);

int  OSA_bufSwitchFull (OSA_BufHndl *hndl, int *bufId);
int  OSA_bufSwitchEmpty(OSA_BufHndl *hndl, int *bufId);

static inline OSA_BufInfo *OSA_bufGetBufInfo(OSA_BufHndl *hndl, int bufId)
{
  #ifdef OSA_ENABLE_PRM_CHECK
  if(hndl==NULL)
    return NULL;
    
  if(bufId>=hndl->numBuf)
    return NULL;
  #endif
      
  return &hndl->bufInfo[bufId];
}

static inline int OSA_bufGetEmpty(OSA_BufHndl *hndl, int *bufId, Uint32 timeout)
{
  int status;

  #ifdef OSA_ENABLE_PRM_CHECK
  if(hndl==NULL || bufId==NULL)
    return OSA_EFAIL;
  #endif
    
  status = OSA_queGet(&hndl->emptyQue, bufId, timeout);
  
  if(status!=OSA_SOK) {
    *bufId = OSA_BUF_ID_INVALID;  
  } 

  return status;
}

static inline int OSA_bufPutFull (OSA_BufHndl *hndl, int bufId)
{
  int status;
  
  #ifdef OSA_ENABLE_PRM_CHECK  
  if(bufId >= hndl->numBuf || bufId < 0 || hndl==NULL)
    return OSA_EFAIL;
  #endif    
    
  status = OSA_quePut(&hndl->fullQue, bufId, OSA_TIMEOUT_FOREVER);
  
  return status;
}

static inline int OSA_bufGetFull(OSA_BufHndl *hndl, int *bufId, Uint32 timeout)
{
  int status;

  #ifdef OSA_ENABLE_PRM_CHECK
  if(hndl==NULL || bufId==NULL)
    return OSA_EFAIL;
  #endif

  status = OSA_queGet(&hndl->fullQue, bufId, timeout);
  
  if(status!=OSA_SOK) {
    *bufId = OSA_BUF_ID_INVALID;  
  } 

  return status;
}

static inline int OSA_bufPutEmpty(OSA_BufHndl *hndl, int bufId)
{
  int status;
  
  #ifdef OSA_ENABLE_PRM_CHECK  
  if(bufId >= hndl->numBuf || bufId < 0 || hndl==NULL)
    return OSA_EFAIL;
  #endif    

  status = OSA_quePut(&hndl->emptyQue, bufId, OSA_TIMEOUT_FOREVER);
  
  return status;
}

#endif /* _OSA_BUF_H_ */



