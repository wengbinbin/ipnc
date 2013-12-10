
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <drv_audio.h>

#include <alsa/asoundlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/soundcard.h>


typedef struct {
	snd_pcm_t *sound_handle;
} DRV_AudioObj;



int DRV_audioOpen(DRV_AudioHndl *hndl, DRV_AudioConfig *config)
{

	int rc = 0;
	unsigned int val = 0;
	int dir = 0;
	unsigned int buffer_time = 0;
	snd_pcm_uframes_t frames;
	snd_pcm_uframes_t buffer_frames;
	snd_pcm_hw_params_t *sound_params = NULL;
	DRV_AudioObj *pObj;

	hndl->hndl = OSA_memAlloc(sizeof(DRV_AudioObj));
	if( hndl->hndl == NULL )
	{
		OSA_ERROR("OSA_memAlloc fail \n");
		goto error_exit;
	}

	pObj = (DRV_AudioObj*)hndl->hndl;
	memset(pObj,0,sizeof(DRV_AudioObj));

	/* Open PCM device for record. */
	rc = snd_pcm_open(&(pObj->sound_handle), "default", SND_PCM_STREAM_CAPTURE, 0);
	if (rc < 0)
	{
		OSA_ERROR("unable to open pcm device: %s\n", snd_strerror(rc));
		goto error_exit;
	}

	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_alloca(&sound_params);

	/* Fill it in with default values. */
	snd_pcm_hw_params_any(pObj->sound_handle, sound_params);

	/* Set the desired hardware parameters. */

	/* Interleaved mode */
	snd_pcm_hw_params_set_access(pObj->sound_handle, sound_params,
				      SND_PCM_ACCESS_RW_INTERLEAVED );

	/* Signed 16-bit little-endian format */
	snd_pcm_hw_params_set_format(pObj->sound_handle, sound_params,
				     SND_PCM_FORMAT_S16_LE);

	/* channels */
	snd_pcm_hw_params_set_channels(pObj->sound_handle, sound_params, config->numChannels);


	/* Sampling rate*/
	val = config->samplingRate;
	snd_pcm_hw_params_set_rate_near(pObj->sound_handle, sound_params, &val, &dir);

	if (val != config->samplingRate)
	{

		OSA_ERROR("Rate doesn't match (requested %iHz, get %iHz)\n", config->samplingRate, val);
		goto error_exit;
	}


	/* Set period size of frames. */
	frames = config->buff_Of_Samples;
	snd_pcm_hw_params_set_period_size_near(pObj->sound_handle,
					       sound_params, &frames, &dir);

	/* set the buffer time */
 	if( buffer_time )
 	{
 		rc = snd_pcm_hw_params_set_buffer_time_near(pObj->sound_handle, sound_params, &buffer_time, &dir);
 		if (rc < 0) {
 		    OSA_ERROR("Unable to set buffer time %i for playback: %s\n", buffer_time, snd_strerror(rc));
 		    goto error_exit;
 		}
 	}else{
		buffer_frames =  frames*4;
		snd_pcm_hw_params_set_buffer_size_near(pObj->sound_handle,
										 sound_params, &buffer_frames);

 	}


	/* Write the parameters to the driver */
	rc = snd_pcm_hw_params(pObj->sound_handle, sound_params);
	if (rc < 0)
	{

		OSA_ERROR("unable to set hw parameters: %s\n",
			snd_strerror(rc));
		goto error_exit;
	}

	/* Use a buffer large enough to hold one period */
	snd_pcm_hw_params_get_period_size(sound_params, &frames, &dir);

	snd_pcm_hw_params_get_period_time(sound_params, &val, &dir);

#ifdef DRV_AUDIO_DEBUG
	OSA_printf("AUDIO : period size = %d frames dir = %d\n", (int)frames,dir);
	OSA_printf("AUDIO : period time = %d us dir = %d\n", val, dir);
#endif

	return OSA_SOK;

error_exit:

	if( hndl->hndl )
	{
		OSA_memFree(hndl->hndl);
	}
	return OSA_EFAIL;



}

int DRV_audioRead(DRV_AudioHndl *hndl,void *pInBuf, int readsamples)
{
	int rc;
	DRV_AudioObj *pObj;
	if(hndl==NULL)
	  return OSA_EFAIL;

	if(hndl->hndl==NULL)
	  return OSA_EFAIL;

	pObj = (DRV_AudioObj *)hndl->hndl;

	/* Read stereo data from codec */
	rc = snd_pcm_readi(pObj->sound_handle, pInBuf, readsamples);
	if (rc == -EPIPE)
	{
		/* EPIPE means overrun */
		OSA_ERROR("overrun occurred\n");
		snd_pcm_prepare(pObj->sound_handle);
		return 0;
	}else{
		if( rc < 0 )
		{
			OSA_ERROR("error from read : %s\n",snd_strerror(rc));
		}

		return rc;
	}
}



int DRV_audioClose(DRV_AudioHndl *hndl)
{
	DRV_AudioObj *pObj;

  if(hndl==NULL)
    return OSA_EFAIL;

  if(hndl->hndl==NULL)
    return OSA_EFAIL;

  pObj = (DRV_AudioObj *)hndl->hndl;

	snd_pcm_close(pObj->sound_handle);

	OSA_memFree(pObj);

	return OSA_SOK;

}


int DRV_audioSetVolume(DRV_AudioHndl *hndl, Uint32 volumeLevel)
{
	return 0;
}

#define SET_BLKSZ

// Open audio capture device
int DRV_audioOpenRec(DRV_AudioHndl *hndl,DRV_AudioConfig *config)
{
    int format;
    int channels;
    int sampleRate;

    int frag;
    int fragsize;

    hndl->fdRec = open(DRV_AUDIO_RECORD_DEVICE_NAME,O_RDONLY);

    if(hndl->fdRec < 0)
    {
        goto error_exit;
    }

#ifdef SET_BLKSZ
    // AYK - 1224
    frag = (16 << 16) | 13; // 16 buffers of 8192 bytes each  
    if(ioctl(hndl->fdRec, SNDCTL_DSP_SETFRAGMENT, &frag) < 0)
    {
        goto error_exit;
    }
#endif

    // Set the sample size(bits per sample)
    format = config->format;
    if (ioctl(hndl->fdRec,SNDCTL_DSP_SETFMT,&format) < 0)
    {
        goto error_exit;
    }

    // Set the no of channels
    channels = config->numChannels;
    if(ioctl(hndl->fdRec,SNDCTL_DSP_CHANNELS,&channels) < 0)
    {
        goto error_exit;
    }

    // AYK - 1224
    if(ioctl(hndl->fdRec, SNDCTL_DSP_GETBLKSIZE, &fragsize) < 0)
    {
        goto error_exit;
    }  

    OSA_printf(" DRV:REC  blockSize = %d,sampleRate = %d\n",fragsize,config->samplingRate);

    return OSA_SOK;

error_exit:

    if(hndl->fdRec > 0)
    {
        close(hndl->fdRec);
    }

    return OSA_EFAIL;
}

// Open audio playback device
int DRV_audioOpenPlay(DRV_AudioHndl *hndl,DRV_AudioConfig *config)
{
    int format;
    int channels;
    int sampleRate;
    int frag;     // AYK - 1224
    int fragsize; // AYK - 1224

    hndl->fdPlay = open(DRV_AUDIO_PLAY_DEVICE_NAME,O_WRONLY);

    if(hndl->fdPlay < 0)
    {
        goto error_exit;
    }

#ifdef SET_BLKSZ
    // AYK - 1224
    frag = (64 << 16) | 10; // 64 buffers of 1024 bytes each  
    if(ioctl(hndl->fdPlay, SNDCTL_DSP_SETFRAGMENT, &frag) < 0)
    {
        goto error_exit;
    }
#endif

    // Set the sample size(bits per sample)
    format = config->format;
    if (ioctl(hndl->fdPlay,SNDCTL_DSP_SETFMT,&format) < 0)
    {
        goto error_exit;
    }

    // Set the no of channels
    channels = config->numChannels;
    if(ioctl(hndl->fdPlay,SNDCTL_DSP_CHANNELS,&channels) < 0)
    {
        goto error_exit;
    }

    // Set the sample rate
    sampleRate = config->samplingRate;
    if(ioctl(hndl->fdPlay,SNDCTL_DSP_SPEED,&sampleRate) < 0)
    {
        goto error_exit;
    }

    // AYK - 1224
    if(ioctl(hndl->fdPlay, SNDCTL_DSP_GETBLKSIZE, &fragsize) < 0)
    {
        goto error_exit;
    }  

    OSA_printf(" DRV:PLAY blockSize = %d,sampleRate = %d\n",fragsize,config->samplingRate);
 
    return OSA_SOK;

error_exit:

    if(hndl->fdPlay > 0)
    {
        close(hndl->fdPlay);
    }

    return OSA_EFAIL;
}

// Record audio
int DRV_audioRecord(DRV_AudioHndl *hndl,Int8 *recBuf,Uint32 recBufSize)
{
    int ret;

    if((ret = read(hndl->fdRec,(void *)recBuf,recBufSize)) < 0)
    {
        return OSA_EFAIL;
    }

    return OSA_SOK;
}

// Playback audio
int DRV_audioPlay(DRV_AudioHndl *hndl,Int8 *playBuf,Uint32 playBufSize)
{
    int ret;

    if((ret = write(hndl->fdPlay,(void *)playBuf,playBufSize)) < 0)
    {
        return OSA_EFAIL;
    }

    return OSA_SOK;
}

// Reset Playback device
int DRV_audioResetPlay(DRV_AudioHndl *hndl)
{
    // Reset the play device
    if (ioctl(hndl->fdPlay,SNDCTL_DSP_RESET,0) < 0)
    {
	return OSA_EFAIL;
    }
    
    return OSA_SOK;
}

// Change samplerate of the playback device
int DRV_audioSetSampleRatePlay(DRV_AudioHndl *hndl,int sampleRate)
{
    // Reset the play device
    if (ioctl(hndl->fdPlay,SNDCTL_DSP_RESET,0) < 0)
    {
	return OSA_EFAIL;
    }

    // Set the sample rate
    if(ioctl(hndl->fdPlay,SNDCTL_DSP_SPEED,&sampleRate) < 0)
    {
        return OSA_EFAIL;
    }

    return OSA_SOK;
}

// Get the output delay in bytes
int DRV_audioGetODelay(DRV_AudioHndl *hndl)
{
    int delay;

    if(ioctl(hndl->fdPlay,SNDCTL_DSP_GETODELAY,&delay) < 0) 
    {
        return OSA_EFAIL; 
    }    

    return delay;
}

// Close audio record device
int DRV_audioCloseRec(DRV_AudioHndl *hndl)
{
    // Reset the capture device
    if (ioctl(hndl->fdRec,SNDCTL_DSP_RESET,0) < 0)
    {
	OSA_printf(" Error:SNDCTL_DSP_RESET failed\n");
    }

    if(hndl->fdRec > 0)
    {
        close(hndl->fdRec);
    }

    return OSA_SOK;
}

// Close audio playback device
int DRV_audioClosePlay(DRV_AudioHndl *hndl)
{
    // Reset the play device
     if (ioctl(hndl->fdPlay,SNDCTL_DSP_RESET,0) < 0)
     {
         OSA_printf(" Error:SNDCTL_DSP_RESET failed\n");
     }

     if(hndl->fdPlay > 0)
     {
         close(hndl->fdPlay);
     }

    return OSA_SOK;
}


