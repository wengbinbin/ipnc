

#include <display_priv.h>
#include <osa_cmem.h>
#include <drv_frameCopy.h>

DISPLAY_LayoutCtrl gDISPLAY_layoutCtrl;

int DISPLAY_layoutCreate(int layoutMode)
{
  int i, k, status=OSA_SOK;
  DISPLAY_LayoutWinCtrl *pWinCtrl;  

  memset(&gDISPLAY_layoutCtrl, 0, sizeof(gDISPLAY_layoutCtrl));
  
  gDISPLAY_layoutCtrl.displayOffsetH = gDISPLAY_ctrl.frameInfo.lineOffsetH;
  gDISPLAY_layoutCtrl.displayOffsetV = gDISPLAY_ctrl.frameInfo.lineOffsetV;
  gDISPLAY_layoutCtrl.displayWidth   = gDISPLAY_ctrl.frameInfo.width;
  gDISPLAY_layoutCtrl.displayHeight  = gDISPLAY_ctrl.frameInfo.height;  
  gDISPLAY_layoutCtrl.displayStartX  = 0;
  gDISPLAY_layoutCtrl.displayStartY  = 0;
    
  status = OSA_mutexCreate(&gDISPLAY_layoutCtrl.mutexLock);
  if(status!=OSA_SOK) {
    OSA_ERROR("OSA_mutexCreate()\n");
    return status;
  }
      
  for(i=0; i<DISPLAY_LAYOUT_MAX_WIN; i++)
  {
    pWinCtrl = &gDISPLAY_layoutCtrl.winCtrl[i];
    
    pWinCtrl->curFullBufId = -1;

    if(i==0 || i==(DISPLAY_LAYOUT_PLAYBACK_WIN))
      pWinCtrl->bufSize = (OSA_align(720, 32)*480*3)/2;
    else
      pWinCtrl->bufSize = (OSA_align(360, 32)*240*3)/2;
        
    pWinCtrl->bufSize = OSA_align(pWinCtrl->bufSize, 32);  
      
    pWinCtrl->bufVirtAddr = OSA_cmemAlloc(pWinCtrl->bufSize*DISPLAY_LAYOUT_MAX_WIN_BUF, 32);
    pWinCtrl->bufPhysAddr = OSA_cmemGetPhysAddr(pWinCtrl->bufVirtAddr);

    if(pWinCtrl->bufVirtAddr==NULL || pWinCtrl->bufPhysAddr==NULL) {
      OSA_ERROR("OSA_cmemAlloc()\n");
      goto error_exit;
    }

    pWinCtrl->bufCreate.numBuf=DISPLAY_LAYOUT_MAX_WIN_BUF;
    for(k=0; k<DISPLAY_LAYOUT_MAX_WIN_BUF; k++) {
      pWinCtrl->bufCreate.bufPhysAddr[k] = pWinCtrl->bufPhysAddr + k*pWinCtrl->bufSize;
      pWinCtrl->bufCreate.bufVirtAddr[k] = pWinCtrl->bufVirtAddr + k*pWinCtrl->bufSize;      
    }

    status = OSA_bufCreate(&pWinCtrl->bufHndl, &pWinCtrl->bufCreate);
    if(status!=OSA_SOK) {
      OSA_ERROR("OSA_bufCreate()\n");
      goto error_exit;
    }
  }
  
  DISPLAY_layoutSetMode(layoutMode);
  
  return OSA_SOK;
  
error_exit:
  
  for(i=0; i<DISPLAY_LAYOUT_MAX_WIN; i++) {
  
    if(gDISPLAY_layoutCtrl.winCtrl[i].bufVirtAddr) {
      OSA_cmemFree(gDISPLAY_layoutCtrl.winCtrl[i].bufVirtAddr);
      OSA_bufDelete(&gDISPLAY_layoutCtrl.winCtrl[i].bufHndl);
    }
  }
  
  OSA_mutexDelete(&gDISPLAY_layoutCtrl.mutexLock);
  
  return OSA_EFAIL;
}
        
int DISPLAY_layoutDelete()
{
  int i;

  for(i=0; i<DISPLAY_LAYOUT_MAX_WIN; i++) {
  
    if(gDISPLAY_layoutCtrl.winCtrl[i].bufVirtAddr) {
      OSA_cmemFree(gDISPLAY_layoutCtrl.winCtrl[i].bufVirtAddr);
      OSA_bufDelete(&gDISPLAY_layoutCtrl.winCtrl[i].bufHndl);
    }
}

  OSA_mutexDelete(&gDISPLAY_layoutCtrl.mutexLock);

  return 0;      
}

int DISPLAY_layoutSetMode(int layoutMode)
{
  DISPLAY_LayoutInfo *layoutInfo;
  int i;
  
  if(layoutMode >= DISPLAY_LAYOUT_MODE_MAX) {
    OSA_ERROR(" Illegal mode (%d)\n", layoutMode);
    return OSA_EFAIL;
  }
    
  OSA_mutexLock(&gDISPLAY_layoutCtrl.mutexLock);

  layoutInfo = &gDISPLAY_layoutCtrl.info;

  memset(layoutInfo, 0, sizeof(*layoutInfo));

  layoutInfo->layoutMode = layoutMode;
  

  
  switch(layoutMode) {
    case DISPLAY_LAYOUT_MODE_PLAYBACK:
      layoutInfo->numWin = 1;
  
      i=DISPLAY_LAYOUT_PLAYBACK_WIN;
      layoutInfo->winInfo[i].startX = 0;
      layoutInfo->winInfo[i].startY = 0;
      layoutInfo->winInfo[i].width  = gDISPLAY_layoutCtrl.displayWidth;
      layoutInfo->winInfo[i].height = gDISPLAY_layoutCtrl.displayHeight;      
      break;  
    default:
    case DISPLAY_LAYOUT_MODE_LIVE_1x1:
      layoutInfo->numWin = 1;
  
      i=0;
      layoutInfo->winInfo[i].startX = 0;
      layoutInfo->winInfo[i].startY = 0;
      layoutInfo->winInfo[i].width  = gDISPLAY_layoutCtrl.displayWidth;
      layoutInfo->winInfo[i].height = gDISPLAY_layoutCtrl.displayHeight;      

      break;
    case DISPLAY_LAYOUT_MODE_LIVE_2x2:
      layoutInfo->numWin = 4;
  
      i=0;
      layoutInfo->winInfo[i].startX = 0;
      layoutInfo->winInfo[i].startY = 0;
      layoutInfo->winInfo[i].width  = gDISPLAY_layoutCtrl.displayWidth/2;
      layoutInfo->winInfo[i].height = gDISPLAY_layoutCtrl.displayHeight/2;      

      i++;
      layoutInfo->winInfo[i].startX = gDISPLAY_layoutCtrl.displayWidth/2;
      layoutInfo->winInfo[i].startY = 0;
      layoutInfo->winInfo[i].width  = gDISPLAY_layoutCtrl.displayWidth/2;
      layoutInfo->winInfo[i].height = gDISPLAY_layoutCtrl.displayHeight/2;      

      i++;
      layoutInfo->winInfo[i].startX = 0;
      layoutInfo->winInfo[i].startY = gDISPLAY_layoutCtrl.displayHeight/2;
      layoutInfo->winInfo[i].width  = gDISPLAY_layoutCtrl.displayWidth/2;
      layoutInfo->winInfo[i].height = gDISPLAY_layoutCtrl.displayHeight/2;      

      i++;
      layoutInfo->winInfo[i].startX = gDISPLAY_layoutCtrl.displayWidth/2;
      layoutInfo->winInfo[i].startY = gDISPLAY_layoutCtrl.displayHeight/2;
      layoutInfo->winInfo[i].width  = gDISPLAY_layoutCtrl.displayWidth/2;
      layoutInfo->winInfo[i].height = gDISPLAY_layoutCtrl.displayHeight/2;      
  
      break;
    case DISPLAY_LAYOUT_MODE_LIVE_3x3:
      layoutInfo->numWin = 9;
        
      i=0;
      layoutInfo->winInfo[i].startX = (0*gDISPLAY_layoutCtrl.displayWidth)/3;
      layoutInfo->winInfo[i].startY = (0*gDISPLAY_layoutCtrl.displayHeight)/3;
      layoutInfo->winInfo[i].width  = gDISPLAY_layoutCtrl.displayWidth/3;
      layoutInfo->winInfo[i].height = gDISPLAY_layoutCtrl.displayHeight/3;      

      i++;
      layoutInfo->winInfo[i].startX = (1*gDISPLAY_layoutCtrl.displayWidth)/3;
      layoutInfo->winInfo[i].startY = (0*gDISPLAY_layoutCtrl.displayHeight)/3;
      layoutInfo->winInfo[i].width  = gDISPLAY_layoutCtrl.displayWidth/3;
      layoutInfo->winInfo[i].height = gDISPLAY_layoutCtrl.displayHeight/3;      

      i++;
      layoutInfo->winInfo[i].startX = (2*gDISPLAY_layoutCtrl.displayWidth)/3;
      layoutInfo->winInfo[i].startY = (0*gDISPLAY_layoutCtrl.displayHeight)/3;
      layoutInfo->winInfo[i].width  = gDISPLAY_layoutCtrl.displayWidth/3;
      layoutInfo->winInfo[i].height = gDISPLAY_layoutCtrl.displayHeight/3;      

      i++;
      layoutInfo->winInfo[i].startX = (0*gDISPLAY_layoutCtrl.displayWidth)/3;
      layoutInfo->winInfo[i].startY = (1*gDISPLAY_layoutCtrl.displayHeight)/3;
      layoutInfo->winInfo[i].width  = gDISPLAY_layoutCtrl.displayWidth/3;
      layoutInfo->winInfo[i].height = gDISPLAY_layoutCtrl.displayHeight/3;      

      i++;
      layoutInfo->winInfo[i].startX = (1*gDISPLAY_layoutCtrl.displayWidth)/3;
      layoutInfo->winInfo[i].startY = (1*gDISPLAY_layoutCtrl.displayHeight)/3;
      layoutInfo->winInfo[i].width  = gDISPLAY_layoutCtrl.displayWidth/3;
      layoutInfo->winInfo[i].height = gDISPLAY_layoutCtrl.displayHeight/3;      

      i++;
      layoutInfo->winInfo[i].startX = (2*gDISPLAY_layoutCtrl.displayWidth)/3;
      layoutInfo->winInfo[i].startY = (1*gDISPLAY_layoutCtrl.displayHeight)/3;
      layoutInfo->winInfo[i].width  = gDISPLAY_layoutCtrl.displayWidth/3;
      layoutInfo->winInfo[i].height = gDISPLAY_layoutCtrl.displayHeight/3;      

      i++;
      layoutInfo->winInfo[i].startX = (0*gDISPLAY_layoutCtrl.displayWidth)/3;
      layoutInfo->winInfo[i].startY = (2*gDISPLAY_layoutCtrl.displayHeight)/3;
      layoutInfo->winInfo[i].width  = gDISPLAY_layoutCtrl.displayWidth/3;
      layoutInfo->winInfo[i].height = gDISPLAY_layoutCtrl.displayHeight/3;      

      i++;
      layoutInfo->winInfo[i].startX = (1*gDISPLAY_layoutCtrl.displayWidth)/3;
      layoutInfo->winInfo[i].startY = (2*gDISPLAY_layoutCtrl.displayHeight)/3;
      layoutInfo->winInfo[i].width  = gDISPLAY_layoutCtrl.displayWidth/3;
      layoutInfo->winInfo[i].height = gDISPLAY_layoutCtrl.displayHeight/3;      

      i++;
      layoutInfo->winInfo[i].startX = (2*gDISPLAY_layoutCtrl.displayWidth)/3;
      layoutInfo->winInfo[i].startY = (2*gDISPLAY_layoutCtrl.displayHeight)/3;
      layoutInfo->winInfo[i].width  = gDISPLAY_layoutCtrl.displayWidth/3;
      layoutInfo->winInfo[i].height = gDISPLAY_layoutCtrl.displayHeight/3;      
      break;
    
    case DISPLAY_LAYOUT_MODE_LIVE_8CH:
      layoutInfo->numWin = 8;
    
      i=0;
      layoutInfo->winInfo[i].startX = 0;
      layoutInfo->winInfo[i].startY = 0;
      layoutInfo->winInfo[i].width  = 3*(gDISPLAY_layoutCtrl.displayWidth/4);
      layoutInfo->winInfo[i].height = 3*(gDISPLAY_layoutCtrl.displayHeight/4);

      i++;
      layoutInfo->winInfo[i].startX = (3*gDISPLAY_layoutCtrl.displayWidth)/4;
      layoutInfo->winInfo[i].startY = (0*gDISPLAY_layoutCtrl.displayHeight)/4;
      layoutInfo->winInfo[i].width  = gDISPLAY_layoutCtrl.displayWidth/4;
      layoutInfo->winInfo[i].height = gDISPLAY_layoutCtrl.displayHeight/4;      

      i++;
      layoutInfo->winInfo[i].startX = (3*gDISPLAY_layoutCtrl.displayWidth)/4;
      layoutInfo->winInfo[i].startY = (1*gDISPLAY_layoutCtrl.displayHeight)/4;
      layoutInfo->winInfo[i].width  = gDISPLAY_layoutCtrl.displayWidth/4;
      layoutInfo->winInfo[i].height = gDISPLAY_layoutCtrl.displayHeight/4;      

      i++;
      layoutInfo->winInfo[i].startX = (3*gDISPLAY_layoutCtrl.displayWidth)/4;
      layoutInfo->winInfo[i].startY = (2*gDISPLAY_layoutCtrl.displayHeight)/4;
      layoutInfo->winInfo[i].width  = gDISPLAY_layoutCtrl.displayWidth/4;
      layoutInfo->winInfo[i].height = gDISPLAY_layoutCtrl.displayHeight/4;      

      i++;
      layoutInfo->winInfo[i].startX = (3*gDISPLAY_layoutCtrl.displayWidth)/4;
      layoutInfo->winInfo[i].startY = (3*gDISPLAY_layoutCtrl.displayHeight)/4;
      layoutInfo->winInfo[i].width  = gDISPLAY_layoutCtrl.displayWidth/4;
      layoutInfo->winInfo[i].height = gDISPLAY_layoutCtrl.displayHeight/4;      

      i++;
      layoutInfo->winInfo[i].startX = (2*gDISPLAY_layoutCtrl.displayWidth)/4;
      layoutInfo->winInfo[i].startY = (3*gDISPLAY_layoutCtrl.displayHeight)/4;
      layoutInfo->winInfo[i].width  = gDISPLAY_layoutCtrl.displayWidth/4;
      layoutInfo->winInfo[i].height = gDISPLAY_layoutCtrl.displayHeight/4;      

      i++;
      layoutInfo->winInfo[i].startX = (1*gDISPLAY_layoutCtrl.displayWidth)/4;
      layoutInfo->winInfo[i].startY = (3*gDISPLAY_layoutCtrl.displayHeight)/4;
      layoutInfo->winInfo[i].width  = gDISPLAY_layoutCtrl.displayWidth/4;
      layoutInfo->winInfo[i].height = gDISPLAY_layoutCtrl.displayHeight/4;      

      i++;
      layoutInfo->winInfo[i].startX = (0*gDISPLAY_layoutCtrl.displayWidth)/4;
      layoutInfo->winInfo[i].startY = (3*gDISPLAY_layoutCtrl.displayHeight)/4;
      layoutInfo->winInfo[i].width  = gDISPLAY_layoutCtrl.displayWidth/4;
      layoutInfo->winInfo[i].height = gDISPLAY_layoutCtrl.displayHeight/4;      
      break;
    
  }
      
  OSA_mutexUnlock(&gDISPLAY_layoutCtrl.mutexLock);  
    
  return OSA_SOK;
}
    
int DISPLAY_layoutGetInfo(DISPLAY_LayoutInfo *info)
{
  OSA_mutexLock(&gDISPLAY_layoutCtrl.mutexLock);
  
  memcpy(info, &gDISPLAY_layoutCtrl.info, sizeof(*info));
  
  OSA_mutexUnlock(&gDISPLAY_layoutCtrl.mutexLock);
  
  return OSA_SOK;
}

OSA_BufInfo *DISPLAY_layoutGetEmptyWinBuf(int winId, int *bufId, int timeout)
{
  int status;
    
  if(winId >= DISPLAY_LAYOUT_MAX_WIN)
    return NULL;
          
  status = OSA_bufGetEmpty(&gDISPLAY_layoutCtrl.winCtrl[winId].bufHndl, bufId, timeout);
  if(status!=OSA_SOK)
    return NULL;
    
  return OSA_bufGetBufInfo(&gDISPLAY_layoutCtrl.winCtrl[winId].bufHndl, *bufId);
}
        
int DISPLAY_layoutPutFullWinBuf(int winId, OSA_BufInfo *buf, int bufId, int layoutModeUsed)
{
  if(winId >= DISPLAY_LAYOUT_MAX_WIN)
    return OSA_EFAIL;
      
  buf->flags = layoutModeUsed;
        
  return OSA_bufPutFull(&gDISPLAY_layoutCtrl.winCtrl[winId].bufHndl, bufId);
}
    
int DISPLAY_layoutRun(DISPLAY_BufInfo *pDisplayBuf, DISPLAY_LayoutInfo *layoutInfo)
{
  int i, bufId, status, winId;

  DISPLAY_LayoutWinCtrl *pWinCtrl;
  Bool doFill;
  OSA_BufInfo *pBufInfo;
  DRV_FrameCopyPrm frameCopyPrm;
  
  //OSA_printf(" DISPLAY: Layout: %d %d\n", layoutInfo->numWin, layoutInfo->layoutMode);

  for(i=0; i<layoutInfo->numWin; i++)
  {
    if(layoutInfo->layoutMode==DISPLAY_LAYOUT_MODE_PLAYBACK) {
      winId = DISPLAY_LAYOUT_PLAYBACK_WIN;
    } else {
      winId = i;
    }
    
		pWinCtrl = &gDISPLAY_layoutCtrl.winCtrl[winId];
		
		status = OSA_bufGetFull(&pWinCtrl->bufHndl, &bufId, OSA_TIMEOUT_NONE);
		if(status==OSA_SOK) {
		  // got new full buffer, release old one
		  if(pWinCtrl->curFullBufId>=0)
  		  OSA_bufPutEmpty(&pWinCtrl->bufHndl, pWinCtrl->curFullBufId);
      
      pWinCtrl->curFullBufId = bufId;
		  pWinCtrl->blankCount=0;
		        
		} else {
		  
		  // no new buffer
      if(winId!=DISPLAY_LAYOUT_PLAYBACK_WIN) {
  		  if(pWinCtrl->blankCount>gDISPLAY_ctrl.blankDetectThres) {
	  	    // blank threshold exceeded, video is probably disconnected, release old buffer
    		  if(pWinCtrl->curFullBufId>=0)
      		  OSA_bufPutEmpty(&pWinCtrl->bufHndl, pWinCtrl->curFullBufId);
        
          pWinCtrl->curFullBufId = -1;		  
	  	  } else {
	  	    // blank threshold not exceeded, increment balnk count
     		  pWinCtrl->blankCount++;
        }
      }
		}

    doFill = TRUE; // fill with blank video
      
    if(pWinCtrl->curFullBufId>=0) {   
    
      // video buffer available for copy 
      
      pBufInfo = OSA_bufGetBufInfo(&pWinCtrl->bufHndl, pWinCtrl->curFullBufId);
      OSA_assert(pBufInfo!=NULL);
      
      if(pBufInfo->flags!=layoutInfo->layoutMode) {
        // video buffer layout resolution doesnot match current layout resolution so release it
        OSA_bufPutEmpty(&pWinCtrl->bufHndl, pWinCtrl->curFullBufId);		  
        pWinCtrl->curFullBufId = -1;
        pBufInfo = NULL;
		  } else {
		    // layout mode matches, copy video to display buffer
		    doFill  = FALSE;
      }
		}
		
    frameCopyPrm.srcPhysAddr = NULL;
    frameCopyPrm.srcVirtAddr = NULL;
    frameCopyPrm.dstPhysAddr = pDisplayBuf->physAddr;
    frameCopyPrm.dstVirtAddr = pDisplayBuf->virtAddr;
    frameCopyPrm.srcOffsetH  = OSA_align(layoutInfo->winInfo[winId].width, 32); 
    frameCopyPrm.srcOffsetV  = layoutInfo->winInfo[winId].height; 
    frameCopyPrm.dstOffsetH  = gDISPLAY_layoutCtrl.displayOffsetH; 
    frameCopyPrm.dstOffsetV  = gDISPLAY_layoutCtrl.displayOffsetV; 
    frameCopyPrm.copyWidth   = layoutInfo->winInfo[winId].width;  
    frameCopyPrm.copyHeight  = layoutInfo->winInfo[winId].height; 
    frameCopyPrm.dataFormat  = DRV_DATA_FORMAT_YUV420; 
    frameCopyPrm.srcStartX   = 0;  
    frameCopyPrm.srcStartY   = 0;  
    frameCopyPrm.dstStartX   = layoutInfo->winInfo[winId].startX;  
    frameCopyPrm.dstStartY   = layoutInfo->winInfo[winId].startY;
          
		if(!doFill) {
      frameCopyPrm.srcPhysAddr = pBufInfo->physAddr;
      frameCopyPrm.srcVirtAddr = pBufInfo->virtAddr;
    }
    
    {
      #ifdef DISPLAY_DEBUG_RUNNING
      OSA_printf(" DISPLAY: %d: %d, %d %dx%d\n", 
        i,
        layoutInfo->winInfo[winId].startX, layoutInfo->winInfo[winId].startY,
        layoutInfo->winInfo[winId].width, layoutInfo->winInfo[winId].height      
        );
      #endif
      
      OSA_prfBegin(&gDISPLAY_ctrl.prfDma);
      
      status = DRV_frameCopy(NULL, &frameCopyPrm);
      if(status!=OSA_SOK)
        OSA_ERROR("DRV_frameCopy()\n");
        
      OSA_prfEnd(&gDISPLAY_ctrl.prfDma, 1);        
    }
  }

  return OSA_SOK;
}

