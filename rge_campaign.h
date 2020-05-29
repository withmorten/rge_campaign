#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <sys/stat.h>
#include <utime.h>

#ifdef _WIN32
#include <direct.h>

#define systemify_path windowsify_path
#else
#define systemify_path unixify_path
#define stricmp strcasecmp
#define _mkdir(path) mkdir(path, S_IRWXU)
#endif

#define free(ptr) do { free(ptr); ptr = NULL; } while(0)
#define fclose(stream) do { if (stream) { fclose(stream); stream = NULL; } } while(0)

#define PATH_MAX_WIN 260

#define VERSION_MAJOR 0
#define VERSION_MINOR 5

typedef struct file_time_info file_time_info;

struct file_time_info
{
	int64_t atime;
	int64_t mtime;
};

#define RGE_MAX_CHAR 255
#define RGE_DE2_MAX_CHAR 256

#define RGE_CAMPAIGN_VERSION 0x30302E31 // 1.00
#define RGE_CAMPAIGN_VERSION_DE1 0x30312E31 // 1.10
#define RGE_CAMPAIGN_VERSION_DE2 0x30302E32 // 2.00

#define RGE_STRING_ID 0x0A60

#define RGE_DE2_DEPENDENCY_NUM 6

typedef struct RGE_Campaign RGE_Campaign;
typedef struct RGE_Campaign_Header RGE_Campaign_Header;
typedef struct RGE_Scenario_Offset RGE_Scenario_Offset;
typedef void * RGE_Scenario;

struct RGE_Campaign_Header
{
	int32_t version;
	char *name;
	uint16_t name_len;
	int32_t scenario_num;
};

struct RGE_Scenario_Offset
{
	int32_t size;
	int32_t offset;
	char *name;
	uint16_t name_len;
	char *file_name;
	uint16_t file_name_len;
};

struct RGE_Campaign
{
	RGE_Campaign_Header campaign_header;
	RGE_Scenario_Offset *scenario_offsets;
};

void printf_help_exit(int exit_code);
void RGE_Campaign_read(char *in_filename, char *out_folder);
void RGE_Campaign_write(char *campaign_filename, char *campaign_name, int32_t scenario_num, char **scenarios);

void *malloc_d(size_t size);
void *calloc_d(size_t nitems, size_t size);
FILE *fopen_d(char *path, const char *mode);
FILE *fopen_r(char *filepath);
uint32_t fsize(char *filepath);
void get_file_time_info(file_time_info *ti, char *filepath);
void set_file_time_info(file_time_info *ti, char *filepath);
char *strndup(const char *str, size_t size);
char *unixify_path(char *path);
char *windowsify_path(char *path);
char *strncpy_d(char *dest, const char *src, size_t n);
int mkdir_p(const char *path);
void mkdir_d(char *path);
