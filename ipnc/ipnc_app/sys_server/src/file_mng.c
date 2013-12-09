/* ===========================================================================
 * @file file_mng.c
 *
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <file_mng.h>
#include <sys_env_type.h>
#include <system_default.h>
#include <sys_info_default.h>

#define MAX_LOG_PAGE_NUM    20
#define NUM_LOG_PER_PAGE    20
#define LOG_ENTRY_SIZE    sizeof(LogEntry_t)

#ifdef __FILE_DEBUG
#define DBG(fmt, args...) fprintf(stdout, "FileMng: Debug " fmt, ##args)
#else
#define DBG(fmt, args...)
#endif

#define ERROR(fmt,args...)    printf("FileMng: Error " fmt, ##args)

typedef struct LogData_t
{
    LogEntry_t tLogData;
    struct LogData_t *pNext;
} LogData_t;

static LogData_t *gLogHead = NULL;

/**
 * @brief    check magic number
 * @param    fp [I ]file pointer
 * @return    error code : SUCCESS(0) or FAIL(-1)
 */
int check_magic_num(FILE *fp)
{
    int ret;
    unsigned long MagicNum;
    if (fread(&MagicNum, 1, sizeof(MagicNum), fp) != sizeof(MagicNum))
    {
        ret = FAIL;
    }
    else
    {
        #if DEBUG==1
        printf("MagicNum=%lx, MAGIC_NUM=%x\n", MagicNum, MAGIC_NUM);
        #endif

        if (MagicNum == MAGIC_NUM)
        {
            ret = SUCCESS;
        }
        else
        {
            ret = FAIL;
        }
    }
    return ret;
}

/**
 * @brief    Add new log event to tail
 * @return    error code : SUCCESS(0) or FAIL(-1)
 */
int AddLogToTail(LogEntry_t *pLog)
{
    LogData_t *pLogData,  *pTail = NULL;
    DBG("pLog->tLogData.event=%s\n", pLog->event);
    DBG("pLog->tLogData.time=%s\n", asctime(&pLog->time));
    pLogData = (LogData_t*)malloc(sizeof(LogData_t));
    if (pLogData == NULL)
    {
        DBG("No enough memory\n");
        if (gLogHead == NULL)
        {
            return FAIL;
        }
        /* Do Nothing if no more memory */
    }
    else
    {
        memcpy(&pLogData->tLogData, pLog, sizeof(LogEntry_t));
        pLogData->pNext = NULL;
        if (gLogHead == NULL)
        {
            gLogHead = pLogData;
        }
        else
        {
            for (pTail = gLogHead; pTail->pNext != NULL;)
            {
                pTail = pTail->pNext;
            }
            pTail->pNext = pLogData;
        }
    }
    return SUCCESS;
}

/**
 * @brief    Add new log event
 * @return    error code : SUCCESS(0) or FAIL(-1)
 */
int AddLog(LogEntry_t *pLog)
{
    LogData_t *pLogData,  *pFrontData = NULL;
    DBG("pLog->tLogData.event=%s\n", pLog->event);
    DBG("pLog->tLogData.time=%s\n", asctime(&pLog->time));
    pLogData = (LogData_t*)malloc(sizeof(LogData_t));
    if (pLogData == NULL)
    {
        DBG("No enough memory\n");
        if (gLogHead == NULL)
        {
            return FAIL;
        }
        /* If no more memory, replace the oldest one with current. */
        pLogData = gLogHead;
        while (pLogData->pNext != NULL)
        {
            pFrontData = pLogData;
            pLogData = pLogData->pNext;
        }
        memcpy(&pLogData->tLogData, pLog, sizeof(LogEntry_t));
        if (pFrontData != NULL)
        {
            pFrontData->pNext = NULL;
            pLogData->pNext = gLogHead;
            gLogHead = pLogData;
        }
    }
    else
    {
        memcpy(&pLogData->tLogData, pLog, sizeof(LogEntry_t));
        pLogData->pNext = gLogHead;
        gLogHead = pLogData;
    }
    DBG("gLogHead->tLogData.event=%s\n", gLogHead->tLogData.event);
    DBG("gLogHead->tLogData.time=%s\n", asctime(&gLogHead->tLogData.time));
    return SUCCESS;
}

/**
 * @brief    Show all system log
 * @return
 */
void ShowAllLog()
{
    LogData_t *ptr;
    for (ptr = gLogHead; ptr != NULL; ptr = ptr->pNext)
    {
        fprintf(stderr, "Event:%s\n", ptr->tLogData.event);
        fprintf(stderr, "Time:%s\n", asctime(&ptr->tLogData.time));
    }
}

/**
 * @brief    Clean system log
 * @return
 */
void CleanLog()
{
    LogData_t *ptr;
    while (gLogHead != NULL)
    {
        ptr = gLogHead;
        gLogHead = gLogHead->pNext;
        free(ptr);
    }
}

/**
 * @brief    read log from log file
 * @return    error code : SUCCESS(0) or FAIL(-1)
 */
int ReadLog()
{
    FILE *fp;
    char Buffer[LOG_ENTRY_SIZE];
    int ret = SUCCESS, count = 0;
    if ((fp = fopen(LOG_FILE, "rb")) == NULL)
    {
        /* log file not exist */
        ret = FAIL;
    }
    else
    {
        if (check_magic_num(fp) == SUCCESS)
        {
            while (count < NUM_LOG_PER_PAGE *MAX_LOG_PAGE_NUM)
            if (fread(Buffer, 1, LOG_ENTRY_SIZE, fp) != LOG_ENTRY_SIZE)
            {
                break;
            }
            else
            {
                if (AddLogToTail((LogEntry_t*)Buffer) != SUCCESS)
                {
                    ret = FAIL;
                    break;
                }
                count++;
            }
        }
        else
        {
            ret = FAIL;
        }
        fclose(fp);
    }
    return ret;
}

/**
 * @brief    create log file
 * @param    "int nPageNum" : [IN]log page number
 * @param    "int nItemIndex" : [IN]log index in that page
 * @return    LogEntry_t*: log data
 */
LogEntry_t *GetLog(int nPageNum, int nItemIndex)
{
    LogData_t *pLog;
    int count, index = nPageNum * NUM_LOG_PER_PAGE + nItemIndex;
    if (nPageNum < 0 || nItemIndex < 0)
    {
        return NULL;
    }
    for (count = 0, pLog = gLogHead; (count < index) && (pLog != NULL); count++)
    {
        pLog = pLog->pNext;
    }
    return  &pLog->tLogData;
}

/**
 * @brief    create log file
 * @param    name [I ]File name to create in nand.
 * @param    pLogData  [I ]pointer to log list
 * @return    error code : SUCCESS(0) or FAIL(-1)
 */
int create_log_file(char *name, LogData_t *pLogData)
{
    FILE *fp;
    int ret, count = 0;
    unsigned long MagicNum = MAGIC_NUM;
    if ((fp = fopen(name, "wb")) == NULL)
    {
        ERROR("Can't create log file\n");
        ret = FAIL;
    }
    else
    {
        if (fwrite(&MagicNum, 1, sizeof(MagicNum), fp) != sizeof(MagicNum))
        {
            ERROR("Writing Magic Number fail\n");
            ret = FAIL;
        }
        else
        {
            ret = SUCCESS;
            while (pLogData != NULL && count < NUM_LOG_PER_PAGE *MAX_LOG_PAGE_NUM)
            if (fwrite(&pLogData->tLogData, 1, LOG_ENTRY_SIZE, fp) != LOG_ENTRY_SIZE)
            {
                ERROR("Writing log fail\n");
                ret = FAIL;
                break;
            }
            else
            {
                count++;
                pLogData = pLogData->pNext;
            }
        }
        fclose(fp);
    }
    return ret;
}

/**
 * @brief    create system file
 * @param    name [I ]File name to create in nand.
 * @param    Global [I ]Pointer to System information
 * @return    error code : SUCCESS(0) or FAIL(-1)
 */
int create_sys_file(char *name, void *Global)
{
    FILE *fp;
    int ret;
    unsigned long MagicNum = MAGIC_NUM;

    #if DEBUG==1
    printf("create_sys_file %s\n", name);
    #endif

    if ((fp = fopen(name, "wb")) == NULL)
    {
        ERROR("Can't create system file\n");
        ret = FAIL;
    }
    else
    {
        if (fwrite(&MagicNum, 1, sizeof(MagicNum), fp) != sizeof(MagicNum))
        {
            ERROR("Writing Magic Number fail\n");
            ret = FAIL;
        }
        else
        {
            if (fwrite(Global, 1, SYS_ENV_SIZE, fp) != SYS_ENV_SIZE)
            {
                ERROR("Writing global fail\n");
                ret = FAIL;
            }
            else
            {
                ret = SUCCESS;
            }
        }
        fclose(fp);
    }
    return ret;
}

/**
 * @brief    read SysInfo from system file
 * @param    "void *Buffer" : [OUT]buffer to store SysInfo
 * @return    error code : SUCCESS(0) or FAIL(-1)
 */
int ReadGlobal(void *Buffer)
{
    FILE *fp;
    int length;

    #if DEBUG==1
    printf("ReadGlobal\n");
    #endif

    if ((fp = fopen(SYS_FILE, "rb")) == NULL)
    {

        #if DEBUG==1
        printf("open %s file fail\n", SYS_FILE);
        #endif

        //printf("ReadGlobal ret == FAIL)\n");
        /* System file not exist */
        return FAIL;
    }
    
    if(fseek(fp,0,SEEK_END)==-1)
    {
        #if DEBUG==1
        printf("check seek fail %sl\n",strerror(errno));
        #endif

        return FAIL;
    }

    if((length=ftell(fp))!=SYS_ENV_SIZE+sizeof(unsigned long))
    {
        #if DEBUG==1
        printf("file szie error lenth=%d,SYS_ENV_SIZE=%d\n",length,SYS_ENV_SIZE);
        #endif

        return FAIL;
    }
    
    if(fseek(fp,0,SEEK_SET)==-1)
    {
        #if DEBUG==1
        printf("check seek fail 0 %sl\n",strerror(errno));
        #endif

        return FAIL;
    }

    if (check_magic_num(fp) != SUCCESS)
    {
        return FAIL;
    }
    
    if (fread(Buffer, 1, SYS_ENV_SIZE, fp) != SYS_ENV_SIZE)
    {

        #if DEBUG==1
        printf("check SYS_ENV_SIZE faill\n");
        #endif

        return FAIL;
    }

    fclose(fp);

    #if DEBUG==1
    printf("check MagicNum okl\n");
    #endif

    return SUCCESS;
}

/**
 * @brief    write SysInfo to system file
 * @param    "void *Buffer" : [IN]buffer of SysInfo
 * @return    error code : SUCCESS(0) or FAIL(-1)
 */
int WriteGlobal(void *Buffer)
{
    int ret;
    ret = create_log_file(LOG_FILE, gLogHead);
    return ret | create_sys_file(SYS_FILE, Buffer);
}

/**
 * @brief    file manager initialization
 * @param    ShareMem [O ]Pointer to share memory where system information will
be stored.
 * @return    error code : SUCCESS(0) or FAIL(-1)
 */
int FileMngInit(void *ShareMem)
{
    //printf("FileMngInit Begin\n");

    int ret;
    DBG("Global value size:%d\n", SYS_ENV_SIZE);
    ret = ReadGlobal(ShareMem);
    //printf("FileMngInit ret == %d)\n",FAIL);
    if (ret == FAIL)
    {
        printf("FileMngInit ret == FAIL)\n");
        ret = create_sys_file(SYS_FILE, &SysInfoDefault);
        printf("create_sys_file Done!!!\n");
        if (ret == SUCCESS)
        {
            printf("create_sys_file ret == SUCCESS!!!\n");
            memcpy(ShareMem, &SysInfoDefault, SYS_ENV_SIZE);
        }
    }
    if (ret == SUCCESS)
    {
        ret = ReadLog();
        if (ret == FAIL)
        {
            ret = create_log_file(LOG_FILE, NULL);
        }
        #ifdef __FILE_DEBUG
        else
        {
            ShowAllLog();
        }
        #endif
    }
    return ret;
}

/**
 * @brief    file manager exit
 * @return    error code : SUCCESS(0) or FAIL(-1)
 */
int FileMngExit()
{
    CleanLog();
    return 0;
}

/**
 * @brief    file manager reset
 * @param    "void *ShareMem" : [IN]pointer to share memory
 * @return    error code : SUCCESS(0) or FAIL(-1)
 */
int FileMngReset(void *ShareMem)
{
    int ret;
    ret = create_sys_file(SYS_FILE, &SysInfoDefault);
    if (ret == SUCCESS)
    {
        memcpy(ShareMem, &SysInfoDefault, SYS_ENV_SIZE);
    }
    return ret;
}

