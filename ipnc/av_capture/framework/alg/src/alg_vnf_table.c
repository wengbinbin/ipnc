#include <alg_vnf.h>

ALG_vnfParams gAlg_vnfPrm = {
            /*default params  - light box, aperture wide open, 2x isif d-gain, 50% knob */
	26,	   //color 0 level 1 offset
	-18,  //color 0 level 1 slope
	74,	   //color 0 level 2 offset
	-52,  //color 0 level 2 slope
	37,		//color 0 level 3 offset
	-24,  //color 0 level 3 slope
	35,	   //color 1 level 1 offset
	37,//color 1 level 1 slope
	30,	   //color 1 level 2 offset
	24,	//color 1 level 2 slope
	19,		//color 1 level 3 offset
	2,  //color 1 level 3 slope
	25,	   //color 2 level 1 offset
	58,	//color 2 level 1 slope
	22,	   //color 2 level 2 offset
	30,	//color 2 level 2 slope
	15,		//color 2 level 3 offset
	12,	//color 2 level 3 slope
	2,
	1,  /* Setting of Edge Enhancement parameters */
	26,
	36,
	36,
	3,
	74,
	84,
	94,
	3,
	37,
	47,
	57,
	0,		 /* Control Param: 0 to 255 */ /* TNF Parameters */
	17,		/* Control Param: 1 to 255 */
	1		 /* 1: TNF applied on Luma Only, 0: TNF applied on both Luma and Chroma */

};


