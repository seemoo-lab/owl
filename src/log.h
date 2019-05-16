/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>

#define LOG_VERSION "0.1.0"

typedef void (*log_LockFn)(void *udata, int lock);

#ifndef LOG_CRIT
#define  LOG_EMERG  0  /* system is unusable */
#define  LOG_ALERT  1  /* action must be taken immediately */
#define  LOG_CRIT  2  /* critical conditions */
#define  LOG_ERR    3  /* error conditions */
#define  LOG_WARNING  4  /* warning conditions */
#define  LOG_NOTICE  5  /* normal but significant condition */
#define  LOG_INFO  6  /* informational */
#define  LOG_DEBUG  7  /* debug-level messages */
#endif
#define  LOG_TRACE  8 /* trace-level messages */

#define log_crit(...) log_log(LOG_CRIT, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERR, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...) log_log(LOG_WARNING, __FILE__, __LINE__, __VA_ARGS__)
#define log_notice(...) log_log(LOG_NOTICE, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...) log_log(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)

void log_set_udata(void *udata);

void log_set_lock(log_LockFn fn);

void log_set_fp(FILE *fp);

void log_set_level(int level);

void log_set_quiet(int enable);

int log_log(int level, const char *file, int line, const char *fmt, ...);

#endif
