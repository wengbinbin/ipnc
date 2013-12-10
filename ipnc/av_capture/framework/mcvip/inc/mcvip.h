/*
  (c) Texas Instruments 2008
*/

#ifndef _MCVIP_H_
#define _MCVIP_H_

#include <osa.h>

/**
  \file mcvip.h
  \brief Multi-Channel Video Input Port (MCVIP)
*/

#define MCVIP_SOK         (0)     ///< Status: Success
#define MCVIP_EFAIL       (-1)    ///< Status: Failure

									// DBian adjust to support 8cif+D1 mode
#define MCVIP_CHANNELS_MAX    (9)    ///< Max channels that can be captured over one video input port 

#define MCVIP_VIDEO_INPUT_PORT_0    0   ///< Video Input Port : 0
#define MCVIP_VIDEO_INPUT_PORT_1    1   ///< Video Input Port : 1
#ifdef 	VANLINK_DVR_DM365
#define MCVIP_VIDEO_INPUT_PORT_MAX  1   ///< Video Input Port : Max
#else
#define MCVIP_VIDEO_INPUT_PORT_MAX  2   ///< Video Input Port : Max
#endif
#define MCVIP_CAPTURE_THR_PRI_LOW   (50)   ///< Capture Thread Priority: Low
#define MCVIP_CAPTURE_THR_PRI_MED   (60)  ///< Capture Thread Priority: Medium
#define MCVIP_CAPTURE_THR_PRI_HIGH  (90) ///< Capture Thread Priority: High (recommended)

#define MCVIP_VIDEO_DECODER_ID_TVP5158 (0)  ///< Video Decoder ID: TVP5158
#ifdef 	VANLINK_DVR_DM365
#define MCVIP_TVP5158_MAX_CASCADE      (1)  ///< TVP5158: Max cascade stages
#else
#define MCVIP_TVP5158_MAX_CASCADE      (2)  ///< TVP5158: Max cascade stages
#endif

#define MCVIP_VIDEO_DECODER_MODE_2CH_D1_PORT_A        0   ///< Video decoder mode: 2CH D1 via video decoder port A
#define MCVIP_VIDEO_DECODER_MODE_2CH_D1_PORT_B        1   ///< Video decoder mode: 2CH D1 via video decoder port B
#define MCVIP_VIDEO_DECODER_MODE_4CH_D1               2   ///< Video decoder mode: 4CH D1
#define MCVIP_VIDEO_DECODER_MODE_4CH_HALF_D1          3   ///< Video decoder mode: 4CH Half-D1
#define MCVIP_VIDEO_DECODER_MODE_4CH_CIF              4   ///< Video decoder mode: 4CH CIF
#define MCVIP_VIDEO_DECODER_MODE_8CH_HALF_D1          5   ///< Video decoder mode: 8CH Half-D1
#define MCVIP_VIDEO_DECODER_MODE_8CH_CIF              6   ///< Video decoder mode: 8CH CIF
#define MCVIP_VIDEO_DECODER_MODE_4CH_HALF_D1_PLUS_D1  7   ///< Video decoder mode: 4CH Half-D1 + D1
#define MCVIP_VIDEO_DECODER_MODE_4CH_CIF_PLUS_D1      8   ///< Video decoder mode: 4CH CIF     + D1
#define MCVIP_VIDEO_DECODER_MODE_8CH_CIF_PLUS_D1      9   ///< Video decoder mode: 8CH CIF     + D1
#define MCVIP_VIDEO_DECODER_MODE_MAX                 10   ///< Video decoder mode: Max

#define MCVIP_VIDEO_DECODER_MODE_2CH_D1_2CH_CIF		 11	 ///<Video decoder mode: 2CH D1 via video decoder port A and 2CH CIF Streaming. //sk_cif
#define MCVIP_VIDEO_DECODER_MODE_4CH_D1_4CH_CIF_A	 12  ///<Video decoder mode: 4CH D1 and 4CH CIF streaming
#define MCVIP_VIDEO_DECODER_MODE_4CH_D1_4CH_CIF_B	 13  ///<Video decoder mode: 4CH Half D1 and 4CH CIF streaming

   
#define MCVIP_VIDEO_IF_MODE_BT656   0   ///< Video interface mode: BT656
#define MCVIP_VIDEO_IF_MODE_BT1120  1   ///< Video interface mode: BT1120

#define MCVIP_VIDEO_SYSTEM_NTSC    0    ///< Video System: NTSC
#define MCVIP_VIDEO_SYSTEM_PAL     1    ///< Video System: PAL

#define MCVIP_BUF_PER_CH_MIN       4    ///< Minimum number of buffers per channel that are needed

#define MCVIP_BUF_MAX     (MCVIP_CHANNELS_MAX*MCVIP_BUF_PER_CH_MIN)    ///< Max number of buffer's that can be specified by application

#define MCVIP_FLAG_ENCODER_DONE   (0x0001)          ///< MCVIP Flag: Encoder buffer usage is done
#define MCVIP_FLAG_DISPLAY_DONE   (0x0002)          ///< MCVIP Flag: Display buffer usage is done
#define MCVIP_FLAG_ALL_DONE       (MCVIP_FLAG_ENCODER_DONE|MCVIP_FLAG_DISPLAY_DONE) ///< MCVIP Flag: All buffer usage is done

/**
  \brief Create parameters
*/
typedef struct {

  int videoInputPort;     ///< [I ] video input port ID, MCVIP_VIDEO_INPUT_PORT_0 or MCVIP_VIDEO_INPUT_PORT_1
  int captureThrPri;      ///< [I ] capture thread priority, 0 to 127
  int videoDecoderId;     ///< [I ] Video decoder ID, MCVIP_VIDEO_DECODER_ID_xxx
  int videoDecoderMode;   ///< [I ] Video decoder mode, MCVIP_VIDEO_DECODER_MODE_xxx
  int videoIfMode;        ///< [I ] Video interface mode, MCVIP_VIDEO_IF_MODE_BTxxx
  int videoSystem;        ///< [I ] Video system, MCVIP_VIDEO_SYSTEM_xxx

	int	videoBrightness[MCVIP_CHANNELS_MAX];		///< [I ] Video Brightness 0 ~ 255;
	int videoContrast[MCVIP_CHANNELS_MAX];			///< [I ] Video Contrast 0 ~ 255;
	int videoSaturation[MCVIP_CHANNELS_MAX];		///< [I ]	Video Saturation 0 ~ 255;
	int videoHue[MCVIP_CHANNELS_MAX];						///< [I ] Video Hue	-180 ~ 180;
	
  int numBuf;             ///< [I ] Number of buffer's to use for capture, must be >= (number of channels*3)
  int bufSize;            ///< [I ] Size of each buffer, must >= ROUND(width, 32)*height*2
  Uint8 *bufPhysAddr[MCVIP_BUF_MAX]; ///< [I ] Physical buffer address of each buffer, must be 32byte aligned
  Uint8 *bufVirtAddr[MCVIP_BUF_MAX]; ///< [I ] Virtual buffer address of each buffer, must be 32byte aligned

  // AYK - 0826
  // Audio parameters
  int format;
  int numChannels;
  int samplingRate;
  int enablePlayback;
  Uint8 audVol[MCVIP_CHANNELS_MAX];
  Uint8	audMute[MCVIP_CHANNELS_MAX];

} MCVIP_CreatePrm;


/**
  \brief Buffer info
*/
typedef struct {

  int chId;       ///< [ O] Channel ID (0..MCVIP_CHANNELS_MAX-1)
  int flags;      ///< [ O] Used internally, SHOULD NOT BE MODIFIED BY USER

  unsigned char *physAddr; ///< [ O] Buffer physical address
  unsigned char *virtAddr; ///< [ O] Buffer physical address

  int timestamp;  ///< [ O] Buffer timestamp in usecs
  int timesec;    ///< [ O] Buffer timestamp in sec

} MCVIP_BufInfo;


/**
  \brief Channel specific info
*/
typedef struct {

  int width;          ///< [ O] data width, in pixels
  int height;         ///< [ O] data height, in lines
  int offsetH;        ///< [ O] horizontal line offset, in pixels
  int offsetV;        ///< [ O] vertical line offset, in lines, ( chroma data offset = lineOffsetH*lineOffsetV bytes)

} MCVIP_ChInfo;

/**
  \brief Channel list
*/
typedef struct {

  int numCh;                               ///< [ O] Number of channels (0..MCVIP_CHANNELS_MAX-1)
  MCVIP_ChInfo  info[MCVIP_CHANNELS_MAX];  ///< [ O] Channel specific info

} MCVIP_ChList;

/**
  \brief DMA data copy API parameters
*/
typedef struct {

  Uint8 *srcPhysAddr; ///< if NULL, do virtual to physical translation inside API
  Uint8 *srcVirtAddr; ///< if NULL, fill destination buffer with 'blank' color

  Uint8 *dstPhysAddr; ///< if NULL, do virtual to physical translation inside API
  Uint8 *dstVirtAddr; ///< cannot be NULL
  
  int srcOffsetH;    ///< Source Line Offset H, in bytes
  int srcOffsetV;    ///< Source Line Offset V, in lines, chorma offset is srcOffsetH*srcOffsetV

  int dstOffsetH;    ///< Source Line Offset H, in bytes
  int dstOffsetV;    ///< Source Line Offset V, in lines, chorma offset is dstOffsetH*dstOffsetV

  int copyWidth;     ///< in bytes
  int copyHeight;    ///< in lines
  
  int fillValueY;    ///<
  int fillValueC;    ///<    
 
} MCVIP_DmaPrm;

/**
  \brief One time MCVIP system init
  
  \param devAddr    [I ] I2C device address's of video decoder's connected to different input ports
  
  \return OSA_SOK on sucess else failure
*/
int MCVIP_init(int devAddr[]);

/**
  \brief MCVIP system exit
  
  \return OSA_SOK on sucess else failure
*/
int MCVIP_exit();

/**
  \brief Get the detected video standard of TVP5158
  
  \return Video standard on sucess else -1
*/
int MCVIP_getDetectedVideoSystem();

/**
  \brief Create MCVIP

  - Creates a thread for capturing data and required resources specific to a video input port
  - Should be called for each HW Video Input port, with appropiate input port ID

  \param prm  [I ] Create parameters

  \return NULL on error, else MCVIP handle
*/
void *MCVIP_create(MCVIP_CreatePrm *prm);

/**
  \brief Delete MCVIP

  - Delete's the thread and other related resources

  \return 0 on success else failure
*/
int MCVIP_delete(void *hndl);

/**
  \brief Get channel specific info

  \param hndl     [I ] Handle (created using MCVIP_create() )
  \param chInfo   [ O] Channel info

  \return 0 on sucess, else failure
*/
int MCVIP_getChList(void *hndl, MCVIP_ChList *chInfo);

/**
  \brief Start capture

  \param hndl     [I ] Handle (created using MCVIP_create() )

  \return 0 on sucess, else failure
*/
int MCVIP_start(void *hndl,int sampleRate);

/**
  \brief Stop capture

  \param hndl     [I ] Handle (created using MCVIP_create() )

  \return 0 on sucess, else failure
*/
int MCVIP_stop(void *hndl);

/**
  \brief Get a demuxed frame buffer

  - This function returns a buffer ID, to get buffer info use the API MCVIP_getBufInfo() with this ID as parameter

  \param hndl     [I ] Handle (created using MCVIP_create() )
  \param id       [ O] buffer ID
  \param timeout  [I ] Timeout (see osa.h)

  \return 0 on sucess, else failure
*/
int MCVIP_getBuf(void *hndl, int *id, unsigned int timeout);

/**
  \brief Release a demuxed frame buffer

  There are two tasks that could use the captured buffer
  1. For encoding
  2. For display
  
  Since these tasks can happen asyncronously with each other, the capture buffer should be released by user only when both
  encoding and display of the buffer is done.
  
  To do this user should set 'flags' parameter appropiately when calling this API
  
  Set flags according to the task that has completed using this buffer
  i.e if display task is done using the buffer set flag to MCVIP_FLAG_DISPLAY_DONE
      if encoder task is done using the buffer set flag to MCVIP_FLAG_ENCODER_DONE
  Only when both MCVIP_FLAG_DISPLAY_DONE, MCVIP_FLAG_ENCODER_DONE are done is the buffer released   
  

  \param hndl     [I ] Handle (created using MCVIP_create() )
  \param id       [I ] buffer ID
  \param flags    [I ] MCVIP_FLAG_ENCODER_DONE or MCVIP_FLAG_DISPLAY_DONE
  
  \return 0 on sucess, else failure
*/
int MCVIP_putBuf(void *hndl, int id, int flags);

/**
  \brief Get buffer info for a particular buffer ID
  
  \param hndl     [I ] Handle (created using MCVIP_create() )
  \param id       [I ] buffer ID

  \return buffer info on sucess else NULL
*/
MCVIP_BufInfo *MCVIP_getBufInfo(void *hndl, int id);


/**
  \brief Save a captured frame to file
  
  \param hndl     [I ] Handle (created using MCVIP_create() )

  \return 0 on success, else failure
*/
int MCVIP_saveFrame(void *hndl);

/**
  \brief Returns number of video channels that are present in given mode
  
  \param  videoDecoderMode  [I ] Video decoder mode, MCVIP_VIDEO_DECODER_MODE_xxx
  
  \return number of video channels
*/
int MCVIP_getNumCh(int videoDecoderMode);

/**
  \brief Returns size of a video frame buffer for a given mode, video system
  
  \param  videoDecoderMode  [I ] Video decoder mode, MCVIP_VIDEO_DECODER_MODE_xxx
  \param  videoSystem       [I ] Video system, MCVIP_VIDEO_SYSTEM_xxx
  
  \return size of a video frame buffer, in bytes
*/
int MCVIP_getBufSize(int videoDecoderMode, int videoSystem);

int MCVIP_forceWakeup(void *hndl);

/**
  \brief DMA based frame copy/fill utility API
  
  \param prm  [I ] Input parameters
  
  \return 0 on sucess, else failure
*/

typedef struct TAG_TVP5158PATCH {
    int status ;
    pthread_mutex_t mutex ;
} TVP5158PATCH ;

int MCVIP_dmaRun(MCVIP_DmaPrm *prm);

int MCVIP_setColorAdjust(void *hndl, int channel, int enableNR);
int MCVIP_setBrightness(void *hndl, int channel, int brightness);
int MCVIP_setContrast(void *hndl, int channel, int contrast);
int MCVIP_setSaturation(void *hndl, int channel, int saturation);
int MCVIP_setHue(void *hndl, int channel, int hue);

int MCVIP_getTVP5158Status(void *hndl, int *chLockStatus);
int MCVIP_getAVDCtrlStatus01(void *hndl);

// AYK - 0826
// AUDIO APIs
int MCVIP_audioInit(void *hndl,MCVIP_CreatePrm *prm);
int MCVIP_audioRecord(void *hndl,char *recBuf,int recBufSize);
int MCVIP_audioPlayback(void *hndl,char *playBuf,int playBufSize);
int MCVIP_audioSetVol(void *hndl,Uint8 channel,Uint8 vol);
int MCVIP_audioSetMute(void *hndl,Uint8 channel,Uint8 enableMute);
int MCVIP_audioResetPlay(void *hndl);
int MCVIP_audioSetSampleRatePlay(void *hndl,int sampleRate);
int MCVIP_audioGetODelay(void *hndl);
int MCVIP_audioExit(void *hndl);

int MCVIP_setNosizeReduction(void *hndl, int channel, int enable);
int MCVIP_setHybridChSelect(void *hndl, int channel);

#endif


