#include <stdlib.h>
#include "platform.h"
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
	for (i=rsize; i<newSize; i++) pRet[i] = nullptr;
	rsize = newSize;
}

#ifdef VISUAL_STUDIO

char** getDirectoryFileNames(const char* spath)
{
	static char** pRet = NULL;
	static unsigned int rsize = 0;
	unsigned int index = 0;

	if (!pRet) {
		enlargeStringArray(pRet, 64, rsize);
	}

	WIN32_FIND_DATA fdata;
	HANDLE hfind = FindFirstFile(std::string(spath).append("\\*").c_str(), &fdata);
	if (hfind == INVALID_HANDLE_VALUE) {
		pRet[0] = NULL;
		return pRet;
	}

	do {
		std::string fname(fdata.cFileName);

		if (fname != "." && fname != ".." && (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY == 0) &&
				((fdata.dwFileAttributes==FILE_ATTRIBUTE_NORMAL) || (fdata.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) || (fdata.dwFileAttributes & FILE_ATTRIBUTE_READONLY))) {
			if (index + 1 > rsize) {
				enlargeStringArray(pRet, rsize + 64, rsize);
			}

			if (pRet[index]) free(pRet[index]);

			pRet[index] = strdup(fname.c_str());

			index++;
		}
	} while (FindNextFile(hfind, &fdata) != 0);

	return pRet;
}
#else

char** getDirectoryFileNames(const char* spath)
{
	static char** pRet = nullptr;
	static unsigned int rsize = 0;

	if (!pRet) {
		enlargeStringArray(pRet, 64, rsize);
	}

	DIR* pdir = opendir(spath);
	if (!pdir) {
		pRet[0] = nullptr;
		return pRet;
	}

	struct dirent *ent;
	unsigned int index = 0;

	while ((ent = readdir(pdir)) != nullptr) {
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
