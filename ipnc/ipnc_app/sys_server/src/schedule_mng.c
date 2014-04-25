/* ===========================================================================
 * @path $(IPNCPATH)\sys_adm\system_server
 *
 * @desc
 * .
 * Copyright (c) Appro Photoelectron Inc.  2008
 *
 * Use of this software is controlled by the terms and conditions found
 * in the license agreement under which this software has been supplied
 *
 * =========================================================================== */
/**
 * @file schedule_mng.c
 * @brief Schedule manager.
 */
 #include<math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Msg_Def.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <sys_env_type.h>
#include <schedule_mng.h>
#include "semaphore_util.h"
#include "file_mng.h"
#include <unistd.h>
#include <sys/time.h>

#include <ApproDrvMsg.h>
#include <Appro_interface.h>
#include <Msg_Def.h>
#include <alarm_msg_drv.h>
#include <sys_env_type.h>
#include <sem_util.h>

#define _SCHED_DEBUG
#ifdef _SCHED_DEBUG
#define __D(fmt, args...) fprintf(stderr,  fmt, ## args)
#else
#define __D(fmt, args...)
#endif

#define __E(fmt, args...) fprintf(stderr, "Error: " fmt, ## args)
#define REC_DST_SD 0x1
#define REC_DST_FTP 0x2

static SemHandl_t gSemSchedule;
static pthread_t gScheduleThread;
static char gbSchedulePause = 0;

/**
 * @brief Tell if schedule manager is pause.
 * @return If schedule manager is paused or not.
 */
int IsSchedulePause()
{
    int ret;
    SemWait(gSemSchedule);
    ret = gbSchedulePause;
    SemRelease(gSemSchedule);
    return ret;
}

/**
 * @brief Pause current schedule.
 */
void PauseSchedule()
{
    SemWait(gSemSchedule);
    gbSchedulePause = 1;
    SemRelease(gSemSchedule);
    /* Wait the recording stop */
    sleep(1);
}

/**
 * @brief Restart scheduling.
 */
void ResumeSchedule()
{
    SemWait(gSemSchedule);
    gbSchedulePause = 0;
    SemRelease(gSemSchedule);
}

/**
 * @brief    show scheduler info
 * @param    "Schedule_t *pSched_info" : scheduler info
 * @param    "int showcnt" : show count
 * @retval    0 : success ; -1 : fail
 */
void ShowSched_info(Schedule_t *pSched_info, int showcnt)
{
    //int        showcnt = 0;

    //while( showcnt < 7 )
    {
        __D("num = %d ", showcnt);
        __D("bStatus = %d ", pSched_info[showcnt].bStatus);
        __D("nDay = %d ", pSched_info[showcnt].nDay);
        __D("nHour = %d ", pSched_info[showcnt].tStart.nHour);
        __D("nMin = %d ", pSched_info[showcnt].tStart.nMin);
        __D("nSec = %d ", pSched_info[showcnt].tStart.nSec);
        __D("nHour = %d ", pSched_info[showcnt].tDuration.nHour);
        __D("nMin = %d ", pSched_info[showcnt].tDuration.nMin);
        __D("nSec = %d \n", pSched_info[showcnt].tDuration.nSec);
        //showcnt++;

    }
}

/**
 * @brief    show current time
 * @param    none
 * @retval    none
 */
void ShowCurTime(void)
{
    #ifdef _SCHED_DEBUG
    char *wday[] =
    {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };
    #endif
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);
    __D("current time is \n");
    __D("year:%d month:%d day:%d \n", (1900+p->tm_year), (1+p->tm_mon), p->tm_mday);
    __D("%s wdays:%d hour:%d min:%d sec:%d\n", wday[p->tm_wday], p->tm_wday, p->tm_hour, p->tm_min, p->tm_sec);

}

/**
 * @brief    get current time in format of "struct tm"
 * @retval    pointer to "struct tm"
 */
struct tm *schedGetCurTime(void)
{
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);

    return p;
}

inline int TranslatDays(int CurDay)
{
    if (CurDay == 0)
    {
        return 7;
    }
    else
    {
        return CurDay;
    }
}

/**
 * @brief    check time range
 * @param    Schedule_t *pSched_info
 * @retval    1 : in range; 0 : not in range
 */
int schedCheckTimeRange(Schedule_t *pSched_info, int schedDay, int schedYear, int schedWeeks, int schedInfinite)
{
    struct tm *pCurTime = schedGetCurTime();
    int cnt = 0;
    int use_hours = 0;
    int hours_end = 0;
    int min_end = 0;
    int IsInRange = 0;

    /****** Check for Expire ********/
    int endDay = schedDay + schedWeeks * 7; //multiply by number of days in week
    int endYear = schedYear;

    if (schedInfinite == 0)
    {
        while (endDay > 365)
        {
            endDay -= 365;
            endYear++;
        }

        if ((pCurTime->tm_yday > endDay) && (pCurTime->tm_year >= endYear))
        {
            printf("WARNING: SCHEDULE EXPIRED AFTER %d WEEKS\n", schedWeeks);
            return IsInRange;
        }
    }
    /*********************************/

    for (cnt = 0; cnt < SCHDULE_NUM; cnt++)
    {
        /*check if enable*/
        if (pSched_info[cnt].bStatus == 0)
        {
            //__D("line_%d \n",__LINE__);
            continue;
        }

        /*check if wdays is match*/
        if (TranslatDays(pCurTime->tm_wday) != pSched_info[cnt].nDay)
        {
            /*check if everyday*/
            if (pSched_info[cnt].nDay != 8)
            {
                //__D("line_%d \n",__LINE__);
                continue;
            }
        }

        /*check if hours after start*/
        if (pCurTime->tm_hour < pSched_info[cnt].tStart.nHour)
        {
            //__D("line_%d \n",__LINE__);
            continue;
        }

        /*check if Mins after start*/
        if (pCurTime->tm_min < pSched_info[cnt].tStart.nMin)
        {
            //__D("line_%d \n",__LINE__);
            continue;
        }

        /*check if hours before end*/
        use_hours = (pSched_info[cnt].tStart.nMin + pSched_info[cnt].tDuration.nMin) / 60+

        pSched_info[cnt].tDuration.nHour;
        hours_end = pSched_info[cnt].tStart.nHour + use_hours;
        if (pCurTime->tm_hour > hours_end)
        {
            //__D("line_%d \n",__LINE__);
            continue;
        }

        /*check if Mins before end*/
        min_end = (pSched_info[cnt].tStart.nMin + pSched_info[cnt].tDuration.nMin) % 60;
        if (pCurTime->tm_hour == hours_end)
        {
            if (pCurTime->tm_min > min_end)
            {
                //__D("line_%d \n",__LINE__);
                continue;
            }
        }

        IsInRange = 1;
        break;
    }
    ShowSched_info(pSched_info, 0);
    ShowCurTime();
    if (IsInRange == 1)
    {
        /*display debug message*/
        __D("Setting %d is scheduling \n", (cnt + 1));
        ShowSched_info(pSched_info, cnt);
    }
    else
    {
        __D("No scheduling \n");
        cnt =  - 1;
    }
    return cnt;
}

/**
 * @brief Get recoding destination
 * @param pSysInfo Pointer to system information
 * @return Recoding destination
 */
int GetRecordDst(SysInfo *pSysInfo)
{
    int dst = 0;
    if (pSysInfo->sdcard_config.sdrenable)
    {
        dst |= REC_DST_SD;
    }
    if (pSysInfo->ftp_config.rftpenable)
    {
        dst |= REC_DST_FTP;
    }
    return dst;
}

/**
 * @brief    Scheduler thread
 * @param    void* args
 * @retval    none
 */
void *scheduleThrFxn(void *args)
{
    void *status = NULL;
    SysInfo *pSysInfo = (SysInfo*)args;
    int nSleepDuration = 5;
    int index = 0, count = 0;
    int i,j;
    int sum=0;
    int isinfinite[4]={0,0,0,0};
//    int isrecord[4]={0,0,0,0};
    int isrange[4]={0,0,0,0};
    int flagchange1=0;
    int flagchange2=0;
    int ret;

    sleep(nSleepDuration);
    printf("scheduleThrFxn beging");
    while (IsFileThreadQuit() == 0)
    {
        printf("scheduleThrFxn beging while\n");
        for(i=0;i<4;i++)  //check 
        {
            //check whethe flag and ScheduleInfiniteEnable are same
            //excute if they are diffrent
            if(isinfinite[i]!=pSysInfo->storage_config[i].nScheduleInfiniteEnable)
            {
                    isinfinite[i]=pSysInfo->storage_config[i].nScheduleInfiniteEnable;
//                    sum+=isinfinite[i]*(int)(pow(2.0,i));
                    flagchange1=1;                   
            }
        }      
        for(i=0;i<4;++i)
        {
            if(pSysInfo->storage_config[i].nScheduleRepeatEnable==0){
                isrange[i]=0;
                continue;     //if there is no record run next channel;
            }
            Schedule_t *pSched_info = &pSysInfo->storage_config[i].aSchedules[0];
            int schedDay = 0, schedYear = 0, schedWeeks = 52, schedInfinite = 0;
            schedInfinite = pSysInfo->storage_config[i].nScheduleInfiniteEnable;
            schedDay = pSysInfo->storage_config[i].schedCurDay;
            schedYear = pSysInfo->storage_config[i].schedCurYear;
            schedWeeks = pSysInfo->storage_config[i].nScheduleNumWeeks;            
            index = schedCheckTimeRange(pSched_info, schedDay, schedYear, schedWeeks, schedInfinite);
//            tCurStatus = GetRecordDst(pSysInfo);
            printf("index=%d\n",index);
            if (index >=0&&isrange[i]==0)
            {
                isrange[i]=1;
                flagchange2=1;
            }
            if (index <0&&isrange[i]>0)
            {
                isrange[i]=0;
                flagchange2=1;
            }      
        }
        
        if(flagchange1==1){
            sum=0;
            for(i=0;i<4;i++) {
                sum+=isinfinite[i]*(int)(pow(2.0,i));             
            }
            ret=SetChRecEnableMsg(sum);
            printf("ret=%d\n",ret);
            flagchange1=0;
            printf("SetChRecEnableMsg1 is success\n");
            printf("sum=%d,%c\n",sum,sum); 
        }
        sleep(nSleepDuration);
        if(flagchange2==1){
            sum=0;
            for(i=0;i<4;i++) {
                sum+=isinfinite[i]*(int)(pow(2.0,i));  
                sum+=isrange[i]*(int)(pow(2.0,i));
            }         
            ret=SetChRecEnableMsg(sum);
            printf("ret=%d\n",ret);
            flagchange2=0;
            printf("SetChRecEnableMsg2 is success\n");
            printf("sum=%d,%c\n",sum,sum); 
        }
        sleep(nSleepDuration);
    }
    return status;
}

/**
 * @breif Schedule manager initial.
 * @param pSysInfo [I] Pointer to system info.
 * @return 0 on success
 */
int ScheduleMngInit(SysInfo *pSysInfo)
{
    if (gSemSchedule == NULL)
    {
        gSemSchedule = MakeSem();
    }
    if (gSemSchedule == NULL)
    {
        return  - 1;
    }
    scheduleThrFxn(pSysInfo);
   /*
    if (pthread_create(&gScheduleThread, NULL, scheduleThrFxn, pSysInfo))
    {
        DestroySem(gSemSchedule);
        __E("scheduleThread create fail\n");
        return  - 1;
    }
    __D("scheduleThread created\n");
    */
    return 0;
}
