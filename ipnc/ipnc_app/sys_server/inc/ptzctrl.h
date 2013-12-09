
#ifndef _PTZCTRL_H_
#define _PTZCTRL_H_

typedef struct {
	int		ptzDatabit;
	int		ptzParitybit;
	int		ptzStopbit;
	int		ptzBaudrate;
	int 	devaddr;
}	PTZ_serialInfo;


#define PTZ_STOP	(0)
#define PAN_RIGHT	(1)
#define	PAN_LEFT	(2)
#define TILT_UP		(3)
#define TILT_DOWN	(4)
#define ZOOM_IN		(5)
#define ZOOM_OUT	(6)
#define FOCUS_OUT	(7)
#define FOCUS_IN	(8)
#define PTZ_ROUND	(9)
#define RIGHT_UP	(10)
#define RIGHT_DOWN	(11)
#define	LEFT_UP		(12)
#define	LEFT_DOWN	(13)
#define LIGHTON		(14)
#define LIGHTOFF	(15)
#define RAINON		(16)
#define RAINOFF		(17)
#define APERTURE_OUT (18)
#define APERTURE_IN	 (19)
#define PRESET   (20)
#define TURNTOPRESET (21)
#define PRESET1 (41) //ADD BY WBB
#define PRESET2 (42)
#define PRESET3 (43)
#define PRESET4 (44)
#define PRESET5 (45)
#define TURNTOPRESET1 (51)
#define TURNTOPRESET2 (52)
#define TURNTOPRESET3 (53)
#define TURNTOPRESET4 (54)
#define TURNTOPRESET5 (55)

int PTZCTRL_setInternalCtrl( int Chl,int ctrlCmd);
int PTZCTRL_setInternalpresetCtrl(int Chl, int ctrlCmd,int npreset);

#endif 
