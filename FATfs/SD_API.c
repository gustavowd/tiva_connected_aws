#include <string.h>
#include "FreeRTOS.h"
#include "SD_API.h"

extern int printf_UART(const char *format, ...);

#if (SD_FAT_MUTEX_EN == 1)
  BRTOS_Mutex *SDCardResource;
#endif

uint8_t  sd_command = SD_INACTIVE;    // Variable to indicate commands for SD
static FATFS FATFS_Obj;

// File object
FIL     file_obj;
// File object 2
FIL     file_obj2;

// File Info object
FILINFO Finfo;
// Directory object
DIR     Dir;
// Read/Write Buffer
uint8_t   Buff[512];

#if _USE_LFN
char Lfname[256];
#endif
  

//Mensagens padrão da API do SD
const char *SD_API_FILE_NOT_FOUND={"\n\rFile or directory not found.\n\r"};
const char *SD_API_FILE_INVALID={"\n\rInvalid file or directory name.\n\r"};
const char *SD_API_CARD_BUSY={"\n\rSD card busy !!!\n\r"};
const char *SD_API_CARD_NOT_PRESENT={"\n\rSD card is not present or not initialized !\n\r"};


#if (SD_FAT_MUTEX_EN == 1)
void InitSDCardResource(uint8_t priority)
{
  // Cria um mutex informando que o recurso está disponível
  // Prioridade máxima a acessar o recurso = priority
  if (OSMutexCreate(&SDCardResource,priority) != ALLOC_EVENT_OK)
  {
    // Oh Oh
    // Não deveria entrar aqui !!!
    while(1){};
  };   
}
#endif



uint8_t InitSD(uint8_t verbose)
{ 
  FRESULT f_res;
  uint8_t   status;

  // Wait signals stabilization
  init_control_pins();
  vTaskDelay(5);
  disk_timerproc();
   
  if ((disk_status(0) & STA_NOINIT) != STA_NOINIT)
  {
    if (verbose == VERBOSE_ON)
    {
      printf_UART("\n\rSD card is already mounted!\n\r");
    }
    return SD_OK;
  }
  
  if ((disk_status(0) & STA_NODISK) != STA_NODISK)
  {
		// Card detected
		// Wait power up stabilization
		vTaskDelay(300);
		status = disk_initialize(0);
		if ((status&0x03) == 0)
		{
			if (verbose == VERBOSE_ON)
			{
				printf_UART("\n\rSD card has been successfully initialized !!!\n\r");
			}
			
			// Mount FAT File System
			f_res = f_mount((uint8_t)0, &FATFS_Obj);
			if (f_res == FR_OK)
			{
				if (verbose == VERBOSE_ON)
					printf_UART("\n\rFAT filesystem mounted !!!\n\r");
				return SD_OK;
			}else
			{
				if (verbose == VERBOSE_ON)
					printf_UART("\n\rFailed to mount SD card !\n\r");
				return MOUNT_SD_FAILS;
			}
			
		}else
		{
			if (verbose == VERBOSE_ON)
				printf_UART("\n\rSD card failure !!!\n\r");
			return INIT_SD_FAILS;
		}
	}else
	{
		if (verbose == VERBOSE_ON)
			printf_UART("\n\rNo SD card detected !!!\n\r");
		return NO_SD_CARD;
	}

  return NO_SD_CARD;
}


uint8_t SafeRemoveSD(uint8_t verbose)
{        
  if ((disk_status(0) & STA_NOINIT) != STA_NOINIT)
  {    
    #if (SD_FAT_MUTEX_EN == 1)
      OSMutexAcquire(SDCardResource);
    #endif    
    
    if (sd_command == SD_INACTIVE)
    {
      // Umount File System
      f_mount(0,NULL);
      set_disk_status(0,STA_NOINIT | disk_status(0));
      
      #if (SD_FAT_MUTEX_EN == 1)
        OSMutexRelease(SDCardResource);
      #endif      
      
      if (verbose == VERBOSE_ON)
      {
        printf_UART("\n\rIt is safe to remove the SD card!\n\r");
      }
      return SD_OK;
    }
    else
    {
      #if (SD_FAT_MUTEX_EN == 1)
        OSMutexRelease(SDCardResource);
      #endif
      
      if (verbose == VERBOSE_ON)
      {
        printf_UART(SD_API_CARD_BUSY);
      }    
      
      return SD_BUSY;
    }
  }
  else
  {
      if (verbose == VERBOSE_ON)
      {
        printf_UART(SD_API_CARD_NOT_PRESENT);
      }
      return SD_FAT_ERROR;
  }    
}


void ListFiles(uint8_t *pname1)
{
	 FRESULT f_res;
	 uint32_t  p1, s1, s2;
	 char   *ptr;
	 FATFS   *fs;				// Pointer to file system object*/
    
	if ((disk_status(0) & STA_NOINIT) != STA_NOINIT)
    {
      if (sd_command == SD_INACTIVE)
      {      
        sd_command = SD_FILE_LISTING;

        // list files
        printf_UART("\n\r");
      	if (*pname1 == 0)
      	{
      	  ptr = ".";
      	}else
      	{
      	  ptr = (char*)pname1;
      	}
      	p1 = s1 = s2 = 0;
				f_res = f_opendir(&Dir, ptr);
				
				if (!f_res) 
				{
  				for(;;)
  				{
            #if _USE_LFN
  					Finfo.lfname = Lfname;
  					Finfo.lfsize = sizeof(Lfname);
            #endif
            
  					f_res = f_readdir(&Dir, &Finfo);
  					if ((f_res != FR_OK) || !Finfo.fname[0]) break;
  					if (Finfo.fattrib & AM_DIR)
  					{
  						s2++;
  					} else
  					{
  						s1++;
  						p1 += Finfo.fsize;
  					}
  					printf_UART("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9u  %s",
  							(Finfo.fattrib & AM_DIR) ? 'D' : '-',
  							(Finfo.fattrib & AM_RDO) ? 'R' : '-',
  							(Finfo.fattrib & AM_HID) ? 'H' : '-',
  							(Finfo.fattrib & AM_SYS) ? 'S' : '-',
  							(Finfo.fattrib & AM_ARC) ? 'A' : '-',
  							(Finfo.fdate >> 9) + 1980, (Finfo.fdate >> 5) & 15, Finfo.fdate & 31,
  							(Finfo.ftime >> 11), (Finfo.ftime >> 5) & 63,                        
  							Finfo.fsize, &(Finfo.fname[0]));
  							
  				  Finfo.fname[0] = 0;
            
            #if _USE_LFN
  					printf_UART("  %s\n\r", Lfname);
            #else
  					printf_UART("\n\r");
            #endif
  				}
				}
				
				printf_UART("%4u File(s), %u bytes total \n\r%4u Dir(s)", s1, p1, s2);
				if (f_getfree(ptr, (DWORD*)&p1, &fs) == FR_OK)
				{
					printf_UART(", %u bytes free \n\r", p1 * fs->csize * 512);
				}
        
        sd_command = SD_INACTIVE;

      }
      else
      {
        printf_UART(SD_API_CARD_BUSY);
      }        
    }
    else
    {
      printf_UART(SD_API_CARD_NOT_PRESENT);
    }
}


                                               




uint8_t ReadFile(char *FileName, uint8_t verbose)
{
	uint32_t  p1, p2, s2;
	uint16_t  cnt = 0;
  
  if ((disk_status(0) & STA_NOINIT) != STA_NOINIT)
  {
    #if (SD_FAT_MUTEX_EN == 1)
      OSMutexAcquire(SDCardResource);
    #endif
    
    if (sd_command == SD_INACTIVE)
    {    
      sd_command = SD_FILE_READING;
      
      if (f_open(&file_obj, (char*)FileName, 	FA_READ) == FR_OK)
      {  
        if (verbose == VERBOSE_ON)
        {          
        	printf_UART("\n\r");
        }
        
		p2 = 0;  			
		SetFatTimer((uint32_t)0);
		p1 = f_size(&file_obj);
        
		while (p1) 
		{
			if (p1 >= sizeof(Buff))	
			{ 
			  cnt = sizeof(Buff);
			  p1 -= sizeof(Buff);
			}
			else 			
			{
			  cnt = (uint16_t)p1;
			  p1 = 0;
			}
			if (f_read(&file_obj, (char*)Buff, cnt, (UINT*)&s2) != FR_OK)
			{
			  break;
			}else
			{
			p2 += s2;
			if (cnt != s2) break;  					                					
								
			Buff[cnt]=0;
	        if (verbose == VERBOSE_ON)
	        {
	        	printf_UART((char*)Buff);
	        }
		 }
		}
				
		GetFatTimer(&s2);
        f_close(&file_obj);
        
        //Sets these variables to inactive states
        sd_command = SD_INACTIVE;        
        
        printf_UART("\n\r%u bytes read with %u bytes/sec.\n\r", p2, s2 ? (p2 * 100 / s2) : 0);
        
        if (verbose == VERBOSE_ON)
        {
        	printf_UART( "\n\r");
        }
        
        #if (SD_FAT_MUTEX_EN == 1)
          OSMutexRelease(SDCardResource);
        #endif        
        
        return SD_FILE_READ;
      } 
      else
      {       
        //If the file was not found, the system is halted
        if (verbose == VERBOSE_ON)
        {
        	printf_UART("\n\r");
        	printf_UART((char*)FileName);
        	printf_UART(" not found.\n\r");
        }
        
        sd_command = SD_INACTIVE;
        
        #if (SD_FAT_MUTEX_EN == 1)
          OSMutexRelease(SDCardResource);
        #endif
        
        return SD_FILE_NOT_FOUND;
      }
    }
    else
    {
      #if (SD_FAT_MUTEX_EN == 1)
        OSMutexRelease(SDCardResource);
      #endif            
      
      if (verbose == VERBOSE_ON)
      {
    	  printf_UART((char*)SD_API_CARD_BUSY);
      }      
      
      return SD_BUSY;      
    }
  }
  else
  {
      if (verbose == VERBOSE_ON)
      {
    	  printf_UART((char*)SD_API_CARD_NOT_PRESENT);
      }
      return SD_FAT_ERROR;
  }
}


uint8_t ChangeDir(char *FileName, uint8_t verbose)
{
  if ((disk_status(0) & STA_NOINIT) != STA_NOINIT)
  {
    #if (SD_FAT_MUTEX_EN == 1)
      OSMutexAcquire(SDCardResource);
    #endif    
    
    if (sd_command == SD_INACTIVE)
    {    
      sd_command = SD_DIR_CHANGING;
      
      if (f_chdir(FileName) == FR_OK)
      {  
        sd_command = SD_INACTIVE;        
        
        #if (SD_FAT_MUTEX_EN == 1)
          OSMutexRelease(SDCardResource);
        #endif        
        
        if (verbose == VERBOSE_ON)
        {
        	printf_UART((char*)"\n\r");
        }
        
        return SD_OPEN_DIR_OK;
      }
      else
      {
        sd_command = SD_INACTIVE;
        
        #if (SD_FAT_MUTEX_EN == 1)
          OSMutexRelease(SDCardResource);
        #endif        
        
        if (verbose == VERBOSE_ON)
        {
        	printf_UART((char*)"\n\rDirectory ");
        	printf_UART((char*)FileName);
        	printf_UART((char*)" does not exist !\n\r");
        }
        return SD_OPEN_DIR_FAILURE;
      }
    }
    else
    {
      #if (SD_FAT_MUTEX_EN == 1)
        OSMutexRelease(SDCardResource);
      #endif      

      if (verbose == VERBOSE_ON)
      {
    	  printf_UART((char*)SD_API_CARD_BUSY);
      }      

      return SD_BUSY;
    }
  }
  else
  {
      if (verbose == VERBOSE_ON)
      {
    	  printf_UART((char*)SD_API_CARD_NOT_PRESENT);
      }
      return SD_FAT_ERROR;
  }
}



uint8_t CreateFile(char *FileName, uint8_t verbose)
{

  if ((disk_status(0) & STA_NOINIT) != STA_NOINIT)
  {
    #if (SD_FAT_MUTEX_EN == 1)
      OSMutexAcquire(SDCardResource);
    #endif       
    
    if (sd_command == SD_INACTIVE)
    {    
      sd_command = SD_FILE_WRITING;
      
      if (f_open(&file_obj, FileName, 	FA_CREATE_NEW) == FR_OK)
      {  
        f_close(&file_obj);
        
        //Sets these variables to inactive states
        sd_command = SD_INACTIVE;        
        
        #if (SD_FAT_MUTEX_EN == 1)
          OSMutexRelease(SDCardResource);
        #endif        
        
        if (verbose == VERBOSE_ON)
        {
          printf_UART((char*)"\n\r");
          printf_UART((char*)FileName);
          printf_UART((char*)" was created successfully.\n\r");
        }          
        
        return SD_CREATE_FILE_OK;
      }
      else
      {
        sd_command = SD_INACTIVE;
        
        #if (SD_FAT_MUTEX_EN == 1)
          OSMutexRelease(SDCardResource);
        #endif        
        
        if (verbose == VERBOSE_ON)
        {
          printf_UART((char*)"\n\r");
          printf_UART((char*)FileName);
          printf_UART((char*)" was not created.\n\r");
        }
        return SD_CREATE_FILE_FAILURE;
      }
    }
    else
    {
      #if (SD_FAT_MUTEX_EN == 1)
        OSMutexRelease(SDCardResource);
      #endif      

      if (verbose == VERBOSE_ON)
      {
    	printf_UART((char*)SD_API_CARD_BUSY);
      }      

      return SD_BUSY;
    }
  }
  else
  {
      if (verbose == VERBOSE_ON)
      {
    	printf_UART((char*)SD_API_CARD_NOT_PRESENT);
      }
      return SD_FAT_ERROR;
  }
}


uint8_t CreateDir(char *FileName, uint8_t verbose)
{
  
  if ((disk_status(0) & STA_NOINIT) != STA_NOINIT)
  {
    #if (SD_FAT_MUTEX_EN == 1)
      OSMutexAcquire(SDCardResource);
    #endif    
        
    if (sd_command == SD_INACTIVE)
    {    
      sd_command = SD_DIR_CREATING;
      
      if (f_mkdir( FileName) == FR_OK)
      {  
        sd_command = SD_INACTIVE;        
        
        #if (SD_FAT_MUTEX_EN == 1)
          OSMutexRelease(SDCardResource);
        #endif        
        
        if (verbose == VERBOSE_ON)
        {
          printf_UART((char*)"\n\rDirectory ");
          printf_UART((char*)FileName);
          printf_UART((char*)" was created successfully.\n\r");
        }          
        
        return SD_CREATE_DIR_OK;
      }
      else
      {
        sd_command = SD_INACTIVE;
        
        #if (SD_FAT_MUTEX_EN == 1)
          OSMutexRelease(SDCardResource);
        #endif        
        
        if (verbose == VERBOSE_ON)
        {
          printf_UART((char*)"\n\rDirectory ");
          printf_UART((char*)FileName);
          printf_UART((char*)" was not created.\n\r");
        }
        return SD_CREATE_DIR_FAILURE;
      }
    }
    else
    {
      #if (SD_FAT_MUTEX_EN == 1)
        OSMutexRelease(SDCardResource);
      #endif      

      if (verbose == VERBOSE_ON)
      {
    	printf_UART((char*)SD_API_CARD_BUSY);
      }

      return SD_BUSY;
    }
  }
  else
  {
      if (verbose == VERBOSE_ON)
      {
    	printf_UART((char*)SD_API_CARD_NOT_PRESENT);
      }
      return SD_FAT_ERROR;
  }
}


uint8_t DeleteFile(char *FileName, uint8_t verbose)
{
  uint8_t sd_status = 0;
  
  if ((disk_status(0) & STA_NOINIT) != STA_NOINIT)
  {  
    #if (SD_FAT_MUTEX_EN == 1)
      OSMutexAcquire(SDCardResource);
    #endif    
    
    if (sd_command == SD_INACTIVE)
    {
      // Indicates that the file will be deleted
      sd_command = SD_FILE_DELETING;
      
      sd_status = f_unlink(FileName);
      
      sd_command = SD_INACTIVE;      
      
      #if (SD_FAT_MUTEX_EN == 1)
        OSMutexRelease(SDCardResource);
      #endif      
      
      if (sd_status == FR_OK)
      {
        if (verbose == VERBOSE_ON)
        {
        	printf_UART((char*)"\n\r");
        	printf_UART((char*)FileName);
        	printf_UART((char*)" deleted! \n\r");
        }
        return SD_FILE_DELETED;
      }
      else
      {
        if (sd_status == FR_DENIED)
        {
          if (verbose == VERBOSE_ON)
          {
        	  printf_UART((char*)"\n\rDelete file or directory denied.\n\r");
        	  printf_UART((char*)"Directory is not empty or file is write-protected.\n\r");
          }
          return SD_DELETE_FILE_DENIED;
        }
        else
        {
          if (verbose == VERBOSE_ON)
          {
        	  printf_UART(SD_API_FILE_NOT_FOUND);
          }
          return SD_FILE_NOT_FOUND;
        }
      }
    }
    else
    {
      #if (SD_FAT_MUTEX_EN == 1)
        OSMutexRelease(SDCardResource);
      #endif      

      if (verbose == VERBOSE_ON)
      {
    	printf_UART((char*)SD_API_CARD_BUSY);
      }

      return SD_BUSY;
    }
  }
  else
  {
      if (verbose == VERBOSE_ON)
      {
    	printf_UART((char*)SD_API_CARD_NOT_PRESENT);
      }
      return SD_FAT_ERROR;
  }
}


uint8_t RenameFile(char *OldFileName,char *NewFileName, uint8_t verbose)
{
  uint8_t sd_status = 0;
  
  if ((disk_status(0) & STA_NOINIT) != STA_NOINIT)
  {  
    #if (SD_FAT_MUTEX_EN == 1)
      OSMutexAcquire(SDCardResource);
    #endif    
    
    if (sd_command == SD_INACTIVE)
    {
      // Indicates that the file will be renamed
      sd_command = SD_FILE_RENAMING;
      
      // Passa para a função os nomes dos arquivos
      sd_status = f_rename(OldFileName, NewFileName);

      sd_command = SD_INACTIVE;
      
      #if (SD_FAT_MUTEX_EN == 1)
        OSMutexRelease(SDCardResource);
      #endif      
      
      if(sd_status == FR_OK)
      {
        if (verbose == VERBOSE_ON)
        {      
          printf_UART((char*)"\n\rFile found and renamed !\n\r");
        }
        return SD_FILE_RENAMED;
      }
      else
      {
        if (verbose == VERBOSE_ON)
        {      
          printf_UART((char*)SD_API_FILE_NOT_FOUND);
        }
        return SD_FILE_NOT_FOUND;
      }
    }
    else
    {
      #if (SD_FAT_MUTEX_EN == 1)
        OSMutexRelease(SDCardResource);
      #endif      

      if (verbose == VERBOSE_ON)
      {
    	printf_UART((char*)SD_API_CARD_BUSY);
      }

      return SD_BUSY;
    }
  }
  else
  {
      if (verbose == VERBOSE_ON)
      {
    	printf_UART((char*)SD_API_CARD_NOT_PRESENT);
      }
      return SD_FAT_ERROR;
  }
}


uint8_t CopyFile(char *SrcFileName,char *DstFileName, uint8_t verbose)
{
  uint32_t  p1, p2, s1, s2;
  char   *NewDstName, *CopyName;
  uint8_t   f_res = 0;
  
  if ((disk_status(0) & STA_NOINIT) != STA_NOINIT)
  {  
    #if (SD_FAT_MUTEX_EN == 1)
      OSMutexAcquire(SDCardResource);
    #endif

    if (sd_command == SD_INACTIVE)
    {
      // Indicates that the file will be renamed
      sd_command = SD_FILE_COPYING;
      
      // Passa para a função os nomes dos arquivos
      if (f_open(&file_obj, SrcFileName, FA_OPEN_EXISTING | FA_READ) != FR_OK)
      {        
        sd_command = SD_INACTIVE;
        
        #if (SD_FAT_MUTEX_EN == 1)
          OSMutexRelease(SDCardResource);
        #endif        
        
        if (verbose == VERBOSE_ON)
        {      
          printf_UART((char*)"\n\rSource file does not exist !\n\r");
        }
        return SD_COPY_FILE_FAILURE;      
      }
      
      if (f_open(&file_obj2, DstFileName, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
      {
        // Complementa nome do arquivo
        CopyName = SrcFileName;
        NewDstName = DstFileName;
        
        while(*NewDstName)
        {
          NewDstName++;
        }
        
        NewDstName--;
        if (*NewDstName != '/')
        {
          NewDstName++;
          *NewDstName = '/';
          NewDstName++;
        }else
        {
          NewDstName++;
        }
        
        while(*CopyName)
        {
          *NewDstName = *CopyName;
          CopyName++;
          NewDstName++;
        }               
        
        if (f_open(&file_obj2, DstFileName, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
        {                
          sd_command = SD_INACTIVE;
          
          #if (SD_FAT_MUTEX_EN == 1)
            OSMutexRelease(SDCardResource);
          #endif          
          
          if (verbose == VERBOSE_ON)
          {                
        	printf_UART((char*)"\n\rDestination file could not be created or already exist !\n\r");
          }
            
          f_close(&file_obj);
          return SD_COPY_FILE_FAILURE;            
        }
      }

      if (verbose == VERBOSE_ON)
      {                			
        printf_UART((char*)"\n\rCopying file ...\n\r");
      }
      
	  SetFatTimer((uint32_t)0);
	  p1 = 0;
		
	  for (;;) 
	  {
		f_res = f_read(&file_obj, Buff, sizeof(Buff), (UINT*)&s1);
		if (f_res || s1 == 0) break;   /* error or eof */
		f_res = f_write(&file_obj2, Buff, s1, (UINT*)&s2);
		p1 += s2;
		if (f_res || s2 < s1) break;   /* error or disk full */											
	  }			
		
	  f_close(&file_obj);
	  f_close(&file_obj2);
      
      if (verbose == VERBOSE_ON)
      {      
        GetFatTimer(&p2);                                   
        printf_UART("\n\r%u bytes copied with %u bytes/sec.\n\r", p1, p2 ? (p1 * 100 / p2) : 0);
      }
      
      sd_command = SD_INACTIVE;
      
      #if (SD_FAT_MUTEX_EN == 1)
        OSMutexRelease(SDCardResource);
      #endif      
      
      return SD_FILE_COPIED;
    }
    else
    {
      #if (SD_FAT_MUTEX_EN == 1)
        OSMutexRelease(SDCardResource);
      #endif      

      if (verbose == VERBOSE_ON)
      {
    	printf_UART((char*)SD_API_CARD_BUSY);
      }

      return SD_BUSY;
    }
  }
  else
  {
      if (verbose == VERBOSE_ON)
      {
    	printf_UART((char*)SD_API_CARD_NOT_PRESENT);
      }
      return SD_FAT_ERROR;
  }
}


uint8_t UpdateFile(char *SrcFileName, char *StringToUpdate, char *UpdatedString, int string_size, uint8_t verbose)
{
  uint32_t  p2 = 0;
  uint8_t sd_status = 0;
  char line[128];

  if ((disk_status(0) & STA_NOINIT) != STA_NOINIT)
  {
    #if (SD_FAT_MUTEX_EN == 1)
      OSMutexAcquire(SDCardResource);
    #endif

    if (sd_command == SD_INACTIVE)
    {
      // Indicates that the file will be renamed
      sd_command = SD_FILE_COPYING;

      // Passa para a função os nomes dos arquivos
      if (f_open(&file_obj, SrcFileName, FA_OPEN_EXISTING | FA_READ) != FR_OK)
      {
        sd_command = SD_INACTIVE;

        #if (SD_FAT_MUTEX_EN == 1)
          OSMutexRelease(SDCardResource);
        #endif

        if (verbose == VERBOSE_ON)
        {
          printf_UART((char*)"\n\rSource file does not exist !\n\r");
        }
        return SD_COPY_FILE_FAILURE;
      }

      if (f_open(&file_obj2, "temporary.txt", FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
      {
		  sd_command = SD_INACTIVE;

		  #if (SD_FAT_MUTEX_EN == 1)
			OSMutexRelease(SDCardResource);
		  #endif

		  if (verbose == VERBOSE_ON)
		  {
			printf_UART((char*)"\n\rTemporary file could not be created or already exist !\n\r");
		  }

		  f_close(&file_obj);
		  return SD_COPY_FILE_FAILURE;
      }

      if (verbose == VERBOSE_ON)
      {
        printf_UART((char*)"\n\rUpdating file ...\n\r");
      }

	  SetFatTimer((uint32_t)0);

	  for (;;)
	  {
		  f_gets(line, 128, &file_obj);
		  if(f_eof(&file_obj)) break;

		  // Se for utilizado comparação com tamanho limitado da string
		  if (string_size){
			  if (strncmp(line,StringToUpdate,string_size) == 0){
				  f_puts(UpdatedString, &file_obj2);
			  }else{
				  f_puts(line, &file_obj2);
			  }
		  }else{
			  if (strcmp(line,StringToUpdate) == 0){
				  f_puts(UpdatedString, &file_obj2);
			  }else{
				  f_puts(line, &file_obj2);
			  }
		  }
	  }

	  f_close(&file_obj);
	  f_close(&file_obj2);

      if (verbose == VERBOSE_ON)
      {
        GetFatTimer(&p2);
        //printf_UART("\n\r%u bytes copied with %u bytes/sec.\n\r", p1, p2 ? (p1 * 100 / p2) : 0);
        printf_UART("\n\rFile updated in %u seconds.\n\r", p2);
      }

      // Lembrar de verificar o status dessas funções !!!!!!!!!!!!!!

      // Apaga o arquivo original
      sd_status = f_unlink(SrcFileName);

      // Renomeia o arquivo temporario para o nome do arquivo original
      if (sd_status == FR_OK){
    	  sd_status = f_rename("temporary.txt", SrcFileName);
      }

      sd_command = SD_INACTIVE;

      #if (SD_FAT_MUTEX_EN == 1)
        OSMutexRelease(SDCardResource);
      #endif

      return SD_FILE_COPIED;
    }
    else
    {
      #if (SD_FAT_MUTEX_EN == 1)
        OSMutexRelease(SDCardResource);
      #endif

      if (verbose == VERBOSE_ON)
      {
    	printf_UART((char*)SD_API_CARD_BUSY);
      }

      return SD_BUSY;
    }
  }
  else
  {
      if (verbose == VERBOSE_ON)
      {
    	printf_UART((char*)SD_API_CARD_NOT_PRESENT);
      }
      return SD_FAT_ERROR;
  }
}


uint8_t WriteFile(char *SrcFileName, char *string, uint8_t verbose)
{
  
  if ((disk_status(0) & STA_NOINIT) != STA_NOINIT)
  {
    #if (SD_FAT_MUTEX_EN == 1)
      OSMutexAcquire(SDCardResource);
    #endif

    if (sd_command == SD_INACTIVE)
    {    
        sd_command = SD_FILE_WRITING;
        
        if (f_open(&file_obj, SrcFileName, 	FA_WRITE) == FR_NO_FILE)
        {     
          f_open(&file_obj, SrcFileName, 	FA_CREATE_NEW);
          f_open(&file_obj, SrcFileName, 	FA_WRITE);
        }
        f_lseek(&file_obj,f_size(&file_obj));

        //f_printf(&file_obj, "User: %s , Password: %s \n\r", user, password);
        f_printf(&file_obj, string);
        f_close(&file_obj);
        
        sd_command = SD_INACTIVE;
        
        #if (SD_FAT_MUTEX_EN == 1)
          OSMutexRelease(SDCardResource);
        #endif        
        
        if (verbose == VERBOSE_ON)
        {
          printf_UART((char*)"\n\rFile written !\n\r");
        }
        
        return SD_FILE_WRITTEN;
    }
    else
    {
      #if (SD_FAT_MUTEX_EN == 1)
        OSMutexRelease(SDCardResource);
      #endif      

      if (verbose == VERBOSE_ON)
      {
    	printf_UART((char*)SD_API_CARD_BUSY);
      }

      return SD_BUSY;
    }
  }
  else
  {
      if (verbose == VERBOSE_ON)
      {
    	printf_UART((char*)SD_API_CARD_NOT_PRESENT);
      }
      return SD_FAT_ERROR;
  }
}


uint8_t file_name_verify(char *pname1,char *pname2, uint8_t *pfile, uint8_t num)
{
   uint8_t i=0;
   uint8_t j=0;
   uint8_t number = num;
   uint32_t estado = NOME;

   while (num) 
   {   
     j=0;
     i=0;
     if(*pfile!=0x20) return API_FILENAME_ERROR; //verifica se foi digitado espaço entre o comando e o nome do arquivo  
     pfile++;         //incrementa endereço do vetor de dados para iniciar a leitura do nome do arquivo que foi digitado          
     
     estado = NOME;   
     while(estado!=FIM) //faz a leitura até que chegue ao final do arquivo
        {
          switch(estado)
            {
              case NOME:   //estado inicial, começa pelo nome do arquivo
                if(*pfile!='.')//faz a leitura do nome até encontar o ponto ou o número máximo de caracteres estourar
                {
                   switch(*pfile)
                   {
                    case 0x5C:
                    //case '/':
                    case '?':
                    case ':':
                    case '*':
                    case '"':
                    case '>':
                    case '<':
                    case '|':
                    return API_FILENAME_ERROR;
                    break;
                    
                    default:
                    break;
                   }
                   
                   if (num == 1)*pname1=*pfile; //faz a leitura letra por letra
                   if (num == 2)*pname2=*pfile; //se existirem dois nomes de arquivo (caso rename) le o segundo arquivo após ler o primeiro 
                   pfile++;
                   if (num == 1) pname1++;
                   if (num == 2) pname2++;
                   i++;                   
                   
                   if(i>=60)//se o nome exceder o número máximo de caracteres
                   {
                      return API_FILENAME_ERROR; 
                   }
                   
                   if ((number > 1) && (num == 2))
                   {
                      if(*pfile==0x20)
                      {
                        estado = FIM;
                      }
                   }
                   else
                   {
                      if(*pfile==0x20)
                      {
                        return API_FILENAME_ERROR; 
                      }                   
                   }
                                      
                   if(*pfile==0)
                   {
                      *pname1=*pfile;
                      *pname2=*pfile;
                      estado = FIM;
                   }                          
                }
                else //quando o ponto for encontrado e o nome estiver correto muda estado para ler a extensão do arquivo
                {
                     if (num == 1)*pname1=*pfile;
                     if (num == 2)*pname2=*pfile;
                     estado = EXTENSAO;
                     pfile++;
                     if (num == 1) pname1++;
                     if (num == 2) pname2++;
                     i++;                   
                }
                
              break;
                
                case EXTENSAO:
                  if((*pfile!=0x20)&&(*pfile!=0))//verifica se não existe espaços ou caracteres incorretos
                  {
                    switch(*pfile)
                    {
                      case 0x5C:
                      //case '/':
                      case '?':
                      case ':':
                      case '*':
                      case '"':
                      case '>':
                      case '<':
                      case '|':
                        return API_FILENAME_ERROR;
                      break;
                    
                      default:
                      break;
                    }                                        
                    
                    if (num == 1)*pname1=*pfile;
                    if (num == 2)*pname2=*pfile;
                    pfile++;
                    if (num == 1) pname1++;
                    if (num == 2) pname2++;
                    j++; 
                    if(j>=4) //se a extensão for maior do que três caracteres retorna erro
                    {
                      return API_FILENAME_ERROR; 
                    }
                  }
                  else
                  {
                    // Um arquivo pode não ter extensão ou ter extensão menor do que 3 caracteres
                    if(j)
                    {                      
                      if (number == 1)
                      {
                        if(*pfile==0x20)
                        {
                          return API_FILENAME_ERROR; 
                        }
                      }
                      estado = FIM; //vai para estado final
                    }
                    else
                    {
                      return API_FILENAME_ERROR; 
                    }
                     
                  }
                break;
                
                default:
                return API_FILENAME_ERROR;
                break;
            }        
        }
        num--;// decrementa num e le o próximo arquivo, se existir
   }
   
   return API_COMMAND_OK; //retorna leitura de aquivos correta

}


