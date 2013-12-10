
#ifndef _ALG_VID_ENC_H_
#define _ALG_VID_ENC_H_

#include <alg.h>
#include <alg_motionDetect.h>
#include <drv_faceDetect.h>

#define ALG_VID_ENC_MODE_STANDARD        0
#define ALG_VID_ENC_MODE_BEST_QUALITY    1
#define ALG_VID_ENC_MODE_FAST_FPS        2

#define ALG_VID_ENC_RATE_CONTROL_RCOFF   0
#define ALG_VID_ENC_RATE_CONTROL_VBR     1
#define ALG_VID_ENC_RATE_CONTROL_CBR     2

#define ALG_VID_ENC_PRESET_AUTO			 0
#define ALG_VID_ENC_PRESET_CUSTOM		 1

#define ALG_VID_ENC_ROI_PRIORITY_HIGH      3
#define ALG_VID_ENC_ROI_PRIORITY_MEDIUM    2
#define ALG_VID_ENC_ROI_PRIORITY_LOW       1
#define ALG_VID_ENC_ROI_PRIORITY_NONE      0

#define ALG_VID_ENC_ROI_MAX_ROI 5

typedef enum {
	ALG_VID_ENC_ROI_TYPE_FACE 			= 0,
	ALG_VID_ENC_ROI_TYPE_BACKGROUND 	= 1,
	ALG_VID_ENC_ROI_TYPE_FOREGROUND 	= 2,
	ALG_VID_ENC_ROI_TYPE_DEFAULT  		= 3,
	ALG_VID_ENC_ROI_TYPE_PRIVACYMASK  	= 4
}ALG_VidEncROIType;

typedef struct {
    Uint32 topLeft_x;
    Uint32 topLeft_y;
    Uint32 bottomRight_x;
    Uint32 bottomRight_y;
    ALG_VidEncROIType roiType;
    Uint32 roiPriority;
}ALG_VidEncROI;

typedef struct {
    Uint16 numROI;
    ALG_VidEncROI roiInfo[ALG_VID_ENC_ROI_MAX_ROI];
}ALG_VidEncROIPrm;

typedef struct {

  Uint16 codec;
  Uint16 dataFormat;
  Uint16 width;
  Uint16 height;
  Uint16 offsetH;
  Uint16 offsetV;
  int    bitrate;
  Uint16 mode;
  Uint16 keyFrameInterval;
  Uint32 mbMvOutEnable;
  Uint16 qValue; // only used for JPG
  Uint16 rateControlType;
  Uint16 vnfEnable;
  Uint16 framerate;
  Uint16 enableROI;
  Int16 forceKeyFrame;
  Uint16 qpinit;
  Uint16 qpmin;
  Uint16 qpmax;
  Uint16 packetsize;
  Uint16 encodePreset;
} ALG_VidEncCreate;

typedef struct {

  Uint16 inStartX;
  Uint16 inStartY;

  Uint8 *inAddr;
  Uint8 *outAddr;

  ALG_MotionDetectMbMvInfo *mbMvInfo;

  Uint32 outDataMaxSize;
  ALG_VidEncROIPrm roiPrm;

  Uint32 snapEnable;
  char snapFilename[48];
} ALG_VidEncRunPrm;

typedef struct {

  Uint32 bytesGenerated;
  Uint32 isKeyFrame;
  Uint16 numMB;

} ALG_VidEncRunStatus;

void *ALG_vidEncCreate(ALG_VidEncCreate *create);
int ALG_vidEncSetBitrate(void *hndl, int bitrate);
int ALG_vidEncSetROI(void *hndl, int flag);
int ALG_vidEncSetFramerate(void *hndl, int framerate);
int ALG_vidEncSetMVdataOut(void *hndl, int mbMvOutEnable);
int ALG_vidEncGetMVdataStatus(void *hndl);
int ALG_vidEncGetMVdataSize(void *hndl);
void *ALG_vidEncGetMVdataAddr(void *hndl);
int ALG_vidEncSetKeyFrameInterval(void *hndl, int keyFrameInterval, int forceFrame);
int ALG_vidEncSetQPSize(void *hndl, int qpinit, int qpmin, int qpmax, int packetsize);
int ALG_vidEncRun(void *hndl, ALG_VidEncRunPrm *prm, ALG_VidEncRunStatus *status);
int ALG_vidEncDelete(void *hndl);


#endif


