#include <stdlib.h>
#include "utils/ErrorReporting.h"
#ifdef VISUAL_STUDIO
#else
#include <dirent.h>
#include <string.h>
#endif

#ifdef WIN64
int setenv(const char *name, const char *value, int overwrite)
{
    int errcode = 0;
    if(!overwrite) {
        char* current = getenv(name);
        if (!current || current[0]==0) return -1;
    }
    return _putenv_s(name, value);
}
#endif

static void enlargeStringArray(char**& pRet, unsigned int newSize, unsigned int& rsize)
{
	pRet = (char**)realloc(pRet, sizeof(char*)*newSize);
	MYASSERT_NOT_NULL_WITH_DESC(pRet,"Failed to allocate string array");

	unsigned int i;
	for (i=rsize; i<newSize; i++) pRet[i] = NULL;
	rsize = newSize;
}

#ifdef VISUAL_STUDIO

char** getDirectoryFileNames(const char* spath)
{
	static char* nothing = NULL;
	return &nothing;
}
#else

char** getDirectoryFileNames(const char* spath)
{
	static char** pRet = NULL;
	static unsigned int rsize = 0;

	if (!pRet) {
		enlargeStringArray(pRet, 64, rsize);
	}

	DIR* pdir = opendir(spath);
	if (!pdir) {
		pRet[0] = NULL;
		return pRet;
	}

	struct dirent *ent;
	unsigned int index = 0;

	while ((ent = readdir(pdir)) != NULL) {
		if (index+1 > rsize) {
			enlargeStringArray(pRet, rsize+64, rsize);
		}

		if (pRet[index]) free(pRet[index]);

		pRet[index] = strdup(ent->d_name);

		index++;
	}

	closedir(pdir);

	return pRet;
}

#endif
