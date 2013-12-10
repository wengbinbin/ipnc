///////////////////////////////////////////////////////////////////////////////
#ifndef _BASKET_H_
#define _BASKET_H_
///////////////////////////////////////////////////////////////////////////////

/*
 * basket.h
 */


#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//////////////////////////////////////////////////////////////////////////
#define SEP_AUD_TASK
#define PREV_FPOS          // AYK - 0201
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
#define BKT_SYSIO_CALL
//////////////////////////////////////////////////////////////////////////


#ifndef MAX_CHANNEL
#define MAX_CHANNEL 8
#endif

#define BKT_OK 1
#define BKT_FULL 2
#define BKT_NOAUDFR   3   // AYK - 0201
#define BKT_NONEXTBID 4   // AYK - 0201
#define BKT_ERR -1

#define S_OK   1
#define S_FAIL 0

#ifndef BOOL
#define BOOL int
#endif

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#define MAX_NUM_FRAME_SEARCH 64 // AYK - 0201
///////////////////////////////////////////////////////////////////////////////
#define SIZE_KB 1024
#define SIZE_MB (SIZE_KB*SIZE_KB)
#define SIZE_GB (SIZE_MB*SIZE_MB)

#define LEN_PATH   64
#define LEN_TITLE  32
#define BKT_PATH_LEN 64
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// seconds
#define SEC_DAY 86400//3600*24
#define SEC_HOUR 3600 // 60*60
#define SEC_MIN  60
//////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//#define BASKET_SIZE (10*SIZE_MB)
//#define BASKET_SIZE 104857600 // (100*SIZE_MB)
#define BASKET_SIZE 209715200 // (200*SIZE_MB)
//#define BASKET_SIZE 314572800 // (300*SIZE_MB)
//#define BASKET_SIZE 629145600 // (600*SIZE_MB)
///////////////////////////////////////////////////////////////////////////////

#define MAX_TIME   0x7FFFFFFF
#define MAX_TIME64 0x7FFFFFFFFFFFFFFF
///////////////////////////////////////////////////////////////////////////////
// basket structure version
#define VER_UD_BASKET_FORMAT 1

#define BKT_PLAYDIR_BACKWARD 0
#define BKT_PLAYDIR_FORWARD  1

#define BKT_FORWARD 1
#define BKT_BACKWARD 0
#define BKT_TIMEPOINT 2
#define BKT_NOSEARCHDIRECTION 3

#define BKT_PLAYMODE_NORMAL 0
#define BKT_PLAYMODE_IFRAME 1

#define PLAY_STATUS_STOP 0
#define PLAY_STATUS_PLAY 1
#define PLAY_STATUS_PAUSE 2
#define PLAY_STATUS_SKIP  3

// basket status ...
// empty means that remains EMPTY baskets.
// full  means that there is no empty baskets.
#define BKT_SPACE_REMAIN (0)
#define BKT_SPACE_FULL (1)

enum _enumFramType{FT_PFRAME=0, FT_IFRAME, FT_AUDIO};

///////////////////////////////////////////////////////////////////////////////
// basket priority
enum _enumBasketPriority{BP_NONE=0, BP_LOW, BP_NORMAL, BP_HIGH, MAX_BP};
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// stream type 
enum _enumStreamType{ST_NONE=0, ST_VIDEO, ST_AUDIO, MAX_ST};
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// save flag
enum _enumSaveFlag{SF_EMPTY=0, SF_USING, SF_STOP, SF_FULL, SF_ERR, SF_BACKUP, MAX_SF};

// event flag
enum _enumEventFlag{EVT_NONE=0, EVT_CONT=1, EVT_MOT, EVT_SENSOR, EVT_VLOSS};

enum _enumBasketRecycleMode{BKT_RECYCLE_ON=0, BKT_RECYCLE_ONCE};
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// file name
#define DEFAULT_DISK_MPOINT "/dvr/data/sdda"

#define NAME_BKTMGR "bktmgr.udf"
#define NAME_BKTREC_INFO "bkt_rec_info.udf"


///////////////////////////////////////////////////////////////////////////////
// identifier
#define ID_TOPMGR_HEADER   0xA0000001
#define ID_TOPMGR_BODY	   0xA0000002

#define ID_BKTMGR_HEADER   0xB0000001
#define ID_BKTMGR_BODY	   0xB0000002

#define ID_BASKET_HDR      0xC0000001
#define ID_BASKET_BODY     0xC0000002

#define ID_VIDEO_HEADER    0xD0000001
#define ID_VIDEO_STREAM	   0xD0000002

#define ID_AUDIO_HEADER    0xE0000001
#define ID_AUDIO_STREAM	   0xE0000002

#define ID_INDEX_HEADER	   0xF0000001
#define ID_INDEX_DATA	   0xF0000002
#define ID_INDEX_FOOTER    0xF0000004
///////////////////////////////////////////////////////////////////////////////

#define MAX_RECDATE 32

#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif//max

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif//min

#define HI32(h) ((short)((long)(h) >> 16))
#define LO32(l) ((short)((long)(l) & 0xffff))
#define MK32(h, l) ((long)(((short)((long)(l) & 0xffff)) | ((long)((short)((long)(h) & 0xffff))) << 16))

#define HI64(l) ((unsigned long)((int64_t)(l) >> 32))
#define LO64(l) ((unsigned long)((int64_t)(l) & 0xffffffff))
#define MK64(h, l) ((int64_t)(((long)((int64_t)(l) & 0xffffffff)) | ((int64_t)((long)((int64_t)(h) & 0xffffffff))) << 32))


#ifdef BKT_SYSIO_CALL

#define BKT_FHANDLE int


#ifndef O_DIRECT
#define O_DIRECT	0400000
#endif

#define OPEN_CREATE(fd, path) (-1 != (fd = open(path, O_RDWR|O_CREAT|O_TRUNC, 0644)))
#define OPEN_EMPTY(fd, path)  (-1 != (fd = open(path, O_RDWR|O_TRUNC, 0644)))
#define OPEN_RDWR(fd, path)   (-1 != (fd = open(path, O_RDWR, 0644)))
#define OPEN_RDONLY(fd, path) (-1 != (fd = open(path, O_RDONLY, 0644)))
//#define OPEN_RDWR(fd, path)   (-1 != (fd = open(path, O_RDWR|O_NODELAY, 0644)))
//#define OPEN_RDONLY(fd, path) (-1 != (fd = open(path, O_RDONLY|O_NODELAY, 0644)))


//#define OPEN_RDWR(fd, path)   (-1 != (fd = open(path, O_RDWR|O_SYNC, 0644)))

//#define OPEN_RDWR(fd, path)   (-1 != (fd = open(path, O_RDWR|O_DIRECT, 0644)))
//#define OPEN_RDONLY(fd, path) (-1 != (fd = open(path, O_RDONLY|O_DIRECT)))

#define SAFE_CLOSE_BKT(fd) if(fd){ close(fd);fd=0;};
#define CLOSE_BKT(fd) close(fd) //(-1 != close(fd))

#define READ_ST(fd, st) ( 0 < read(fd, &st, sizeof(st)))
#define READ_HDR(fd, st) ( 0 < read(fd, &st, sizeof(st)))
#define READ_STRC(fd, st) read(fd, &st, sizeof(st))
#define READ_PTSZ(fd, pt, sz)   ( 0 < read(fd, pt, sz))

#define WRITE_ST(fd, st) ( 0 < write(fd, &st, sizeof(st)))
#define WRITE_PTSZ(fd, pt, sz) ( 0 < write(fd, pt, sz))

#define LSEEK_END(fd, pos) ( -1 != lseek(fd, (off_t)pos, SEEK_END))
#define LSEEK_CUR(fd, pos) ( -1 != lseek(fd, (off_t)pos, SEEK_CUR))
#define LSEEK_SET(fd, pos) ( -1 != lseek(fd, (off_t)pos, SEEK_SET))

#define LTELL(fd) lseek(fd, (off_t)0, SEEK_CUR)

#else

#define BKT_FHANDLE FILE*

#define OPEN_CREATE(fd, path) (NULL != (fd = fopen(path, "wb+")))
#define OPEN_EMPTY(fd, path) (NULL != (fd = fopen(path, "wb+")))
#define OPEN_RDONLY(fd, path) (NULL != (fd = fopen(path, "rb")))
#define OPEN_RDWR(fd, path) (NULL != (fd = fopen(path, "rb+")))

#define SAFE_CLOSE_BKT(fd) if(fd){ fclose(fd);fd=NULL;};
#define CLOSE_BKT(fd) fclose(fd)

#define READ_ST(fd, st)  ( sizeof(st) == fread(&st, 1, sizeof(st), fd))
#define READ_HDR(fd, st) ( sizeof(st) == fread(&st, 1, sizeof(st), fd))
#define READ_STRC(fd, st) fread(&st, 1, sizeof(st), fd) 
#define READ_PTSZ(fd, pt, size)   ( size == fread(pt, 1, size, fd))

#define WRITE_ST(fd, st) ( sizeof(st) == fwrite(&st, 1, sizeof(st), fd))
#define WRITE_PTSZ(fd, pt, sz)   ( sz == fwrite(pt, 1, sz, fd))

#define LSEEK_END(fd, pos) (-1 != fseek(fd, pos, SEEK_END))
#define LSEEK_CUR(fd, pos) (-1 != fseek(fd, pos, SEEK_CUR))
#define LSEEK_SET(fd, pos) (-1 != fseek(fd, pos, SEEK_SET))

#define LTELL(fd) ftell(fd)

#endif//BKT_SYSIO_CALL
//////////////////////////////////////////////////////////////////////////


#define BKT_STATUS_CLOSED (0)
#define BKT_STATUS_OPENED (1)
#define BKT_STATUS_IDLE (2)
#define BKT_STATUS_FULL (3)

////////////////////////////////////////////////////////////////////////////////


#define SIZEOF_CHAR 1
#define TOTAL_MINUTES_DAY (1440)//24*60
////////////////////////////////////////////////////////////////////////////////

//#pragma pack(push, 1)

typedef struct _tBasketTimeStamp
{
    long sec;	//seconds
	long usec;  //microseconds (1/1000000 seconds)
}T_TIMESTAMP, T_TS;
#define SIZEOF_TIMESTAMP 8

typedef struct _tBasketTime
{
    T_TIMESTAMP t1[MAX_CHANNEL];
    T_TIMESTAMP t2[MAX_CHANNEL];
}T_BASKET_TIME;

typedef struct 
{
	int year;
	int mon;
	int day;
	int hour;
	int min;
	int sec;
}T_BKT_TM;
void BKT_GetLocalTime(long sec, T_BKT_TM* ptm);

//////////////////////////////////////////////////////////////////////////
//#define BKT_BBUF_SIZE 0x200000 //2048*1024
//#define BKT_BBUF_SIZE 0x100000 //1024*1024
#define BKT_BBUF_SIZE 524288 //512*1024
//#define BKT_BBUF_SIZE 393216 //384*1024
//#define BKT_BBUF_SIZE 262144 //256*1024
//#define BKT_BBUF_SIZE 131072 //128*1024
#define BKT_IBUF_SIZE 1024
#define BKT_RBUF_SIZE 1024
//////////////////////////////////////////////////////////////////////////

typedef struct _tBasketRdbParam
{
	int  ch;
	long sec;  // seconds
	int  evt;  // save flag(continuous, motion, alarm )
	long bid;  // basket id
	long idx_pos; // index file pointer
}T_BASKET_RDB_PARAM;
#define SIZEOF_RDB_PRM 20 

typedef struct _tBasketRdb
{
	long bid; // basket id;
	long idx_pos; // index file pointer
}T_BASKET_RDB;
#define SIZEOF_RDB 8

// basket recording information
typedef struct _tBasketRecordInfo
{
	int  save_flag;
	long bid;
	char path[LEN_PATH];
}T_BKTREC_INFO;

typedef struct _tBktMgrHeader
{
    long id;
    long mgrid;	// numbering
    long date;		// seconds since the Epoch
    long latest_update;	// 
    long bkt_count;
    long reserved;
}T_BKTMGR_HDR;

// basket info...stream data...
typedef struct _tBktMgrBody
{
    long id;
    long bid;
    char path[LEN_PATH];
    long latest_update;
    long lPriority;
    long s_type;
	T_BASKET_TIME ts;
    long save_flag;
    long lRecycleCount;
    long reserved;
}T_BKTMGR_BODY;

typedef struct _tBasketHeader
{
    long id;
    long bid;
    long latest_update;
    long lPriority;
    long s_type; // video or audio
	T_BASKET_TIME ts;
    long save_flag;
    long totalframecount;
    long reserved;
}T_BASKET_HDR;


typedef struct _tVideoRecParam
{
    long  ch;
    long  framesize;
    T_TS  ts;

    long  event;
    short frametype;
    short framerate;
    short width;
    short height;
    char* buff;
    char* camName;

    short  audioONOFF; // AYK - 0201
	int   capMode; // BK - 0428 : 2D1, 4CIF, 8CIF, 4D1

}T_VIDEO_REC_PARAM;

typedef struct _tAudioRecParam
{
    long  ch;
    long  framesize;
    T_TS  ts;
    
    long samplingrate;
    long bitspersample;
    long achannel;// stero or mono?
    
    char* buff;
}T_AUDIO_REC_PARAM;

///////////////////////////////////////////////////////////////////////////////
// stream common header
typedef struct _tStreamHeader
{
    long id; // video or audio
    long ch;
    long framesize;
    T_TS ts;
    
    long frametype; // video frame type.i or p frame...

#ifdef PREV_FPOS    // AYK - 0201
    long prevFpos;
#endif

	long reserved1;
	long reserved2;
	long reserved3;
	long reserved4;

}T_STREAM_HDR;

#ifdef PREV_FPOS    // AYK - 0201
    #define SIZEOF_STREAM_HDR (44)
#else
    #define SIZEOF_STREAM_HDR (40)
#endif

///////////////////////////////////////////////////////////////////////////////
// video
typedef struct _tVideoStreamHeader
{
    short audioONOFF;	// 0:No, 1:Yes
    short framerate;
    long  event;		// C, M, S, ...
    short width;
    short height;
    char  camName[16];

	int  capMode;// 2D1, 4CIF, 8CIF, 4D1

	long reserved1;
	long reserved2;

}T_VIDEO_STREAM_HDR;
#define SIZEOF_VSTREAM_HDR (40)

///////////////////////////////////////////////////////////////////////////////
// audio
typedef struct _tAudioStreamHeader
{
    long samplingrate;
    long bitspersample;
    long achannel;// stero or mono?
    long codec;
    
    long  reserved1;
	long  reserved2;
}T_AUDIO_STREAM_HDR;
#define SIZEOF_ASTREAM_HDR (24)


///////////////////////////////////////////////////////////////////////////////
// index
typedef struct _tIndexHeader
{
	long   id;
    long   bid;
    long   count;
	T_BASKET_TIME ts;
    long   reserved;
}T_INDEX_HDR;
#define SIZEOF_INDEX_HDR 144

typedef struct _tIndexData{
	long  id;		// index data id
	T_TS  ts;		// time stamp..sec and usec
	long  fpos;		// file position
	long  ch;
	long  s_type;	// stream type audio, video, etc..
	long  event;
	short width;	// video frame width
	short height;	// video frame height

	int   capMode;

    long  reserved1;
	long  reserved2;

}T_INDEX_DATA;
#define SIZEOF_INDEX_DATA 44

//#pragma pack(pop)

int  alphasort_reverse(const void *a, const void *b);

// search minimum tiem stamp of array time stamp
long BKT_GetMinTS(T_TIMESTAMP *arr_t);
long BKT_GetMaxTS(T_TIMESTAMP *arr_t);

////////////////////////////////////////////////////////////////////////////////
// search ....outer interfaces 


/*
 * RETURN VALUE
 *
 * PARAMETER
 *	
 */

// 0:on, 1:once

#define BKT_REC_CLOSE_SLIENT 0
#define BKT_REC_CLOSE_BUF_FLUSH 1

long BKT_closeAll();

int  BKT_SetRecycleMode(int recycle);


// BK - End

inline long BKT_GetRecordingIndexCount();
inline long BKT_transIndexPosToNum(long fpos);
inline long BKT_transIndexNumToPos(int index_number);

//////////////////////////////////////////////////////////////////////////
// recording interfaces
int BKT_isRecordingBasket(long bid);
//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif


///////////////////////////////////////////////////////////////////////////////
#endif//_BASKET_H_
///////////////////////////////////////////////////////////////////////////////

