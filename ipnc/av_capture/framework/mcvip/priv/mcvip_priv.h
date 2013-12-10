

#ifndef _MCVIP_PRIV_H_
#define _MCVIP_PRIV_H_

#include <mcvip.h>
#include <osa_que.h>
#include <osa_tsk.h>
#include <osa_mutex.h>
#include <drv_dma.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev.h>
#include <linux/videodev2.h> 

#include <drv_i2c.h>

// AYK - 0826
#include <drv_audio.h>

#define MCVIP_CLEAR_BUF_DURING_INIT
#define MCVIP_DEBUG
#define MCVIP_PRF_PRINT

#define MCVIP_STATE_NOT_CREATED 0
#define MCVIP_STATE_CREATED     1

#define MCVIP_STACK_SIZE    (20*KB)

#define MCVIP_CMD_CREATE    (0x00300)
#define MCVIP_CMD_START     (0x00301)
#define MCVIP_CMD_STOP      (0x00302)
#define MCVIP_CMD_DELETE    (0x00303)

#define MCVIP_V4L2_INPUT_BUF_MAX  (5)

typedef struct {

  int curBufId;
  int curField;
  int curLine[2];
  
  int prevXY;
  int prevLineNum;
  
  Uint8 *curDstPhysAddr;
  Uint8 *curDstVirtAddr;
  
  int errorFrame;
  int errorLine;
  
  int isFirstFrame;
  int skipCount;

} MCVIP_ChDemuxInfo;

typedef struct {

  int width;
  int height;
  int offsetH;
  int offsetV;

} MCVIP_V4l2FrameInfo;

typedef struct {

  int state;
  
  OSA_TskHndl tskHndl;
  OSA_MbxHndl mbxHndl;
  
  OSA_QueHndl fullQue;
  OSA_QueHndl emptyQue[MCVIP_CHANNELS_MAX];

  OSA_MutexHndl putBufLock;
  
  MCVIP_CreatePrm createPrm;
  
  MCVIP_BufInfo bufInfo[MCVIP_BUF_MAX];
  
  MCVIP_ChList chList;
  
  DRV_DmaDemux dmaDemux;
  
  unsigned long dmaDemuxSrcPhysAddr[DRV_DMA_MAX_DEMUX_TRANSFERS];
  unsigned long dmaDemuxDstPhysAddr[DRV_DMA_MAX_DEMUX_TRANSFERS];
  
  MCVIP_ChDemuxInfo chDemuxInfo[MCVIP_CHANNELS_MAX];
  int tmpDemuxQue[MCVIP_BUF_MAX];
  
  int saveFrame;
  int saveFileIndex;

  int v4l2FileHndl;
  
  char v4l2DeviceName[20];
  
  char v4l2InputToUse[20]; // filled by MCVIP_tvp5158GetInfo()
  v4l2_std_id v4l2StdToUse; // filled by MCVIP_tvp5158GetInfo()
  MCVIP_V4l2FrameInfo v4l2FrameInfo; // filled by MCVIP_tvp5158GetInfo()
  
  Uint8 *v4l2CaptureBufVirtAddr[MCVIP_V4L2_INPUT_BUF_MAX];
  Uint8 *v4l2CaptureBufPhysAddr[MCVIP_V4L2_INPUT_BUF_MAX];  
  
  struct v4l2_buffer v4l2CaptureBufCur;
  
  int v4l2CaptureCount;
  
  int errorLine;
  int chBase;
  Uint32 initTimeInMsecs;
  
  DRV_DmaChHndl dmaDemuxHndl;

  OSA_PrfHndl prfCapture;
  OSA_PrfHndl prfDemux;
  OSA_PrfHndl prfDma;
  OSA_PrfHndl prfQueWait;  

  DRV_I2cHndl *pI2cHndl[MCVIP_TVP5158_MAX_CASCADE];
  
  int frameCount;
  
  int chLockStatus[MCVIP_CHANNELS_MAX];
  
  int skipField;
  
  int missedFrameCount;

  // AYK - 0826
  DRV_AudioHndl audioHndl;

} MCVIP_Hndl;

typedef struct {

  DRV_DmaChHndl dmaMemcpyChHndl;
  OSA_MutexHndl dmaLock;

  int	videoSystem;
  DRV_I2cHndl i2cHndl[MCVIP_VIDEO_DECODER_MODE_MAX][MCVIP_TVP5158_MAX_CASCADE];

} MCVIP_Ctrl;

typedef struct {
  
  int id;
  Uint8 *virtAddr;
  Uint8 *physAddr;
  int timestamp;

} MCVIP_V4l2Buf;

extern MCVIP_Hndl  gMCVIP_hndl[MCVIP_VIDEO_INPUT_PORT_MAX];
extern MCVIP_Ctrl  gMCVIP_ctrl;

int MCVIP_getChInfo(int videoDecoderMode, int videoSystem, MCVIP_ChInfo *info);

int MCVIP_demuxInit(MCVIP_Hndl *hndl);
int MCVIP_demuxRun(MCVIP_Hndl *hndl, MCVIP_V4l2Buf *buf);

int MCVIP_tskMain(struct OSA_TskHndl *pTsk, OSA_MsgHndl *pMsg, Uint32 curState );

int MCVIP_tskCreate(MCVIP_Hndl *hndl);
int MCVIP_tskDelete(MCVIP_Hndl *hndl);
int MCVIP_tskStart(MCVIP_Hndl *hndl,int sampleRate);
int MCVIP_tskStop(MCVIP_Hndl *hndl);

int MCVIP_v4l2Create(MCVIP_Hndl *hndl);
int MCVIP_v4l2Delete(MCVIP_Hndl *hndl);
int MCVIP_v4l2Start(MCVIP_Hndl *hndl,int sampleRate);
int MCVIP_v4l2Stop(MCVIP_Hndl *hndl);
int MCVIP_v4l2GetBuf(MCVIP_Hndl *hndl, MCVIP_V4l2Buf *buf);
int MCVIP_v4l2PutBuf(MCVIP_Hndl *hndl, MCVIP_V4l2Buf *buf);
int MCVIP_v4l2ForceWakeup(MCVIP_Hndl *hndl);

int MCVIP_v4l2SetColorAdjust(MCVIP_Hndl *hndl, int enableNR, int channel);
int MCVIP_v4l2SetBrightness(MCVIP_Hndl *hndl, int channel);
int MCVIP_v4l2SetContrast(MCVIP_Hndl *hndl, int channel);
int MCVIP_v4l2SetSaturation(MCVIP_Hndl *hndl, int channel);
int MCVIP_v4l2SetHue(MCVIP_Hndl *hndl, int channel);
int MCVIP_v4l2SetNoiseReduction(MCVIP_Hndl *hndl, int channel, int enable);
int MCVIP_v4l2SetHybridChSelect(MCVIP_Hndl *hndl, int channel);

int MCVIP_tvp5158setColorAdjust(MCVIP_Hndl *hndl, int enableNR, int channel);
int MCVIP_tvp5158SetBrightness(MCVIP_Hndl *hndl, int channel);
int MCVIP_tvp5158SetContrast(MCVIP_Hndl *hndl, int channel);
int MCVIP_tvp5158SetSaturation(MCVIP_Hndl *hndl, int channel);
int MCVIP_tvp5158SetHue(MCVIP_Hndl *hndl, int channel);
int MCVIP_tvp5158SetNoiseReduction(MCVIP_Hndl *hndl, int channel, int enable);

int MCVIP_tvp5158GetInfo(MCVIP_Hndl *hndl);
int MCVIP_tvp5158Setup(MCVIP_Hndl *hndl);
int MCVIP_tvp5158Start(MCVIP_Hndl *hndl,int sampleRate);
int MCVIP_tvp5158Stop(MCVIP_Hndl *hndl);
int MCVIP_tvp5158GetChLockStatus(MCVIP_Hndl *hndl);
int MCVIP_tvp5158GetAVDOuputCtrlStatus01(MCVIP_Hndl *hndl);

int MCVIP_tvp5158SetAudVol(MCVIP_Hndl *hndl, Uint8 channel);
int MCVIP_tvp5158SetSampleRate(MCVIP_Hndl *hndl,int sampleRate);
int MCVIP_tvp5158SetAudMute(MCVIP_Hndl *hndl, Uint8 channel);
int MCVIP_tvp5158SetHybridChSelect(MCVIP_Hndl *hndl, int channel);

int	TVP5158_setColorAdjust(DRV_I2cHndl *i2cHndl, int brightness, int contrast, int saturation, int hue, int enableNR, int channel);
int TVP5158_setBrightness(DRV_I2cHndl *i2cHndl, int brightness, int channel);
int TVP5158_setContrast(DRV_I2cHndl *i2cHndl, int contrast, int channel);
int TVP5158_setSaturation(DRV_I2cHndl *i2cHndl, int saturation, int channel);
int TVP5158_setHue(DRV_I2cHndl *i2cHndl, int hue, int channel);

int TVP5158_setNoiseReduction(DRV_I2cHndl *i2cHndl, int enable, int channel);

int TVP5158_setOfmMode(DRV_I2cHndl *i2cHndl, int mode, int videoIfMode, int videoSystem, int cascadeStage);
int TVP5158_reset(DRV_I2cHndl *i2cHndl);
int TVP5158_checkChipId(DRV_I2cHndl *i2cHndl);
int TVP5158_patchDownload(DRV_I2cHndl *i2cHndl);
Uint8 TVP5158_getStatus(DRV_I2cHndl *i2cHndl, Uint8 chNum);
Uint8 TVP5158_getStdStatus(DRV_I2cHndl *i2cHndl);
int TVP5158_setStdStatus(DRV_I2cHndl * i2cHndl);
Uint8 TVP5158_getAVDoutputCtrlStatus01(DRV_I2cHndl * i2cHndl);

// AYK - 0731
int TVP5158_setAudioMode(DRV_I2cHndl *i2cHndl, int mode, int cascadeStage,int sampleRate);
int TVP5158_setAudVol(DRV_I2cHndl *i2cHndl, Uint8 vol, Uint8 channel);
int TVP5158_setSampleRate(DRV_I2cHndl *i2cHndl,int sampleRate);

int TVP5158_setAudMute(DRV_I2cHndl *i2cHndl, int enableMute, Uint8 channel);

int TVP5158_setHybridChSelect(DRV_I2cHndl *i2cHndl, int channel);

#endif


