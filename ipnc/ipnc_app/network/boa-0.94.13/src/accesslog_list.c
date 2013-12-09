#include "web_translate.h"
#include <sys_env_type.h>
#include "file_msg_drv.h"
#include <net_config.h>
#include <sysctrl.h>
#include "file_list.h"
#include<stdlib.h>
#include <system_default.h>

#define SUCCESS 0
#define FAIL	-1

#define MAGIC_NUM	0x56313930    //0x29C51100
#define SYS_ENV_SIZE	sizeof(SysInfo)
#define MAX_LOG_PAGE_NUM	20
#define NUM_LOG_PER_PAGE	20
#define LOG_ENTRY_SIZE	sizeof(LogEntry_t)

typedef struct AccessLogData_t{
	LogEntry_t tLogData;
	struct AccessLogData_t* pNext;
}AccessLogData_t;

static AccessLogData_t* gAccessLogHead = NULL;

char html_header3[] = "<HTML><META HTTP-EQUIV=\"CACHE-CONTROL\" CONTENT=\"NO-CACHE\"><TITLE>Event: Access Log</TITLE><style>BODY {	PADDING-RIGHT: 0px; MARGIN-TOP: 10px; PADDING-LEFT: 10px; MARGIN-LEFT: auto; WIDTH: auto; MARGIN-RIGHT: auto; background-color:#182439; FONT-FAMILY: Tahoma, Geneva, Arial, \"Arial Narrow\", Verdana, sans-serif; font-size:18px; color:#FFFFFF}A.linktxt:link {PADDING-RIGHT: 0px; PADDING-LEFT: 0px; FONT-WEIGHT: normal; FONT-SIZE: 18px; PADDING-BOTTOM: 0px; MARGIN: 0px; COLOR: #FFFFFF; PADDING-TOP: 0px; TEXT-DECORATION: underline}.linktxt:visited {PADDING-RIGHT: 0px; PADDING-LEFT: 0px; FONT-WEIGHT: normal; FONT-SIZE: 18px; PADDING-BOTTOM: 0px; MARGIN: 0px; COLOR: #FFFFFF; PADDING-TOP: 0px; TEXT-DECORATION: underline} A.linktxt:active { PADDING-RIGHT: 0px; PADDING-LEFT: 0px; FONT-WEIGHT: normal; FONT-SIZE: 18px; PADDING-BOTTOM: 0px; MARGIN: 0px; COLOR: #FFFFFF; PADDING-TOP: 0px; TEXT-DECORATION: underline} A.linktxt:hover { FONT-WEIGHT: normal; FONT-SIZE: 18px; COLOR: #FFFFFF; TEXT-DECORATION: underline}.heading{ FONT-FAMILY: Tahoma, Geneva, Arial, \"Arial Narrow\", Verdana, sans-serif; font-size:18pt; color:#FFFFFF } .normaltxt{ 	FONT-FAMILY: Courier New, Tahoma, Geneva, Arial, \"Arial Narrow\", Verdana, sans-serif; font-size:18px; color:#FFFFFF }.tableheadtxt{ 	FONT-FAMILY: Courier New, Tahoma, Geneva, Arial, \"Arial Narrow\", Verdana, sans-serif; font-size:18px; color:#000000} </style><BODY><PRE style='font-family:\"Courier New\"; font-size:18px'><PRE style='font-family:\"Courier New\"; font-size:18px'><span class=heading>Event: Access Log</span><br><br><div STYLE=\"position:relative;width:620;height:80%;overflow-y:scroll;z-index:10;\"><table border=0 cellspacing=1 cellpadding=1 class=normaltxt width=600><tr align=center bgcolor=#EEEEEE class=tableheadtxt><td>Item</td><td>Date and Time</td><td>Events</td></tr><script>var z=\"sddel.htm\";function F(b){var o=\"\";var i;for(i=0;i<b;i++){o+=' ';}return o;}"
"function A(a,b,c){var o=\'';o+='<tr valign=middle><td><A HREF=\"?FILE='+a+'\" class=\"linktxt\">'+a+'</A></td><td>';o+=F(15-a.length)+b+' </td><td> '+c+' </td></tr><tr><td colspan=5><hr size=1 color=#3030A0></td></tr>';document.write(o);}";


char html_content3[100];
char html_end3[] = "<HR>HTTP Server</A><BR></PRE></BODY></HTML>";
char html_empty3[] = "NO FILE MATCH *.AVI *.JPG *.YUV";



int check_magic_num(FILE *fp)
{
	int ret;
	unsigned long MagicNum;
	if(fread(&MagicNum, 1, sizeof(MagicNum), fp) != sizeof(MagicNum)){
		ret = FAIL;
	} else {
		if(MagicNum == MAGIC_NUM){
			ret = SUCCESS;
		} else {
			ret = FAIL;
		}
	}
	return ret;
}

int ReadLog1()
{
	FILE *fp;
	char Buffer[LOG_ENTRY_SIZE];
	int ret = SUCCESS, count = 0;
	LogEntry_t  log;

	fp = fopen(LOG_FILE, "rb");
      if((fp = fopen(LOG_FILE, "rb")) == NULL)
	{
		/* log file not exist */
		ret = FAIL;
	}
      else
      {    if(check_magic_num(fp) == SUCCESS){
              while(count < 5)
			{
                          //if(fread(Buffer, 1, LOG_ENTRY_SIZE,fp) != LOG_ENTRY_SIZE){
				if(fread(&log, 1, LOG_ENTRY_SIZE,fp) != LOG_ENTRY_SIZE){
					break;
				}

				fprintf(stderr,"Event %s \n",log.event);
			 count ++;
			}
		}
	       else {
			ret = FAIL;
		}


	}

	return 1;
}

int AccessLog_List_To_html(void *pOutMem, int MaxSize)
{
    char http_ip_addr[100];
	char	tempbuff[100], timeStr[32], subStr[15];
	FILE *fp;
	char Buffer[LOG_ENTRY_SIZE];
	int ret = SUCCESS, count = 0, ctr = 1;
	LogEntry_t  log;

	GetIP_Addr(http_ip_addr);
//	fprintf(stderr,"para_netip %s \n",http_ip_addr);
	strncpy( pOutMem, html_header3, MaxSize );

    fp = fopen(LOG_FILE, "rb");
    if((fp = fopen(LOG_FILE, "rb")) == NULL)
	{
		/* log file not exist */
		ret = FAIL;
	}
    else
    {
		if(check_magic_num(fp) == SUCCESS)
	    {
            while(count < NUM_LOG_PER_PAGE * MAX_LOG_PAGE_NUM)
		    {
				if(fread(&log, 1, LOG_ENTRY_SIZE,fp) != LOG_ENTRY_SIZE)
			    {
	                ret = SUCCESS;
					break;
                }
			   //memcpy(subStr, log.event, 14);
			   //if(strcmp(subStr, "admin login on") == 0)
			   if(strstr(log.event, "login") != NULL)
			   {
					 sprintf(timeStr, "%04d-%02d-%02d %02d:%02d:%02d", log.time.tm_year + 1900,log.time.tm_mon + 1, log.time.tm_mday, log.time.tm_hour, log.time.tm_min, log.time.tm_sec);
			         sprintf(html_content3,"A(\"%d\",\"%s\",\"%s\");", ctr++, timeStr, log.event);
	                 if( MaxSize > strlen(pOutMem) )
		                strncat(pOutMem, html_content3, MaxSize-strlen(pOutMem));
			         else
			         {
		                 fprintf(stderr,"MaxSize is not enough!\n");
				         break;
			         }
			   }
			   count++;
		   }
	    }
	    else
	    {
			return FAIL;
	    }
        fclose(fp);

	}

	if( MaxSize > strlen(pOutMem) )
	       strncat(pOutMem, "</SCRIPT>\n</table>\n</DIV>\n<HR size=1 color=#C0C0C0 width=620 align=left>\n", MaxSize-strlen(pOutMem));
	else
		fprintf(stderr,"MaxSize is not enough!\n");
//       sprintf(tempbuff,"<HR size=1 color=#C0C0C0 width=620 align=left>HTTP Server at <A HREF=\"http://%s\">ipnc</A><BR></PRE></BODY></HTML>", http_ip_addr);

//	if( MaxSize > strlen(pOutMem) )
//		strncat(pOutMem, tempbuff, MaxSize-strlen(pOutMem));
//	else
//		fprintf(stderr,"MaxSize is not enough!\n");

   return ret;
}
void cleanAccessLog()
{
	//AccessLogData_t *ptr;
	LogEntry_t  log;
	FILE *fp,*fp1;
	char subStr[15], Buffer[LOG_ENTRY_SIZE];
	int ret = SUCCESS;
	int bytesRead, count=0;
	char ch;
	unsigned long MagicNum = LOG_MAGIC_NUM;

	if((fp1 = fopen(ACCESS_LOG_FILE, "wb")) == NULL){
		fprintf(stderr,"\r\nCan't create log file\n");
		return FAIL;
	} else {
		if(fwrite(&MagicNum, 1, sizeof(MagicNum), fp1) != sizeof(MagicNum)){
			fprintf(stderr,"\r\nWriting Magic Number fail\n");
			return FAIL;
        }
	}

    fp = fopen(LOG_FILE, "rb");
    if(fp == NULL)
	{
		fprintf(stderr,"\r\nCan't open log file\n");
		return FAIL;
	}


      if(check_magic_num(fp) == SUCCESS)
      {
	     while(count < NUM_LOG_PER_PAGE * MAX_LOG_PAGE_NUM)
	     {
		    if(fread(&log, 1, LOG_ENTRY_SIZE,fp) != LOG_ENTRY_SIZE)
		    {
				break;
		    }
		    else
		    {
			  //if(strncmp(log.event, "admin login on", 14) == 0)
			  if(strstr(log.event, "login") != NULL)
			  {
			 	 continue;
			  }
			  else
			  {
			       fwrite(&log, 1, LOG_ENTRY_SIZE, fp1);
			  }
		    }
		    count++;
	     	}
      	}


	fclose(fp);
	fclose(fp1);

	if((fp = fopen(ACCESS_LOG_FILE, "rb")) == NULL){
		fprintf(stderr,"\r\nCan't open access log file for reading\n");
		return FAIL;
	}

	if((fp1 = fopen(LOG_FILE, "wb")) == NULL){
		fprintf(stderr,"\r\nCan't open log file for writing\n");
		return FAIL;
	}

    if(fwrite(&MagicNum, 1, sizeof(MagicNum), fp1) != sizeof(MagicNum)){
		fprintf(stderr,"\r\nWriting Magic Number fail\n");
		return FAIL;
    }

	 if(check_magic_num(fp) == SUCCESS)
	 {
		while(count < NUM_LOG_PER_PAGE * MAX_LOG_PAGE_NUM)
		{
			if(fread(&log, 1, LOG_ENTRY_SIZE,fp) != LOG_ENTRY_SIZE)
			{
				break;
			}
			else
			{
		        fwrite(&log, 1, LOG_ENTRY_SIZE, fp1);
            }
		      count++;
		}

	}

    fclose(fp);
	fclose(fp1);

	return ret;
}

