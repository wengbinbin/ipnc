

#include <alg_priv.h>
#include <vs_ti.h>
#include <alg_vstab.h>
#include <osa_cmem.h>
#include <drv_ipipe.h>

#define ALG_BSC_DATA_SIZE_MAX   (DRV_IPIPE_BSC_BUFFER_MAX)

typedef struct {

  ALG_VstabCreate createPrm;
  
  Uint32 prevBscDataBuf[ ALG_BSC_DATA_SIZE_MAX/4 ];
  
  int runCount;

} ALG_VsObj;

void *ALG_vstabCreate(ALG_VstabCreate *create)
{
  ALG_VsObj *pObj;
  int status;
  IVS1_Params vsPrm;

  if(create==NULL)
    return NULL;
    
  if(create->pBscConfig==NULL)
    return NULL;
  
  pObj = OSA_memAlloc(sizeof(*pObj));
  if(pObj==NULL)
    return NULL;
    
  memset(pObj, 0, sizeof(*pObj));
  memcpy(&pObj->createPrm, create, sizeof(pObj->createPrm));

  vsPrm.size        = sizeof(vsPrm);    
  vsPrm.bscInWidth  = create->stabFrameWidth;
  vsPrm.bscInHeight = create->stabFrameHeight;
  vsPrm.row_hpos    = create->pBscConfig->rowStartH;
  vsPrm.row_vpos    = create->pBscConfig->rowStartV;
  vsPrm.row_hskip   = create->pBscConfig->rowSkipH;
  vsPrm.row_vskip   = create->pBscConfig->rowSkipV;
  vsPrm.row_hnum    = create->pBscConfig->rowNumH;
  vsPrm.row_vnum    = create->pBscConfig->rowNumV;
  vsPrm.row_vct     = create->pBscConfig->rowNumVectors;
  vsPrm.row_shf     = create->pBscConfig->rowInShift;
  vsPrm.col_hpos    = create->pBscConfig->colStartH;
  vsPrm.col_vpos    = create->pBscConfig->colStartV;
  vsPrm.col_hskip   = create->pBscConfig->colSkipH;
  vsPrm.col_vskip   = create->pBscConfig->colSkipV;
  vsPrm.col_hnum    = create->pBscConfig->colNumH;
  vsPrm.col_vnum    = create->pBscConfig->colNumV;
  vsPrm.col_vct     = create->pBscConfig->colNumVectors;
  vsPrm.col_shf     = create->pBscConfig->colInShift;

  status = VS_TI_create(&vsPrm);
  if(status!=OSA_SOK) {
    OSA_ERROR("VS_TI_create()\n");
    OSA_memFree(pObj);
    return NULL;
  }

  return pObj;
}

int ALG_vstabRun(void *hndl, ALG_VstabRunPrm *prm, ALG_VstabStatus *status)
{
  ALG_VsObj *pObj = (ALG_VsObj *)hndl;
  IVS1_InArgs inArgs;
  IVS1_OutArgs outArgs;
  int ret;

  if(pObj==NULL || prm==NULL)
    return OSA_EFAIL;

  OSA_cmemCacheInv(prm->bscDataVirtAddr, ALG_BSC_DATA_SIZE_MAX);
  
  status->startX = (pObj->createPrm.totalFrameWidth-pObj->createPrm.stabFrameWidth)/2;
  status->startY = (pObj->createPrm.totalFrameHeight-pObj->createPrm.stabFrameHeight)/2;  
    
  pObj->runCount++;
  
  //if(pObj->runCount > 1) 
  {
    inArgs.size = sizeof(inArgs);
    inArgs.CurBscMemAddrH  = (Uint32)prm->bscDataVirtAddr + ALG_BSC_DATA_SIZE_MAX/2;
    inArgs.CurBscMemAddrV  = (Uint32)prm->bscDataVirtAddr;
    inArgs.PrevBscMemAddrH = (Uint32)pObj->prevBscDataBuf + ALG_BSC_DATA_SIZE_MAX/2;
    inArgs.PrevBscMemAddrV = (Uint32)pObj->prevBscDataBuf ;

    outArgs.size = sizeof(outArgs);
  
    ret = VS_TI_apply(&inArgs, &outArgs);
    if(ret!=OSA_SOK)
      return OSA_EFAIL;
    
    #if 1
    status->startX = (outArgs.startX*pObj->createPrm.stabFrameWidth)/outArgs.winWidth;  
    status->startX /= 4;
    #endif
    
    status->startY = (outArgs.startY*pObj->createPrm.stabFrameHeight)/outArgs.winHeight;    
    status->startY /= 4;  
    
    #ifdef ALG_VS_DEBUG
    OSA_printf(" ALG: Vs: %ld,%ld %ldx%ld -> %d, %d\n", 
      outArgs.startX, outArgs.startY, outArgs.winWidth, outArgs.winHeight, status->startX, status->startY
      ); 
    #endif 
  } 
    
  memcpy(pObj->prevBscDataBuf, prm->bscDataVirtAddr, ALG_BSC_DATA_SIZE_MAX);
  
  return OSA_SOK;
}

int ALG_vstabDelete(void *hndl)
{
  ALG_VsObj *pObj = (ALG_VsObj *)hndl;
  
  if(pObj==NULL)
    return OSA_EFAIL;
    
  VS_TI_delete();
  
  OSA_memFree(pObj);
  
  return OSA_SOK;
}

