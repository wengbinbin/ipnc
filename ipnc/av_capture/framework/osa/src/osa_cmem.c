
#include <osa_cmem.h>
#include <osa_cpuload.h>
//#define OSA_DEBUG_CMEM

Uint8 *OSA_cmemAllocCached(Uint32 size, Uint32 alignment)
{
  CMEM_AllocParams  prm;
  Uint8 *virtAddr;
  
  prm.type = CMEM_HEAP;
  prm.flags = CMEM_CACHED;
  prm.alignment = alignment;
  
  virtAddr = (Uint8*)CMEM_alloc(size, &prm);
  
  #ifdef OSA_DEBUG_CMEM  
  OSA_printf(" OSA_CMEM: %08x %d bytes\n", (Uint32)virtAddr, size );    
  #endif
#ifdef CMEM_PROFILE
{
	extern OSA_cmemStat CmemStat;
	int idx=0;
	CmemStat.size_cached +=size;
	while(CmemStat.info[idx].size!=0 && idx<CMEM_MAXIDX) idx++;
	if (idx <CMEM_MAXIDX){
	  CmemStat.info[idx].type=CMEM_CACHED;
	  CmemStat.info[idx].size=size;
		  CmemStat.info[idx].vaddr=virtAddr;
		  CmemStat.info[idx].paddr=OSA_cmemGetPhysAddr(virtAddr);
	}
	CmemStat.count++;
}
#endif
      
  return virtAddr;
}

Uint8 *OSA_cmemAlloc(Uint32 size, Uint32 alignment)
{
  CMEM_AllocParams  prm;
  Uint8 *virtAddr;
  
  prm.type = CMEM_HEAP;
  prm.flags = CMEM_NONCACHED;
  prm.alignment = alignment;
  
  virtAddr = (Uint8*)CMEM_alloc(size, &prm);
  
  #ifdef OSA_DEBUG_CMEM  
  OSA_printf(" OSA_CMEMALLOC: %08x %d bytes\n", (Uint32)virtAddr, size );    
  #endif

#ifdef CMEM_PROFILE
{
  extern OSA_cmemStat CmemStat;
  int idx=0;
	CmemStat.size_nocache +=size;
	while(CmemStat.info[idx].size!=0 && idx<CMEM_MAXIDX) idx++;
	if (idx <CMEM_MAXIDX){
		CmemStat.info[idx].type=CMEM_NONCACHED;
		CmemStat.info[idx].size=size;
		CmemStat.info[idx].vaddr=virtAddr;
		CmemStat.info[idx].paddr=OSA_cmemGetPhysAddr(virtAddr);
	}
	CmemStat.count++;
}
#endif
      
  return virtAddr;
}


int OSA_cmemFree(Uint8 *ptr)
{
  CMEM_AllocParams  prm;
  
  if(ptr==NULL)
    return OSA_EFAIL;
    
  prm.type = CMEM_HEAP;
  prm.flags = 0;
  prm.alignment = 0;

  #ifdef OSA_DEBUG_CMEM  
  OSA_printf(" OSA_CMEMFREE: %08x %d bytes\n", (Uint32)ptr);    
  #endif



#ifdef CMEM_PROFILE
{
	extern OSA_cmemStat CmemStat;
	int i;
	for (i=0;i<CMEM_MAXIDX;i++){
		if (CmemStat.info[i].vaddr == (Uint32)ptr){
			if (CmemStat.info[i].type == CMEM_CACHED){
				CmemStat.size_cached  -=CmemStat.info[i].size;
			}
			else if (CmemStat.info[i].type == CMEM_NONCACHED){
				CmemStat.size_nocache -=CmemStat.info[i].size;
			}
			CmemStat.info[i].type=0;
			CmemStat.info[i].size=0;
			CmemStat.info[i].vaddr=NULL;
			CmemStat.info[i].paddr=NULL;
			CmemStat.count--;
			break;
		}
	}
}
#endif

  return CMEM_free(ptr, &prm);
}

Uint8* OSA_cmemGetPhysAddr(Uint8 *virtAddr)
{
  Uint8 *physAddr;
  if(virtAddr==NULL)
    return NULL;
    
  physAddr = (Uint8*)CMEM_getPhys(virtAddr);
  
  #ifdef OSA_DEBUG_CMEM
  OSA_printf(" OSA_CMEM: %08x, %08x\n", (Uint32)virtAddr, (Uint32)physAddr );    
  #endif
  
  return physAddr;
}

int OSA_cmemTestMain(int argc, char **argv)
{ 
  Uint32 size;
  Uint8 *virtAddr[100], *physAddr[100];
  int i, num=1;
    
  CMEM_init();

  if(argc < 3) {
    OSA_printf(" USAGE : %s cmem <size>\n", argv[0]);
    return 0;
  }
  
  size = atoi(argv[2])/num;
  
  for(i=0; i<num; i++) {
    virtAddr[i] = OSA_cmemAlloc(size, 32);
    
    physAddr[i] = OSA_cmemGetPhysAddr(virtAddr[i]);
  
    if(virtAddr[i]!=NULL)
      printf("VIRT=0x%08x PHYS=0x%08x SIZE=%d\n", (Uint32)virtAddr[i], (Uint32)physAddr[i], size);
    else
      printf("ERROR: could not allocate 0x%08x size memory\n", size);
  }
  
  for(i=0; i<num; i++)
    OSA_cmemFree(virtAddr[i]);
 
  CMEM_exit();

  return 0;
}

