/*
 *  Copyright 2007
 *  Texas Instruments Incorporated
 *
 *  All rights reserved.  Property of Texas Instruments Incorporated
 *  Restricted rights to use, duplicate or disclose this code are
 *  granted through contract.
 *
 */
/*
 *  ======== ae_ti.c ========
 *  TI's implementation of the AE algorithm.
 *
 *  This file contains an implementation of the IALG interface
 *  required by xDAIS.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iae.h"

#include "ae_ti.h"
#include "ae_ti_priv.h"

extern IALG_Fxns AE_TI_IALG;
//#define __DEBUG
#ifdef __DEBUG
#define dprintf printf
#else
#define dprintf
#endif
#define IALGFXNS  \
    &AE_TI_IALG,        /* module ID */                         \
    NULL,               /* activate */                          \
    AE_TI_alloc,        /* alloc */                             \
    NULL,               /* control (NULL => no control ops) */  \
    NULL,               /* deactivate */                        \
    AE_TI_free,         /* free */                              \
    AE_TI_init,         /* init */                              \
    NULL,               /* moved */                             \
    NULL                /* numAlloc (NULL => IALG_MAXMEMRECS) */

/*
 *  ======== AE_TI_IAE ========
 *  This structure defines TI's implementation of the IAE interface
 *  for the AE_TI module.
 */
IAE_Fxns AE_TI_AE = {    /* module_vendor_interface */
    {IALGFXNS},
    AE_TI_process,
    AE_TI_control,
};

/*
 *  ======== AE_TI_IALG ========
 *  This structure defines TI's implementation of the IALG interface
 *  for the AE_TI module.
 */
IALG_Fxns AE_TI_IALG = {      /* module_vendor_interface */
    IALGFXNS
};

/*
 *  ======== AE_TI_alloc ========
 */
Int AE_TI_alloc(const IALG_Params *algParams,
    IALG_Fxns **pf, IALG_MemRec memTab[])
{
    IAE_Params *params = (IAE_Params *)algParams;
    int numTabs = 1;

    /* Request memory for my object */
    memTab[0].size = sizeof(AE_TI_Obj);
    memTab[0].alignment = 0;
    memTab[0].space = IALG_EXTERNAL;
    memTab[0].attrs = IALG_PERSIST;

    memTab[1].size = sizeof(XDAS_UInt32) * (params->numHistory + 1);
    memTab[1].alignment = 0;
    memTab[1].space = IALG_EXTERNAL;
    memTab[1].attrs = IALG_PERSIST;
    numTabs++;
    return (numTabs);
}

/*
 *  ======== AE_TI_free ========
 */
Int AE_TI_free(IALG_Handle handle, IALG_MemRec memTab[])
{
    AE_TI_Obj *h = (AE_TI_Obj *)handle;
    int numTabs = 0;
    /* Request memory for my object */
    memTab[0].size = sizeof(AE_TI_Obj);
    memTab[0].alignment = 0;
    memTab[0].space = IALG_EXTERNAL;
    memTab[0].attrs = IALG_PERSIST;

    memTab[1].size = sizeof(XDAS_UInt32) * h->numHistory + 1;
    memTab[1].alignment = 0;
    memTab[1].space = IALG_EXTERNAL;
    memTab[1].attrs = IALG_PERSIST;
    numTabs++;
    return (numTabs);
}

/*
 *  ======== AE_TI_initObj ========
 */
Int AE_TI_init(IALG_Handle handle,
    const IALG_MemRec memTab[], IALG_Handle p,
    const IALG_Params *algParams)
{
    AE_TI_Obj *h = (AE_TI_Obj *)handle;
    IAE_Params *params = (IAE_Params *)algParams;
    int i;

    if(handle == NULL) return (IAE_EFAIL);
    if(params == NULL) {
        /* no static parameters passed in, use default */
	h->numHistory = 1;
	h->numSmoothSteps = 1;
	h->historyBrightness = NULL;
    } else if(params->size != sizeof(IAE_Params)){
        return (IAE_EUNSUPPORTED);
    }else{
	h->numHistory = params->numHistory + 1;
        h->numSmoothSteps = params->numSmoothSteps;
        h->historyBrightness = memTab[1].base;
        for( i = 0; i < h->numHistory; i ++){
		h->historyBrightness[i] = -1;
	}
        h->avgY = -1;
    }
    h->numRanges = 0;
    h->exposureTimeStepSize = 1;
    h->targetBrightness = 200;
    h->targetBrightnessRange.min = h->targetBrightness - 20;
    h->targetBrightnessRange.max = h->targetBrightness + 20;
    h->thrld = 40;
    h->locked = FALSE;

    return (IAE_EOK);
}

/*
 *  ======== AE_TI_process ========
 */
#define SAT_Y              180
XDAS_Int32 AE_TI_process(IAE_Handle handle, IAE_InArgs *inArgs, IAE_OutArgs *outArgs,
    IAEWB_Rgb *rgbData, XDAS_UInt8 *weight, void *customData)
{
    static int frames = 0;
    int width = inArgs->statMat.winCtHorz;
    int height = inArgs->statMat.winCtVert;
    int numPixels = inArgs->statMat.pixCtWin;
    int i;
    int j;
    unsigned int redSum = 0;
    unsigned int blueSum = 0;
    unsigned int greenSum = 0;
    unsigned int redSum_unsat = 0;
    unsigned int blueSum_unsat = 0;
    unsigned int greenSum_unsat = 0;
    unsigned int totalY, totalY_unsat;
    unsigned int avgY;
    unsigned int weightSum = 0;
    unsigned int weightSum_unsat = 0;
    unsigned int rY, bY, gY;
    unsigned int newExposureTime = inArgs->curAe.exposureTime;
    unsigned int newSensorGain = inArgs->curAe.sensorGain;
    unsigned int newApertureLevel = inArgs->curAe.apertureLevel;
    unsigned int newIpipeGain = inArgs->curAe.ipipeGain;
    unsigned int curY, curY_unsat, curY_sat;
    AE_TI_Obj *h = (AE_TI_Obj *)handle;
    long long  adjRatio;
    unsigned int y, max_y = 0, min_y = 255 * numPixels;
    unsigned int temp;


    /* now calculate the average weighted luminance value */
    rY = (RY * inArgs->curWb.rGain + 512) >> 10;
    gY = (GY * inArgs->curWb.gGain + 512) >> 10;
    bY = (BY * inArgs->curWb.bGain + 512) >> 10;


    dprintf("ae: awb gains = %d, %d, %d, numPixels = %d, %d, %d\n", inArgs->curWb.rGain, inArgs->curWb.gGain, inArgs->curWb.bGain,
           numPixels,width, height );
    dprintf("ae: ipipe gainss = %d, %d, %d\n", inArgs->curAe.ipipeGain,inArgs->curAe.exposureTime, inArgs->curAe.sensorGain);


    /* first calculate sumation of all R, G, B values */
    for(i = 0; i < height; i ++){
        for(j = 0; j < width; j ++){
            redSum += ((unsigned int)rgbData[i * width + j].r * weight[i * width + j]);
            greenSum += ((unsigned int)rgbData[i * width + j].g * weight[i * width + j]);
            blueSum += ((unsigned int)rgbData[i * width + j].b * weight[i * width + j]);
            weightSum += weight[i * width + j];

            y = (rgbData[i * width + j].r * rY
              + rgbData[i * width + j].g * gY
              + rgbData[i * width + j].b * bY + 128)>>8;
            if(max_y < y) max_y = y;
            if(min_y > y) min_y = y;
            if(y < SAT_Y * numPixels){
                redSum_unsat += ((unsigned int)rgbData[i * width + j].r * weight[i * width + j])>>8;
                greenSum_unsat += ((unsigned int)rgbData[i * width + j].g * weight[i * width + j])>>8;
                blueSum_unsat += ((unsigned int)rgbData[i * width + j].b * weight[i * width + j])>>8;
                weightSum_unsat += weight[i * width + j];
            }
        }
    }
    weightSum_unsat *= numPixels;
    totalY_unsat = redSum_unsat * rY + greenSum_unsat * gY + blueSum_unsat * bY;
    curY_unsat = (totalY_unsat + (weightSum_unsat >> 1)) / weightSum_unsat;

    weightSum *= numPixels;
    dprintf("min_y = %d\n", min_y);
    dprintf("R = %d, G = %d, B = %d\n", redSum/weightSum, greenSum/weightSum, blueSum/weightSum );
#if 1
    redSum >>=8;
    greenSum >>=8;
    blueSum >>=8;
    totalY = redSum * rY + greenSum * gY + blueSum * bY;
#else
    totalY = redSum;
    if(totalY < greenSum){
        totalY = greenSum;
    }
    if(totalY < blueSum){
        totalY = blueSum;
    }
#endif
    curY_sat = (totalY + (weightSum >> 1)) / weightSum;

    if(0){//(weightSum - weightSum_unsat)* 10  < weightSum){
        curY = curY_unsat;
    }else{
        curY = curY_sat;
    }
    if(curY == 0) curY = 1;

    dprintf("curY before dgain = %d\n", curY);
    /* now update the history brightnesss data */
    /* check if current brightness is within range of the history average*/
    if(abs(curY * h->numHistory - h->avgY) < h->thrld * h->numHistory * 4 && h->avgY != -1 ) {
        /* update avgY */
        h->avgY = h->avgY + curY - h->historyBrightness[h->numHistory - 1];

        /* update history */
        for(i = h->numHistory - 1; i > 0; i-- ){
            h->historyBrightness[i] = h->historyBrightness[i-1];
        }
        h->historyBrightness[0] = curY;
    }else {
        /* dramatic brightness change, this may signal scene change */
        /* abandon all past history brightness values */
        h->avgY = curY * h->numHistory;
        for(i = 0; i < h->numHistory; i ++){
            h->historyBrightness[i] = curY;
        }
        h->locked = FALSE;
    }
#if 0
    curY = (curY * inArgs->curAe.ipipeGain + 512) >> 10;
    avgY = (h->avgY * inArgs->curAe.ipipeGain + 512) >> 10;
#else
    avgY = h->avgY;
#endif
    if(curY == 0) curY = 1;
    if(avgY == 0) avgY = 1;
    dprintf("currentY = %d, avgY = %d\n", curY, (int)(avgY/h->numHistory));
    if(abs((int)avgY - (int)h->targetBrightness*h->numHistory) < h->thrld * h->numHistory && h->locked){
        outArgs->nextAe = inArgs->curAe;
	return(IAE_EOK);
    }
    if((curY > h->targetBrightnessRange.min && curY < h->targetBrightnessRange.max)
     ||(avgY > h->targetBrightnessRange.min * h->numHistory
        && avgY < h->targetBrightnessRange.max * h->numHistory)){
        outArgs->nextAe = inArgs->curAe;
        h->locked = TRUE;
        return(IAE_EOK);
    }
    dprintf("ae is not converged\n");
    dprintf("ae frames = %d, curY = %d, avgy = %d, target = %d, thrld = %d\n", frames++, curY, avgY, h->targetBrightness,
         h->thrld);
    dprintf("sat_sum = %d, %d, un_sat_sum = %d, %d\n", curY_sat, weightSum, curY_unsat, weightSum_unsat);
    h->locked = FALSE;
    /* At this point, the average Y value and target Y value are
     * used to calculate the adjustment ratio to cur AE settings
     * Q10 format is used here to allow enouth accuracy
     */
    adjRatio = h->targetBrightness * 1024 / curY;
    dprintf("numHistory = %d, adjRatio = %d\n",h->numHistory, adjRatio);

    adjRatio = (((adjRatio - 1024)*7)>>3) + 1024;

    /* adjust the ratio according to number of smooth steps */
    if(h->numSmoothSteps > 1){// && adjRatio < 1500) {
        adjRatio = (adjRatio - 1024)/(int)h->numSmoothSteps + 1024;
    }
    for(i = 0; i < h->numRanges; i ++){
        if(h->sensorGainRange[i].max != 0 && h->sensorGainRange[i].min == h->sensorGainRange[i].max){
            /* recalculate the adjRatio */
            adjRatio = (adjRatio * newSensorGain) / h->sensorGainRange[i].max;
            newSensorGain = h->sensorGainRange[i].max;

        }
        if(h->exposureTimeRange[i].max != 0 && h->exposureTimeRange[i].min == h->exposureTimeRange[i].max){
            /* recalculate the adjRatio */
            adjRatio = (adjRatio * newExposureTime) / h->exposureTimeRange[i].max;
            newExposureTime = h->exposureTimeRange[i].max;
        }
        if(h->apertureLevelRange[i].max != 0 && h->apertureLevelRange[i].min == h->apertureLevelRange[i].max){
            /* recalculate the adjRatio */
            adjRatio = (adjRatio * newApertureLevel) / h->apertureLevelRange[i].max;
            newApertureLevel = h->apertureLevelRange[i].max;
        }
       if(h->ipipeGainRange[i].max != 0 && h->ipipeGainRange[i].min == h->ipipeGainRange[i].max){
            /* recalculate the adjRatio */
            adjRatio = (adjRatio * newIpipeGain) / h->ipipeGainRange[i].max;
            newIpipeGain = h->ipipeGainRange[i].max;
        }
    }
    dprintf("ae: adjRatio here = %d\n", adjRatio);
    /* Use the range values to calculate the actual adjustment needed */
    for(i = 0; i < h->numRanges && adjRatio != 1024; i ++){
        /* first decide which parameter to change */
        if(h->exposureTimeRange[i].max > h->exposureTimeRange[i].min){
            temp = newExposureTime;
            newExposureTime =
              ((adjRatio * newExposureTime + 512) >> 10)
              / h->exposureTimeStepSize * h->exposureTimeStepSize;
            dprintf("we are here exposure time %d, %d, %d\n", newExposureTime,
                    h->exposureTimeRange[i].max, h->exposureTimeRange[i].min);
            if(h->exposureTimeRange[i].max  <  newExposureTime){
                newExposureTime
                  = h->exposureTimeRange[i].max / h->exposureTimeStepSize
                    * h->exposureTimeStepSize;
            }
            if(h->exposureTimeRange[i].min > newExposureTime){
                newExposureTime
                  = (h->exposureTimeRange[i].min + h->exposureTimeStepSize - 1)
                    / h->exposureTimeStepSize * h->exposureTimeStepSize;
            }
            /* update the adjRatio here */
            adjRatio = (adjRatio * temp + (newExposureTime >> 1)) /newExposureTime;
	    dprintf("adjRatio, exposure = %d, %d\n", adjRatio, newExposureTime);
        }else if(h->sensorGainRange[i].max > h->sensorGainRange[i].min){
            temp = newSensorGain;
            newSensorGain = (adjRatio * newSensorGain + 512) >> 10;
            if(newSensorGain > h->sensorGainRange[i].max){
                newSensorGain = h->sensorGainRange[i].max;
            }
            if( newSensorGain < h->sensorGainRange[i].min) {
                newSensorGain = h->sensorGainRange[i].min;
            }
            /* update the adjRatio here */
            adjRatio = (adjRatio * temp + (newSensorGain >> 1)) / newSensorGain;
	    dprintf("adjRatio, again = %d, %d\n", adjRatio, newSensorGain);
        }else if(h->apertureLevelRange[i].max > h->apertureLevelRange[i].min){
            temp = newApertureLevel;
            newApertureLevel = (adjRatio * newApertureLevel + 512) >> 10;
            if(newApertureLevel > h->apertureLevelRange[i].max){
                newApertureLevel = h->apertureLevelRange[i].max;
            }
            if( newApertureLevel < h->apertureLevelRange[i].min) {
                newApertureLevel = h->apertureLevelRange[i].min;
            }
            /* update the adjRatio here */
            adjRatio = (adjRatio * temp + (newApertureLevel >> 1)) / newApertureLevel;
	    dprintf("adjRatio, newApertureLevel = %d, %d\n", adjRatio, newApertureLevel);

        }else if(h->ipipeGainRange[i].max > h->ipipeGainRange[i].min){
	    temp = newIpipeGain;
            newIpipeGain = (adjRatio * newIpipeGain + 512) >> 10;
            if(newIpipeGain > h->ipipeGainRange[i].max){
                newIpipeGain = h->ipipeGainRange[i].max;
            }
            if( newIpipeGain < h->ipipeGainRange[i].min) {
                newIpipeGain = h->ipipeGainRange[i].min;
            }
            /* update the adjRatio here */
            adjRatio = (adjRatio * temp + (newIpipeGain >> 1)) / newIpipeGain;
	    dprintf("adjRatio, dGain = %d, %d\n", adjRatio, newIpipeGain);
        }
    }
    /* now output the AE settings for next frame(s) */
    outArgs->nextAe.exposureTime = newExposureTime;
    outArgs->nextAe.apertureLevel = newApertureLevel;
    outArgs->nextAe.sensorGain = newSensorGain;
    outArgs->nextAe.ipipeGain = newIpipeGain;
    dprintf("nextExposure: %d, curExposure:%d\n", outArgs->nextAe.exposureTime,
        inArgs->curAe.exposureTime);
    dprintf("nextGain: %d, curGain:%d\n", outArgs->nextAe.sensorGain,
        inArgs->curAe.sensorGain);
    dprintf("nextIGain: %d, curIGain:%d\n\n", outArgs->nextAe.ipipeGain,
        inArgs->curAe.ipipeGain);
    return (IAE_EOK);
}

/*
 *  ======== AE_TI_control ========
 */
XDAS_Int32 AE_TI_control(IAE_Handle handle, IAE_Cmd id,
    IAE_DynamicParams *params, IAE_Status *status)
{
    XDAS_Int32 retVal;
    AE_TI_Obj *h = (AE_TI_Obj *)handle;
    int i;
    /* validate arguments - this codec only supports "base" xDM. */
    if (params->size != sizeof(*params)){

        return (IAE_EUNSUPPORTED);
    }

    switch (id) {
         case IAE_CMD_SET_CONFIG:
            if(params->numRanges > IAE_MAX_RANGES) {
                retVal = IAE_EFAIL;
            }else {
                h->numRanges = params->numRanges;
                for(i = 0; i < h->numRanges; i ++){
                    h->exposureTimeRange[i] = params->exposureTimeRange[i];
                    h->apertureLevelRange[i] = params->apertureLevelRange[i];
                    h->sensorGainRange[i] = params->sensorGainRange[i];
                    h->ipipeGainRange[i] = params->ipipeGainRange[i];
                }
                h->targetBrightnessRange = params->targetBrightnessRange;
                h->thrld = params->thrld;
                h->targetBrightness = params->targetBrightness;
                h->exposureTimeStepSize = params->exposureTimeStepSize;
                dprintf("exposureTime Range = %d, %d, %d\n", h->exposureTimeRange[0].min, h->exposureTimeRange[0].max, h->exposureTimeStepSize);
                dprintf("setup dynamic parameters \n");
                retVal = IAE_EOK;
            }
            break;
         case IAE_CMD_GET_CONFIG:
            params->numRanges = h->numRanges;
            for(i = 0; i < h->numRanges; i ++){
                params->exposureTimeRange[i] = h->exposureTimeRange[i];
                params->apertureLevelRange[i] = h->apertureLevelRange[i];
                params->sensorGainRange[i] = h->sensorGainRange[i];
                params->ipipeGainRange[i] = h->ipipeGainRange[i];
            }
            params->targetBrightnessRange = h->targetBrightnessRange;
            params->thrld = h->thrld;
            params->targetBrightness = h->targetBrightness;
            params->exposureTimeStepSize = h->exposureTimeStepSize;
            retVal = IAE_EOK;
            break;

        default:
            /* unsupported cmd */
            retVal = IAE_EUNSUPPORTED;
            break;
    }
    return (retVal);
}

