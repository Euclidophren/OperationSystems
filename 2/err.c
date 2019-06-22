#include "err.h"

static void err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
    char buf[MAXLINE];
    
    vsnprintf(buf, MAXLINE - 1, fmt, ap);
    
    if (errnoflag)
    {
        snprintf(buf + strlen(buf), MAXLINE - strlen(buf) - 1, ": %s",
                 strerror(error)); // strerror получает код ошибки и возвращает соответствующее сообщение
    }
    
    strcat(buf, "\n");
    fflush(stdout); /* в случае, когда stdout и stderr - одно и то же устройство */
    fputs(buf, stderr);
    fflush(NULL); /* сбрасывает все выходные потоки */
}

/*
 * Фатальные ошибки, связанные с системными вызовами,
 * выводит сообщение, создает файл core и завершает работу процесса.
 */
void err_dump(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    err_doit(1, errno, fmt, ap);
    va_end(ap);
    abort(); /* записать дамп памяти в файл и завершить процесс */
    exit(1); /* этот вызов никогда не должен выполниться */
}

/*
 * Фатальные ошибки, не связанные с системными вызовами,
 * выводит сообщение и завершает работу процесса.
 */
void err_quit(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    err_doit(0, 0, fmt, ap);
    va_end(ap);
    exit(1);
}

/* 
 * Фатальные ошибки, связанные с системными вызовами, 
 * выводит сообщение и завершает процесс.
*/
void err_sys(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    err_doit(1, errno, fmt, ap);
    exit(1);
}

/* 
 * Нефатальные ошибки, связанные с системными вызовами, 
 * выводит сообщение возвращает управление.
*/
void err_ret(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    err_doit(1, errno, fmt, ap);
    va_end(ap);
}

#ifdef PATH_MAX
static long pathmax = PATH_MAX;
#else
static long pathmax = 0;
#endif

static long posix_version = 0;
static long xsi_version = 0;

#define PATH_MAX_GUESS 1024

char* path_alloc(size_t *sizep)
{
    char *ptr;
    size_t size;
    
    if (posix_version == 0)
    {
        posix_version = sysconf(_SC_VERSION);
    }
    if (xsi_version == 0)
    {
        xsi_version = sysconf(_SC_XOPEN_VERSION);
    }
    
    if (pathmax == 0)
    {
        errno = 0;
        if ((pathmax = pathconf("/", _PC_PATH_MAX)) < 0) 
        {
            if (errno == 0)
            {
                pathmax = PATH_MAX_GUESS;
            }
            else
            {
                err_sys("ошибка вызова pathconf с параметром _PC_PATH_MAX");
            }
        } 
        else
        {
            pathmax++; /* добавить 1, так как путь относительно корня */
        }
    }

    if ((posix_version < 200112L) && (xsi_version < 4))
    {
        size = pathmax + 1;
    }
    else
    {
        size = pathmax;
    }
    if ((ptr = malloc(size)) == NULL)
    {
        err_sys("malloc error for pathname");
    }
    if (sizep != NULL)
    {
        *sizep = size;
    }
    
    return(ptr);
} 
