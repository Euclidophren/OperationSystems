#include <dirent.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/termios.h>
#include <sys/ioctl.h>
#include "err.h"

typedef int Myfunc(const char *, const struct stat *, int);

static Myfunc myfunc;
static int myftw(char *);
static int dopath();

static long nreg, ndir, nblk, nchr, nfifo, nslink, nsock, ntot;

int main(int argc, char *argv[])
{
    int ret;

    if (argc != 2)
    {
        printf("Использование: ftw <начальный_каталог>");
    }
    ret = myftw(argv[1]);
    ntot = nreg + ndir + nblk + nchr + nfifo + nslink + nsock;
    if (ntot == 0)
    {
        ntot = 1;
    }

    exit(ret);
}

static char *fullpath;
static size_t pathlen;

#define OFFSET 5
static char tree_string[1000] = "\0";
static char* delim = "";
static int n_pos = 0;

static void lvldown()
{
    for (int i = 0; i < OFFSET - 1; i++)
    {
        tree_string[n_pos++] = ' ';
    }
    tree_string[n_pos++] = '|';
    tree_string[n_pos] = '\0';
}

static void lvlup()
{
    n_pos -= OFFSET;
    tree_string[n_pos] = '\0';
}

static int myftw(char *pathname)
{
    fullpath = path_alloc(&pathlen);

    if (pathlen <= strlen(pathname))
    {
        pathlen = strlen(pathname) * 2;
        if ((fullpath = realloc(fullpath, pathlen)) == NULL)
        {
            printf("ошибка вызова realloc");
        }
    }

    strcpy(fullpath, pathname);
    return(dopath());
}

static int dopath()
{
    struct stat statbuf;
    struct dirent *dirp;
    DIR *dp;
    int ret, n;

    if ((ret = lstat(fullpath, &statbuf)) == -1)
    {
        printf("ошибка вызова функции stat для %s: ", fullpath);
        if(errno == EBADF)
            printf("Неверный файловый дескриптор");
        if(errno == ENOENT)
            printf("Компонент полного имени файла не"
                        "существует или полное имя является пустой строкой");
        if(errno == ENOTDIR)
            printf("Компонент пути не является каталогом");
        if(errno == ELOOP)
            printf("Слишком много символьных ссылок в пути");
        if(errno == EFAULT)
            printf("Некорректный адрес");
        if(errno == EACCES)
            printf("Запрещен доступ");
        if(errno == ENOMEM)
                printf("Недостаточно памяти в системе");
        if(errno == ENAMETOOLONG)
            printf("Слишком длинное имя файла");
        return(ret);
    }

    if (S_ISDIR(statbuf.st_mode) == 0)
    {
        return(0);
    }

    printf("%s%s%s\n", tree_string, delim, fullpath);
    delim = "_____";
    n = strlen(fullpath);
    if (n + NAME_MAX + 2 > pathlen)
    {
        pathlen *= 2;

        if ((fullpath = realloc(fullpath, pathlen)) == NULL)
        {
            printf("ошибка вызова realloc");
        }
    }
    fullpath[n++] = '/';
    fullpath[n] = 0;

    if ((dp = opendir(fullpath)) == NULL)
    {
        printf("закрыт доступ к каталогу %s", fullpath);
        return(0);
    }

    chdir(fullpath);

    while ((dirp = readdir(dp)) != NULL)
    {
        if (strcmp(dirp->d_name, ".") != 0 &&
            strcmp(dirp->d_name, "..") != 0)
        {
            strcpy(&fullpath[n], dirp->d_name);
            lvldown();
            if ((ret = dopath()) != 0)
                lvlup();
            lvlup();
        }
    }

    fullpath[n-1] = 0;

    chdir("..");
    
    if (closedir(dp) < 0)
    {
        printf("невозможно закрыть каталог %s", fullpath);
    }

    return(ret);
}
