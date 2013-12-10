/*=======================================================================
 *
 *            Texas Instruments Internal Reference Software
 *
 *                           DSPS R&D Center
 *                     Video and Image Processing
 *
 *         Copyright (c) 2009 Texas Instruments, Incorporated.
 *                        All Rights Reserved.
 *
 *
 *          FOR TI INTERNAL USE ONLY. NOT TO BE REDISTRIBUTED.
 *
 *                    TI INTERNAL - TI PROPRIETARY
 *
 *
 *  Contact: Wei Hong (weihong@ti.com)
 *
 *=======================================================================
 *
 *  File: FacetrackerStruct.h
 *
 *=======================================================================
 *
 *
 =======================================================================*/


#ifndef _Facetracker_STRUCT_INCLUDED_

#define _Facetracker_STRUCT_INCLUDED_

#define MAX_FACE_NUMBER		(35)

//Facetracker structure: Contains all Facetracker parameters.
//Make sure that 16 and 32 bit variables are aligned
typedef struct {

	//Frame buffer pointer
	Uint8    *frmBuffer_1;
	Uint8 	 *frmBuffer_2;

	Uint16   inpImgSizeV;
	Uint16   inpImgSizeH;

// Max number of frames during which a face can be tracked
// without any positive detection by the face detector
// This number is usually depends the frame rate
// For 30 fps, setting it to 90 means the tracker will 
// be active for up to 3 seconds if no face is detected
    Uint16   maxNumOfTrackingFrames;
// Amount of expansion applied to the ROI box
// in the left, right, top, bottom sides .
// For expandRoiLeft and expandRoiRight, the unit is 1/8 of the face detector bounding box's width.
// For instance, if expandRoiLeft= 2, it means the amount of expansion to the left is 2/8 of the face detector box's width
// For expandRoiTop and expandRoiBottom, the unit is 1/8 of the face detector bounding box's height.
// For instance, if expandRoiTop= 3, it means the amount of expansion to the left is 3/8 of the face detector box's height
    Uint16   expandRoiLeft;
    Uint16   expandRoiRight;
    Uint16   expandRoiTop;
    Uint16   expandRoiBottom;

	Uint16   face_number;
	Uint16   face_pos[MAX_FACE_NUMBER][4];
    Uint16   roi_pos[MAX_FACE_NUMBER][4];

	Uint16   face_number_previous;
	Uint16   face_pos_previous[MAX_FACE_NUMBER][4];
    
    Uint16   face_pos_lastdetect[MAX_FACE_NUMBER][4];

	Uint16   tracking_counter[MAX_FACE_NUMBER];
	Uint16   tracking_counter_previous[MAX_FACE_NUMBER];

} Facetrackerstruct;

void TI_track_face_run(Facetrackerstruct* facetracker);

#endif
