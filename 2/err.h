#include <pthread.h>
#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <limits.h> 
#include <unistd.h>

#define MAXLINE 4096

static void err_doit(int errnoflag, int error, const char *fmt, va_list ap);
void err_dump(const char *fmt, ...);
void err_quit(const char *fmt, ...);
void err_sys(const char *fmt, ...);
void err_ret(const char *fmt, ...);
char* path_alloc(size_t *sizep);
