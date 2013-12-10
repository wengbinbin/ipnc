#ifndef _AV_SERVER_API_H_
#define _AV_SERVER_API_H_

#include <avserver_config.h>

int AVSERVER_init();
int AVSERVER_exit();

int AVSERVER_start(AVSERVER_Config *config);
int AVSERVER_stop();

int AVSERVER_ldcEnable(int streamId, Bool enable);
int AVSERVER_snfEnable(int streamId, Bool enable);
int AVSERVER_tnfEnable(int streamId, Bool enable);
int AVSERVER_vsEnable(int streamId, Bool enable);
int AVSERVER_swosdEnable(int streamId, Bool enable);
int AVSERVER_fileSaveEnable(int streamId, Bool enable);
int AVSERVER_encryptEnable(int streamId, Bool enable);
int AVSERVER_faceDetectEnable(Bool enable);
int AVSERVER_pMaskEnable(Bool enable);
int AVSERVER_winmodeEnable(Bool enable);
int AVSERVER_aewbEnable(Bool enable);
int AVSERVER_afEnable(Bool enable);
int AVSERVER_aewbSetType(int type);
int AVSERVER_aewbPriority(int value);
int AVSERVER_SetDisplay(int type);

int AVSERVER_setEncBitrate(int streamId, int bitrate);
int AVSERVER_setEncFramerate(int streamId, unsigned int fps);

int AVSERVER_setMotion(int MotioneEnable, int MotioneCEnable, int MotioneCValue, int MotioneLevel, int block);
int AVSERVER_lock();
int AVSERVER_unlock();

int AVSERVER_profileInfoShow();
int AVSERVER_swosdText(char *strText, int nLength);
int AVSERVER_swosdEnableText(int enable);
int AVSERVER_histEnable(int enable);
int AVSERVER_gbceEnable(int enable);
int AVSERVER_swosdEnableLogo(int enable);

#endif  /*  _AV_SERVER_H_  */

