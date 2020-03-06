#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "makedir.h"

void makeDir(const char *path){
    DWORD errorCode;
    if( !CreateDirectory(path, NULL) &&
       (errorCode = GetLastError()) != ERROR_ALREADY_EXISTS)
    {
        char errMsgBuf[256];
        FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            errorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            errMsgBuf,
            sizeof(errMsgBuf) / sizeof(errMsgBuf[0]),
            NULL
        );

        fprintf(stderr, "Couldn't create directory %s: %s\n", path, errMsgBuf);
        exit(EXIT_FAILURE);
    }
}
