#ifndef LOGGER_THREAD_H
#define LOGGER_THREAD_H

#include "tx_api.h"

/*
Use this funciton to initialize the memory needed by the logger thread and start the logger thread
*/
UINT SysLogger_Init(void);

/*
Send a log to the logger as you do with printf
*/
void System_Log(const char *format, ...);

#endif