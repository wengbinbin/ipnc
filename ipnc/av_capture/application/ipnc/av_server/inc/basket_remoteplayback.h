///////////////////////////////////////////////////////////////////////////////
#ifndef _BASKET_REMOTEPLAYBACK_H_
#define _BASKET_REMOTEPLAYBACK_H_ 
///////////////////////////////////////////////////////////////////////////////

/*
 * basket_remoteplayback.h
 */

#include <basket.h>

//#ifdef __cplusplus
//extern "C"{
//#endif
//////////////////////////////////////////////////////////////////////////

#define BKTRMP_STATUS_OPEN 1
#define BKTRMP_STATUS_CLOSE 0


typedef struct
{
#ifdef BKT_SYSIO_CALL
	int hbkt; // handle of basket file
	int hidx; // handle of index file
#else
	FILE* hbkt;// handle of basket file
	FILE* hidx;// handle of index file
#endif

	long bid; // was opened basket file's ID
	long fpos_bkt;
	
	long sec; // last read seconds channels without distinction

	int status; // basket open status	

	int iframe_cnt; // intra-frame total count
	int iframe_cur; // intra-frame current number. zero base
	
	char target_path[LEN_PATH]; // target storage path..
	
}T_BKTRMP_CTRL;

typedef struct {

	int frameWidth;
	int frameHeight;

}T_BKTRMP_OPENED_PARAM;

typedef struct {

	long sec; // begin seconds
	int frameWidth;
	int frameHeight;

}T_BKTRMP_OPENPARAM;

typedef struct {
	int ch;
	int evt;
	int year;
	int mon;
	char *pRecDayTBL; // 31 days
}T_BKTRMP_RECDAYS_PARAM;

typedef struct {
	int ch;
	int evt;
	int year;
	int mon;
	int day;
	char *pRecHourTBL; // 24*60
}T_BKTRMP_RECHOUR_PARAM;

typedef struct
{
    long fpos;
    long ch;
    
    T_TS ts;
	
    long  framesize;
    short framerate;
    short frameWidth;
    short frameHeight;
    int   frametype;
    
    long  streamtype;
    long  event;
    
    long  samplingrate;
    long  achannel; // mono or stero
    long  bitpersample;
    char  camName[16];
    
    unsigned char *vbuffer; // video read buffer pointer
    unsigned char *abuffer; // audio read buffer pointer
	
    long bktId;
    long fpos_bkt;
	
    short audioONOFF; // AYK - 0201
	int   capMode;  // 2D1, 4CIF, 8CIF, 4D1
	
	int*  decFlags; // current decode flag, 0:no, 1:decode
	
}T_BKTRMP_PARAM;


//////////////////////////////////////////////////////////////////////////
// streaming interfaces
void BKTRMP_Close(T_BKTRMP_CTRL* pRmpCtrl);
int BKTRMP_Open(T_BKTRMP_CTRL* pRmpCtrl, long beginsec);
int BKTRMP_OpenContinueNext(T_BKTRMP_CTRL* pRmpCtrl);
int BKTRMP_OpenContinuePrev(T_BKTRMP_CTRL* pRmpCtrl);
long BKTRMP_SearchPrevBID(T_BKTRMP_CTRL* pRmpCtrl);
long BKTRMP_SearchNextBID(T_BKTRMP_CTRL* pRmpCtrl);

int BKTRMP_ReadNextFrame(T_BKTRMP_CTRL* pRmpCtrl, T_BKTRMP_PARAM *dp);
int BKTRMP_ReadNextIFrame(T_BKTRMP_CTRL* pRmpCtrl, T_BKTRMP_PARAM *dp);

int  BKTRMP_GetRecDayData(T_BKTRMP_CTRL* pRmpCtrl, int year, int mon, char* pRecDayTBL); // 31 day
int  BKTRMP_GetRecHourData(T_BKTRMP_CTRL* pRmpCtrl, int ch, int year, int mon, int day, char* pRecDayTBL); // 1440 minutes (24*60)
int  BKTRMP_GetRecHourDataAllCh(T_BKTRMP_CTRL* pRmpCtrl, int year, int mon, int day, char* pRecDayTBL); // 1440*MAX_CHANNEL minutes (24*60*MAX_CHANNEL)
int  BKTRMP_GetRecHourDataUnion(T_BKTRMP_CTRL* pRmpCtrl, T_BKTRMP_RECHOUR_PARAM* pParam); // 1440 minutes (24*60)
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//#ifdef __cplusplus
//}
//#endif

///////////////////////////////////////////////////////////////////////////////
#endif//_BASKET_REMOTEPLAYBACK_H_
///////////////////////////////////////////////////////////////////////////////

