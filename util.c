#include "rge_campaign.h"

void *malloc_d(size_t size)
{
    void *m = malloc(size);

    if(m == NULL && size != 0)
    {
        printf("error: couldn't malloc() %u bytes of memory, exiting\n", (unsigned int)size);
        exit(1);
    }

    return m;
}

void *calloc_d(size_t nitems, size_t size)
{
	void *m = calloc(nitems, size);

	if (m == NULL && size != 0)
	{
		printf("error: couldn't calloc() %u bytes of memory, exiting\n", (unsigned int)(nitems * size));
		exit(1);
	}

	return m;
}

FILE *fopen_d(char *path, const char *mode)
{
    FILE *f = fopen(path, mode);

    if(f == NULL)
    {
        printf("error: couldn't fopen() file %s, exiting\n", systemify_path(path));
        exit(1);
    }

    return f;
}

FILE *fopen_r(char *filepath)
{
    FILE *f;
    char *path, *file;

    filepath = unixify_path(filepath);

	file = strrchr(filepath, '/');

	if (file)
	{
		path = strndup(filepath, strlen(filepath) - strlen(file));

		mkdir_d(path);
	}

    f = fopen_d(filepath, "wb");

    return f;
}

int fsize(char *filepath)
{
#ifdef _WIN32
	HANDLE h = CreateFile_d(filepath, FILE_READ_ATTRIBUTES, 0, NULL, OPEN_EXISTING, 0, NULL);
	DWORD fs = GetFileSize(h, NULL);
	CloseHandle(h);

	return fs;
#else
	struct stat st;

	stat(filepath, &st);

	return st.st_size;
#endif
}

void get_file_time_info(file_time_info *ti, char *filepath)
{
#ifdef _WIN32
	HANDLE h = CreateFile_d(filepath, FILE_READ_ATTRIBUTES, 0, NULL, OPEN_EXISTING, 0, NULL);

	GetFileTime(h, &ti->ctime, &ti->atime, &ti->mtime);

	CloseHandle(h);
#else
	struct stat st;

	stat(filepath, &st);

	ti->atime = st.st_atime;
	ti->mtime = st.st_mtime;
	ti->ctime = st.st_ctime;
#endif
}

void set_file_time_info(file_time_info *ti, char *filepath)
{
#ifdef _WIN32
	HANDLE h = CreateFile_d(filepath, FILE_WRITE_ATTRIBUTES, 0, NULL, OPEN_EXISTING, 0, NULL);
	
	SetFileTime(h, &ti->ctime, &ti->atime, &ti->mtime);

	CloseHandle(h);
#else
	struct utimbuf ut;

	ut.actime = ti->atime;
	ut.modtime = ti->mtime;

	utime(filepath, &ut);
#endif
}

char *strndup(const char *str, size_t size)
{
    char *dup = calloc_d(1, size + 1);

    return memcpy(dup, str, size);
}

char *unixify_path(char *path)
{
    char *p;

    for(p = path; *p; p++)
    {
        if(*p == '\\')
        {
            *p = '/';
        }
    }

    if(path[strlen(path) - 1] == '/')
    {
        path[strlen(path) - 1] = '\0';
    }

    return path;
}

char *windowsify_path(char *path)
{
    char *p;

    for(p = path; *p; p++)
    {
        if(*p == '/')
        {
            *p = '\\';
        }
    }

    if(path[strlen(path) - 1] == '\\')
    {
        path[strlen(path) - 1] = '\0';
    }

    return path;
}

int mkdir_p(const char *path)
{
    // adapted from https://gist.github.com/JonathonReinhart/8c0d90191c38af2dcadb102c4e202950
    // adapted from https://stackoverflow.com/a/2336245/119527

    char _path[PATH_MAX_WIN];
    char *p;

    errno = 0;

    if(strlen(path) + 1 > PATH_MAX_WIN)
    {
        errno = ENAMETOOLONG;
        return -1;
    }

    strcpy(_path, unixify_path((char *)path));

    for(p = _path + 1; *p; p++)
    {
        if(*p == '/')
        {
            *p = '\0';

            if(_mkdir(_path) != 0)
            {
                if(errno != EEXIST && (errno == EACCES && !strchr(_path, ':')))
                {
                    return -1;
                }
            }

            *p = '/';
        }
    }

    if(_mkdir(_path) != 0) {
        if(errno != EEXIST)
        {
            return -1;
        }
    }

    return 0;
}

void mkdir_d(char *path)
{
    if(mkdir_p(path))
    {
        printf("error: couldn't create directory %s, exiting\n", systemify_path(path));
        exit(1);
    }
}

#ifdef _WIN32
HANDLE CreateFile_d(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	HANDLE h = CreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

	if (h == INVALID_HANDLE_VALUE)
	{
		printf("error: couldn't open file %s with CreateFile(), exiting\n", windowsify_path((char *)lpFileName));
		exit(1);
	}

	return h;
}
#endif