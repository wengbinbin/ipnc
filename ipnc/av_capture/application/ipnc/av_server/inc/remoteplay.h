
#ifndef _REMOTEPLAY_H_
#define _REMOTEPLAY_H_


int REMOTEPLAY_create(void);
int REMOTEPLAY_delete(void);
int REMOTEPLAY_start(int channel, unsigned long rectime,unsigned char* inferFIFOName);
int PLAYBACK_pauseCh(int channel,unsigned char* inferFIFOName) ;
int PLAYBACK_streamCh(int channel,unsigned char* inferFIFOName) ;
int PLAYBACK_stopCh(int channel,unsigned char* inferFIFOName) ;
int PLAYBACK_status(int channel,unsigned char* inferFIFOName) ;

#endif

