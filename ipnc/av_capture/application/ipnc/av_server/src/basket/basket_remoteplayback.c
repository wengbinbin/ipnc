/*
* basket_remoteplayback.c
* 2010.4.8
*/

#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>

#include <unistd.h>
#include <sys/time.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <basket_remoteplayback.h>

//////////////////////////////////////////////////////////////////////////
#define BKTRMP_DEBUG

#ifdef BKTRMP_DEBUG
#define TRACE0_BKTRMP(msg, args...) fprintf(stderr, "[BKTRMP] - " msg, ##args)
#define TRACE1_BKTRMP(msg, args...) fprintf(stderr, "[BKTRMP] - %s(%d):" msg,  __FUNCTION__, __LINE__, ##args)
#define TRACE2_BKTRMP(msg, args...) fprintf(stderr, "[BKTRMP] - %s(%d):\t%s:" msg, __FILE__, __LINE__, __FUNCTION__, ##args)
#else
#define TRACE0_BKTRMP(msg, args...) ((void)0)
#define TRACE1_BKTRMP(msg, args...) ((void)0)
#define TRACE2_BKTRMP(msg, args...) ((void)0)
#endif
//////////////////////////////////////////////////////////////////////////

int  BKTRMP_Open(T_BKTRMP_CTRL* pRmpCtrl, long beginsec)
{
	BKTRMP_Close(pRmpCtrl);

	T_BKT_TM tm1;
	BKT_GetLocalTime(beginsec, &tm1);

#ifdef BKT_SYSIO_CALL
	int fd;
#else
	FILE *fd;
#endif

	char path[LEN_PATH];
	T_INDEX_HDR ihd;

	char evts[TOTAL_MINUTES_DAY*MAX_CHANNEL];
	T_BASKET_RDB rdbs[TOTAL_MINUTES_DAY*MAX_CHANNEL];

	long bid  = 0;
	long ipos = 0;

	int size_evt = TOTAL_MINUTES_DAY*MAX_CHANNEL;
	int ch, i, firstCh;
      
	sprintf(path, "%s/rdb/%04d%02d%02d", pRmpCtrl->target_path, tm1.year, tm1.mon, tm1.day);

	if(!OPEN_RDONLY(fd, path))
	{
		TRACE1_BKTRMP("failed open rdb %s\n", path);
		return BKT_ERR;
	}

	TRACE0_BKTRMP("opened rdb file for remote playback. %s\n", path);

	if(!READ_PTSZ(fd, evts, sizeof(evts)))
	{
		TRACE1_BKTRMP("failed read evts.\n");
		CLOSE_BKT(fd);
		return BKT_ERR;
	}

	// read rdb(bid, pos)
	if(!READ_PTSZ(fd, rdbs, sizeof(rdbs)))
 	{
 		TRACE1_BKTRMP("failed read rdb data\n");
 		CLOSE_BKT(fd);
 		return BKT_ERR;
	}
	CLOSE_BKT(fd); // done read rec info.

	int bFound=0;
	bid = 0;
	for(i = tm1.hour*60+tm1.min; i < TOTAL_MINUTES_DAY ; i++)
	{
		for( ch = 0 ; ch < MAX_CHANNEL ; ch++)
		{
			if(evts[TOTAL_MINUTES_DAY*ch+i] != 0 && rdbs[TOTAL_MINUTES_DAY*ch+i].bid != 0)
			{
				if(bFound==0)
					bFound = 1;
				
				if(bid==0)
				{
					bid  = rdbs[TOTAL_MINUTES_DAY*ch+i].bid;
					ipos = rdbs[TOTAL_MINUTES_DAY*ch+i].idx_pos;
				}
				else
				{
					// rarely appears
					if(bid != rdbs[TOTAL_MINUTES_DAY*ch+i].bid)
						break;
					else
					{
						ipos = min(ipos, rdbs[TOTAL_MINUTES_DAY*ch+i].idx_pos);
					}
				}
				
				TRACE0_BKTRMP("found start rec time. CH:%d, %02d:%02d, BID:%ld, idx_pos:%ld\n", ch, i/60, i%60, rdbs[TOTAL_MINUTES_DAY*ch+i].bid, rdbs[TOTAL_MINUTES_DAY*ch+i].idx_pos);
			}
		}
		
		if(bFound)
			break;
	}
	
	if(bFound == 0)
	{
		TRACE0_BKTRMP("failed find start record time. path:%s, ts:%04d/%02d/%02d %02d:%02d:%02d\n", path, tm1.year, tm1.mon, tm1.day, tm1.hour, tm1.min, tm1.sec);
		return BKT_ERR;
	}
	
	// open basket
	sprintf(path, "%s/%08ld.bkt", pRmpCtrl->target_path, bid);
	if(!OPEN_RDONLY(pRmpCtrl->hbkt, path))
	{
		TRACE1_BKTRMP("failed open basket file for decoding -- path:%s\n", path);
		return BKT_ERR;
	}

	// open index
	sprintf(path, "%s/%08ld.idx", pRmpCtrl->target_path, bid);
	if(!OPEN_RDONLY(pRmpCtrl->hidx, path))
	{
		TRACE1_BKTRMP("failed open index file for decoding -- path:%s\n", path);
		CLOSE_BKT(pRmpCtrl->hbkt);
		return BKT_ERR;
	}
	
	// read index header
	if(!READ_ST(pRmpCtrl->hidx, ihd))
	{
		TRACE1_BKTRMP("failed read index hdr\n");
		BKTRMP_Close(pRmpCtrl);
		return BKT_ERR;
	}
	
	// calculate i-frame count
	pRmpCtrl->iframe_cnt = BKT_isRecordingBasket(bid)? BKT_GetRecordingIndexCount():ihd.count;
	
	// set index file pointer to begin
	if(!LSEEK_SET(pRmpCtrl->hidx, ipos))
	{
		TRACE1_BKTRMP("failed seek index.\n");
		BKTRMP_Close(pRmpCtrl);
		return BKT_ERR;
	}

	long idx_pos=ipos;
	long bkt_pos=sizeof(T_BASKET_HDR);
	T_INDEX_DATA idd;
	if(!READ_ST(pRmpCtrl->hidx, idd))
	{
		TRACE1_BKTRMP("failed read index data. firstCh=%d, idd.id=0x%08x, idd.ch=%d\n", firstCh, idd.id, idd.ch);
		BKTRMP_Close(pRmpCtrl);
		return BKT_ERR;
	}
	bkt_pos = idd.fpos;

	// set index file pointer to begin
	if(!LSEEK_SET(pRmpCtrl->hidx, ipos))
	{
		TRACE1_BKTRMP("failed seek index.\n");
		BKTRMP_Close(pRmpCtrl);
		return BKT_ERR;
	}

	/*
	// search basket file point with found index file pointer.
	long idx_pos=ipos;
	long bkt_pos=sizeof(T_BASKET_HDR);
	while(1)
	{
		if(!READ_ST(pRmpCtrl->hidx, idd))
		{
			TRACE1_BKTRMP("failed read index data. firstCh=%d, idd.id=0x%08x, idd.ch=%d\n", firstCh, idd.id, idd.ch);
			BKTRMP_Close(pRmpCtrl);
			return BKT_ERR;
		}
		
		if( idd.ch == firstCh )
		{
			bkt_pos = idd.fpos;
			idx_pos = LTELL(pRmpCtrl->hidx) - sizeof(idd);
			TRACE0_BKTRMP("found index fpos:%ld, saved fpos:%ld\n", idx_pos, ipos);
			break;
		}
	}
	*/

	pRmpCtrl->iframe_cur = BKT_transIndexPosToNum(idx_pos);
	TRACE0_BKTRMP("read index hdr, curiframe:%d, totiframe:%d\n", pRmpCtrl->iframe_cur, pRmpCtrl->iframe_cnt);
	
	if(!LSEEK_SET(pRmpCtrl->hbkt, bkt_pos))
	{
		TRACE1_BKTRMP("failed seek basket:%ld.\n", idd.fpos);
		BKTRMP_Close(pRmpCtrl);
		return BKT_ERR;
	}

	if(!LSEEK_SET(pRmpCtrl->hidx, idx_pos))
	{
		TRACE1_BKTRMP("failed seek index:%ld.\n", idx_pos);
		BKTRMP_Close(pRmpCtrl);
		return BKT_ERR;
	}
	
	pRmpCtrl->bid = bid;
	pRmpCtrl->sec = idd.ts.sec;
	pRmpCtrl->status = BKTRMP_STATUS_OPEN;

	TRACE0_BKTRMP("open basket file for streaming -- CH:%ld, BID:%ld, B-POS:%ld, I-POS:%ld \n", idd.ch, bid, bkt_pos, idx_pos);

	return BKT_OK;
}

int  BKTRMP_GetRecDayData(T_BKTRMP_CTRL* pRmpCtrl, int year, int mon, char* pRecMonTBL)
{
	int i, bFindOK=0;

	char path[LEN_PATH];
	
	for(i=0;i<31;i++)
	{
		sprintf(path, "%s/rdb/%04d%02d%02d", pRmpCtrl->target_path, year, mon, i+1);

		if(-1 != access(path, 0))
		{
			*(pRecMonTBL+i) = 1;

			if(!bFindOK)
				bFindOK=1;
		}
	}
	
	return bFindOK;
}

int  BKTRMP_GetRecHourData(T_BKTRMP_CTRL* pRmpCtrl, int ch, int year, int mon, int day, char* pRecDayTBL)
{
#ifdef BKT_SYSIO_CALL
	int fd;
#else
	FILE *fd;
#endif

	char path[LEN_PATH];
	sprintf(path, "%s/rdb/%04d%02d%02d", pRmpCtrl->target_path, year, mon, day);

	if(!OPEN_RDONLY(fd, path))
	{
		TRACE1_BKTRMP("failed open rdb %s\n", path);
		return BKT_ERR;
	}

	long offset = TOTAL_MINUTES_DAY*ch;
	
	if(!LSEEK_SET(fd, offset))
	{
		TRACE1_BKTRMP("failed seek rdb.\n");
		CLOSE_BKT(fd);
		return BKT_ERR;
	}

	if(!READ_PTSZ(fd, pRecDayTBL, TOTAL_MINUTES_DAY))
	{
		TRACE1_BKTRMP("failed read rdb CH:%d, offset:%ld\n", ch, offset);
		CLOSE_BKT(fd);
		return BKT_ERR;
	}
	
	TRACE0_BKTRMP("Succeeded Get record hour info, ch:%d, year:%d, mon:%d, day:%d\n", ch, year, mon, day);
	CLOSE_BKT(fd);
	
	return 0;
	
}

int  BKTRMP_GetRecHourDataAllCh(T_BKTRMP_CTRL* pRmpCtrl, int year, int mon, int day, char* pRecDayTBL)
{
#ifdef BKT_SYSIO_CALL
	int fd;
#else
	FILE *fd;
#endif

	char path[LEN_PATH];
	sprintf(path, "%s/rdb/%04d%02d%02d", pRmpCtrl->target_path, year, mon, day);

	if(!OPEN_RDONLY(fd, path))
	{
		TRACE1_BKTRMP("failed open rdb %s\n", path);
		return BKT_ERR;
	}

	if(!READ_PTSZ(fd, pRecDayTBL, TOTAL_MINUTES_DAY*MAX_CHANNEL))
	{
		TRACE1_BKTRMP("failed read rdb ch all data.\n");
		CLOSE_BKT(fd);
		return BKT_ERR;
	}
	
	TRACE0_BKTRMP("Succeeded Get record hour info all, year:%d, mon:%d, day:%d\n", year, mon, day);
	CLOSE_BKT(fd);
	
	return 0;
}

int  BKTRMP_GetRecHourDataUnion(T_BKTRMP_CTRL* pRmpCtrl, T_BKTRMP_RECHOUR_PARAM* prm)
{
#ifdef BKT_SYSIO_CALL
	int fd;
#else
	FILE *fd;
#endif
	
	int c,m;
	char cRecTblAll[TOTAL_MINUTES_DAY*MAX_CHANNEL];
	char path[LEN_PATH];


	sprintf(path, "%s/rdb/%04d%02d%02d", pRmpCtrl->target_path, prm->year, prm->mon, prm->day);
	
	if(!OPEN_RDONLY(fd, path))
	{
		TRACE1_BKTRMP("failed open rdb %s\n", path);
		return BKT_ERR;
	}
	
	if(!READ_PTSZ(fd, cRecTblAll, TOTAL_MINUTES_DAY*MAX_CHANNEL))
	{
		TRACE1_BKTRMP("failed read rdb ch all data.\n");
		CLOSE_BKT(fd);
		return BKT_ERR;
	}
	CLOSE_BKT(fd);

	for(m=0;m<TOTAL_MINUTES_DAY;m++)
	{
		for(c=0;c<MAX_CHANNEL;c++)
		{
			if(cRecTblAll[m+c*TOTAL_MINUTES_DAY] != 0)
			{
				prm->pRecHourTBL[m] = 1;
				//TRACE0_BKTRMP("get rec hour. %02d:%02d\n", m/60,m%60);
				break;
			}
		}
	}
	
	return 0;
}

void BKTRMP_Close(T_BKTRMP_CTRL* pRmpCtrl)
{
	pRmpCtrl->status = BKTRMP_STATUS_CLOSE;

	SAFE_CLOSE_BKT(pRmpCtrl->hbkt);
	SAFE_CLOSE_BKT(pRmpCtrl->hidx);
	
	TRACE0_BKTRMP("close basket for remote playback\n");
}

int  BKTRMP_OpenContinueNext(T_BKTRMP_CTRL* pRmpCtrl)
{
	// get next basket id
	long nextbid = BKTRMP_SearchNextBID(pRmpCtrl);
	
	if( nextbid == BKT_ERR )
		return BKT_ERR;

	BKTRMP_Close(pRmpCtrl);
	
	char path[LEN_PATH];
	sprintf(path, "%s/%08ld.bkt", pRmpCtrl->target_path, nextbid);

	// open basket
	if(!OPEN_RDONLY(pRmpCtrl->hbkt, path))
	{
		TRACE1_BKTRMP("failed open basket file -- path:%s\n", path);
		return BKT_ERR;
	}

	// open index
	sprintf(path, "%s/%08ld.idx", pRmpCtrl->target_path, nextbid);
	if(!OPEN_RDONLY(pRmpCtrl->hidx, path))
	{
		TRACE1_BKTRMP("failed open index file -- path:%s\n", path);
		CLOSE_BKT(pRmpCtrl->hbkt);
		return BKT_ERR;
	}
	
	T_INDEX_HDR ihd;
	if(!READ_ST(pRmpCtrl->hidx, ihd))
	{
		TRACE1_BKTRMP("failed read index hdr\n");
		BKTRMP_Close(pRmpCtrl);
		return BKT_ERR;
	}

	if(!LSEEK_SET(pRmpCtrl->hbkt, sizeof(T_BASKET_HDR)))
	{
		TRACE1_BKTRMP("failed seek basket.\n");
		BKTRMP_Close(pRmpCtrl);
		return BKT_ERR;
	}

	pRmpCtrl->bid = nextbid;
	pRmpCtrl->iframe_cnt = BKT_isRecordingBasket(nextbid)? BKT_GetRecordingIndexCount():ihd.count;
	pRmpCtrl->iframe_cur = 0;

	pRmpCtrl->sec = BKT_GetMinTS(ihd.ts.t1);
	pRmpCtrl->status = BKTRMP_STATUS_OPEN;
	pRmpCtrl->fpos_bkt = LTELL(pRmpCtrl->hbkt);

	TRACE0_BKTRMP("open next basket. BID:%ld, iframe-cur:%d, iframe_total:%d, sec:%ld\n", nextbid, pRmpCtrl->iframe_cur, pRmpCtrl->iframe_cnt, pRmpCtrl->sec);
	
	return BKT_OK;
}

int  BKTRMP_OpenContinuePrev(T_BKTRMP_CTRL* pRmpCtrl)
{
	// get previous basket id
	long prevbid = BKTRMP_SearchPrevBID(pRmpCtrl);
	
	if( prevbid == BKT_ERR )
		return BKT_ERR;

	BKTRMP_Close(pRmpCtrl);
	
	char path[LEN_PATH];
	sprintf(path, "%s/%08ld.bkt", pRmpCtrl->target_path, prevbid);

	// open basket
	if(!OPEN_RDONLY(pRmpCtrl->hbkt, path))
	{
		TRACE1_BKTRMP("failed open basket file -- path:%s\n", path);
		return BKT_ERR;
	}

	// open index
	sprintf(path, "%s/%08ld.idx", pRmpCtrl->target_path, prevbid);
	if(!OPEN_RDONLY(pRmpCtrl->hidx, path))
	{
		TRACE1_BKTRMP("failed open index file -- path:%s\n", path);
		CLOSE_BKT(pRmpCtrl->hbkt);
		return BKT_ERR;
	}
	
	T_INDEX_HDR ihd;
	if(!READ_ST(pRmpCtrl->hidx, ihd))
	{
		TRACE1_BKTRMP("failed read index hdr\n");
		BKTRMP_Close(pRmpCtrl);
		return BKT_ERR;
	}

	if(ihd.count != 0)
	{
		if(!LSEEK_CUR(pRmpCtrl->hidx, sizeof(T_INDEX_DATA)*(ihd.count-1)))
		{
			TRACE1_BKTRMP("failed seek index.\n");
			BKTRMP_Close(pRmpCtrl);
			return BKT_ERR;
		}
		T_INDEX_DATA idd;
		if(!READ_ST(pRmpCtrl->hidx, idd))
		{
			TRACE1_BKTRMP("failed read index data\n");
			BKTRMP_Close(pRmpCtrl);
			return BKT_ERR;
		}

		if(!LSEEK_SET(pRmpCtrl->hbkt, idd.fpos))
		{
			TRACE1_BKTRMP("failed seek basket.\n");
			BKTRMP_Close(pRmpCtrl);
			return BKT_ERR;
		}
	}
	else
	{
		long prev_sec = 0;
		long prev_usec = 0;
		int index_count=0, bkt_fpos=0;
		T_INDEX_DATA idd;
		while(READ_ST(pRmpCtrl->hidx, idd))
		{
			if( (idd.ts.sec <= prev_sec && idd.ts.usec <= prev_usec)
				|| idd.id != ID_INDEX_DATA
				|| idd.fpos <= bkt_fpos)
			{
				TRACE1_BKTRMP("Fount last record index pointer. idd.id=0x%08x, sec=%d, usec=%d, count=%d\n", idd.id, idd.ts.sec, idd.ts.usec, index_count);
				break;
			}
			
			prev_sec  = idd.ts.sec;
			prev_usec = idd.ts.usec;
			bkt_fpos = idd.fpos;
			index_count++;
		}

		if(index_count != 0)
		{
			//////////////////////////////////////////////////////////////////////////
			if(!LSEEK_SET(pRmpCtrl->hbkt, bkt_fpos))
			{
				TRACE1_BKTRMP("failed seek basket.\n");
				BKTRMP_Close(pRmpCtrl);
				return BKT_ERR;
			}
			
			if(!LSEEK_SET(pRmpCtrl->hidx, sizeof(idd)*index_count))
			{
				TRACE1_BKTRMP("failed seek index.\n");
				BKTRMP_Close(pRmpCtrl);
				return BKT_ERR;
			}
		}
		else
		{
			if(!LSEEK_END(pRmpCtrl->hbkt, 0))
			{
				TRACE1_BKTRMP("failed seek basket.\n");
				BKTRMP_Close(pRmpCtrl);
				return BKT_ERR;
			}

			int offset = sizeof(T_INDEX_DATA);
			if(!LSEEK_END(pRmpCtrl->hidx, -offset))
			{
				TRACE1_BKTRMP("failed seek idx.\n");
				BKTRMP_Close(pRmpCtrl);
				return BKT_ERR;
			}
		}
	}

	pRmpCtrl->bid = prevbid;
	pRmpCtrl->iframe_cnt = BKT_isRecordingBasket(prevbid)? BKT_GetRecordingIndexCount():ihd.count;
	pRmpCtrl->iframe_cur = pRmpCtrl->iframe_cnt - 1;
	pRmpCtrl->status = BKTRMP_STATUS_OPEN;
	pRmpCtrl->sec = BKT_GetMaxTS(ihd.ts.t2);
	pRmpCtrl->fpos_bkt = LTELL(pRmpCtrl->hbkt);
	
	TRACE0_BKTRMP("open prev basket. BID:%ld, iframe-current:%d, iframe_total:%d\n", prevbid, pRmpCtrl->iframe_cur, pRmpCtrl->iframe_cnt);
		
	return BKT_OK;
}

int  BKTRMP_ReadNextIFrame(T_BKTRMP_CTRL* pRmpCtrl, T_BKTRMP_PARAM *dp)
{
	if(BKT_isRecordingBasket(pRmpCtrl->bid))
	{
		pRmpCtrl->iframe_cnt = BKT_GetRecordingIndexCount();
		//TRACE0_BKTRMP("get total i-frame count:%d\n", pRmpCtrl->gBKTRMP_Ctrl);
	}
	
	if(pRmpCtrl->iframe_cur + 1 >= pRmpCtrl->iframe_cnt) // find next basket
	{
		//TRACE0_BKTRMP("Iframe index exceed total count. iframe_cur=%d, iframe_cnt=%d\n", pRmpCtrl->iframe_cur, pRmpCtrl->iframe_cnt);
		if( BKT_ERR == BKTRMP_OpenContinueNext(pRmpCtrl))
			return BKT_ERR;
	}
	else
	{
		pRmpCtrl->iframe_cur += 1;
	}
	
	//TRACE0_BKTRMP("seek idx next iframe:%d, fpos:%d\n", pRmpCtrl->iframe_cur, BKT_transIndexNumToPos(pRmpCtrl->iframe_cur));
	
	if(!LSEEK_SET(pRmpCtrl->hidx, BKT_transIndexNumToPos(pRmpCtrl->iframe_cur)))
	{
		TRACE1_BKTRMP("failed seek read next frame. iframe:%d\n", pRmpCtrl->iframe_cur);
		return BKT_ERR;
	}	
	
	T_INDEX_DATA idd;
	T_STREAM_HDR shd;
	T_VIDEO_STREAM_HDR vhd;
	
	int search_next_iframe=FALSE;
	int ret;
	
	while(pRmpCtrl->status == BKTRMP_STATUS_OPEN)
	{
		if(!READ_ST(pRmpCtrl->hidx, idd))
		{
			TRACE1_BKTRMP("failed read idx data\n");
			break;
		}

		//TRACE0_BKTRMP("Read index data. id=0x%08x, ch=%d, fpos=%d\n", idd.id, idd.ch, idd.fpos);
		
		if(idd.fpos >= pRmpCtrl->fpos_bkt)
		{
			if(!LSEEK_SET(pRmpCtrl->hbkt, idd.fpos))
			{
				TRACE1_BKTRMP("failed seek next index data. i-fpos:%ld\n", idd.fpos);
				return BKT_ERR;
			}

			ret = READ_STRC(pRmpCtrl->hbkt, shd);
#ifdef BKT_SYSIO_CALL
			if(ret < 1)
			{
				if(ret == 0)
				{
					TRACE1_BKTRMP("failed read stream header. EOF. cur-iframe:%d, tot-iframe:%d\n", pRmpCtrl->iframe_cur, pRmpCtrl->iframe_cnt);
				}
				else
				{
					search_next_iframe = TRUE;
					TRACE1_BKTRMP("failed read stream header. go next iframe. shd.id:0x%08lx, framesize:%ld, cur-iframe:%d, tot-iframe:%d\n", shd.id, shd.framesize, pRmpCtrl->iframe_cur, pRmpCtrl->iframe_cnt);
				}
				break;
			}
#else
			if(ret != sizeof(shd))
			{
				if(0 != feof(pRmpCtrl->hbkt))
				{
					TRACE1_BKTRMP("failed read stream header. EOF. cur-iframe:%d, tot-iframe:%d\n", pRmpCtrl->iframe_cnt, pRmpCtrl->iframe_cnt);
				}
				else
				{
					search_next_iframe = TRUE;
					TRACE1_BKTRMP("failed read stream header. go next iframe. shd.id:0x%08lx, framesize:%ld, cur-iframe:%d, tot-iframe:%d\n", shd.id, shd.framesize, pRmpCtrl->iframe_cnt, pRmpCtrl->iframe_cnt);
				}
				
				break;
			}
#endif

			if(shd.id != ID_VIDEO_HEADER) // Must be Video Frame. because searching iframe by index.
			{
				TRACE1_BKTRMP("failed read next stream header.CH:%ld, SHID:0x%08lx\n", shd.ch, shd.id);
				search_next_iframe = TRUE;
				break;
				//return BKT_ERR;
			}

			if(!READ_ST(pRmpCtrl->hbkt, vhd)) 
			{
				TRACE1_BKTRMP("failed read next video header.\n");
				return BKT_ERR;
			}
			
			if(!READ_PTSZ(pRmpCtrl->hbkt, dp->vbuffer, shd.framesize))
			{
				TRACE1_BKTRMP("failed read next video i-frame data.\n");
				return BKT_ERR;
			}

			dp->ch          = shd.ch;
			dp->framesize   = shd.framesize;
			dp->frametype   = shd.frametype;
			dp->ts.sec      = shd.ts.sec;
			dp->ts.usec     = shd.ts.usec;
			dp->framerate   = vhd.framerate;
			dp->frameWidth  = vhd.width;
			dp->frameHeight = vhd.height;
			dp->fpos        = 1;
			dp->streamtype  = ST_VIDEO;
			dp->event       = vhd.event;
            dp->audioONOFF  = vhd.audioONOFF; // AYK - 0201
			dp->capMode     = vhd.capMode;

			if(strlen(vhd.camName))
				strncpy(dp->camName, vhd.camName, 16);

			pRmpCtrl->sec = shd.ts.sec;
			pRmpCtrl->fpos_bkt = LTELL(pRmpCtrl->hbkt);

            dp->bktId    = pRmpCtrl->bid;
            dp->fpos_bkt = pRmpCtrl->fpos_bkt;

			//TRACE0_BKTRMP("ReadNextIFrame, read frame ch=%d, size:%ld\n", shd.ch, shd.framesize);
			
			return BKT_OK;
		}
		else
		{
			pRmpCtrl->iframe_cur +=1;
			
			if(pRmpCtrl->iframe_cur >= pRmpCtrl->iframe_cnt) // find next basket
			{
				break;
			}
			else if(!LSEEK_SET(pRmpCtrl->hidx, BKT_transIndexNumToPos(pRmpCtrl->iframe_cur)))
			{
				TRACE1_BKTRMP("failed seek read prev frame. iframe-cur:%d\n", pRmpCtrl->iframe_cur);
				return BKT_ERR;
			}
		}
		
	}// end while
	
	return BKTRMP_ReadNextIFrame(pRmpCtrl, dp);
}

// normal read with audio
int  BKTRMP_ReadNextFrame(T_BKTRMP_CTRL* pRmpCtrl, T_BKTRMP_PARAM *dp)
{
	T_STREAM_HDR shd;
	T_VIDEO_STREAM_HDR vhd;
	T_AUDIO_STREAM_HDR ahd;
	
	int search_next_iframe=0;
	int ret;

	// read shd
	while(pRmpCtrl->status == BKTRMP_STATUS_OPEN)
	{
		//TRACE0_BKTRMP("read stream header\n");

		ret = READ_STRC(pRmpCtrl->hbkt, shd);

		//TRACE0_BKTRMP("read stream header done. ID:0x%08lx, CH:%ld, SIZE:%ld, TYPE:%ld, SEC:%ld\n", shd.id, shd.ch, shd.framesize, shd.frametype, shd.ts.sec);
#ifdef BKT_SYSIO_CALL
		if(ret < 1)
		{
			if(ret == 0)
			{
				TRACE1_BKTRMP("failed read stream header. EOF. cur-iframe:%d, tot-iframe:%d\n", pRmpCtrl->iframe_cur, pRmpCtrl->iframe_cnt);
			}
			else
			{
				search_next_iframe = TRUE;
				TRACE1_BKTRMP("failed read stream header. go next iframe. shd.id:0x%08lx, framesize:%ld, cur-iframe:%d, tot-iframe:%d\n", shd.id, shd.framesize, pRmpCtrl->iframe_cur, pRmpCtrl->iframe_cnt);
			}
			break;
		}
#else
		if(ret != sizeof(shd))
		{
			if(0 != feof(pRmpCtrl->hbkt))
			{
				TRACE1_BKTRMP("failed read stream header. EOF. cur-iframe:%d, tot-iframe:%d\n", pRmpCtrl->iframe_cnt, pRmpCtrl->iframe_cnt);
			}
			else
			{
				search_next_iframe = TRUE;
				TRACE1_BKTRMP("failed read stream header. go next iframe. shd.id:0x%08lx, framesize:%ld, cur-iframe:%d, tot-iframe:%d\n", shd.id, shd.framesize, pRmpCtrl->iframe_cur, pRmpCtrl->iframe_cnt);
			}
			break;
		}
#endif
		if(shd.id == ID_VIDEO_HEADER)
		{
			if(shd.ts.sec - pRmpCtrl->sec > 4 || shd.ts.sec - pRmpCtrl->sec < 0) // maybe max interval frame. for broken stream or motion.
			{
				search_next_iframe = TRUE;
				TRACE0_BKTRMP("time interval is too long. So search next frame. shd.id:0x%08lx\n", shd.id);
				break;
			}
			
			if(!READ_ST(pRmpCtrl->hbkt, vhd))
			{
				TRACE1_BKTRMP("failed read video header\n");
				return BKT_ERR;
			}
			
			if(!READ_PTSZ(pRmpCtrl->hbkt, dp->vbuffer, shd.framesize))
			{
				TRACE1_BKTRMP("failed read video data\n");
				return BKT_ERR;
			}
			
			dp->ch          = shd.ch;
			dp->framesize   = shd.framesize;
			dp->frametype   = shd.frametype;
			dp->ts.sec      = shd.ts.sec;
			dp->ts.usec     = shd.ts.usec;
			dp->framerate   = vhd.framerate;
			dp->frameWidth  = vhd.width;
			dp->frameHeight = vhd.height;
			dp->fpos = (shd.frametype == FT_IFRAME);
			dp->streamtype  = ST_VIDEO;
			dp->event       = vhd.event;
			dp->audioONOFF  = vhd.audioONOFF; // BK - 100119
			dp->capMode     = vhd.capMode;
			
			pRmpCtrl->sec = shd.ts.sec;
			pRmpCtrl->fpos_bkt = LTELL(pRmpCtrl->hbkt);
			
			if(shd.frametype == FT_IFRAME )
			{
				if(strlen(vhd.camName))
					strncpy(dp->camName, vhd.camName, 16);
				
				pRmpCtrl->iframe_cur += 1;
				
				if( BKT_isRecordingBasket(pRmpCtrl->bid))
					pRmpCtrl->iframe_cnt = BKT_GetRecordingIndexCount();
			}
			
			dp->bktId      = pRmpCtrl->bid;
			dp->fpos_bkt   = pRmpCtrl->fpos_bkt;
			
			//TRACE_BKTDEC("Read frame, frameType:%d, iframe-num:%d, size:%ld\n", shd.frametype, pRmpCtrl->iframe_cur, shd.framesize);
			
			return BKT_OK;
		}
		else if( shd.id == ID_AUDIO_HEADER )
		{
			// read audio stream header
			if(!READ_ST(pRmpCtrl->hbkt, ahd))
			{
				TRACE1_BKTRMP("failed read audio stream header\n");
				return BKT_ERR;
			}
			
			if(!READ_PTSZ(pRmpCtrl->hbkt, dp->abuffer, shd.framesize))
			{
				TRACE1_BKTRMP("failed read audio stream\n");
				return BKT_ERR;
			}
			
			dp->ch           = shd.ch;
			dp->ts.sec       = shd.ts.sec;
			dp->ts.usec      = shd.ts.usec;
			dp->streamtype   = ST_AUDIO;
			dp->framesize    = shd.framesize;
			dp->samplingrate = ahd.samplingrate;
			dp->fpos = 0;
			
			return BKT_OK;

		}
		else
		{
			search_next_iframe = TRUE;
			TRACE0_BKTRMP("maybe appeared broken stream .Search next i-frame. shd.id:0x%08lx, framesize:%ld\n", shd.id, shd.framesize);
			break;
		}

	}// while
	
	/*
	if(search_next_iframe == TRUE)	
	{
		return BKTRMP_ReadNextIFrame(pRmpCtrl, dp);
	}
	else //if(pRmpCtrl->iframe_cur >= pRmpCtrl->iframe_cnt)
	{
		if( BKT_ERR == BKTRMP_OpenContinueNext(pRmpCtrl))
			return BKT_ERR;	
			
		return BKTRMP_ReadNextIFrame(pRmpCtrl, dp);
	}
	*/

	return BKTRMP_ReadNextIFrame(pRmpCtrl, dp);

	//return BKT_ERR;
}


long BKTRMP_SearchPrevBID(T_BKTRMP_CTRL* pRmpCtrl)
{
	T_BKT_TM tm;
	BKT_GetLocalTime(pRmpCtrl->sec, &tm);

	char path[LEN_PATH];
	sprintf(path, "%s/rdb/%04d%02d%02d", pRmpCtrl->target_path, tm.year, tm.mon, tm.day);

#ifdef BKT_SYSIO_CALL
	int fd;
#else
	FILE *fd;
#endif

	if(!OPEN_RDONLY(fd, path))
	{
		TRACE1_BKTRMP("failed open rdb %s\n", path);
		return BKT_ERR;
	}
	
	if(!LSEEK_SET(fd, TOTAL_MINUTES_DAY*MAX_CHANNEL))
	{
		TRACE1_BKTRMP("failed seek rdb\n");
		CLOSE_BKT(fd);
		return BKT_ERR;
	}
	
	T_BASKET_RDB rdbs[TOTAL_MINUTES_DAY*MAX_CHANNEL];

	if(!READ_PTSZ(fd, rdbs, sizeof(rdbs)))
	{
		TRACE1_BKTRMP("failed read rdbs data\n");
		CLOSE_BKT(fd);
		return BKT_ERR;
	}
	CLOSE_BKT(fd);
	
	int i, ch;
	for(i=tm.hour*60+tm.min-1;i>=0;i--)
	{
		for(ch=0;ch<MAX_CHANNEL;ch++)
		if(rdbs[TOTAL_MINUTES_DAY*ch+i].bid != 0
			&& rdbs[TOTAL_MINUTES_DAY*ch+i].bid !=pRmpCtrl->bid)
		{
			TRACE0_BKTRMP("found prev BID:%ld, CH:%d\n", rdbs[TOTAL_MINUTES_DAY*ch+i].bid, ch);
			
			return rdbs[TOTAL_MINUTES_DAY*ch+i].bid; // found!!
		}
	}

	return BKT_ERR;// not found
}

long BKTRMP_SearchNextBID(T_BKTRMP_CTRL* pRmpCtrl)
{
	if(BKT_isRecordingBasket(pRmpCtrl->bid))
	{
		TRACE0_BKTRMP("There is no a next basekt. BID:%ld is recording.\n", pRmpCtrl->bid);
		return BKT_ERR;
	}

	T_BKT_TM tm;
	BKT_GetLocalTime(pRmpCtrl->sec, &tm);

	char path[LEN_PATH];
	sprintf(path, "%s/rdb/%04d%02d%02d", pRmpCtrl->target_path, tm.year, tm.mon, tm.day);

#ifdef BKT_SYSIO_CALL
	int fd;
#else
	FILE *fd;
#endif

	if(!OPEN_RDONLY(fd, path))
	{
		TRACE1_BKTRMP("failed open rdb %s\n", path);
		return BKT_ERR;
	}
	
	if(!LSEEK_SET(fd, TOTAL_MINUTES_DAY*MAX_CHANNEL))
	{
		TRACE1_BKTRMP("failed seek rdb\n");
		CLOSE_BKT(fd);
		return BKT_ERR;
	}
	
	T_BASKET_RDB rdbs[TOTAL_MINUTES_DAY*MAX_CHANNEL];
	
	if(!READ_PTSZ(fd, rdbs, sizeof(rdbs)))
	{
		TRACE1_BKTRMP("failed read rdbs data\n");
		CLOSE_BKT(fd);
		return BKT_ERR;
	}
	CLOSE_BKT(fd);
	
	int i, ch;
	for(i=tm.hour*60+tm.min+1;i<TOTAL_MINUTES_DAY;i++)
	{
		for(ch=0;ch<MAX_CHANNEL;ch++)
		{
			if(rdbs[TOTAL_MINUTES_DAY*ch+i].bid != 0 
				&& rdbs[TOTAL_MINUTES_DAY*ch+i].bid != pRmpCtrl->bid)
			{
				TRACE0_BKTRMP("found next bid %ld, CH:%d\n", rdbs[TOTAL_MINUTES_DAY*ch+i].bid, ch);
				
				return rdbs[TOTAL_MINUTES_DAY*ch+i].bid; // found next basket id!!
			}
		}
	}

	return BKT_ERR;// not found
}

long BKTRMP_ReadPrevIFrame(T_BKTRMP_CTRL* pRmpCtrl, T_BKTRMP_PARAM *dp)
{
	if(pRmpCtrl->iframe_cur  - 1 < 0) // find prev basket
	{
		if( BKT_ERR == BKTRMP_OpenContinuePrev(pRmpCtrl))
			return BKT_ERR;
	}
	else
	{
		pRmpCtrl->iframe_cur -= 1;
	}

	if(!LSEEK_SET(pRmpCtrl->hidx, BKT_transIndexNumToPos(pRmpCtrl->iframe_cur)))
	{
		TRACE1_BKTRMP("failed seek read prev frame. iframe:%d\n", pRmpCtrl->iframe_cur);
		return BKT_ERR;
	}

	T_INDEX_DATA idd;
	T_STREAM_HDR shd;
	T_VIDEO_STREAM_HDR vhd;

	int search_prev_iframe=FALSE;
	int ret;

	while(pRmpCtrl->status == BKTRMP_STATUS_OPEN)
	{
		// read index data.
		ret = READ_STRC(pRmpCtrl->hidx, idd);

		// check read data
#ifdef BKT_SYSIO_CALL
		if(ret < 1)
		{
			if(ret == 0)
			{
				TRACE1_BKTRMP("failed read index data. EOF. idd.id:0x%08lx, cur-iframe:%d, tot-iframe:%d\n", idd.id, pRmpCtrl->iframe_cur, pRmpCtrl->iframe_cnt);
			}
			else
			{
				search_prev_iframe = TRUE;
				TRACE1_BKTRMP("failed read index data. maybe broken. go prev iframe. idd.id:0x%08lx, cur-iframe:%d, tot-iframe:%d\n", idd.id, pRmpCtrl->iframe_cur, pRmpCtrl->iframe_cnt);
			}
			break;
		}
#else
		if(ret != sizeof(idd))
		{
			if(0 != feof(pRmpCtrl->hidx))
			{
				TRACE1_BKTRMP("failed read index data. EOF. idd.id:0x%08lx, cur-iframe:%d, tot-iframe:%d\n", idd.id, pRmpCtrl->iframe_cur, pRmpCtrl->iframe_cnt);
			}
			else
			{
				search_prev_iframe = TRUE;
				TRACE1_BKTRMP("failed read index data. maybe broken. go prev iframe. idd.id:0x%08lx, cur-iframe:%d, tot-iframe:%d\n", idd.id, pRmpCtrl->iframe_cur, pRmpCtrl->iframe_cnt);
			}
			break;
		}
#endif//BKT_SYSIO_CALL

		//TRACE0_BKTRMP("index data fpos:%d, ch:%d, sec:%ld, curiframe:%d, totiframe:%d\n", idd.fpos, idd.ch, idd.ts.sec, pRmpCtrl->iframe_cur, pRmpCtrl->iframe_cnt);

		//if(pRmpCtrl->sec - idd.ts.sec >= 0)
		if(idd.fpos < pRmpCtrl->fpos_bkt)
		{
			if(!LSEEK_SET(pRmpCtrl->hbkt, idd.fpos))
			{
				TRACE1_BKTRMP("failed seek prev index data. i-fpos:%ld\n", idd.fpos);
				return BKT_ERR;
			}

			ret = READ_STRC(pRmpCtrl->hbkt, shd);
#ifdef BKT_SYSIO_CALL
			if(ret < 1)
			{
				if(ret == 0)
				{
					TRACE1_BKTRMP("failed read stream header. EOF. cur-iframe:%d, tot-iframe:%d\n", pRmpCtrl->iframe_cur, pRmpCtrl->iframe_cnt);
				}
				else
				{
					search_prev_iframe = TRUE;
					TRACE1_BKTRMP("failed read stream header. go prev iframe. shd.id:0x%08lx, framesize:%ld, cur-iframe:%d, tot-iframe:%d\n", shd.id, shd.framesize, pRmpCtrl->iframe_cur, pRmpCtrl->iframe_cnt);
				}
				break;
			}
#else
			if(ret != sizeof(shd))
			{
				if(0 != feof(pRmpCtrl->hbkt))
				{
					TRACE1_BKTRMP("failed read stream header. EOF. cur-iframe:%d, tot-iframe:%d\n", pRmpCtrl->iframe_cur, pRmpCtrl->iframe_cnt);
				}
				else
				{
					search_prev_iframe = TRUE;
					TRACE1_BKTRMP("failed read stream header. go prev iframe. shd.id:0x%08lx, framesize:%ld, cur-iframe:%d, tot-iframe:%d\n", shd.id, shd.framesize, pRmpCtrl->iframe_cur, pRmpCtrl->iframe_cnt);
				}

				break;
			}
#endif//BKT_SYSIO_CALL

			if(shd.id != ID_VIDEO_HEADER) // Must be Video Frame. because searching iframe by index.
			{
				TRACE1_BKTRMP("failed read prev stream header. CH:%ld, SHID:0x%08lx\n", shd.ch, shd.id);
				break;
				//return BKT_ERR;
			}

			if(!READ_ST(pRmpCtrl->hbkt, vhd))
			{
				TRACE1_BKTRMP("failed read prev video header.\n");
				break;
				//return BKT_ERR;
			}

			if(!READ_PTSZ(pRmpCtrl->hbkt, dp->vbuffer, shd.framesize))
			{
				TRACE0_BKTRMP("failed read prev video i-frame data.\n");
				break;
				//return BKT_ERR;
			}

			dp->ch          = shd.ch;
			dp->framesize   = shd.framesize;
			dp->frametype   = shd.frametype;
			dp->ts.sec      = shd.ts.sec;
			dp->ts.usec     = shd.ts.usec;
			dp->framerate   = vhd.framerate;
			dp->frameWidth  = vhd.width;
			dp->frameHeight = vhd.height;
			dp->fpos        = 1;
			dp->streamtype  = ST_VIDEO;
			dp->event       = vhd.event;
			dp->audioONOFF  = vhd.audioONOFF; // BK - 100119
			dp->capMode     = vhd.capMode;

			if(strlen(vhd.camName))
				strncpy(dp->camName, vhd.camName, 16);

			pRmpCtrl->sec = shd.ts.sec;
            pRmpCtrl->fpos_bkt = LTELL(pRmpCtrl->hbkt);

            dp->bktId    = pRmpCtrl->bid;
            dp->fpos_bkt = pRmpCtrl->fpos_bkt;

			return BKT_OK;
		}
		else
		{
			pRmpCtrl->iframe_cur -=1;

			if(pRmpCtrl->iframe_cur < 0 ) // find prev basket.
			{
				break;
			}
			else if(!LSEEK_SET(pRmpCtrl->hidx, BKT_transIndexNumToPos(pRmpCtrl->iframe_cur)))
			{
				TRACE1_BKTRMP("failed seek read prev frame. iframe-cur:%d\n", pRmpCtrl->iframe_cur);
				return BKT_ERR;
			}
		}

	}// end while

	if(search_prev_iframe == TRUE)
	{
		return BKTRMP_ReadPrevIFrame(pRmpCtrl, dp);
	}

	return BKTRMP_ReadPrevIFrame(pRmpCtrl, dp);
}

////////////////////////////
// EOF basket_remoteplayback.c

