#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#include <fileapi.h>
#include <handleapi.h>

#define systemify_path windowsify_path
#else
#include <sys/stat.h>
#include <utime.h>

#define systemify_path unixify_path
#define stricmp strcasecmp
#define _mkdir(path) mkdir(path, S_IRWXU)
#endif

#define PATH_MAX_WIN 260

enum read_mode
{
	READ_MODE_LIST,
	READ_MODE_EXTRACT
};

#define VERSION_MAJOR 0
#define VERSION_MINOR 1

typedef struct file_time_info file_time_info;

struct file_time_info
{
#ifdef _WIN32
	FILETIME ctime;
	FILETIME atime;
	FILETIME mtime;
#else
	time_t atime;
	time_t mtime;
	time_t ctime;
#endif
};

#define RGE_MAX_CHAR 255
#define RGE_CAMPAIGN_VERSION 0x30302E31

typedef struct RGE_Campaign RGE_Campaign;
typedef struct RGE_Campaign_Header RGE_Campaign_Header;
typedef struct RGE_Scenario_Offset RGE_Scenario_Offset;
typedef void * RGE_Scenario;

struct RGE_Campaign_Header
{
	int32_t version;
	char name[RGE_MAX_CHAR];
	int32_t scenario_num;
};

struct RGE_Scenario_Offset
{
	int32_t size;
	int32_t offset;
	char name[RGE_MAX_CHAR];
	char file_name[RGE_MAX_CHAR];
};

struct RGE_Campaign
{
	RGE_Campaign_Header campaign_header;
	RGE_Scenario_Offset *scenario_offsets;
};

void printf_help_exit(int exit_code);
void RGE_Campaign_read(char *in_filename, char *out_folder, enum read_mode read_mode);
void RGE_Campaign_write(char *campaign_filename, char *campaign_name, int scenario_num, char **scenarios);

void *malloc_d(size_t size);
void *calloc_d(size_t nitems, size_t size);
FILE *fopen_d(char *path, const char *mode);
FILE *fopen_r(char *filepath);
int fsize(char *filepath);
void get_file_time_info(file_time_info *ti, char *filepath);
void set_file_time_info(file_time_info *ti, char *filepath);
char *strndup(const char *str, size_t size);
char *unixify_path(char *path);
char *windowsify_path(char *path);
int mkdir_p(const char *path);
void mkdir_d(char *path);
#ifdef _WIN32
HANDLE CreateFile_d(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
#endif
