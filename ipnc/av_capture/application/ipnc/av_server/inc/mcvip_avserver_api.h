#ifndef _AV_SERVER_API_H_
#define _AV_SERVER_API_H_

#include <avserver_config.h>

int mcvip_AvServer_init();
int mcvip_AvServer_exit();

int mcvip_AvServer_start(AVSERVER_Config *config);
int mcvip_AvServer_stop();

int mcvip_AvServer_ldcEnable(int streamId, Bool enable);
int mcvip_AvServer_snfEnable(int streamId, Bool enable);
int mcvip_AvServer_tnfEnable(int streamId, Bool enable);
int mcvip_AvServer_vsEnable(int streamId, Bool enable);
int mcvip_AvServer_swosdEnable(int streamId, Bool enable);
int mcvip_AvServer_fileSaveEnable(int streamId, Bool enable);
int mcvip_AvServer_encryptEnable(int streamId, Bool enable);
int mcvip_AvServer_faceDetectEnable(Bool enable);
int mcvip_AvServer_pMaskEnable(Bool enable);
int mcvip_AvServer_winmodeEnable(Bool enable);
int mcvip_AvServer_aewbEnable(Bool enable);
int mcvip_AvServer_afEnable(Bool enable);
int mcvip_AvServer_aewbSetType(int type);
int mcvip_AvServer_aewbPriority(int value);
int mcvip_AvServer_SetDisplay(int type);

int mcvip_AvServer_setEncBitrate(int streamId, int bitrate);
int mcvip_AvServer_setEncFramerate(int streamId, unsigned int fps);

int mcvip_AvServer_setMotion(int MotioneEnable, int MotioneCEnable, int MotioneCValue, int MotioneLevel, int block);
int mcvip_AvServer_lock();
int mcvip_AvServer_unlock();

int mcvip_AvServer_profileInfoShow();
int mcvip_AvServer_swosdText(char *strText, int nLength);
int mcvip_AvServer_swosdEnableText(int enable);
int mcvip_AvServer_histEnable(int enable);
int mcvip_AvServer_gbceEnable(int enable);
int mcvip_AvServer_swosdEnableLogo(int enable);

#endif  /*  _AV_SERVER_H_  */

