
#include <mcvip_priv.h>
#include <osa_cmem.h>

//#define MCVIP_DEBUG_HANG

#define MCVIP_XY_F0_BLANK 0xAB
#define MCVIP_XY_F0_VALID 0x80
#define MCVIP_XY_F1_BLANK 0xEC
#define MCVIP_XY_F1_VALID 0xC7

#define MCVIP_XY_INVALID  0xFF

#define MCVIP_TVP5158_METADATA_LENGTH   (4*2)

#define TVP5158_BOP       (1<<6)
#define TVP5158_EOP       (1<<5)

#define TVP5158_CH_VALID  ((1<<7)|(1<<31))
#define TVP5158_VDET      (1<<12)
#define TVP5158_EOL       (1<<13)
#define TVP5158_BOL       (1<<14)

#define TVP5158_F_BIT		  (1<<30)
#define TVP5158_DUMMY		  (0x01010101)

#define MCVIP_SKIP_COUNT  (12)
#define MCVIP_START_LINE_NUM  (0)

int MCVIP_demuxInit(MCVIP_Hndl *hndl)
{
  int i, status;
  MCVIP_BufInfo *pBufInfo;
    
  for(i=0; i<hndl->chList.numCh; i++)
  {
    hndl->chDemuxInfo[i].errorFrame=0;
    hndl->chDemuxInfo[i].errorLine=0;    
    hndl->chDemuxInfo[i].curField = 0;
    hndl->chDemuxInfo[i].curLine[0] = 0;
    hndl->chDemuxInfo[i].curLine[1] = 0;  
    hndl->chDemuxInfo[i].isFirstFrame = 1;
    hndl->chDemuxInfo[i].skipCount = MCVIP_SKIP_COUNT;
    
    hndl->chDemuxInfo[i].prevXY  = MCVIP_XY_INVALID;
    hndl->chDemuxInfo[i].prevLineNum  = MCVIP_START_LINE_NUM-1;
    
    status = OSA_queGet(&hndl->emptyQue[i], &hndl->chDemuxInfo[i].curBufId, OSA_TIMEOUT_FOREVER);
    OSA_assertSuccess(status);

    pBufInfo = MCVIP_getBufInfo((void*)hndl, hndl->chDemuxInfo[i].curBufId);
    pBufInfo->chId = i;
          
    OSA_assert(pBufInfo!=NULL);
          
    hndl->chDemuxInfo[i].curDstPhysAddr = pBufInfo->physAddr;  
    hndl->chDemuxInfo[i].curDstVirtAddr = pBufInfo->virtAddr;    
  }

  hndl->skipField = FALSE;
  
  if( hndl->createPrm.videoDecoderMode==MCVIP_VIDEO_DECODER_MODE_4CH_HALF_D1
    ||hndl->createPrm.videoDecoderMode==MCVIP_VIDEO_DECODER_MODE_8CH_HALF_D1 
    ||(hndl->createPrm.videoDecoderMode==MCVIP_VIDEO_DECODER_MODE_4CH_HALF_D1_PLUS_D1 && i < (hndl->chList.numCh-1))	//SK_PLUSD1
    ) {
    hndl->skipField = TRUE;          
  }    
  
  for(i=0; i<MCVIP_BUF_MAX; i++)
    hndl->tmpDemuxQue[i] = -1;  

  hndl->dmaDemux.srcChromaOffset = 0; // since in DM355 UV data is interleaved with Y
  hndl->dmaDemux.dstChromaOffset = 0; // since in DM355 UV data is interleaved with Y
  hndl->dmaDemux.copyWidth    = hndl->chList.info[0].width*2; // since in DM355 UV data is interleaved with Y
  hndl->dmaDemux.srcPhysAddrList = hndl->dmaDemuxSrcPhysAddr;
  hndl->dmaDemux.dstPhysAddrList = hndl->dmaDemuxDstPhysAddr;
  
  hndl->chBase = 0;
  if( hndl->createPrm.videoDecoderMode==MCVIP_VIDEO_DECODER_MODE_2CH_D1_PORT_A
    ||hndl->createPrm.videoDecoderMode==MCVIP_VIDEO_DECODER_MODE_2CH_D1_PORT_B
    ||hndl->createPrm.videoDecoderMode==MCVIP_VIDEO_DECODER_MODE_2CH_D1_2CH_CIF	//sk_cif
    ) {
    hndl->chBase = 2;    
  }
   
//  hndl->initTimeInMsecs = OSA_getCurTimeInMsec(); // AYK - 1217
  
  return OSA_SOK;
}

int MCVIP_demuxRun(MCVIP_Hndl *hndl, MCVIP_V4l2Buf *buf)
{
  int tmpDemuxQueIdx=0, i, numCh, eol, bol, extraCh, status, newBufId;
  volatile Uint8 *curAddr;
  Uint16 chId, lineNum=0;
  Uint8 *destAddr;
  Uint8 *srcAddr;
  Bool isDone;
  MCVIP_ChDemuxInfo *pChInfo;
  MCVIP_BufInfo *pBufInfo;
  Uint32 destAddrOffset, srcAddrOffset, dstLineOffsetH, curTime;
  volatile Uint32 metaData, validFlag;
  Uint8 curXY;
  int videoDecoderMode;

  struct timeval t; // AYK - 1217

	//struct tm tm1;//for test
  hndl->dmaDemux.numLines     = 0;
  
  numCh = hndl->chList.numCh;

  curAddr = buf->virtAddr;
  
  validFlag = TVP5158_CH_VALID|TVP5158_VDET|TVP5158_EOL|TVP5158_BOL;
  
  videoDecoderMode = hndl->createPrm.videoDecoderMode;

//  curTime = OSA_getCurTimeInMsec()-hndl->initTimeInMsecs; // AYK - 1217

  // AYK - 1217
  gettimeofday(&t,(int)NULL);
    
  for(i=0; i<numCh; i++) {
    pChInfo = &hndl->chDemuxInfo[i];
    
    if(pChInfo->isFirstFrame) {
          
      if(pChInfo->skipCount)
        pChInfo->skipCount--;
    } else {
      pChInfo->skipCount = 0;
    }
  }
  
//  OSA_cmemCacheInv(buf->virtAddr, hndl->v4l2FrameInfo.offsetH*2*hndl->v4l2FrameInfo.height);

  for(i=0; i<hndl->v4l2FrameInfo.height; i++) {
    
    metaData =  ( (Uint32)curAddr[1] << 0 ) |
                ( (Uint32)curAddr[3] << 8 ) |
                ( (Uint32)curAddr[5] << 16 ) |
                ( (Uint32)curAddr[7] << 24 ) 
                ;   

    curXY = metaData >> 24;
            
    if(hndl->skipField) {                
      if(curXY==MCVIP_XY_F1_VALID) {
        curAddr += hndl->v4l2FrameInfo.offsetH*2;
        continue;
      }
    }

    if(  curXY==MCVIP_XY_F1_BLANK 
      || curXY==MCVIP_XY_F0_BLANK     
      ) {
      curAddr += hndl->v4l2FrameInfo.offsetH*2;
      continue;
    }        
                   
    eol = 0;
    extraCh = 0;
    chId = 0xFF;

    if( (metaData & TVP5158_CH_VALID) == TVP5158_CH_VALID )        
    {
      if((metaData & TVP5158_VDET)==0) {
        chId = metaData & 0x07; 
        pChInfo = &hndl->chDemuxInfo[chId];
        pChInfo->curLine[0]=0; 
        pChInfo->curLine[1]=0;        
        pChInfo->isFirstFrame = 1;   
        pChInfo->errorFrame = 0;
        pChInfo->errorLine = 0;        
        pChInfo->prevLineNum = MCVIP_START_LINE_NUM-1;
        pChInfo->skipCount = MCVIP_SKIP_COUNT;
//        OSA_printf("MCVIP_demuxRun(1) chId[%d]\n",chId);
        chId = 0xFF;
      }
    }
    if( (metaData & validFlag) == validFlag )
    {
      chId = metaData & 0x07;
      if(hndl->chBase) {
        if(chId>=hndl->chBase)
          chId = chId - hndl->chBase;
      }
    } else if( (metaData & (TVP5158_CH_VALID|TVP5158_VDET)) == (TVP5158_CH_VALID|TVP5158_VDET)) {
    	
      // extra D1 channel
      chId = numCh-1;
      extraCh = 1;      
	  eol = (metaData & TVP5158_EOL) ? 1 : 0;
	  bol = (metaData & TVP5158_BOL) ? 1 : 0;
    }
      
    if(chId < numCh) {    
      lineNum = ((metaData & 0x007F0000) >> 16)|((metaData & 0x00000300) >> 1);

      if(lineNum>(hndl->chList.info[chId].height/2 - 1)) 
      {
//      	OSA_printf("MCVIP_demuxRun(2) chId[%d]\n",chId);
        chId = 0xFF; //skip line
      }
    }    
    
        
    if(chId < numCh) {    
      pChInfo = &hndl->chDemuxInfo[chId];  

      curXY = metaData >> 24;
      
      if(   curXY==MCVIP_XY_F0_VALID
         || curXY==MCVIP_XY_F0_BLANK
        ) { 
        pChInfo->curField = 0;
      } else
      if(   curXY==MCVIP_XY_F1_VALID
         || curXY==MCVIP_XY_F1_BLANK      
       ) {
        pChInfo->curField = 1;
      } else{
//      	OSA_printf("MCVIP_demuxRun(3) chId[%d]\n",chId);
        chId = 0xFF;
      }
    }

    if(chId < numCh) {    

      if(pChInfo->skipCount){
//      	OSA_printf("MCVIP_demuxRun(4) chId[%d]\n",chId);
        chId = 0xFF;
      }
        
      if(chId < numCh) {            
          
        if(pChInfo->isFirstFrame && pChInfo->curField==1 ) {
          // sync to even field
//          OSA_printf("MCVIP_demuxRun(5) chId[%d]\n",chId);
          chId = 0xFF;
        } else {
          pChInfo->isFirstFrame = 0;
        }
      }
    }

    if(chId < numCh) {    
      pChInfo->curLine[pChInfo->curField] = lineNum;
      
      if(pChInfo->prevLineNum>=0) 
      {
        if( (pChInfo->prevLineNum+1) != lineNum) {
          pChInfo->errorFrame = 1;
          pChInfo->errorLine++;
        }
      } else {
        if( lineNum > 1 ) {
          pChInfo->errorFrame = 1;
          pChInfo->errorLine++;
        }
      }
      
      if( (!extraCh) || (extraCh && eol) ) {        
        pChInfo->prevLineNum  = lineNum;        
      }

  	  // DBian - 100202 change the destadd logic for extraCh
	  dstLineOffsetH = hndl->chList.info[chId].offsetH;
      destAddrOffset = (2*pChInfo->curLine[pChInfo->curField]
                        + pChInfo->curField)*dstLineOffsetH
	  					+ eol*hndl->chList.info[chId].width/2;
						
      destAddrOffset *= 2;
                                
      srcAddrOffset = (curAddr-buf->virtAddr)+MCVIP_TVP5158_METADATA_LENGTH;

      destAddr = pChInfo->curDstPhysAddr + destAddrOffset;
      srcAddr  = buf->physAddr + srcAddrOffset;
      
      hndl->dmaDemuxSrcPhysAddr[hndl->dmaDemux.numLines] = (unsigned long)srcAddr;
      hndl->dmaDemuxDstPhysAddr[hndl->dmaDemux.numLines] = (unsigned long)destAddr;        
      hndl->dmaDemux.numLines++;
      
      isDone = FALSE;     

//Anshuman - June 28 2010 - 
// Added the logic of clearing the prevLineNum only when the EOL is received for hybrid channel
// like D1 in this case. Earlier the prevLineNum was cleared even if the first half line of D1 channel was received
      if( (!extraCh) || (extraCh && eol) ) {        
        if(lineNum==(hndl->chList.info[chId].height/2-1)) 
            pChInfo->prevLineNum = MCVIP_START_LINE_NUM-1;     
      }

      if( (   pChInfo->curLine[0]== (hndl->chList.info[chId].height/2 - 1) 
           && pChInfo->curLine[1]== (hndl->chList.info[chId].height/2 - 1)
          )
          ||
          (
            pChInfo->curLine[0]== (hndl->chList.info[chId].height/2 - 1) 
            && hndl->skipField
          )
        ) {
        
          // valid frame, all lines received
          isDone = TRUE;            
      } 

      if(isDone) {

        pChInfo->curLine[0]=0;
        pChInfo->curLine[1]=0;     
        
        pChInfo->isFirstFrame = 1;
        pChInfo->skipCount=0; 
//        pChInfo->prevLineNum = MCVIP_START_LINE_NUM-1;              

#if 0	// DBian - 100203 save the extraCh buffer to check, debug purpose only
{
	FILE *fptr;
	static int cntDB=0, ccc=0;
	unsigned char *buf;
	int width, height, size;
	char filename[20];
	
	height = hndl->chList.info[chId].offsetV;
	width = hndl->chList.info[chId].offsetH;
	size = width * height * 2;
	buf = pChInfo->curDstVirtAddr;
//	printf("Full Frame:: ch=%d %dx%d size=%d\n",chId, width,height,size);

	if (extraCh) cntDB++;
	if (extraCh){ //&& cntDB == 10){
		sprintf(filename, "D1_%04d.yuv", cntDB);
		//fptr = fopen("D1.yuv","wb+");
		fptr = fopen(filename,"wb+");
		if (fptr != NULL){
			printf("save extraCh D1 buffer, %dx%d size=%d\n",width,height,size);
			fwrite(buf,size,1,fptr);
			fclose(fptr);
		}
	}
	if (!extraCh && cntDB == 12 && ccc==0){
		ccc=1;
		fptr = fopen("cif.yuv","wb+");
		if (fptr != NULL){
			printf("save cif buffer, %dx%d size=%d\n",width,height,size);
			fwrite(buf,size,1,fptr);
			fclose(fptr);
		}
	}
}
#endif
        if(!pChInfo->errorFrame || (pChInfo->errorFrame && pChInfo->errorLine < 5)) 
        {
          OSA_prfBegin(&hndl->prfQueWait);
          
          status = OSA_queGet( &hndl->emptyQue[chId], &newBufId, OSA_TIMEOUT_FOREVER);
                    
          OSA_prfEnd(&hndl->prfQueWait, 1);
          
          if(status!=OSA_SOK) {
            #ifdef MCVIP_DEBUG
            OSA_printf(" MCVIP: Could not get new buffer for demux ch=%d\n", chId);
            #endif
          } 
            
          
          if(status==OSA_SOK) {
        
            hndl->tmpDemuxQue[tmpDemuxQueIdx] = pChInfo->curBufId;
            tmpDemuxQueIdx++;
          
            pBufInfo = MCVIP_getBufInfo((void*)hndl, pChInfo->curBufId);
//            pBufInfo->timestamp = curTime; // AYK - 1217
            
            // AYK - 1217
            pBufInfo->timesec   = t.tv_sec;
            pBufInfo->timestamp = t.tv_usec;  

			
			
            #ifdef MCVIP_DEBUG_HANG
			localtime_r(&t.tv_sec, &tm1);
			OSA_printf("date:%d-%d-%d %d:%d\n",tm1.tm_year+1900,tm1.tm_mon+1,tm1.tm_mday,tm1.tm_hour,tm1.tm_min);
            OSA_printf(" Demux   Ch %d: Buf %d\n", chId, pChInfo->curBufId);
            #endif
         
            pChInfo->curBufId = newBufId;
          
            if(pChInfo->curBufId==0xFF)
              return OSA_EFAIL;
            
            #ifdef MCVIP_DEBUG_HANG                  
            OSA_printf(" Demux   Ch %d: New %d\n", chId, pChInfo->curBufId);
            #endif
          }        
          pBufInfo = MCVIP_getBufInfo((void*)hndl, pChInfo->curBufId);
                    
          OSA_assert(pBufInfo!=NULL);
          
          pBufInfo->chId = chId;          
//          OSA_printf("MCVIP_demuxRun() pBufInfo->chId[%d]\n",pBufInfo->chId);
          pChInfo->curDstVirtAddr = pBufInfo->virtAddr;  
          pChInfo->curDstPhysAddr = pBufInfo->physAddr;
        }
        
        if(pChInfo->errorFrame && pChInfo->errorLine > 4) {
          #ifdef MCVIP_DEBUG
          OSA_printf(" Frame %5d: Missing lines %d ch=%d i=%d\n", hndl->frameCount, pChInfo->errorLine,chId,i);
          #endif
        }
        pChInfo->errorFrame = 0;
        pChInfo->errorLine = 0;        
      }  
      
    } 
    
    curAddr += hndl->v4l2FrameInfo.offsetH*2;
  } 

  OSA_prfBegin(&hndl->prfDma);
  DRV_dmaDemux(&hndl->dmaDemuxHndl, &hndl->dmaDemux);
    
  OSA_prfEnd(&hndl->prfDma, 1);  

  #ifdef MCVIP_DEBUG_HANG
  OSA_printf(" Demux End\n");
  #endif

  for(i=0; i<tmpDemuxQueIdx; i++) {
    if(hndl->tmpDemuxQue[i]>=0) {
//      OSA_printf(" Demux Put   : Buf %d\n", hndl->tmpDemuxQue[i]);
      OSA_quePut(&hndl->fullQue, hndl->tmpDemuxQue[i], OSA_TIMEOUT_FOREVER);
      hndl->tmpDemuxQue[i] = -1;
    }
  }

  return OSA_SOK;
}

