#include <avserver.h>
#include <sys/time.h>
#include <osa_cmem.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>



/* The number of channels of the audio codec */
#define NUM_CHANNELS           (1)

/* Number of samples to process at once */
#define NUMSAMPLES             1000

#define RAWBUFSIZE             NUMSAMPLES * 2

#define INPUTBUFSIZE           RAWBUFSIZE

/* The max amount of bytes of speech data to process at once */
#define ENCODEDBUFSIZE         RAWBUFSIZE / 2

Uint32 AUDIO_GetTimeStamp(void)
{
	struct timeval timeval;

	gettimeofday(&timeval, NULL);

	return (timeval.tv_sec * 1000) + (timeval.tv_usec + 500) / 1000;;
}

int AUDIO_audioEncode(int streamId,short *dst, short *src, short bufsize)
{
	int ret = OSA_EFAIL;

	switch( streamId == ALG_AUD_CODEC_G711 )
	{
		default:
		case ALG_AUD_CODEC_G711:
			ret = ALG_ulawEncode(dst, src, bufsize);
		break;
	}

	return ret;
}

int AUDIO_GetClkDiv(void)
{

	int dev_fp;
	void *pMem_map;
	unsigned long phyAddr= 0x01C40000;
	unsigned long length = 0x50;
	unsigned int *pPERI_CLKCTL = NULL;
	int audioClkDiv = -1;

	if ((dev_fp=open("/dev/mem",O_RDWR|O_SYNC))==-1)
	{
		OSA_ERROR("dev_fp Fail!! \n");
		return audioClkDiv;
	}
	pMem_map=mmap(	(void	*)phyAddr,length,
					PROT_READ|PROT_WRITE|PROT_EXEC,MAP_SHARED,
					dev_fp,phyAddr);

	if (pMem_map!=(void *)phyAddr)
	{
		OSA_ERROR("pMem_map Fail!! \n");
		return audioClkDiv;
	}


	pPERI_CLKCTL 		= (unsigned int *)( (char *)pMem_map+0x48);

	audioClkDiv = (((*pPERI_CLKCTL)>>7)&(0x1ff));

	if( pMem_map )
		munmap(pMem_map, length);

	if( dev_fp >= 0)
		close(dev_fp);

	//OSA_printf("audioClkDiv = %d \n", audioClkDiv);

	return (audioClkDiv+1);
}


int AUDIO_GetSampleRate( void )
{
	static int CurrentStatus = -255;
	int fd_proc = -1;
	int ret = -1;
	char StrBuffer[300];
	char TmpBuf[50];
	char *pStr = NULL;
	int getval = 0;
	char delima_buf[] = ":\r\n ";
	int clkdiv = 0;
	double sample_rate = 0;

	if( CurrentStatus >= -1 )
	{
		OSA_ERROR("CurrentStatus is = %d \n", CurrentStatus);
		return CurrentStatus;
	}


	fd_proc = open("/proc/davinci_clocks", O_RDONLY);
	if( fd_proc < 0 )
	{
		OSA_ERROR("GetSampleRate open file fail \n");
		ret = -1;
		goto CHECK_CPU_END;

	}

	ret = read(fd_proc,  StrBuffer, sizeof(StrBuffer)-1);
	if( ret <= 0 )
	{
		OSA_ERROR("read device error !!");
		ret = -1;
		goto CHECK_CPU_END;

	}

	ret = -1;
	StrBuffer[sizeof(StrBuffer)-1] = '\0';
	pStr = strtok(StrBuffer,delima_buf );

	while( pStr != NULL )
	{
		sscanf( pStr,"%s",TmpBuf);

		if( strncmp(TmpBuf, "VOICECODEC_CLK", sizeof("VOICECODEC_CLK")) == 0 )
		{

			pStr = strtok(NULL, delima_buf);

			sscanf( pStr,"%d",&getval);

			//OSA_printf("VOICECODEC_CLK = %d \n", getval);

			ret = getval;
			goto CHECK_CPU_END;


		}

		pStr = strtok(NULL, delima_buf);

	}

CHECK_CPU_END:

	if( fd_proc >= 0 )
		close(fd_proc);

	clkdiv = AUDIO_GetClkDiv();
	if( clkdiv > 0 )
	{
		sample_rate = 1.0/((1.0/((double)ret/(double)clkdiv))*256.0);
	}else{
		OSA_ERROR("Audio get clk div fail!! Set sample rate as default 16000!!!");
		sample_rate = 16000;
	}

	ret = (int)(sample_rate+50)/1000*1000;

	CurrentStatus = ret;

	return ret;
}


int AUDIO_audioTskCreate()
{
	int ret = OSA_EFAIL;
	DRV_AudioConfig Audioconfig;

	Audioconfig.deviceId			= 0;
	Audioconfig.numChannels			= NUM_CHANNELS;
	Audioconfig.samplingRate		= gAVSERVER_config.audioConfig.samplingRate;
	Audioconfig.buff_Of_Samples 	= NUMSAMPLES;

  	ret = DRV_audioOpen(&gAUDIO_ctrl.audioHndl, &Audioconfig);
 	if( ret != OSA_SOK )
 	{
 		return ret;
 	}

	gAUDIO_ctrl.streamId 			= ALG_AUD_CODEC_G711;
	gAUDIO_ctrl.encodedBuffer = (short *)OSA_cmemAlloc(ENCODEDBUFSIZE,32);
	if (gAUDIO_ctrl.encodedBuffer == NULL)
	{
		OSA_ERROR("Failed to allocate encodedBuffer\n");
		return OSA_EFAIL;

	}

  	/* Allocate input buffer */
  	gAUDIO_ctrl.inputBuffer 	= OSA_memAlloc(INPUTBUFSIZE);
	if (gAUDIO_ctrl.inputBuffer == NULL)
	{
		OSA_ERROR("Failed to allocate input buffer\n");
		return OSA_EFAIL;
	}

  return OSA_SOK;
}

int AUDIO_audioTskDelete()
{

	DRV_audioClose(&gAUDIO_ctrl.audioHndl);

	if( gAUDIO_ctrl.inputBuffer )
  {
		OSA_memFree(gAUDIO_ctrl.inputBuffer);
  }


  if( gAUDIO_ctrl.encodedBuffer )
  {
		OSA_cmemFree((Uint8 *)gAUDIO_ctrl.encodedBuffer);
  }

  return OSA_SOK;
}

int AUDIO_audioTskRun()
{
	int			rc = 0;
	int			numBytes = RAWBUFSIZE;
	int     encodedBufferSize = 0;
	AUDIO_BufInfo AudioBufInfo;
	static Uint32	AudioLastTime = 0;
	int AudioInterval =  ((NUMSAMPLES*1000)/gAVSERVER_config.audioConfig.samplingRate);
	int AudioNow =  0;
	int AudioTmp =  0;

	/* Read stereo data from codec */
	rc = DRV_audioRead(&gAUDIO_ctrl.audioHndl, gAUDIO_ctrl.inputBuffer, NUMSAMPLES);
	if (rc == 0)
	{
		/* EPIPE means overrun */
		return OSA_SOK;
	}
	else if (rc < 0)
	{
		return OSA_EFAIL;
	}
	else if (rc != (int)NUMSAMPLES)
	{
		OSA_ERROR("short read, read %d frames\n",rc);
		numBytes = rc<<1;
	}
	else
	{
		numBytes = RAWBUFSIZE;
	}
	if( AudioLastTime == 0 )
	{
		AudioNow =  AudioLastTime = AUDIO_GetTimeStamp();

	}else{
		AudioNow = AUDIO_GetTimeStamp();
		if( (AudioNow - AudioLastTime) < AudioInterval )
		{

			if( (AudioNow - AudioLastTime) < 10 )
			{
				return OSA_SOK;
			}
			AudioTmp = AudioNow;
			AudioNow = AudioLastTime+AudioInterval;
			AudioLastTime = AudioTmp;
		}else{
			AudioLastTime = AudioNow;
		}
	}


	AudioBufInfo.timestamp	 =  AudioNow;

	encodedBufferSize = AUDIO_audioEncode(gAUDIO_ctrl.streamId,
										gAUDIO_ctrl.encodedBuffer,
										(short *)gAUDIO_ctrl.inputBuffer,numBytes);

	if( encodedBufferSize <= 0 )
	{
		OSA_ERROR("Audio encode error !!\n");
		return OSA_EFAIL;
	}

	AudioBufInfo.encFrameSize = encodedBufferSize;
	AudioBufInfo.virtAddr			= gAUDIO_ctrl.encodedBuffer;

	AUDIO_streamShmCopy(gAUDIO_ctrl.streamId, &AudioBufInfo);

  return OSA_SOK;
}

int AUDIO_audioTskMain(struct OSA_TskHndl *pTsk, OSA_MsgHndl *pMsg, Uint32 curState )
{
  int status;
  Bool done=FALSE, ackMsg = FALSE;
  Uint16 cmd = OSA_msgGetCmd(pMsg);

  #ifdef AVSERVER_DEBUG_AUDIO_THR
  OSA_printf(" AUDIO: Recevied CMD = 0x%04x\n", cmd);
  #endif

  if(cmd!=AVSERVER_CMD_CREATE) {
    OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
    return OSA_SOK;
  }

  #ifdef AVSERVER_DEBUG_AUDIO_THR
  OSA_printf(" AUDIO: Create...\n");
  #endif

  status = AUDIO_audioTskCreate();

  OSA_tskAckOrFreeMsg(pMsg, status);

  if(status!=OSA_SOK) {
    OSA_ERROR("AUDIO_audioTskCreate()\n");
    return OSA_SOK;
  }

  #ifdef AVSERVER_DEBUG_AUDIO_THR
  OSA_printf(" AUDIO: Create...DONE\n");
  #endif

  status = OSA_tskWaitMsg(pTsk, &pMsg);
  if(status!=OSA_SOK)
    return OSA_SOK;

  cmd = OSA_msgGetCmd(pMsg);

  if(cmd==AVSERVER_CMD_DELETE) {

    done = TRUE;
    ackMsg = TRUE;

  } else {

    #ifdef AVSERVER_DEBUG_AUDIO_THR
    OSA_printf(" AUDIO: Start...\n");
    #endif

    OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
  }

  #ifdef AVSERVER_DEBUG_AUDIO_THR
  OSA_printf(" AUDIO: Start...DONE\n");
  #endif

  while(!done) {

    status = AUDIO_audioTskRun();

    if(status!=OSA_SOK) {
      OSA_ERROR("AUDIO_audioTskRun()\n");
      break;
    }

    status = OSA_tskCheckMsg(pTsk, &pMsg);

    if(status!=OSA_SOK)
      continue;

    #ifdef AVSERVER_DEBUG_AUDIO_THR
    OSA_printf(" AUDIO: Received CMD = 0x%04x\n", cmd);
    #endif

    cmd = OSA_msgGetCmd(pMsg);

    switch(cmd) {
      case AVSERVER_CMD_DELETE:
        done = TRUE;
        ackMsg = TRUE;
        break;

      default:

        #ifdef AVSERVER_DEBUG_AUDIO_THR
        OSA_printf(" AUDIO: Unknown CMD = 0x%04x\n", cmd);
        #endif

        OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);
        break;
    }
  }

  #ifdef AVSERVER_DEBUG_AUDIO_THR
  OSA_printf(" AUDIO: Delete...\n");
  #endif

  AUDIO_audioTskDelete();

  if(ackMsg)
    OSA_tskAckOrFreeMsg(pMsg, OSA_SOK);

  #ifdef AVSERVER_DEBUG_AUDIO_THR
  OSA_printf(" AUDIO: Delete...DONE\n");
  #endif

  return OSA_SOK;
}

int AUDIO_audioCreate()
{
  int status;

  status = OSA_tskCreate( &gAUDIO_ctrl.audioTsk, AUDIO_audioTskMain, AUDIO_CAPTURE_THR_PRI, AUDIO_CAPTURE_STACK_SIZE, 0);
  if(status!=OSA_SOK) {
    OSA_ERROR("OSA_tskCreate()\n");
    return status;
  }

  return status;
}

int AUDIO_audioDelete()
{
  int status;

  status = OSA_tskDelete( &gAUDIO_ctrl.audioTsk );

  return status;
}
