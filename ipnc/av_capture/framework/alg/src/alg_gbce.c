#include <alg_priv.h>
#include <csl_ipipe.h>
#include <drv_csl.h>
#include <osa_cmem.h>
#include <osa_file.h>

#include "alg_ti_gbce.h"

GBCE_CONTEXT g_gbceHandle;
#define GBCE_PERSISTENT_MEM_SIZE (128)
#define GBCE_SCRATCH_MEM_SIZE (1024*24)

#define GBCE_APPLY_PERIOD	(50)  // Once every 50 frames

int g_per_mem[GBCE_PERSISTENT_MEM_SIZE];
int g_src_mem[GBCE_SCRATCH_MEM_SIZE];

void TI_GBCE_setup(void)
{
    GBCE_CreationInputParamsStruct *gbceCIP =
        &(g_gbceHandle.creationInputParams);

    gbceCIP->PersistentMemory = (unsigned char*)g_per_mem;
    gbceCIP->ScratchMemory = (unsigned char*)g_src_mem;

    gbceCIP->PersistentMemorySize = GBCE_PERSISTENT_MEM_SIZE;
    gbceCIP->ScratchMemorySize = GBCE_SCRATCH_MEM_SIZE;

    // GBCE API
    gbceCIP->SetDefaultParams = 1;
    GBCE_algConfig(&g_gbceHandle);

    // GBCE API
    GBCE_init(&g_gbceHandle);

}

void TI_GBCE_run(int enable)
{
#if 0 //add by sxh csl
    static int gbce_frameCnt = 0;
    static int enabled = FALSE;
    CSL_IpipeGbceConfig gbceConfig;

    if (gbce_frameCnt == 2)
    {
        if (enable)
        {
            Uint32 Htable[256], expo;
            int status, again;
            GBCE_ImageInputParamsStruct *gbceIIP =
                &(g_gbceHandle.imageInputParams);
            GBCE_ImageOutputParamsStruct *gbceIOP =
                &(g_gbceHandle.imageOutputParams);

            //read histogram, exposure, A gain, aperture
            CSL_ipipeHistogramReadIdleY(&gCSL_ipipeHndl, Htable);
            gbceIIP->InputTable = (int*)&Htable[0];

            status = DRV_imgsGetEshutter(&expo);
            gbceIIP->ExposureTime = expo;

            status |= DRV_imgsGetAgain(&again); //true_gain * 8
            gbceIIP->AnalogGain = again * (256 / 8);
            gbceIIP->Aperture = 0;

            GBCE_run(&g_gbceHandle);

            if (gbceIOP->gbceStatus == 1)
            {
                gbceConfig.enable = TRUE;
                gbceConfig.type = CSL_IPIPE_GBCE_METHOD_Y_VALUE;
                gbceConfig.gainTable = gbceIOP->ToneTable;
                CSL_ipipeSetGbceConfig(&gCSL_ipipeHndl, &gbceConfig);
                enabled = TRUE;
                //OSA_printf("\n\nDEBUG: GBCE ON\n\n");
            }
        }
        else
        {
            if (enabled)
            {
                enabled = FALSE;
                gbceConfig.enable = FALSE;
                gbceConfig.type = CSL_IPIPE_GBCE_METHOD_Y_VALUE;
                gbceConfig.gainTable = NULL;
                CSL_ipipeSetGbceConfig(&gCSL_ipipeHndl, &gbceConfig);
            }
        }
    }

    gbce_frameCnt++;
    if(gbce_frameCnt>=GBCE_APPLY_PERIOD)
      gbce_frameCnt = 0;

#endif
}

