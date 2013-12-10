
#ifndef _ALG_H_
#define _ALG_H_

#include <osa.h>
#include <drv.h>

//#define ALG_DEBUG

#define ALG_VID_DATA_FORMAT_YUV422    (DRV_DATA_FORMAT_YUV422)
#define ALG_VID_DATA_FORMAT_YUV420    (DRV_DATA_FORMAT_YUV420)

#define ALG_VID_DATA_FORMAT_PROGRESSIVE  (0<<4)
#define ALG_VID_DATA_FORMAT_INTERLACED   (1<<4)

typedef enum{
	ALG_VID_CODEC_H264=0,
	ALG_VID_CODEC_MPEG4,
	ALG_VID_CODEC_MJPEG,
	ALG_VID_CODEC_VNF,
	ALG_VID_CODEC_NONE=-1
} ALG_VID_CODEC_TYPE;

#define ALG_AUD_CODEC_G711     0


int ALG_sysInit();
int ALG_sysExit();


#endif




