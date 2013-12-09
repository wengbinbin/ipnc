#include "web_translate.h"
#include <sys_env_type.h>
#include "file_msg_drv.h"
#include <net_config.h>
#include <sysctrl.h>
#include "file_list.h"

#define FALSE 0
#define TRUE !FALSE

#undef st_mtime
#undef st_atime
#undef st_ctime

struct stat64 {
      unsigned long long    st_dev;
      unsigned char     		__pad0[4];
#define STAT64_HAS_BROKEN_ST_INO    1
      unsigned long     		__st_ino;
      unsigned int      			st_mode;
      unsigned int      			st_nlink;
      unsigned long     		st_uid;
      unsigned long     		st_gid;
      unsigned long long    st_rdev;
      unsigned char     		__pad3[4];
      long long   				st_size;
      unsigned long			st_blksize;
      unsigned long long    st_blocks;
      unsigned long     		st_atime;
      unsigned long     		st_atime_nsec;
      unsigned long     		st_mtime;
      unsigned int				st_mtime_nsec;
      unsigned long     		st_ctime;
      unsigned long     		st_ctime_nsec;
      unsigned long long      st_ino;
};
extern int stat64 (__const char *__path, struct stat64 *__statbuf);

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

char pathname[MAXPATHLEN];
//char html_header[] = "<HTML><META HTTP-EQUIV=\"CACHE-CONTROL\" CONTENT=\"NO-CACHE\"><TITLE>Local Storage - SDCard</TITLE><BODY><PRE style='font-family:\"Courier New\";'><H1>Filelist of Memory Card</H1><BR>Filename         Date       Time   Size<BR><HR><BR><script>\r\nvar z=\"sddel.htm\";\r\nfunction F(b){var o=\"\";var i;for(i=0;i<b;i++){o+=' ';}return o;}\r\nfunction A(a,b,c,d){var o=\'';o+='<A HREF=\"?FILE='+a+'\">'+a+'</A>';o+=F(15-a.length)+b+' '+c+' '+d;o+=F(9-d.length)+'<A HREF=\"'+z+'?FILE='+a+'\">Delete</A>\\n';document.write(o);}";

char html_header[] = "<HTML><META HTTP-EQUIV=\"CACHE-CONTROL\" CONTENT=\"NO-CACHE\"><TITLE>Local Storage - SDCard</TITLE><style>BODY {	PADDING-RIGHT: 0px; MARGIN-TOP: 10px; PADDING-LEFT: 10px; MARGIN-LEFT: auto; WIDTH: auto; MARGIN-RIGHT: auto; background-color:#182439; FONT-FAMILY: Tahoma, Geneva, Arial, \"Arial Narrow\", Verdana, sans-serif; font-size:18px; color:#FFFFFF}A.linktxt:link {PADDING-RIGHT: 0px; PADDING-LEFT: 0px; FONT-WEIGHT: normal; FONT-SIZE: 18px; PADDING-BOTTOM: 0px; MARGIN: 0px; COLOR: #FFFFFF; PADDING-TOP: 0px; TEXT-DECORATION: underline}.linktxt:visited {PADDING-RIGHT: 0px; PADDING-LEFT: 0px; FONT-WEIGHT: normal; FONT-SIZE: 18px; PADDING-BOTTOM: 0px; MARGIN: 0px; COLOR: #FFFFFF; PADDING-TOP: 0px; TEXT-DECORATION: underline} A.linktxt:active { PADDING-RIGHT: 0px; PADDING-LEFT: 0px; FONT-WEIGHT: normal; FONT-SIZE: 18px; PADDING-BOTTOM: 0px; MARGIN: 0px; COLOR: #FFFFFF; PADDING-TOP: 0px; TEXT-DECORATION: underline} A.linktxt:hover { FONT-WEIGHT: normal; FONT-SIZE: 18px; COLOR: #FFFFFF; TEXT-DECORATION: underline}.heading{ FONT-FAMILY: Tahoma, Geneva, Arial, \"Arial Narrow\", Verdana, sans-serif; font-size:18pt; color:#FFFFFF } .normaltxt{ 	FONT-FAMILY: Courier New, Tahoma, Geneva, Arial, \"Arial Narrow\", Verdana, sans-serif; font-size:18px; color:#FFFFFF }.tableheadtxt{ 	FONT-FAMILY: Courier New, Tahoma, Geneva, Arial, \"Arial Narrow\", Verdana, sans-serif; font-size:18px; color:#000000} </style><BODY><PRE style='font-family:\"Courier New\"; font-size:18px'><PRE style='font-family:\"Courier New\"; font-size:18px'><span class=heading>Filelist of Memory Card</span><br><br><div STYLE=\"position:relative;width:620;height:80%;overflow-y:scroll;z-index:10;\"><table border=0 cellspacing=1 cellpadding=1 class=normaltxt width=600><tr align=center bgcolor=#EEEEEE class=tableheadtxt><td>Filename</td><td>Date</td><td>Time</td><td>Size</td><td>Action</td></tr><script>var z=\"sddel.htm\";function F(b){var o=\"\";var i;for(i=0;i<b;i++){o+=' ';}return o;}"
"function A(a,b,c,d){var o=\'';o+='<tr valign=middle><td><A HREF=\"?FILE='+a+'\" class=\"linktxt\">'+a+'</A></td><td>';o+=F(15-a.length)+b+' </td><td> '+c+' </td><td> '+d;o+=F(9-d.length)+'</td><td><A HREF=\"'+z+'?FILE='+a+'\" class=\"linktxt\">Delete</A></td></tr><tr><td colspan=5><hr size=1 color=#3030A0></td></tr>';document.write(o);}";

char html_content[100];
char html_end[] = "<HR>HTTP Server</A><BR></PRE></BODY></HTML>";
char html_empty[] = "NO FILE MATCH *.AVI *.JPG *.YUV";

char html_log_header[] = "<HTML><META HTTP-EQUIV=\"CACHE-CONTROL\" CONTENT=\"NO-CACHE\"><TITLE>Camera Logs</TITLE><style>BODY {	PADDING-RIGHT: 0px; MARGIN-TOP: 10px; PADDING-LEFT: 10px; MARGIN-LEFT: auto; WIDTH: auto; MARGIN-RIGHT: auto; background-color:#182439; FONT-FAMILY: Tahoma, Geneva, Arial, \"Arial Narrow\", Verdana, sans-serif; font-size:18px; color:#FFFFFF}A.linktxt:link {PADDING-RIGHT: 0px; PADDING-LEFT: 0px; FONT-WEIGHT: normal; FONT-SIZE: 18px; PADDING-BOTTOM: 0px; MARGIN: 0px; COLOR: #FFFFFF; PADDING-TOP: 0px; TEXT-DECORATION: underline}.linktxt:visited {PADDING-RIGHT: 0px; PADDING-LEFT: 0px; FONT-WEIGHT: normal; FONT-SIZE: 18px; PADDING-BOTTOM: 0px; MARGIN: 0px; COLOR: #FFFFFF; PADDING-TOP: 0px; TEXT-DECORATION: underline} A.linktxt:active { PADDING-RIGHT: 0px; PADDING-LEFT: 0px; FONT-WEIGHT: normal; FONT-SIZE: 18px; PADDING-BOTTOM: 0px; MARGIN: 0px; COLOR: #FFFFFF; PADDING-TOP: 0px; TEXT-DECORATION: underline} A.linktxt:hover { FONT-WEIGHT: normal; FONT-SIZE: 18px; COLOR: #FFFFFF; TEXT-DECORATION: underline}.heading{ FONT-FAMILY: Tahoma, Geneva, Arial, \"Arial Narrow\", Verdana, sans-serif; font-size:18pt; color:#FFFFFF } .normaltxt{ 	FONT-FAMILY: Courier New, Tahoma, Geneva, Arial, \"Arial Narrow\", Verdana, sans-serif; font-size:18px; color:#FFFFFF }.tableheadtxt{ 	FONT-FAMILY: Courier New, Tahoma, Geneva, Arial, \"Arial Narrow\", Verdana, sans-serif; font-size:18px; color:#000000} </style><BODY><PRE style='font-family:\"Courier New\"; font-size:18px'><PRE style='font-family:\"Courier New\"; font-size:18px'><span class=heading>NO LOG ENTRIES</span><br><br><div STYLE=\"position:relative;width:620;height:80%;overflow-y:scroll;z-index:10;\"><table border=0 cellspacing=1 cellpadding=1 class=normaltxt width=600><tr align=center bgcolor=#EEEEEE class=tableheadtxt></tr><script>var z=\"sddel.htm\";function F(b){var o=\"\";var i;for(i=0;i<b;i++){o+=' ';}return o;}";

#if 0
main(int argc,char **argv)
{
	int count = 0;
	long freespace = 0;
	char *pInputDir = NULL;
	char *pOutDir = NULL;
	char *pOutFile = NULL;
	char *pOutDisk = NULL;

	if (argc < 5)
	{
		printf("Usage: %s <pathname> <disk_pathname> <ouput_dir> <ouput_file>\n",argv[0]);
		exit(1);
	}
	pInputDir	= argv[1];
	pOutDisk 	= argv[2];
	pOutDir 	= argv[3];
	pOutFile 	= argv[4];

	if (chdir(pInputDir) != 0)
	{
		printf("Error in chdir \n");
		exit(1);
	}
#if 1
	List_files_To_html(pInputDir,pOutDir,pOutFile,pOutDisk);
#else

	count = List_files();
	if( count > 0 )
	{
		printf("Number of files = %d\n",count);
	}

	freespace = GetDiskfreeSpace(pOutDisk);
	printf("Free Spces = %dKBytes \n",freespace);

	printf("\n");

#endif

}
#endif
/*
0	:file exist
-1	:file not exist
*/
int CheckFileExist(char *pDir,char *pFilename)
{
	struct stat finfo;
	int ret = -1;

	if( pDir != NULL )
	{
		if (chdir(pDir) != 0)
		{
			return -1;
		}
	}
	ret = stat( pFilename, &finfo );
	chdir("/");
	return  ret;
}

int DeleteFile(char *pDir,char *pFilename)
{
	int ret = -1;
	if( pDir != NULL )
	{
		if (chdir(pDir) != 0)
		{
			return -1;
		}
	}
	ret = remove(pFilename);
	chdir("/");
	return ret;
}

int Ouput_html_empty(char *pOutDir,char *pOutFile)
{
	FILE *pfd = NULL;
	int error_code = 0;

	if (chdir(pOutDir) != 0)
	{
		printf("Error in chdir \n");
		error_code = -1;
		goto QUIT_EMPTY;
	}

	remove(pOutFile);

	pfd = fopen(pOutFile,"wb");
	if( pfd == NULL )
	{
		printf("Error in fopen \n");
		error_code = -1;
		goto QUIT_EMPTY;
	}
	if( fseek( pfd, 0, SEEK_SET ) < 0)
	{
		printf("Error in fseek \n");
		error_code = -1;
		goto QUIT_EMPTY;
	}

	if( fwrite( html_empty , sizeof(html_empty), 1, pfd ) != sizeof(html_empty))
	{
		printf("Error in fwrite \n");
		error_code = -1;
		goto QUIT_EMPTY;
	}
QUIT_EMPTY:
	if( pfd != NULL )
	{
		fflush(pfd);
		fclose(pfd);
	}

	return error_code;

}

int OuputMem_html_empty(void *pMem, int maxsize)
{
	char tempbuff[100];
	char http_ip_addr[100];

	GetIP_Addr(http_ip_addr);

	strncpy( pMem, html_empty, (maxsize) );
	sprintf(tempbuff,"<HR>HTTP Server at <A HREF=\"http://%s\">ipnc</A><BR></PRE></BODY></HTML>", http_ip_addr);
	strncat(pMem, tempbuff, maxsize);

	return 0;

}

int OuputLog_html_empty(void *pMem, int maxsize)
{
	char http_ip_addr[100];
	GetIP_Addr(http_ip_addr);

	strncpy( pMem, html_log_header, maxsize );

	return 0;

}


static FILE_INFO *pLIST_MEM = NULL;

FILE_INFO *Get_File_List( char *pDir, int *pCnt )
{

	int count,i;
	struct direct **files;
	struct stat64 finfo;
	struct tm *pTime;

	if (chdir(pDir) != 0)
	{
		printf("Error in chdir \n");
		goto FAIL_GET_LIST;
	}
	count = scandir( pDir, &files, file_select, alphasort);
	/* If no files found, make a non-selectable menu item */
	if(count <= 0)
	{
		printf("No files in this directory \n");
		goto FAIL_GET_LIST;
	}

	pLIST_MEM = malloc(sizeof(FILE_INFO)*count);
	if( pLIST_MEM == NULL )
	{
		printf("No memory \n");
		goto FAIL_GET_LIST;
	}

	for (i=0;i<count;i++)
	{

		if(!stat64(files[i]->d_name,&finfo))
		{
			pTime = localtime(&finfo.st_ctime);
			sprintf(pLIST_MEM[i].name,"%s",files[i]->d_name);
			sprintf(pLIST_MEM[i].date,"%d/%02d/%02d",(1900+pTime->tm_year),( 1+pTime->tm_mon), pTime->tm_mday);
			sprintf(pLIST_MEM[i].time,"%02d:%02d:%02d",pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
			sprintf(pLIST_MEM[i].size,"%lldK",((long long)finfo.st_size/(long long)1024));
			/*fprintf(stderr,"%s	%s	%s	%s \n",	pLIST_MEM[i].name,
										pLIST_MEM[i].date,
										pLIST_MEM[i].time,
										pLIST_MEM[i].size);*/
		}else{
			sprintf(pLIST_MEM[i].name,"%s",files[i]->d_name);
			sprintf(pLIST_MEM[i].date,"%d/%02d/%02d",(1900+0),(1+0), 0);
			sprintf(pLIST_MEM[i].time,"%02d:%02d:%02d",0, 0, 0);
			sprintf(pLIST_MEM[i].size,"%dK",(int)(0/1024));
		}


	}

	*pCnt = count;
	return pLIST_MEM;

FAIL_GET_LIST:
	*pCnt = 0;
	pLIST_MEM = NULL;
	return pLIST_MEM;
}

void Clean_File_List(void)
{
	if( pLIST_MEM!=NULL )
	{
		free((void *)pLIST_MEM);
		pLIST_MEM = NULL;
	}

}

int List_files_To_html(char *pInputDir,char *pOutDir,char *pOutFile,char *pOutDisk)
{

	int 	count = 0;
	int 	i = 0;
	FILE 	*pfd = NULL;
	int 	error_code = 0;
	FILE_INFO *pFilelist = NULL;

	//printf("LINE= %d \n",__LINE__);

	if (chdir(pInputDir) != 0)
	{
		printf("Error in chdir \n");
		error_code = -1;
		goto QUIT_SAVE;
	}

	pFilelist = Get_File_List( pInputDir, &count );

	//printf("LINE= %d \n",__LINE__);

	if( pFilelist == NULL )
	{
		Ouput_html_empty( pOutDir, pOutFile);

		printf("Error get file list \n");
		error_code = -1;
		goto QUIT_SAVE;
	}

	//printf("LINE= %d \n",__LINE__);

	if (chdir(pOutDir) != 0)
	{
		printf("Error in chdir \n");
		error_code = -1;
		goto QUIT_SAVE;
	}

	//printf("LINE= %d \n",__LINE__);

	remove(pOutFile);

	//printf("LINE= %d \n",__LINE__);

	pfd = fopen(pOutFile,"wb");
	if( pfd == NULL )
	{
		printf("Error in fopen \n");
		error_code = -1;
		goto QUIT_SAVE;
	}
	if( fseek( pfd, 0, SEEK_SET ) < 0)
	{
		printf("Error in fseek \n");
		error_code = -1;
		goto QUIT_SAVE;
	}

	//printf("LINE= %d \n",__LINE__);

	if( fwrite( html_header , sizeof(html_header)-1, 1, pfd ) != 1)
	{
		printf("Error in fwrite \n");
		error_code = -1;
		goto QUIT_SAVE;
	}

	//printf("LINE= %d \n",__LINE__);

	for (i=0;i<count;i++)
	{
		sprintf(html_content,"A(\"%s\",\"%s\",\"%s\",\"%s\");",pFilelist[i].name,
													pFilelist[i].date,
													pFilelist[i].time,
													pFilelist[i].size);
		fprintf(pfd,"%s",html_content);
	}

	fprintf(pfd,"</SCRIPT>\n");
	fprintf(pfd,"%d file and %d KBytes free\n",count, (int)GetDiskfreeSpace(pOutDisk));

	if( fwrite( html_end , sizeof(html_end), 1, pfd ) != 1)
	{
		printf("Error in fwrite \n");
		error_code = -1;
		goto QUIT_SAVE;
	}

QUIT_SAVE:
	if( pfd != NULL )
	{
		fflush(pfd);
		fclose(pfd);
	}

	Clean_File_List();

	if( error_code < 0)
		return error_code;
	else
		return count;

}

void GetIP_Addr(char *pMem )
{
	NET_IPV4 ip;

	ip.int32 = net_get_ifaddr("eth0");

	sprintf(pMem, "%d.%d.%d.%d", ip.str[0], ip.str[1], ip.str[2], ip.str[3]);

	return ;
}

int MEM_List_files_To_html(char *pInputDir,char *pOutDisk, void *pOutMem, int MaxSize)
{

	char http_ip_addr[100];
	int 	count = 0;
	int 	i = 0;
	int 	error_code = 0;
	FILE_INFO *pFilelist = NULL;
	char	tempbuff[100];
	char 	recodingName[30];
	int 	ret = 0;
	int 	string_length = 0;
	ret = ControlSystemData(SFIELD_GET_SD_FILE_NAME, (void *)recodingName, sizeof(recodingName));
	if(ret >= 0 )
	{
		string_length = ret;
		recodingName[string_length] = '\0';
	}else{
		string_length = 0;
	}


	GetIP_Addr(http_ip_addr);
	fprintf(stderr,"para_netip %s \n",http_ip_addr);

	if (chdir(pInputDir) != 0)
	{
		printf("Error in chdir \n");
		error_code = -1;
		goto QUIT_MEM_SAVE;
	}
	pFilelist = Get_File_List( pInputDir, &count );

	if( pFilelist == NULL )
	{
		fprintf(stderr,"Error get file list \n");
		error_code = -1;
		goto QUIT_MEM_SAVE;
	}

	strncpy( pOutMem, html_header, MaxSize );


	for (i=0;i<count;i++)
	{
		if( string_length > 0 )
		{

			if( strncmp(pFilelist[i].name, recodingName, sizeof(recodingName) ) == 0 )
			{
				continue;
			}
		}

		sprintf(html_content,"A(\"%s\",\"%s\",\"%s\",\"%s\");",pFilelist[i].name,
													pFilelist[i].date,
													pFilelist[i].time,
													pFilelist[i].size);

		if( MaxSize > strlen(pOutMem) )
			strncat(pOutMem, html_content, MaxSize-strlen(pOutMem));
	}
	if( MaxSize > strlen(pOutMem) )
//		strncat(pOutMem, "</SCRIPT>\n", MaxSize-strlen(pOutMem));
	       strncat(pOutMem, "</SCRIPT>\n</table>\n</DIV>\n<HR size=1 color=#C0C0C0 width=620 align=left>\n", MaxSize-strlen(pOutMem));

	else
		fprintf(stderr,"MaxSize is not enough!\n");

	sprintf(tempbuff, "%d file and %d KBytes free\n",count, (int)GetDiskfreeSpace(pOutDisk));

	if( MaxSize > strlen(pOutMem) )
		strncat(pOutMem, tempbuff, MaxSize-strlen(pOutMem));
	else
		fprintf(stderr,"MaxSize is not enough!\n");

//	sprintf(tempbuff,"<HR>HTTP Server at <A HREF=\"http://%s\">ipnc</A><BR></PRE></BODY></HTML>", http_ip_addr);
//       sprintf(tempbuff,"<HR size=1 color=#C0C0C0 width=620 align=left>HTTP Server at <A HREF=\"http://%s\">ipnc</A><BR></PRE></BODY></HTML>", http_ip_addr);

//	if( MaxSize > strlen(pOutMem) )
//		strncat(pOutMem, tempbuff, MaxSize-strlen(pOutMem));
//	else
//		fprintf(stderr,"MaxSize is not enough!\n");

	//strncat(pOutMem, html_end, MaxSize);


QUIT_MEM_SAVE:

	Clean_File_List();
	chdir("/");

	if( error_code < 0)
	{
		OuputMem_html_empty( pOutMem,MaxSize);
		return error_code;
	}
	else
	{
		return count;
	}

}


int List_files(void)
{

	int count,i;
	struct direct **files;
	struct stat64 finfo;
	struct tm *pTime;
	char	datebuff[30];
	char	timebuff[30];


	if (getcwd(pathname,sizeof(pathname)) == NULL )
	{
		printf("Error getting path \n");
		return -1;
	}
	printf("Current Working Directory = %s \n",pathname);

	count = scandir( pathname, &files, file_select, alphasort);
	/* If no files found, make a non-selectable menu item */
	if(count <= 0)
	{
		printf("No files in this directory \n");
		return -1;
	}


	printf("\n");
	for (i=0;i<count;i++)
	{

		if(!stat64(files[i]->d_name,&finfo))
		{
			pTime = localtime(&finfo.st_ctime);
			sprintf(datebuff,"%d/%02d/%02d",(1900+pTime->tm_year),( 1+pTime->tm_mon), pTime->tm_mday);
			sprintf(timebuff,"%02d:%02d:%02d",pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
			printf("%s	%s	%s	%dK \n",files[i]->d_name,datebuff,timebuff,(int)(finfo.st_size/1024));
		}else{
			printf("%s  \n",files[i]->d_name);
		}


	}


	return count;

}

long long GetDiskfreeSpace(char *pDisk)
{
	long long freespace = 0;
	struct statfs disk_statfs;

	if( statfs(pDisk, &disk_statfs) >= 0 )
	{
		freespace = (((long long)disk_statfs.f_bsize  * (long long)disk_statfs.f_bfree)/(long long)1024);
	}
	/**
	fprintf(stderr,"GetDiskfreeSpace %lli \n",freespace);
	fprintf(stderr,"f_type: 0x%X\n", disk_statfs.f_type);
	fprintf(stderr,"f_bsize: %d\n", disk_statfs.f_bsize);
	fprintf(stderr,"f_blocks: %li\n", disk_statfs.f_blocks);
	fprintf(stderr,"f_bfree: %li\n", disk_statfs.f_bfree);
	fprintf(stderr,"f_bavail: %li\n", disk_statfs.f_bavail);
	fprintf(stderr,"f_files: %li\n", disk_statfs.f_files);
	fprintf(stderr,"f_ffree: %li\n", disk_statfs.f_ffree);
	*/
	return freespace;
}

long long GetDiskusedSpace(char *pDisk)
{
	long long usedspace = 0;;

	struct statfs disk_statfs;

	if( statfs(pDisk, &disk_statfs) >= 0 )
	{
		usedspace = (((long long)disk_statfs.f_bsize * (((long long)disk_statfs.f_blocks) - (long long)disk_statfs.f_bfree)/(long long)1024));
	}
	return usedspace;
}
char *Compare_List[] =
{
	".avi",
	".jpg",
	".yuv"
};

int file_select(const struct dirent   *entry)
{
	char *ptr;
	int cmp_num = sizeof(Compare_List)/sizeof(Compare_List[0]);
	int cnt = 0;
	int	IsFind = 0;

	if ((strcmp(entry->d_name, ".")== 0) ||
			(strcmp(entry->d_name, "..") == 0))
			 return (FALSE);

	/* Check for filename extensions */
	ptr = rindex(entry->d_name, '.');
	if( ptr != NULL )
	{
		for( cnt = 0; cnt < cmp_num; cnt++ )
		{
			if( strcmp(ptr,Compare_List[cnt]) == 0 )
			{
				IsFind = 1;
				break;
			}
		}
		if( IsFind == 1 )
		{
			return (TRUE);
		}else{
			return (FALSE);
		}
	}
	else
	{
		return(FALSE);
	}
}
