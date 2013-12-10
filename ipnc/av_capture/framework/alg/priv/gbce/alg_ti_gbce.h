/*=======================================================================
 *
 *            Texas Instruments Internal Reference Software
 *
 *                           DSPS R&D Center
 *                     Video and Image Processing
 *         
 *         Copyright (c) 2006 Texas Instruments, Incorporated.
 *                        All Rights Reserved.
 *      
 *
 *          FOR TI INTERNAL USE ONLY. NOT TO BE REDISTRIBUTED.
 *
 *                    TI INTERNAL - TI PROPRIETARY
 *
 *
 *  Contact: Rajesh Narasimha     <rajeshn@ti.com>
 *
 *=======================================================================
 *
 *  Revision 1.0 (02-April-2009)
 *
 =======================================================================*/

//This is the interface to the GBCE library

//GBCE input parameters during creation
typedef struct
{

    //Pointer to external scratch memory
    unsigned char *ScratchMemory;
    int ScratchMemorySize;

    //Pointer to internal persistent memory - Store the points on the curve
    unsigned char *PersistentMemory;
    int PersistentMemorySize;

    unsigned char UseInputGammaTable;
    short int SetDefaultParams;

    // TI_GBCE_STAB parameters
    short int BufferLength;
    short int GBCECount;

    // Output GBCE Table parameters 
    short int BitsBeforeGamma;
    short int BitsAfterGamma;

    short int BitsPreviewInGamma;
    short int BitsPreviewOutGamma;

    unsigned char PreviewMode;
    unsigned char CaptureMode;


    // Gamma Parameters
    //OUTDOOR
    short int EnhanceDarkRegionEnableOutdoor;
    short int EnhanceBrightRegionEnableOutdoor;
    short int GammaSlopeOutdoor;        //Indicates which Gamma to use (2.2 - 1.6) 
    short int StandardGammaEnableOutdoor;

    //INDOOR
    short int EnhanceDarkRegionEnableIndoor;
    short int EnhanceBrightRegionEnableIndoor;
    short int StandardGammaEnableIndoor;
    short int GammaSlopeIndoor; //Indicates which Gamma to use (2.2 - 1.6) 

    //DARK
    short int EnhanceDarkRegionEnableDark;
    short int EnhanceBrightRegionEnableDark;
    short int StandardGammaEnableDark;
    short int GammaSlopeDark;   //Indicates which Gamma to use (2.2 - 1.6) 

    short int GainSaturationValueOutdoor;
    short int GainSaturationValueIndoor;
    short int GainSaturationValueDark;

    unsigned char OutdoorGainApplyVideoSurv;
    unsigned char IndoorGainApplyVideoSurv;
    unsigned char DarkGainApplyVideoSurv;

    // These variables will be taken out of the library as inputs   
    //GBCE Strength
    short int StregnthOutdoor;
    short int StregnthIndoor;
    short int StregnthDark;

    unsigned char OutdoorGainControl;
    unsigned char IndoorGainControl;
    unsigned char DarkGainControl;

    unsigned char histGainOutdoor;
    unsigned char histGainIndoor;
    unsigned char histGainDark;

    unsigned char OutdoorShadowScale;
    unsigned char OutdoorMidtoneScale;
    unsigned char OutdoorHighlightScale;

    unsigned char IndoorShadowScale;
    unsigned char IndoorMidtoneScale;
    unsigned char IndoorHighlightScale;

    unsigned char DarkShadowScale;
    unsigned char DarkMidtoneScale;
    unsigned char DarkHighlightScale;


/*

*/


} GBCE_CreationInputParamsStruct;


//GBCE output parameters during creation
typedef struct
{
    short int errorCode;
    int ScratchMemoryUsed;
    int PersistentMemoryUsed;

} GBCE_CreationOutputParamsStruct;


//GBCE input parameters during a run for one image
typedef struct
{
    int ExposureTime;
    int AnalogGain;
    int DigitalGain;
    int Aperture;
    int *InputTable;            //Input Histogram

    short int SkyDetect;
    short int FoliageDetect;
    short int BacklitDetect;

} GBCE_ImageInputParamsStruct;


//GBCE output parameters during a run for one image
typedef struct
{
    //All the outputs go here...3 tables and the status
    short int gbceStatus;
    int *GammaTable;            //Gamma Table- Pointed by gbce->GammaTable
    int *ToneTable;             //Tone Table
    int *InvTable;              // Inv Tone Table
    int *GBCETable;             //GBCE Table

    unsigned char GammaTableChangeFlag;

    //GBCE lower Cutoff
    short int GbceLowerCutoff;

} GBCE_ImageOutputParamsStruct;


//GBCE CONTEXT
typedef struct
{
    unsigned short gbceHandle[200];
    GBCE_CreationInputParamsStruct creationInputParams;
    GBCE_CreationOutputParamsStruct creationOutputParams;
    GBCE_ImageInputParamsStruct imageInputParams;
    GBCE_ImageOutputParamsStruct imageOutputParams;
} GBCE_CONTEXT;


void GBCE_algConfig(GBCE_CONTEXT * gbceCon);    //Call this function first and make sure it executes without error.
void GBCE_init(GBCE_CONTEXT * gbceCon); //Call this function second to initialize memory. 
void GBCE_run(GBCE_CONTEXT * gbceCon);  //Call this function for each image.
