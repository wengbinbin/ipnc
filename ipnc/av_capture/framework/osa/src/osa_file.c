

#include <osa_file.h>


int OSA_fileReadFile(char *fileName, Uint8 *addr, Uint32 readSize, Uint32 *file_offset, Uint32 *actualReadSize)
{
  int retVal = OSA_SOK;
  Uint8  *curAddr;
  
  Uint32 readDataSize, fileSize, chunkSize=1024*100;
  Uint32 userReadSize;

  FILE *hndlFile;

  #ifdef OSA_DEBUG_FILE
  OSA_printf(" [OSA_FILE ] Reading file [%s] ... ", fileName);
  #endif
  
  hndlFile = fopen(fileName, "rb");

  if(hndlFile == NULL) {
  	retVal = OSA_EFAIL;
    goto exit;
	}

  if (file_offset!=NULL){
	fseek(hndlFile, 0L, SEEK_END);
	fileSize = ftell(hndlFile);
    if (*file_offset >= fileSize)
  	  return OSA_EFAIL;
    fseek(hndlFile, *file_offset, SEEK_SET);
  }
  
  curAddr = addr;
  fileSize = 0;

  userReadSize = readSize;

  while(1) {
    if(userReadSize != 0) {
	  if(chunkSize > userReadSize)
		chunkSize = userReadSize;
	  readDataSize = fread(curAddr, 1, chunkSize, hndlFile);
	  fileSize += readDataSize;
	  if(chunkSize != readDataSize)
		break;
	  if(fileSize>=userReadSize)
		break;
	  curAddr += chunkSize;
	}
	else {
	  readDataSize = fread(curAddr, 1, chunkSize, hndlFile);
      fileSize+=readDataSize;
      if(chunkSize!=readDataSize)
        break;
      curAddr+=chunkSize;
    }
  }
  #ifdef OSA_DEBUG_FILE
  OSA_printf("Done. [%d bytes] \r\n", fileSize);
  #endif
  fclose(hndlFile);
  if(actualReadSize != NULL)
    *actualReadSize = fileSize;
  if (file_offset != NULL)
    *file_offset += fileSize;

exit:
  if(retVal!=OSA_SOK) {
    #ifdef OSA_DEBUG_FILE
    OSA_printf("ERROR \r\n");
    #endif
    fileSize=0;
  }
  return retVal;
}

int OSA_fileWriteFile(char *fileName, Uint8 *addr, Uint32 size)
{
  int retVal = OSA_SOK;
  Uint32 writeDataSize;
  
  Bool errorInWriting = FALSE;

  FILE *hndlFile;

	if(size==0)
		return OSA_SOK;

  #ifdef OSA_DEBUG_FILE
  OSA_printf(" [FILE ] Writing to file [%s] (%d bytes) ... ", fileName, size);
  #endif
  hndlFile = fopen(fileName, "wb");

  if(hndlFile == NULL) {
  	retVal = OSA_EFAIL;
    goto exit;
	}
	
  {
    // write in units of chunkSize
    Int32 fileSize, chunkSize = 96*1024;
    Int8  *curAddr;

    
    fileSize = size;
    curAddr  = (Int8*)addr;
    while(fileSize>0) {
      if(fileSize<chunkSize) {
        chunkSize = fileSize;
      }
      writeDataSize=0;
      writeDataSize = fwrite(curAddr, 1, chunkSize, hndlFile);
      if(writeDataSize!=chunkSize) {
        // error in writing, abort
        errorInWriting = TRUE;
        retVal = OSA_EFAIL;
        break;
      }
      curAddr += chunkSize;
      fileSize -= chunkSize;
    }
    writeDataSize = size - fileSize;
  }

  #ifdef OSA_DEBUG_FILE
  OSA_printf("Done. [%d bytes] \r\n", writeDataSize);
  #endif

  fclose(hndlFile);

exit:
  if(retVal!=OSA_SOK) {
    #ifdef OSA_DEBUG_FILE
    OSA_printf("ERROR \r\n");
    #endif
  }
  return retVal;

}

