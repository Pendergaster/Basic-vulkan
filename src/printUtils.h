/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef COLORPRINT_H
#define COLORPRINT_H

#include <stdio.h>
#include <stdarg.h>
#include "defs.h"

// used functions
#if 0
#define LOG_COLOR(COLOR,...) _LOG_COLOR((COLOR),__FILE__, __LINE__,stdout,__VA_ARGS__)
#define LOG_ERR(COLOR,...) _LOG_COLOR((COLOR),__FILE__, __LINE__,stderr,__VA_ARGS__)


#define LOG(MSG,...) _LOG(__FILE__, __LINE__,stdout,(MSG),__VA_ARGS__)
#define ASSERT_MESSAGE(condition,...) _ASSERT_MESSAGE(((condition) != 0),__FILE__, __LINE__,__VA_ARGS__)
#define ABORT(...) _ABORT(__FILE__, __LINE__,__VA_ARGS__)
#endif

#if !defined(YCMIGNORE)

#define LOG(MSG,...) _LOG(__FILE__, __LINE__,stdout,MSG, ##__VA_ARGS__)
#define ASSERT_MESSAGE(condition,...) _ASSERT_MESSAGE(((condition) != 0),(#condition), __FILE__, __LINE__ VA_ARGS(__VA_ARGS__))
#define ABORT(...) _ABORT(__FILE__, __LINE__ VA_ARGS(__VA_ARGS__))
#define LOG_COLOR(COLOR,...) _LOG_COLOR((COLOR),__FILE__, __LINE__,stdout VA_ARGS(__VA_ARGS__))

#define LOG_ERR(COLOR,...) _LOG_COLOR((COLOR),__FILE__, __LINE__,stderr,__VA_ARGS__)

#else

#define LOG(...)
#define ASSERT_MESSAGE(...)
#define ABORT(...)
#define LOG_COLOR(...)
#define LOG_ERR(...)

#endif




static void _LOG(const char* file,const u32 row,FILE* stream,char* format,...);
static void _LOG_COLOR (u32 color,const char* file,const u32 row,FILE* stream,char* format,...);

#if defined(WINDOWS_PLATFORM)
#include <windows.h>

static HANDLE consoleHandle = NULL;


#define CONSOLE_COLOR_BLUE FOREGROUND_BLUE
#define CONSOLE_COLOR_GREEN FOREGROUND_GREEN
#define CONSOLE_COLOR_RED FOREGROUND_RED

static void colored_print_init() {
    consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if(INVALID_HANDLE_VALUE == consoleHandle) {
        consoleHandle = NULL;
        LOG("failed to load console handle");
    } else {
        LOG_COLOR(CONSOLE_COLOR_GREEN,"Colored print initialized");
    }
}



#elif defined(LINUX_PLATFORM)

//#define CONSOLE_COLOR_BLUE  34
//#define CONSOLE_COLOR_GREEN 32
//#define CONSOLE_COLOR_RED   31

#define CREATE_ENUM(ENUM, VOID) ENUM,
#define CREATE_COLOR_STRINGS(VOID,VALUE) "\x1b["#VALUE"m",

#define COLOR_TYPES(MODE)\
	MODE(CONSOLE_COLOR_BLUE, 34)\
	MODE(CONSOLE_COLOR_GREEN, 32)\
	MODE(CONSOLE_COLOR_RED, 31)

static const char* COLOR_STRINGS[] = {
    COLOR_TYPES(CREATE_COLOR_STRINGS)
};

enum COLOR_VALUES {
    COLOR_TYPES(CREATE_ENUM)
        max_color
};

static void* consoleHandle = NULL;
static void colored_print_init() {
    LOG_COLOR(CONSOLE_COLOR_GREEN,"colored printing");
    (void)consoleHandle;
}

#define ANSI_COLOR_RESET "\x1b[0m"

#endif // WIN32


static void _LOG_COLOR (u32 color,const char* file,const u32 row,FILE* stream,char* format,...) {
    (void) color;

#if defined(WINDOWS_PLATFORM)
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo = {};
    WORD saved_attributes = 0;
    // only do colored print if handle is initialized
    if(NULL != consoleHandle) {
        GetConsoleScreenBufferInfo(consoleHandle, &consoleInfo);
        saved_attributes = consoleInfo.wAttributes;

        SetConsoleTextAttribute(consoleHandle, color);
    }

#endif

#if defined(LINUX_PLATFORM)
    va_list args;
    va_start (args, format);
    fprintf(stream,LOG_STR"%s",COLOR_STRINGS[color] );
    vfprintf (stream,format, args);
    fprintf(stream,"%s in file : %s:%d \n",ANSI_COLOR_RESET, file, row);
#elif defined(WINDOWS_PLATFORM)
    va_list args;
    va_start (args, format);
    fprintf(stream,LOG_STR);
    vfprintf (stream,format, args);

    fprintf(stream," in file : %s:%d \n", file, row);
#endif
    fflush(stream);

    va_end (args);

#if defined(WINDOWS_PLATFORM)
    if(NULL != consoleHandle) {
        SetConsoleTextAttribute(consoleHandle, saved_attributes);
    }
#endif
}

static void _ASSERT_MESSAGE(u8 condition,const char* condStr,const char* file,const u32 row,char* format,...) {
    //static void _ASSERT_MESSAGE(u8 condition,const char* file,const u32 row,char* format,...) {
    if(!condition) {
#if defined(WINDOWS_PLATFORM)
        // save last state
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo = {};
        WORD saved_attributes = 0;
        // only do colored print if handle is initialized
        if(NULL != consoleHandle) {
            GetConsoleScreenBufferInfo(consoleHandle, &consoleInfo);
            saved_attributes = consoleInfo.wAttributes;

            SetConsoleTextAttribute(consoleHandle, CONSOLE_COLOR_RED);
        }
#endif
        va_list args;
        va_start (args, format);
        fprintf(stderr,"(%s) ASSERTION FAILED: ",condStr);
        vfprintf (stderr,format, args);
        fprintf(stderr," in file : %s:%d \n", file, row);
        fflush(stderr);

        va_end (args);
        // set state back
        _Exit(1);

#if defined(WINDOWS_PLATFORM)
        if(NULL != consoleHandle) {
            SetConsoleTextAttribute(consoleHandle, saved_attributes);
        }
#endif
        _Exit(1);
    }
}

static void _ABORT(const char* file,const u32 row,char* format,...) {
#if defined(WINDOWS_PLATFORM)
    // save last state
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo = {};
    WORD saved_attributes = 0;
    // only do colored print if handle is initialized
    if(NULL != consoleHandle) {
        GetConsoleScreenBufferInfo(consoleHandle, &consoleInfo);
        saved_attributes = consoleInfo.wAttributes;

        SetConsoleTextAttribute(consoleHandle, CONSOLE_COLOR_RED);
    }
#endif

    va_list args;
    va_start (args, format);
    fprintf(stderr,"ERROR OCCURED: ");
    vfprintf (stderr,format, args);
    fprintf(stderr," in file : %s:%d \n", file, row);
    fflush(stderr);

    va_end (args);
    // set state back

#if defined(WINDOWS_PLATFORM)
    if(NULL != consoleHandle) {
        SetConsoleTextAttribute(consoleHandle, saved_attributes);
    }
#endif
    _Exit(1);
}


static void _LOG(const char* file,const u32 row,FILE* stream,char* format,...) {

    va_list args;
    va_start (args, format);
    fprintf(stream,LOG_STR);
    vfprintf (stream,format, args);
    fprintf(stream," in file : %s:%d \n", file, row);
    fflush(stream);

    va_end (args);
}



#endif //COLORPRINT_H


