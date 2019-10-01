/************************************************************
* Check license.txt in project root for license information *
*********************************************************** */

#ifndef COLORPRINT_H
#define COLORPRINT_H

#include <stdio.h>
#include <stdarg.h>
#include "defs.h"

// used functions
#define LOG_COLOR(COLOR,...) _LOG_COLOR((COLOR),__FILE__, __LINE__,stdout,__VA_ARGS__)
#define LOG_COLOR_ERR(COLOR,...) _LOG_COLOR((COLOR),__FILE__, __LINE__,stderr,__VA_ARGS__)


#define LOG(MSG,...) _LOG(__FILE__, __LINE__,stdout,(MSG),__VA_ARGS__)
#define ASSERT_MESSAGE(condition,...) _ASSERT_MESSAGE(((condition) != 0),__FILE__, __LINE__,__VA_ARGS__)
#define ABORT(...) _ABORT(__FILE__, __LINE__,__VA_ARGS__)


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

#define CONSOLE_COLOR_BLUE  0
#define CONSOLE_COLOR_GREEN 0
#define CONSOLE_COLOR_RED	0


static void* consoleHandle = NULL;
static void init_colored_print() {
	LOG("TODO colored printing");
}



#endif // WIN32


static void _LOG_COLOR (u32 color,const char* file,const u32 row,FILE* stream,char* format,...) {

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
	va_list args;
	va_start (args, format);
	fprintf(stream,LOG_STR);
	vfprintf (stream,format, args);
	fprintf(stream," in file : %s:%d \n", file, row);
	fflush(stream);

	va_end (args);

#if defined(WINDOWS_PLATFORM)
	if(NULL != consoleHandle) {
		SetConsoleTextAttribute(consoleHandle, saved_attributes);
	}
#endif
}

static void _ASSERT_MESSAGE(u8 condition,const char* file,const u32 row,char* format,...) {
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
		fprintf(stderr,"ASSERTION FAILED: ");
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


