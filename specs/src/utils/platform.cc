#include <stdlib.h>

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
