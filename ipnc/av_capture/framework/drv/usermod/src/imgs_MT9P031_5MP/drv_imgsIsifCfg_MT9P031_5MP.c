
#include <drv_imgs.h>

DRV_ImgsIsifConfig gDRV_imgsIsifConfig_Common = {
/*
  .ccdcParams = {
  
    .hLpfEnable = FALSE,
    
    .inDataMsbPosition = CSL_CCDC_MSB_POSITION_BIT15;//CSL_CCDC_MSB_POSITION_BIT11,//add by sxh
    
    .lin = {
      .enable = FALSE,
    },
    
    .colPat = {
      .colPat0 = {
        { CSL_CCDC_COL_PAT_Gr, CSL_CCDC_COL_PAT_R  },
        { CSL_CCDC_COL_PAT_B , CSL_CCDC_COL_PAT_Gb },        


      },
      .colPat1 = {
        { CSL_CCDC_COL_PAT_Gr, CSL_CCDC_COL_PAT_R  },
        { CSL_CCDC_COL_PAT_B , CSL_CCDC_COL_PAT_Gb },        
      },
    },
    
    .gainOffset = {
      .gainR                  = 0x200,
      .gainGr                 = 0x200,
      .gainGb                 = 0x200,
      .gainB                  = 0x200,
      .offset                 = 0,
      .ipipeGainEnable        = TRUE,
      .ipipeOffsetEnable      = TRUE,
      .h3aGainEnable          = TRUE,
      .h3aOffsetEnable        = TRUE,
      .sdramOutGainEnable     = TRUE,
      .sdramOutOffsetEnable   = TRUE,
    },
    
    .dfc = {
      .enable = FALSE,
    },
    
    .clamp = {
      .enable = FALSE,
      .dcOffset = 0x0, 
    },
    
    .lsc = {
      .enable = FALSE,
      
    },
    
  },  
  
  .syncConfig = {
  
    .interlaceMode = TRUE,
    .wenUseEnable  = FALSE,

    .fidPolarity   = CSL_CCDC_SIGNAL_POLARITY_POSITIVE,
    .hdPolarity    = CSL_CCDC_SIGNAL_POLARITY_POSITIVE,
    .vdPolarity    = CSL_CCDC_SIGNAL_POLARITY_POSITIVE,        
    .hdVdDir       = CSL_CCDC_SIGNAL_DIR_INPUT,
    .fidDir        = CSL_CCDC_SIGNAL_DIR_INPUT,

    .hdWidth = 0,
    .vdWidth = 0,

    .pixelsPerLine = 0,
    .linesPerFrame = 0,
  },
  */
  ///////////////////////add by sxh/////////////////////////////////////
  .syncConfig = {
  
    .interlaceMode = TRUE,
    .wenUseEnable  = FALSE,

    .fidPolarity   = CSL_CCDC_SIGNAL_POLARITY_POSITIVE,
    .hdPolarity    = CSL_CCDC_SIGNAL_POLARITY_POSITIVE,
    .vdPolarity    = CSL_CCDC_SIGNAL_POLARITY_NEGATIVE,        
    .hdVdDir       = CSL_CCDC_SIGNAL_DIR_INPUT,
    .fidDir        = CSL_CCDC_SIGNAL_DIR_INPUT,

    .hdWidth = 0,
    .vdWidth = 0,

    .pixelsPerLine = 0,
    .linesPerFrame = 0,
  },
 .ccdcParams = {
  
    .hLpfEnable = FALSE,
    
    .inDataMsbPosition = CSL_CCDC_MSB_POSITION_BIT15,//CSL_CCDC_MSB_POSITION_BIT11,//add by sxh
    
    .lin = {
      .enable = FALSE,
    },
    
    .colPat = {
      .colPat0 = {
        { CSL_CCDC_COL_PAT_Ye, CSL_CCDC_COL_PAT_Cy},
        { CSL_CCDC_COL_PAT_G , CSL_CCDC_COL_PAT_Mg },         


      },
      .colPat1 = {
        { CSL_CCDC_COL_PAT_Ye, CSL_CCDC_COL_PAT_Cy},
        { CSL_CCDC_COL_PAT_G , CSL_CCDC_COL_PAT_Mg },       
      },
    },
    
    .gainOffset = {
      .gainR                  = 0x200,
      .gainGr                 = 0x200,
      .gainGb                 = 0x200,
      .gainB                  = 0x200,
      .offset                 = 0,
      .ipipeGainEnable        = FALSE,
      .ipipeOffsetEnable      = FALSE,
      .h3aGainEnable          = FALSE,
      .h3aOffsetEnable        = FALSE,
      .sdramOutGainEnable     = FALSE,
      .sdramOutOffsetEnable   = FALSE,
    },
    
    .dfc = {
      .enable = FALSE,
    },
    
    .clamp = {
      .enable = FALSE,
      .dcOffset = 0x0, 
    },
    
    .lsc = {
      .enable = FALSE,
      
    },
    
  },  

  .rec656Config={
  	.enable = TRUE,
	.errorCorrectEnable = TRUE,
	.dataWidth = CSL_CCDC_REC656_DATA_WIDTH_8BIT,
  },
/*
  .colPatConfig={

  	 .colPat0 = {
        { CSL_CCDC_COL_PAT_Ye, CSL_CCDC_COL_PAT_Cy},
        { CSL_CCDC_COL_PAT_G , CSL_CCDC_COL_PAT_Mg },        


      },
      .colPat1 = {
        { CSL_CCDC_COL_PAT_Ye, CSL_CCDC_COL_PAT_Cy},
        { CSL_CCDC_COL_PAT_G , CSL_CCDC_COL_PAT_Mg },     
      },
  },*/ //add by sxh 1109
  ////////////////////////add by sxh//////////////////////////////////////
};
