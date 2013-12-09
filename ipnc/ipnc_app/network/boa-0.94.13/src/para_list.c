#include "web_translate.h"
#include <sys_env_type.h>
#include "file_msg_drv.h"
#include <net_config.h>
#include <sysctrl.h>
#include "file_list.h"
#include "para_list.h"
#include<stdlib.h>


char html_header1[] = "<HTML><META HTTP-EQUIV=\"CACHE-CONTROL\" CONTENT=\"NO-CACHE\"><TITLE>Current Parameter List</TITLE><style>BODY {	PADDING-RIGHT: 0px; MARGIN-TOP: 10px; PADDING-LEFT: 10px; MARGIN-LEFT: auto; WIDTH: auto; MARGIN-RIGHT: auto; background-color:#182439; FONT-FAMILY: Tahoma, Geneva, Arial, \"Arial Narrow\", Verdana, sans-serif; font-size:18px; color:#FFFFFF}A.linktxt:link {PADDING-RIGHT: 0px; PADDING-LEFT: 0px; FONT-WEIGHT: normal; FONT-SIZE: 18px; PADDING-BOTTOM: 0px; MARGIN: 0px; COLOR: #FFFFFF; PADDING-TOP: 0px; TEXT-DECORATION: underline}.linktxt:visited {PADDING-RIGHT: 0px; PADDING-LEFT: 0px; FONT-WEIGHT: normal; FONT-SIZE: 18px; PADDING-BOTTOM: 0px; MARGIN: 0px; COLOR: #FFFFFF; PADDING-TOP: 0px; TEXT-DECORATION: underline} A.linktxt:active { PADDING-RIGHT: 0px; PADDING-LEFT: 0px; FONT-WEIGHT: normal; FONT-SIZE: 18px; PADDING-BOTTOM: 0px; MARGIN: 0px; COLOR: #FFFFFF; PADDING-TOP: 0px; TEXT-DECORATION: underline} A.linktxt:hover { FONT-WEIGHT: normal; FONT-SIZE: 18px; COLOR: #FFFFFF; TEXT-DECORATION: underline}.heading{ FONT-FAMILY: Tahoma, Geneva, Arial, \"Arial Narrow\", Verdana, sans-serif; font-size:18pt; color:#FFFFFF } .normaltxt{ 	FONT-FAMILY: Courier New, Tahoma, Geneva, Arial, \"Arial Narrow\", Verdana, sans-serif; font-size:18px; color:#FFFFFF }.tableheadtxt{ 	FONT-FAMILY: Courier New, Tahoma, Geneva, Arial, \"Arial Narrow\", Verdana, sans-serif; font-size:18px; color:#000000} </style><BODY><PRE style='font-family:\"Courier New\"; font-size:18px'><PRE style='font-family:\"Courier New\"; font-size:18px'><span class=heading>Current Parameter List</span><br><br><div STYLE=\"position:relative;width:620;height:80%;overflow-y:scroll;z-index:10;\"><table border=0 cellspacing=1 cellpadding=1 class=normaltxt width=600><tr align=center bgcolor=#EEEEEE class=tableheadtxt><td>Parameter Name</td><td>Current Value</td><td>Default Value</td></tr><script>var z=\"sddel.htm\";function F(b){var o=\"\";var i;for(i=0;i<b;i++){o+=' ';}return o;}"
"function A(a,b,c){var o=\'';o+='<tr valign=middle><td><A HREF=\"?FILE='+a+'\" class=\"linktxt\">'+a+'</A></td><td>';o+=F(15-a.length)+b+' </td><td> '+c+' </td></tr><tr><td colspan=5><hr size=1 color=#3030A0></td></tr>';document.write(o);}";


char html_content1[100];
char html_end1[] = "<HR>HTTP Server</A><BR></PRE></BODY></HTML>";
char html_empty1[] = "NO FILE MATCH *.AVI *.JPG *.YUV";

#if 1
void GetStreamType(char *sType)
{
   SysInfo* pSysInfo = GetSysInfo();

   if(pSysInfo->lan_config.nVideocodecmode >= 0 && pSysInfo->lan_config.nVideocodecmode <=2)
   	{
            strcpy(sType, "Single");
	}
	else  if(pSysInfo->lan_config.nVideocodecmode >= 3 && pSysInfo->lan_config.nVideocodecmode <=7)
	{
            strcpy(sType, "Dual");
	}
	else
	{
            strcpy(sType, "Tri-Stream");
	}

}

void GetCodecCombo(char *cCombo)
{
       SysInfo* pSysInfo = GetSysInfo();

       switch(pSysInfo->lan_config.nVideocodecmode)
       {
           case 0:
		   	strcpy(cCombo, "H.264" );
			break;
	     case 1:
		   	strcpy(cCombo, "MPEG4" );
			break;
	     case 2:
		   	strcpy(cCombo, "MegaPixel JPEG");
			break;
	     case 3:
		   	strcpy(cCombo, "H.264 + JPEG");
			break;
	     case 4:
		   	strcpy(cCombo, "MPEG4 + JPEG");
			break;
	     case 5:
		   	strcpy(cCombo, "Dual H.264");
			break;
	     case 6:
		   	strcpy(cCombo, "Dual MPEG4");
			break;
	     case 7:
		   	strcpy(cCombo, "H264 + MPEG4");
			break;
	     case 8:
		   	strcpy(cCombo, "Dual H.264 + JPEG");
			break;
	     case 9:
		   	strcpy(cCombo, "Dual MPEG4 + JPEG");
			break;
	     default: ;

	 }

}

void GetResolution(char *resl)
{
       SysInfo* pSysInfo = GetSysInfo();

       switch(pSysInfo->lan_config.nVideocodecmode)
       {
           case 0:
		   	if(pSysInfo->lan_config.nVideocodecres == 0)
		   	     strcpy(resl, "H264:CIF" );
			else if(pSysInfo->lan_config.nVideocodecres == 1)
			     strcpy(resl, "H264:D1" );
			else if(pSysInfo->lan_config.nVideocodecres == 2)
			     strcpy(resl, "H264:SXGA" );
			else if(pSysInfo->lan_config.nVideocodecres == 3)
			     strcpy(resl, "H264:1080" );

			break;
	     case 1:
		   	if(pSysInfo->lan_config.nVideocodecres == 0)
		   	     strcpy(resl, "MPEG:720" );
			else if(pSysInfo->lan_config.nVideocodecres == 1)
		   	     strcpy(resl, "MPEG:D1" );
			else if(pSysInfo->lan_config.nVideocodecres == 2)
		   	     strcpy(resl, "MPEG:SXGA" );
			else if(pSysInfo->lan_config.nVideocodecres == 3)
		   	     strcpy(resl, "MPEG:1080" );

			break;
	     case 2:
		   	if(pSysInfo->lan_config.nVideocodecres == 0)
		   	     strcpy(resl, "JPG:1600x1200" );
			else if(pSysInfo->lan_config.nVideocodecres == 1)
			     strcpy(resl, "JPG:2048x1536" );

			break;
	     case 3:
		   	if(pSysInfo->lan_config.nVideocodecres == 0)
		   	     strcpy(resl, "H264:720,JPEG:VGA" );
		    else if(pSysInfo->lan_config.nVideocodecres == 1)
		   	     strcpy(resl, "H264:D1,JPEG:D1" );
			else if(pSysInfo->lan_config.nVideocodecres == 2)
			     strcpy(resl, "H264:720,JPEG:720" );

			break;
	     case 4:
		   	if(pSysInfo->lan_config.nVideocodecres == 0)
		   	     strcpy(resl, "MPEG4:720,JPEG:352" );
		    else if(pSysInfo->lan_config.nVideocodecres == 1)
		   	     strcpy(resl, "MPEG4:D1,JPEG:D1" );
		    else if(pSysInfo->lan_config.nVideocodecres == 2)
		   	     strcpy(resl, "MPEG4:720,JPEG:720" );

			break;
	     case 5:
		   	if(pSysInfo->lan_config.nVideocodecres == 0)
		   	     strcpy(resl, "H264:720,H264:QVGA" );
		    else if(pSysInfo->lan_config.nVideocodecres == 1)
		   	     strcpy(resl, "H264:D1,H264:D1" );
		    else if(pSysInfo->lan_config.nVideocodecres == 2)
		   	     strcpy(resl, "H264:1080,H264:QVGA" );

			break;
	     case 6:
		   	if(pSysInfo->lan_config.nVideocodecres == 0)
		   	     strcpy(resl, "MPEG4:720,MPEG4:QVGA" );
		 	else if(pSysInfo->lan_config.nVideocodecres == 1)
		   	     strcpy(resl, "MPEG4:D1,MPEG4:D1" );
			else if(pSysInfo->lan_config.nVideocodecres == 2)
		   	     strcpy(resl, "MPEG4:1080,MPEG4:QVGA" );

			break;
	     case 7:
		   	strcpy(resl, "H264:D1,MPEG4:D1" );
			break;
	     case 8:
		   	strcpy(resl, "H264:720,JPEG:VGA,H264:QVGA");
			break;
	     case 9:
		   	strcpy(resl, "MPEG4:720,JPEG:VGA,MPEG4:QVGA");
			break;
	     default: ;

	 }

}

void GetFrameRate(char *frameRate)
{
      SysInfo* pSysInfo = GetSysInfo();

       switch(pSysInfo->lan_config.nVideocodecmode)
       {
          case 0:
		   	 fprintf(stderr, "framerate    %d",pSysInfo->lan_config.nFrameRate1 );
		   	 sprintf(frameRate, "%d", (int)pSysInfo->lan_config.nFrameRate1);
			  break;
	     case 1:
		   	  strcpy(frameRate, "1");
			  break;
	     case 2:
		   	  strcpy(frameRate, "2");
			  break;
	     case 3:
		   	strcpy(frameRate, pSysInfo->lan_config.nFrameRate1);
			strcat(frameRate, "  ");
			strcat(frameRate, pSysInfo->lan_config.nFrameRate2);
			break;
	     case 4:
		   	strcpy(frameRate, pSysInfo->lan_config.nFrameRate1);
			strcat(frameRate, "  ");
			strcat(frameRate, pSysInfo->lan_config.nFrameRate2);
			break;
	     case 5:
		   	strcpy(frameRate, pSysInfo->lan_config.nFrameRate1);
			strcat(frameRate, "  ");
			strcat(frameRate, pSysInfo->lan_config.nFrameRate2);
			break;
	     case 6:
		   	strcpy(frameRate, pSysInfo->lan_config.nFrameRate1);
			strcat(frameRate, "  ");
			strcat(frameRate, pSysInfo->lan_config.nFrameRate2);
			break;
	     case 7:
		   	strcpy(frameRate, pSysInfo->lan_config.nFrameRate1);
			strcat(frameRate, "  ");
			strcat(frameRate, pSysInfo->lan_config.nFrameRate2);
			break;
	     case 8:
		   	strcpy(frameRate, pSysInfo->lan_config.nFrameRate1);
			strcat(frameRate, "  ");
			strcat(frameRate, pSysInfo->lan_config.nFrameRate2);
			strcat(frameRate, "  ");
			strcat(frameRate, pSysInfo->lan_config.nFrameRate3);
			break;
	     case 9:
		   	strcpy(frameRate, pSysInfo->lan_config.nFrameRate1);
			strcat(frameRate, "  ");
			strcat(frameRate, pSysInfo->lan_config.nFrameRate2);
			strcat(frameRate, "  ");
			strcat(frameRate, pSysInfo->lan_config.nFrameRate3);
			break;

	     default: ;

       }

}

void GetFaceDetect(char *faceDetect)
{
     SysInfo* pSysInfo = GetSysInfo();

     if(pSysInfo->face_config.fdetect == 1)
     {
          strcpy(faceDetect, "Enable");
     }
     else
     {
          strcpy(faceDetect, "Disable");
     }

}

void GetPrivacyMask(char *privacyMask)
{
     SysInfo* pSysInfo = GetSysInfo();

     if(pSysInfo->face_config.pmask == 1)
     {
           strcpy(privacyMask, "Enable");
     }
     else
     {
          strcpy(privacyMask, "Disable");
     }

}

void GetFaceRecognition(char *fRecognition)
{
     SysInfo* pSysInfo = GetSysInfo();

     if(pSysInfo->face_config.frecog == 0)
     {
          strcpy(fRecognition, "OFF");
     }
     else if(pSysInfo->face_config.frecog == 1)
     {
          strcpy(fRecognition, "RECOGNIZE USER");
     }
	 else if(pSysInfo->face_config.frecog == 2)
     {
          strcpy(fRecognition, "REGISTER USER");
     }
	 else if(pSysInfo->face_config.frecog == 3)
     {
          strcpy(fRecognition, "CLEAR ALL USERS");
     }

}

void GetBLC(char *blc)
{
     SysInfo* pSysInfo = GetSysInfo();

     if(pSysInfo->lan_config.nBacklightControl == 1)
     {
           strcpy(blc, "Enable");
     }
     else
     {
          strcpy(blc, "Disable");
     }

}

void GetBackLight(char *backLight)
{
     SysInfo* pSysInfo = GetSysInfo();

     if(pSysInfo->lan_config.nBackLight == 0)
     {
           strcpy(backLight, "Min");
     }
     else if(pSysInfo->lan_config.nBackLight== 1)
     {
          strcpy(backLight, "Mid");
     }
     else
     {
          strcpy(backLight, "Max");
     }

}

void GetGBCE(char *gbce)
{
     SysInfo* pSysInfo = GetSysInfo();

     if(pSysInfo->lan_config.gbce == 1)
     {
           strcpy(gbce, "Enable");
     }
     else
     {
          strcpy(gbce, "Disable");
     }

}

void GetWhiteBalance(char *wbalance)
{
     SysInfo* pSysInfo = GetSysInfo();

     if(pSysInfo->lan_config.nWhiteBalance == 0)
     {
          strcpy(wbalance, "Auto");
     }
     else if(pSysInfo->lan_config.nWhiteBalance== 1)
     {
          strcpy(wbalance, "Indoor");
     }
     else
     {
          strcpy(wbalance, "Outdoor");
     }

}

void GetDayNightMode(char *mode)
{
     SysInfo* pSysInfo = GetSysInfo();

     if(pSysInfo->lan_config.nDayNight == 1)
     {
           strcpy(mode, "Day");
     }
     else
     {
          strcpy(mode, "Night");
     }

}

void GetHistogram(char *histogram)
{
     SysInfo* pSysInfo = GetSysInfo();

     if(pSysInfo->lan_config.histogram == 1)
     {
          strcpy(histogram, "Enable");
     }
     else
     {
          strcpy(histogram, "Disable");
     }

}

void GetVStab(char *vStab)
{
     SysInfo* pSysInfo = GetSysInfo();

     if(pSysInfo->lan_config.AdvanceMode&FFLAG_VS)
     {
          strcpy(vStab, "Enable");
     }
     else
     {
          strcpy(vStab, "Disable");
     }

}

void GetLDC(char *ldc)
{
     SysInfo* pSysInfo = GetSysInfo();

     if(pSysInfo->lan_config.AdvanceMode&FFLAG_LDC)
     {
          strcpy(ldc, "Enable");
     }
     else
     {
          strcpy(ldc, "Disable");
     }

}

void GetImgSensorMode(char *imgsenormode)
{
     SysInfo* pSysInfo = GetSysInfo();

     if(pSysInfo->lan_config.nBinning == 0)
     {
          strcpy(imgsenormode, "binning");
     }
     else if(pSysInfo->lan_config.nBinning == 1)
     {
          strcpy(imgsenormode, "skipping");
     }
     else
     {
          strcpy(imgsenormode, "window");
     }

}

void Get2AESwitch(char *AEWSwitch)
{
     SysInfo* pSysInfo = GetSysInfo();

     if(pSysInfo->lan_config.nAEWswitch== 0)
     {
           strcpy(AEWSwitch, "NONE");
     }
     else if(pSysInfo->lan_config.nAEWswitch== 1)
     {
          strcpy(AEWSwitch, "APPRO");
     }
     else
     {
           strcpy(AEWSwitch, "TI");
     }

}

void Get2AEType(char *AEWType)
{
     SysInfo* pSysInfo = GetSysInfo();

     if(pSysInfo->lan_config.nAEWtype== 0)
     {
           strcpy(AEWType, "OFF");
     }
     else if(pSysInfo->lan_config.nAEWtype== 1)
     {
          strcpy(AEWType, "Auto Exposure");
     }
     else if(pSysInfo->lan_config.nAEWtype== 2)
     {
           strcpy(AEWType, "Auto White Balance");
     }
      else
     {
           strcpy(AEWType, "Auto Exposure + Auto White Balance");
     }
}

void GetNfs(char *nfs)
{
     SysInfo* pSysInfo = GetSysInfo();

     if(pSysInfo->lan_config.AdvanceMode&FFLAG_SNF)
     {
           strcpy(nfs, "ON");
     }
     else
     {
           strcpy(nfs, "OFF");
     }

}

void GetTnf(char *tnflt)
{
     SysInfo* pSysInfo = GetSysInfo();

     if(pSysInfo->lan_config.AdvanceMode&FFLAG_TNF)
     {
          strcpy(tnflt, "Enable");
     }
     else
     {
          strcpy(tnflt, "Disable");
     }
}

void GetAudio(char *audio)
{
     SysInfo* pSysInfo = GetSysInfo();

     if(pSysInfo->audio_config.audioenable== 1)
     {
          strcpy(audio, "Enable");
     }
     else
     {
          strcpy(audio, "Disable");
     }

}

void GetAlarm(char *alarm)
{
     SysInfo* pSysInfo = GetSysInfo();

     if(pSysInfo->lan_config.nAlarmEnable== 1)
     {
           strcpy(alarm, "Enable");
     }
     else
     {
          strcpy(alarm, "Disable");
     }

}

void GetMotionDetectionAlarm(char *motionAlarm)
{
     SysInfo* pSysInfo = GetSysInfo();

     if(pSysInfo->motion_config.motionenable== 1)
     {
          strcpy(motionAlarm, "Enable");
     }
     else
     {
          strcpy(motionAlarm, "Disable");
     }

}

void GetEthernetLostAlarm(char *ethLostAlarm)
{
     SysInfo* pSysInfo = GetSysInfo();

     if(pSysInfo->lan_config.lostalarm== 1)
     {
          strcpy(ethLostAlarm, "Enable");
     }
     else
     {
          strcpy(ethLostAlarm, "Disable");
     }

}

void GetExternTriggers(char *extTriggersAlarm)
{
     SysInfo* pSysInfo = GetSysInfo();

     if(pSysInfo->lan_config.nExtAlarm== 1)
     {
          strcpy(extTriggersAlarm, "Enable");
     }
     else
     {
          strcpy(extTriggersAlarm, "Disable");
     }

}

void PutFrameRate(void *pOutMem, int MaxSize)
{
     char String[32];
     SysInfo* pSysInfo = GetSysInfo();

     if(pSysInfo->lan_config.nDayNight == 0)
	{
             sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Frame Rate", "Auto", "30 fps");
			   if( MaxSize > strlen(pOutMem) )
		         strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));
	}
      else
	{
	     if(pSysInfo->lan_config.nVideocodecmode == 0 || pSysInfo->lan_config.nVideocodecmode == 1)
      	  {
            if(pSysInfo->lan_config.nVideocodecres == 3)
            {
			   switch(pSysInfo->lan_config.nFrameRate1)
			   {
                   case 0:
					    strcpy(String,"15 fps");
						break;
				   case 1:
					    strcpy(String,"12 fps");
						break;
				   case 2:
					    strcpy(String,"7.5 fps");
						break;
			   }
			}
			else
			{
				switch(pSysInfo->lan_config.nFrameRate1)
			   {
                   case 0:
					    strcpy(String,"30 fps");
						break;
				   case 1:
					    strcpy(String,"24 fps");
						break;
				   case 2:
					    strcpy(String,"15 fps");
						break;
				   case 3:
					    strcpy(String,"7.5 fps");
						break;
			   }
			}
			   sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Frame Rate", String, "30 fps");
			   if( MaxSize > strlen(pOutMem) )
		         strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));
	      }
	      else if(pSysInfo->lan_config.nVideocodecmode == 2)
	      	{
                if(pSysInfo->lan_config.nVideocodecres == 0)
                {
                   switch(pSysInfo->lan_config.nFrameRate1)
                   {
				     case 0:
					        strcpy(String,"24 fps");
					        break;
				     case 1:
					        strcpy(String,"15 fps");
					        break;
				     case 2:
					        strcpy(String,"7.5 fps");
						  break;
                   }
			    }
			    else
			    {
                   switch(pSysInfo->lan_config.nFrameRate1)
                   {
				     case 0:
					        strcpy(String,"15 fps");
					        break;
				     case 1:
					        strcpy(String,"12 fps");
					        break;
				     case 2:
					        strcpy(String,"7.5 fps");
						  break;
                   }
			  }

		    sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Frame Rate", String, "30 fps");
		    if( MaxSize > strlen(pOutMem) )
		    strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

		}
		else if(pSysInfo->lan_config.nVideocodecmode == 3 || pSysInfo->lan_config.nVideocodecmode == 4)
		{
               switch(pSysInfo->lan_config.nFrameRate1)
			   {
                   case 0:
					    strcpy(String,"30 fps");
						break;
				   case 1:
					    strcpy(String,"24 fps");
						break;
				   case 2:
					    strcpy(String,"15 fps");
						break;
				   case 3:
					    strcpy(String,"7.5 fps");
						break;

			   }
			   sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Frame Rate(Stream1)", String, "30 fps");
			   if( MaxSize > strlen(pOutMem) )
		         strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

			   switch(pSysInfo->lan_config.nFrameRate2)
			   {
                   case 0:
					    strcpy(String,"30 fps");
						break;
				   case 1:
					    strcpy(String,"24 fps");
						break;
				   case 2:
					    strcpy(String,"15 fps");
						break;
				   case 3:
				   	    strcpy(String,"7.5 fps");
						break;
			   }
			   sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Frame Rate(Stream2)", String, "N/A");
			   if( MaxSize > strlen(pOutMem) )
		         strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

		}
		else if(pSysInfo->lan_config.nVideocodecmode == 5 || pSysInfo->lan_config.nVideocodecmode == 6)
		{
             if(pSysInfo->lan_config.nVideocodecres == 2)
             {
               switch(pSysInfo->lan_config.nFrameRate1)
			   {
                   case 0:
					    strcpy(String,"15 fps");
						break;
				   case 1:
					    strcpy(String,"12 fps");
						break;
				   case 2:
					    strcpy(String,"7.5 fps");
						break;
			   }
			 }
			 else
			 {
				 switch(pSysInfo->lan_config.nFrameRate1)
			   {
                   case 0:
					    strcpy(String,"30 fps");
						break;
				   case 1:
					    strcpy(String,"24 fps");
						break;
				   case 2:
					    strcpy(String,"15 fps");
						break;
				   case 3:
					    strcpy(String,"7.5 fps");
						break;
			   }
			 }
			 sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Frame Rate(Stream1)", String, "30 fps");
			 if( MaxSize > strlen(pOutMem) )
		         strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

			 if(pSysInfo->lan_config.nVideocodecres == 2)
             {
			   switch(pSysInfo->lan_config.nFrameRate2)
			   {
                   case 0:
					    strcpy(String,"15 fps");
						break;
				   case 1:
					    strcpy(String,"12 fps");
						break;
				   case 2:
					    strcpy(String,"7.5 fps");
						break;
			   }
			 }
			 else
			 {
			   switch(pSysInfo->lan_config.nFrameRate2)
			   {
				 case 0:
					    strcpy(String,"30 fps");
						break;
				   case 1:
					    strcpy(String,"24 fps");
						break;
				   case 2:
					    strcpy(String,"15 fps");
						break;
				   case 3:
					    strcpy(String,"7.5 fps");
						break;
			   }
			 }
			 sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Frame Rate(Stream2)", String, "N/A");
			 if( MaxSize > strlen(pOutMem) )
		         strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

		}
		else if( pSysInfo->lan_config.nVideocodecmode == 7 )
		{
               switch(pSysInfo->lan_config.nFrameRate1)
			   {
                   case 0:
					    strcpy(String,"30 fps");
						break;
				   case 1:
					    strcpy(String,"24 fps");
						break;
				   case 2:
					    strcpy(String,"15 fps");
						break;
				   case 3:
					    strcpy(String,"7.5 fps");
						break;
			   }
			   sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Frame Rate(Stream1)", String, "30 fps");
			   if( MaxSize > strlen(pOutMem) )
		         strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

                 switch(pSysInfo->lan_config.nFrameRate2)
			    {
                   case 0:
					    strcpy(String,"30 fps");
						break;
				   case 1:
					    strcpy(String,"24 fps");
						break;
				   case 2:
					    strcpy(String,"15 fps");
						break;
				   case 3:
					    strcpy(String,"7.5 fps");
						break;
			     }
			   sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Frame Rate(Stream2)", String, "N/A");
			   if( MaxSize > strlen(pOutMem) )
		         strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));


		}
		else if(pSysInfo->lan_config.nVideocodecmode == 8 || pSysInfo->lan_config.nVideocodecmode == 9)
		{
               switch(pSysInfo->lan_config.nFrameRate1)
			   {
                   case 0:
					    strcpy(String,"30 fps");
						break;
				   case 1:
					    strcpy(String,"24 fps");
						break;
				   case 2:
					    strcpy(String,"15 fps");
						break;
				   case 3:
					    strcpy(String,"7.5 fps");
						break;
			   }
			   sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Frame Rate(Stream1)", String, "30 fps");
			   if( MaxSize > strlen(pOutMem) )
		         strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

               switch(pSysInfo->lan_config.nFrameRate2)
			   {
				  case 0:
					    strcpy(String,"30 fps");
						break;
				   case 1:
					    strcpy(String,"24 fps");
						break;
				   case 2:
					    strcpy(String,"15 fps");
						break;
				   case 3:
					    strcpy(String,"7.5 fps");
						break;
			   }

			   sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Frame Rate(Stream2)", String, "N/A");
			   if( MaxSize > strlen(pOutMem) )
		         strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

			   switch(pSysInfo->lan_config.nFrameRate3)
			   {
                  case 0:
					    strcpy(String,"30 fps");
						break;
				   case 1:
					    strcpy(String,"24 fps");
						break;
				   case 2:
					    strcpy(String,"15 fps");
						break;
				   case 3:
					    strcpy(String,"7.5 fps");
						break;
			   }

			   sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Frame Rate(Stream3)", String, "N/A");
			   if( MaxSize > strlen(pOutMem) )
		         strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

		}
	}

}

void PutBitRate(void *pOutMem, int MaxSize)
{
     char String[32];
     SysInfo* pSysInfo = GetSysInfo();

     if(pSysInfo->lan_config.nVideocodecmode == 0 ||  pSysInfo->lan_config.nVideocodecmode == 1)
     {
          sprintf(String, "%d", pSysInfo->lan_config.nMpeg41bitrate/1000);

          sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Bit Rate", String, "4000 Kbps");
			   if( MaxSize > strlen(pOutMem) )
		         strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));
     }
     else  if(pSysInfo->lan_config.nVideocodecmode == 2)
     {
          sprintf(String, "%d", pSysInfo->lan_config.njpegquality);
          sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Quality Factor", String, "N/A");
			   if( MaxSize > strlen(pOutMem) )
		         strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));
     }
     else if(pSysInfo->lan_config.nVideocodecmode == 3 || pSysInfo->lan_config.nVideocodecmode == 4)
     {
          sprintf(String, "%d", pSysInfo->lan_config.nMpeg41bitrate/1000);
          sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Bit Rate(Stream1)", String, "4000 Kbs");
			   if( MaxSize > strlen(pOutMem) )
		         strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	   sprintf(String, "%d", pSysInfo->lan_config.njpegquality);
          sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Quality Factor(Stream2)", String, "N/A");
			   if( MaxSize > strlen(pOutMem) )
		         strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));
     }
     else if(pSysInfo->lan_config.nVideocodecmode == 5 || pSysInfo->lan_config.nVideocodecmode == 6 || pSysInfo->lan_config.nVideocodecmode == 7 )
     {
          sprintf(String, "%d", pSysInfo->lan_config.nMpeg41bitrate/1000);
          sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Bit Rate(Stream1)", String, "4000 Kbps");
			   if( MaxSize > strlen(pOutMem) )
		         strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	   sprintf(String, "%d", pSysInfo->lan_config.nMpeg42bitrate/1000);
          sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Bit Rate(Stream2)", String, "N/A");
			   if( MaxSize > strlen(pOutMem) )
		         strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));
     }
     else if(pSysInfo->lan_config.nVideocodecmode == 8 || pSysInfo->lan_config.nVideocodecmode == 9 )
     {
          sprintf(String, "%d", pSysInfo->lan_config.nMpeg41bitrate/1000);
          sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Bit Rate(Stream1)", String, "4000 Kbps");
			   if( MaxSize > strlen(pOutMem) )
		         strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));
	   sprintf(String, "%d", pSysInfo->lan_config.njpegquality);
          sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Quality Factor(Stream2)", String, "N/A");
			   if( MaxSize > strlen(pOutMem) )
		         strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	   sprintf(String, "%d", pSysInfo->lan_config.nMpeg42bitrate/1000);
          sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Bit Rate(Stream3)", String, "N/A");
			   if( MaxSize > strlen(pOutMem) )
		         strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));
     }



}
#endif

int Curr_Param_List_To_html(void *pOutMem, int MaxSize)
{
     char http_ip_addr[100];
	 char tempbuff[100];
	 char String[50];
#if 1
	GetIP_Addr(http_ip_addr);
	fprintf(stderr,"para_netip %s \n",http_ip_addr);

	strncpy( pOutMem, html_header1, MaxSize );

	GetStreamType(String);
      sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Stream Type", String, "Single");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	GetCodecCombo(String);
	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Codec Combo", String, "H.264");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	GetResolution(String);
	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Resolution", String, "H264:720");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	PutFrameRate(pOutMem, MaxSize);

	PutBitRate(pOutMem, MaxSize);

/*	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Bit Rate", "BitRate", "30");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));
*/
	GetFaceDetect(String);
	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Face Detect", String, "Disable");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	GetPrivacyMask(String);
	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Privacy Mask", String, "Disable");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	GetFaceRecognition(String);
	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Face Recognition", String, "OFF");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	GetBLC(String);
	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Back Light Compensation", String, "Enable");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	GetBackLight(String);
	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Back Light", String, "Mid");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	GetGBCE(String);
	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","GBCE", String, "Disable");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	GetWhiteBalance(String);
	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","White Balance", String, "Auto");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	GetDayNightMode(String);
	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Day Night Mode", String, "Day");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	GetHistogram(String);
	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Histogram", String, "Disable");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	GetVStab(String);
	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Video Stablization", String, "Disable");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	GetLDC(String);
	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Lens Distortion Correction", String, "Disable");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	GetImgSensorMode(String);
	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Image Sensor Mode", String, "binning");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

    Get2AESwitch(String);
	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","2A Engine", String, "APPRO");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	Get2AEType(String);
	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","2A Mode", String, "Auto Exposure + Auto White Balance");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

    GetNfs(String);
	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Spatial Noise Filter", String, "OFF");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	GetTnf(String);
	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Temporal Noise Filter", String, "Disable");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	GetAudio(String);
	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Audio", String, "Disable");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	GetAlarm(String);
	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Alarm", String, "Disable");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	GetMotionDetectionAlarm(String);
	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Motion Detection Alarm", String, "Disable");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	GetEthernetLostAlarm(String);
	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Ethernet Lost Alarm", String, "Disable");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	GetExternTriggers(String);
	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","External Triggers Alarm", String, "Disable");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));



	if( MaxSize > strlen(pOutMem) )
	       strncat(pOutMem, "</SCRIPT>\n</table>\n</DIV>\n<HR size=1 color=#C0C0C0 width=620 align=left>\n", MaxSize-strlen(pOutMem));
	else
		fprintf(stderr,"MaxSize is not enough!\n");

//       sprintf(tempbuff,"<HR size=1 color=#C0C0C0 width=620 align=left>HTTP Server at <A HREF=\"http://%s\">ipnc</A><BR></PRE></BODY></HTML>", http_ip_addr);

//	if( MaxSize > strlen(pOutMem) )
//		strncat(pOutMem, tempbuff, MaxSize-strlen(pOutMem));
//	else
//		fprintf(stderr,"MaxSize is not enough!\n");
#endif
}
int Default_Param_List_To_html(void *pOutMem, int MaxSize)
{
     char http_ip_addr[100];
	 char	tempbuff[100];
	 char streamType[32], codecCombo[32], resolution[32], dResolution[32];
	 int i;
#if 0
	GetIP_Addr(http_ip_addr);
	fprintf(stderr,"para_netip %s \n",http_ip_addr);

	strncpy( pOutMem, html_header1, MaxSize );

	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Stream Type", "Single", "Single");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Codec Combo", "H.264", "H.264");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));

	sprintf(html_content1,"A(\"%s\",\"%s\",\"%s\");","Resolution", "H264:720", "H264:720");
	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, html_content1, MaxSize-strlen(pOutMem));


	if( MaxSize > strlen(pOutMem) )
	       strncat(pOutMem, "</SCRIPT>\n</table>\n</DIV>\n<HR size=1 color=#C0C0C0 width=620 align=left>\n", MaxSize-strlen(pOutMem));
	else
		fprintf(stderr,"MaxSize is not enough!\n");
#endif
}


