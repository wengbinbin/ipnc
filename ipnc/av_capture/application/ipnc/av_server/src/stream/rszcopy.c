/*
 *  ======== rszcopy.c ========
 *
 *  EDMA memory copy implementation of rzcopy.h interface.
 *
 *  NOTE: this uses an undocumented, unsupported API that is subject to change.
 *  A supported interface will be provided in a later release.
 *
 *  Note this implementation assumes 2 bytes / pixel (YUV422I).
 */

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include "rszcopy.h"

#define IOCMEMCPY    0x7
#define DEVNAME      "/dev/dm350mmap"

typedef struct Rszcopy_Object {
    int width;
    int height;
    int srcPitch;
    int dstPitch;
    int edmaFd;
} Rszcopy_Object;

typedef struct _edma_params {
    unsigned long src;
    unsigned long dst;
    unsigned int srcmode;
    unsigned int srcfifowidth;
    int srcbidx;
    int srccidx;
    unsigned int dstmode;
    unsigned int dstfifowidth;
    int dstbidx;
    int dstcidx;
    int acnt;
    int bcnt;
    int ccnt;
    int bcntrld;
    int syncmode;
} edma_params;

/* mutex to protect the DMA function (which is not re-entrant) */
static pthread_mutex_t dmaLock = PTHREAD_MUTEX_INITIALIZER;

/*
 *  ======== Rszcopy_config ========
 *  Configure the copy object for the desired dimensions
 */
int Rszcopy_config(Rszcopy_Handle rsz,
                          int width, int height, int srcPitch, int dstPitch)
{
    rsz->width = width;
    rsz->height = height;
    rsz->srcPitch = srcPitch;
    rsz->dstPitch = dstPitch;

    return (RSZCOPY_SUCCESS);
}

/*
 *  ======== Rszcopy_create ========
 *  Create an empty copy object
 */
Rszcopy_Handle Rszcopy_create(int rszRate)
{
    Rszcopy_Handle rsz;
    
    if ((rsz = calloc(1, sizeof(Rszcopy_Object))) != NULL) {
        if ((rsz->edmaFd = open(DEVNAME, O_RDWR | O_SYNC)) == -1) {
            fprintf(stderr, "Rszcopy: failed to open device\n");
            free(rsz);
            rsz = NULL;
        }
    }

    return (rsz);
}

/*
 *  ======== Rszcopy_delete ========
 *  Delete a copy object
 */
void Rszcopy_delete(Rszcopy_Handle rsz)
{
    if (rsz) {
        if (rsz->edmaFd > 0) {
            close(rsz->edmaFd);
        }
        
        free(rsz);
    }
}

/*
 *  ======== Rszcopy_execute ========
 *  Copy a block of memory using an EDMA copy. Since the copy function is not
 *  re-entrant, a mutex is used to insure only one thread uses it at a time.
 */
int Rszcopy_execute(Rszcopy_Handle rsz, unsigned long srcBuf,
                          unsigned long dstBuf)
{
    edma_params edmaparams;
    int width;
    int status;

    /*
    status = DM350MM_memcpy(dst, src, rsz->width * 2, rsz->height,
        rsz->dstPitch) == 0 ? RSZCOPY_SUCCESS : RSZCOPY_FAILURE;
    */

    pthread_mutex_lock(&dmaLock);

    width = rsz->srcPitch;

    edmaparams.srcmode = 0;     //INC mode
    edmaparams.srcfifowidth = 0;        //Don't care
    edmaparams.srcbidx = width;
    edmaparams.srccidx = width * rsz->height;

    edmaparams.dstmode = 0;     //INC mode
    edmaparams.dstfifowidth = 0;        //Don't care
    edmaparams.dstbidx = rsz->dstPitch;
    edmaparams.dstcidx = rsz->dstPitch * rsz->height;

    edmaparams.src = srcBuf; // CMEM_getPhys(srcBuf);
    edmaparams.dst = dstBuf; // CMEM_getPhys(dstBuf);

    edmaparams.acnt = width;
    edmaparams.bcnt = rsz->height;
    edmaparams.ccnt = 1;
    edmaparams.bcntrld = rsz->height;        //important setting
    edmaparams.syncmode = 1;    //AB-Sync

    if (ioctl(rsz->edmaFd, IOCMEMCPY, &edmaparams) == -1) {
        status = RSZCOPY_FAILURE;
        fprintf(stderr, "Rszcopy: Failed to do memcpy\n");
    }
    else {
        status = RSZCOPY_SUCCESS;
    }

    pthread_mutex_unlock(&dmaLock);

    return (status);
}
