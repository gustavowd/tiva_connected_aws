#include "diskio.h"
#include "ff.h"


// Enable SD/FAT file system mutex
#define SD_FAT_MUTEX_EN      0

#define SUPPORT_BMP			 0


/* Error Codes */
enum
{
	SD_OK,
	COMMAND_FAILS,
	INIT_FAILS,
	WRITE_COMMAND_FAILS,
	WRITE_DATA_FAILS,
	READ_COMMAND_FAILS,
	READ_DATA_FAILS,
	NO_SD_CARD,
	INIT_SD_FAILS,
	MOUNT_SD_FAILS
};

/// SD defines
#define API_COMMAND_FAIL     (uint8_t)0x80
#define API_COMMAND_OK       (uint8_t)0x81
#define API_FILENAME_ERROR   (uint8_t)0x82

#define NO_CAPS              (uint8_t)0x83
#define CAPS_1               (uint8_t)0x84
#define CAPS_2               (uint8_t)0x85
#define CAPS_12              (uint8_t)0x86

#define CONSOLE_BUFFER_SIZE  64
#define WRITE_BUFFER_SIZE    512



uint8_t InitSD(uint8_t verbose);
uint8_t SafeRemoveSD(uint8_t verbose);
void ListFiles(uint8_t *pname1);
uint8_t ReadFile(char *FileName, uint8_t verbose);
uint8_t RenameFile(char *OldFileName,char *NewFileName, uint8_t verbose);
uint8_t CreateFile(char *FileName, uint8_t verbose);
uint8_t CreateDir(char *FileName, uint8_t verbose);
uint8_t DeleteFile(char *FileName, uint8_t verbose);
uint8_t file_name_verify(char *pname1,char *pname2, uint8_t *pfile, uint8_t num);
uint8_t ChangeDir(char *FileName, uint8_t verbose);
uint8_t CopyFile(char *SrcFileName,char *DstFileName, uint8_t verbose);
uint8_t UpdateFile(char *SrcFileName, char *StringToUpdate, char *UpdatedString, int string_size, uint8_t verbose);
uint8_t WriteFile(char *SrcFileName, char *string, uint8_t verbose);
void InitSDCardResource(uint8_t priority);
#if 0
uint8_t WriteUptimeLog(uint8_t verbose);
uint8_t WriteFile(uint8_t *FileName, void (*fill_buffer)(void),uint8_t verbose);
uint8_t FormatDisk(uint8_t verbose);
uint8_t cmd_verify_zeros(uint8_t *pfile, uint8_t num);
#endif


extern FIL      file_obj;

//Mensagens padrão da API do SD
extern const char *SD_API_FILE_NOT_FOUND;
extern const char *SD_API_FILE_INVALID;
extern const char *SD_API_CARD_BUSY;
extern const char *SD_API_CARD_NOT_PRESENT;


/******** COMANDS USED IN THE APPLICATION *************/
enum
{
 FILE_FOUND_DUMMY,
 FILE_NOT_FOUND_DUMMY,
 FILE_CREATE_OK_DUMMY,
 NO_FILE_ENTRY_AVAILABLE_DUMMY,
 NO_FAT_ENTRY_AVAIlABLE_DUMMY,
 ERROR_IDLE_DUMMY,
 FILE_DELETED_DUMMY,
 SD_FAT_OK,
 SD_FAT_ERROR,
 SD_BUSY,
 SD_INACTIVE,
 SD_CARD_FORMATTING,
 SD_CARD_FORMATTED,
 SD_FILE_RENAMING,
 SD_FILE_RENAMED,
 SD_FILE_WRITING,   
 SD_FILE_DELETING, 
 SD_FILE_DELETED, 
 SD_DELETE_FILE_DENIED, 
 SD_FILE_LISTING,
 SD_FILE_READING, 
 SD_FILE_READ,
 SD_DIR_CREATING,
 SD_DIR_CHANGING,
 SD_FILE_COPYING,
 SD_FILE_COPIED,
 SD_COPY_FILE_FAILURE,
 SD_FILE_NOT_FOUND,
 SD_FILE_FOUND,
 SD_CREATE_FILE_FAILURE,
 SD_CREATE_FILE_OK,
 SD_CREATE_DIR_OK,
 SD_CREATE_DIR_FAILURE, 
 SD_OPEN_DIR_OK,
 SD_OPEN_DIR_FAILURE,  
 SD_FILE_WRITE_FAILURE,
 SD_FILE_WRITTEN,
 VERBOSE_ON,
 VERBOSE_OFF
};

enum   //estados existentes na máqiuna
{
    NOME,
    EXTENSAO,
    FIM,
};

