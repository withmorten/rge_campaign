#include "rge_campaign.h"

void *malloc_d(size_t size)
{
    void *m = malloc(size);

    if (m == NULL && size != 0)
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

    if (f == NULL)
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

uint32_t fsize(char *filepath)
{
	struct stat st;

	stat(filepath, &st);

	return (uint32_t)st.st_size;
}

void get_file_time_info(file_time_info *ti, char *filepath)
{
	struct stat st;

	stat(filepath, &st);

	ti->atime = st.st_atime;
	ti->mtime = st.st_mtime;
}

void set_file_time_info(file_time_info *ti, char *filepath)
{
	struct utimbuf ut;

	ut.actime = ti->atime;
	ut.modtime = ti->mtime;

	utime(filepath, &ut);
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

char *strncpy_d(char *dest, const char *src, size_t n)
{
	if (strlen(src) + 1 > n)
	{
		printf("Error: String too long: %s\n", src);
		exit(1);
	}

	return strncpy(dest, src, n);
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
