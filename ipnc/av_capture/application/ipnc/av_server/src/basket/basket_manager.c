#include <sys/ioctl.h>
#include <sys/mount.h>
#include <errno.h>
#include <basket.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <basket_rec.h>

void BKT_MGR_FormatSDD()
{
	BKT_closeAll();

	sleep(1);

	int res = umount("/dvr/data/sdda");
	if(res < 0)
	{
		perror( "perror says(format) failed umount /dvr/data/sdda");
	}
	sync();
	
	sleep(1);

	system("/opt/dvr/prep-sdd.sh");
	
	
	// make baskets
	int count = BKTREC_CreateBasket(DEFAULT_DISK_MPOINT);
	
}
int  BKT_MGR_deleteRecordData()
{
	int nret = BKTREC_deleteBasket(DEFAULT_DISK_MPOINT);

	if(nret != BKT_ERR)
	{
		char buf[255];

		sleep(1);
		
		sprintf(buf, "/bin/mount %s -o remount", DEFAULT_DISK_MPOINT);

		// 090928...
		system(buf);

		sleep(1);
		

		return TRUE;

	}

	return FALSE;
	
	/*
	char buf[255];
	sprintf(buf, "/bin/mount %s -o remount", DEFAULT_DISK_MPOINT);
	// 090928...
	system(buf);
	
	sleep(1);
	
	// make baskets
	int count = BKTREC_CreateBasket(DEFAULT_DISK_MPOINT);
	
	return count;
	*/
}


