#include <stdio.h>
#include <stdlib.h>

#define CPULOAD_PROFILE
#define CMEM_PROFILE

#ifdef CPULOAD_PROFILE
int OSA_loadBegin();
int OSA_loadPerf();
#endif

#ifdef CMEM_PROFILE
#define CMEM_MAXIDX	128


typedef struct{
	int type;
	int size;
	unsigned int vaddr;
	unsigned int paddr;
}OSA_cmemInfo;
typedef struct{
	int count;
	int size_cached;
	int size_nocache;
	OSA_cmemInfo info[CMEM_MAXIDX];
}OSA_cmemStat;
extern OSA_cmemStat CmemStat;
int OSA_cmemPerf();
#endif
