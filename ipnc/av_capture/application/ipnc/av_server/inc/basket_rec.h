///////////////////////////////////////////////////////////////////////////////
#ifndef _BASKET_REC_H_
#define _BASKET_REC_H_
///////////////////////////////////////////////////////////////////////////////

/*
 * basket_rec.h
 */

#include <basket.h>

///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C"{
#endif
///////////////////////////////////////////////////////////////////////////////

typedef struct 
{
	int  fps[MAX_CHANNEL];
	int  bps[MAX_CHANNEL];
	long prev_recv_sec[MAX_CHANNEL];
	
	long bid;		    // basket id
	long fpos_bkt;      // basket file position
	long fpos_idx;      // index file position
	long prev_day;
	
	char path_bkt[LEN_PATH];
	char path_idx[LEN_PATH];
	
	long prev_min[MAX_CHANNEL]; // each basket timestamp all channel
	
	T_BASKET_TIME bkt_secs; // each basket timestamp all channel
	T_BASKET_TIME idx_secs; // each index timestamp all channel
	
	char b_buf[BKT_BBUF_SIZE]; // stream buffer
	int  b_pos;
	
	char i_buf[BKT_IBUF_SIZE]; // index buffer
	int  i_pos;
	int  idx_pos[MAX_CHANNEL];
	
	char r_buf[BKT_RBUF_SIZE]; // rdb buffer
	int  r_pos;
	
	char target_path[LEN_PATH];
}BKTREC_CTRL;

/*
 * RETURN VALUE
 *	On success, count of BASKET_FILES is returned. On error, -1 is returned.
 * PARAMETER
 *	path is the pathname of mounted storage(HDD). ex) /mnt/disk1, /mnt/disk2
 */
long BKTREC_CreateBasket(const char *path);

/*
 * RETURN VALUE
 *	On success, count of deleted files, On error, -1 is returned.
 * PARAMETER
 *	path is the pathname of mounted storage(HDD). ex) /mnt/disk1, /mnt/disk2
 */

long BKTREC_deleteBasket(const char *path);

long BKTREC_open(const char *target_mpoint);
long BKTREC_SaveFull();
long BKTREC_exit( int save_flag, int flushbuffer);
long BKTREC_WriteVideoStream(T_VIDEO_REC_PARAM* pv);
long BKTREC_WriteAudioStream(T_AUDIO_REC_PARAM* pa);

void BKTREC_setTargetDisk(const char *path);
int  BKTREC_IsOpenBasket();
long BKTREC_FlushWriteBuffer();




///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
#endif//_BASKET_REC_H_
///////////////////////////////////////////////////////////////////////////////

