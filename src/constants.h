#define CURRENT_MAJOR_VERSION 1
#define CURRENT_MINOR_VERSION 7
#define CURRENT_PATCH_VERSION 0
#define CURRENT_VERSION_STRING "1.7.0"


#ifdef _WIN32

#ifndef KEY_FOR_JOYSTICK_TYPE
#define KEY_FOR_JOYSTICK_TYPE

#ifdef _WIN32
    #include <windows.h>
#endif


typedef struct KeyForJoystick {
        BYTE      bVk;
        BYTE      bScan;
        DWORD     dwFlags;
    }KeyForJoystick;
#endif
#endif
