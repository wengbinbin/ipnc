#include <stdio.h>
#include <stdlib.h>
#include <osa_cmem.h>
#include "osa_cpuload.h"

//#define LOAD_DEBUG

#ifdef CPULOAD_PROFILE
typedef struct{
	unsigned long user;			// user space
	unsigned long nice;			// user low priority
	unsigned long system;		// system
	unsigned long idle;			// idle
	unsigned long iowait;		// iowait
	unsigned long irq;			// hw irq
	unsigned long softirq;		// sw irq
	unsigned long user_rt;		// steal
	unsigned long system_rt;	// quest

	struct timeval time;	
}OSA_cpuLoad;

typedef struct{
	unsigned long utime;			// user space
	unsigned long stime;			// user space
	unsigned long cutime;			// user space
	unsigned long cstime;			// user space
}OSA_procLoad;

OSA_cpuLoad Cpuload_Start;
OSA_procLoad Procload_Start;
OSA_cpuLoad Cpuload_Last;
OSA_procLoad Procload_Last;
int CpuLoad_StartFlag=0;

static int OSA_getprocstat(OSA_cpuLoad *cpuload, OSA_procLoad *procload)
{
	FILE *fp_in;
	int i,tmp,cnt=0;
	char *ptr, buf[10];
	
	fp_in = fopen("/proc/stat","r");
	if (fp_in == NULL)
	{
		printf("Error open file [/proc/stat]\n");
		return -1;
	}
	rewind(fp_in);
	fflush(fp_in);

	fscanf(fp_in, "%*s %lu %lu %lu %lu %lu %lu %lu %lu %lu", 
		&cpuload->user, 
		&cpuload->nice, 
		&cpuload->system, 
		&cpuload->idle, 
		&cpuload->iowait, 
		&cpuload->irq, 
		&cpuload->softirq, 
		&cpuload->user_rt, 
		&cpuload->system_rt); 
	fclose(fp_in);

	fp_in = fopen("/proc/self/stat","r");
	if (fp_in == NULL)
	{
		printf("Error open file [/proc/self/stat]\n");
		return -2;
	}
	rewind(fp_in);
	fflush(fp_in);
	fscanf(fp_in, "%*d %*s %*s %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %lu %lu %lu %lu %*", 
		&procload->utime, 
		&procload->stime, 
		&procload->cutime, 
		&procload->cstime);
	fclose(fp_in);
	
	gettimeofday(&cpuload->time, NULL);
	
	return 0;
}

int OSA_loadBegin()
{
	OSA_getprocstat(&Cpuload_Start, &Procload_Start);
	memcpy(&Cpuload_Last,&Cpuload_Start,sizeof(OSA_cpuLoad));
	memcpy(&Procload_Last,&Procload_Start,sizeof(OSA_procLoad));
	CpuLoad_StartFlag = 1;
	return 0;
}

static int OSA_loadPrint(char *modestr, OSA_cpuLoad *Cpuload,OSA_procLoad *Procload)
{
	printf("\n%s%6ums\t",modestr,(Cpuload->time.tv_sec*1000+Cpuload->time.tv_usec/1000)%100000);
	printf("%6d|",Cpuload->user);
	printf("%6d|",Cpuload->nice);
	printf("%6d|",Cpuload->system);
	printf("%6d|",(Cpuload->idle>1000000)?999999:Cpuload->idle);
	printf("%6d|",Cpuload->iowait);
	printf("%6d|",Cpuload->irq);
	printf("%6d|",Cpuload->softirq);
	printf("%6d|",Cpuload->user_rt);
	printf("%6d|",Cpuload->system_rt);

	printf("%6d|",Procload->utime);
	printf("%6d|",Procload->stime);
	printf("%6d|",Procload->cutime);
	printf("%6d|",Procload->cstime);
	
	return 0;
}

int OSA_loadPerf()
{
	OSA_cpuLoad Cpuload_Now,Cpuload_Diff;
	OSA_procLoad Procload_Now,Procload_Diff;
	char *loadstr[]={"user","nice"," sys","idle","io_w"," irq","sirq","u_rt","s_rt"};
	char *procstr[]={"  ut","  st"," cut"," cst"};
	int i,sum,load,sys;
	
	if (CpuLoad_StartFlag==0){
		OSA_loadBegin();
		return 0;
	}
	OSA_getprocstat(&Cpuload_Now, &Procload_Now);

	printf("\n\t\t");
	for (i=0;i<9;i++)
		printf("  %s ",loadstr[i]);
	for (i=0;i<4;i++)
		printf("  %s ",procstr[i]);

#ifdef LOAD_DEBUG
	OSA_loadPrint("all",&Cpuload_Now,&Procload_Now);
#endif
	Cpuload_Diff.user 	= Cpuload_Now.user 		- Cpuload_Start.user;
	Cpuload_Diff.nice 	= Cpuload_Now.system 	- Cpuload_Start.system;
	Cpuload_Diff.system = Cpuload_Now.system 	- Cpuload_Start.system;
	Cpuload_Diff.idle 	= Cpuload_Now.idle 		- Cpuload_Start.idle;
	Cpuload_Diff.irq 	= Cpuload_Now.irq 		- Cpuload_Start.irq;
	Cpuload_Diff.iowait = Cpuload_Now.iowait 		- Cpuload_Start.iowait;
	Cpuload_Diff.softirq= Cpuload_Now.softirq 	- Cpuload_Start.softirq;
	Cpuload_Diff.user_rt= Cpuload_Now.user_rt 	- Cpuload_Start.user_rt;
	Cpuload_Diff.system_rt= Cpuload_Now.system_rt - Cpuload_Start.system_rt;
	Cpuload_Diff.time.tv_sec= Cpuload_Now.time.tv_sec - Cpuload_Start.time.tv_sec;
	Cpuload_Diff.time.tv_usec= Cpuload_Now.time.tv_usec - Cpuload_Start.time.tv_usec;

	Procload_Diff.utime	= Procload_Now.utime- Procload_Start.utime;
	Procload_Diff.stime	= Procload_Now.stime- Procload_Start.stime;
	Procload_Diff.cutime= Procload_Now.cutime- Procload_Start.cutime;
	Procload_Diff.cstime= Procload_Now.cstime- Procload_Start.cstime;
	OSA_loadPrint("diff",&Cpuload_Diff,&Procload_Diff);
	sys=load=sum = 0;
	sys += Cpuload_Diff.user;
	sys += Cpuload_Diff.nice;
	sys += Cpuload_Diff.system;
	sys += Cpuload_Diff.irq;
	sys += Cpuload_Diff.softirq;
	sys += Cpuload_Diff.user_rt;
	sys += Cpuload_Diff.system_rt;
	sum = sys;
	sum += Cpuload_Diff.idle;
	sum += Cpuload_Diff.iowait;
	load += Procload_Diff.utime;
	load += Procload_Diff.stime;
	load += Procload_Diff.cutime;
	load += Procload_Diff.cstime;
	printf("\n Average sys=%u.%u%% app=%u.%u%%",sys*100/sum,(sys*10000/sum)%100,load*100/sum,(load*10000/sum)%100);
	Cpuload_Diff.user 	= Cpuload_Now.user 		- Cpuload_Last.user;
	Cpuload_Diff.nice 	= Cpuload_Now.system 	- Cpuload_Last.system;
	Cpuload_Diff.system = Cpuload_Now.system 	- Cpuload_Last.system;
	Cpuload_Diff.idle 	= Cpuload_Now.idle 		- Cpuload_Last.idle;
	Cpuload_Diff.irq 	= Cpuload_Now.irq 		- Cpuload_Last.irq;
	Cpuload_Diff.iowait 	= Cpuload_Now.iowait 		- Cpuload_Last.iowait;
	Cpuload_Diff.softirq= Cpuload_Now.softirq 	- Cpuload_Last.softirq;
	Cpuload_Diff.user_rt= Cpuload_Now.user_rt 	- Cpuload_Last.user_rt;
	Cpuload_Diff.system_rt= Cpuload_Now.system_rt - Cpuload_Last.system_rt;
	Cpuload_Diff.time.tv_sec= Cpuload_Now.time.tv_sec - Cpuload_Last.time.tv_sec;
	Cpuload_Diff.time.tv_usec= Cpuload_Now.time.tv_usec - Cpuload_Last.time.tv_usec;
	Procload_Diff.utime	= Procload_Now.utime- Procload_Last.utime;
	Procload_Diff.stime	= Procload_Now.stime- Procload_Last.stime;
	Procload_Diff.cutime= Procload_Now.cutime- Procload_Last.cutime;
	Procload_Diff.cstime= Procload_Now.cstime- Procload_Last.cstime;
	OSA_loadPrint("diff",&Cpuload_Diff,&Procload_Diff);
	sys=load=sum = 0;
	sys += Cpuload_Diff.user;
	sys += Cpuload_Diff.nice;
	sys += Cpuload_Diff.system;
	sys += Cpuload_Diff.irq;
	sys += Cpuload_Diff.softirq;
	sys += Cpuload_Diff.user_rt;
	sys += Cpuload_Diff.system_rt;
	sum = sys;
	sum += Cpuload_Diff.idle;
	sum += Cpuload_Diff.iowait;
	load += Procload_Diff.utime;
	load += Procload_Diff.stime;
	load += Procload_Diff.cutime;
	load += Procload_Diff.cstime;
	printf("\n Recent sys=%u.%u%% app=%u.%u%%\n",sys*100/sum,(sys*10000/sum)%100,load*100/sum,(load*10000/sum)%100);
	memcpy(&Cpuload_Last,&Cpuload_Now,sizeof(OSA_cpuLoad));
	memcpy(&Procload_Last,&Procload_Now,sizeof(OSA_procLoad));
	return 0;
}

#ifdef LOAD_DEBUG
int main(int argc, char *argv[])
{
	int i,cnt=0,time,cntall;
	
	if (argc==3)
		cntall = atoi(argv[2]);
	else if (argc==2)
		time = atoi(argv[1]);	
	else
		time = 2;
	OSA_loadBegin();
	while(1){
		cnt++;
		if (cnt==cntall) break;
		OSA_loadPerf();
		sleep(time);
	}
	return 0;
}
#endif
#endif

#ifdef CMEM_PROFILE
OSA_cmemStat CmemStat={0,0,0};
int OSA_cmemPerf(int details)
{
	int idx;
	printf("\n CMEM_STAT cached_size=%d(%2.2fMB) noncached_size=%d(%4.2fMB) cnt=%d\n",
		CmemStat.size_cached,(float)CmemStat.size_cached/1024/1024,
		CmemStat.size_nocache,(float)CmemStat.size_nocache/1024/1024,
		CmemStat.count);
	if (details){
		for (idx=0;idx<CMEM_MAXIDX;idx++){
		  if (CmemStat.info[idx].size != 0)
			printf("idx=%2d %s virtadd=0x%08x phyadd=0x%08x size=%7dB(%4.2fKB)\n",
				idx,(CmemStat.info[idx].type==CMEM_CACHED)?" cached":"nocache",
				CmemStat.info[idx].vaddr,
				CmemStat.info[idx].paddr,
				CmemStat.info[idx].size,(float)CmemStat.info[idx].size/1024);
		}
	}
}
#endif
