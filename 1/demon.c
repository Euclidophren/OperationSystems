#include <syslog.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/stat.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <signal.h> 
#include <string.h>
#include <errno.h>
#include <time.h>

#define LOCKFILE "var/run/daemon.pid"
#define LOCKMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

int lockfile(int fd)
{
    struct flock fl;
    fl.l_type = F_WRLCK; //тип блокировки 
    fl.l_start = 0; // смещение начала блокируемой области
    fl.l_whence = SEEK_SET; // смещение относительно чего?(в данном случае начало файла) 
    fl.l_len = 0; // размер блокируемой области
    fl.l_pid = getpid(); // id процесса, заблокировавшего область

    return fcntl(fd, F_SETLK, &fl);
}

int already_running()
{
    int fd;
    char buf[16];

    fd = open(LOCKFILE, O_RDWR|O_CREAT, LOCKMODE); // преобразовывает путь к файлу в дескриптор
    if (fd < 0)
    {
        syslog(LOG_ERR, "невозможно открыть %s: %s", LOCKFILE, strerror(errno));
        exit(1);
    }
    if (lockfile(fd) < 0)
    {
        if (errno == EACCES || errno == EAGAIN)  // EACCES - запрет доступа, EAGAIN - ресурс недоступен
        {
            close(fd);
            return 1;
        }
        syslog(LOG_ERR, "невозможно установить блокировку на %s: %s", LOCKFILE, strerror(errno));
        exit(1);
    }

    ftruncate(fd, 0); 
    sprintf(buf, "%ld", (long)getpid());
    write(fd, buf, strlen(buf) + 1);
    return 0;
}

void daemonize(const char *cmd)
{
    int fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl; // описывает ограничения, установленные на ресурс
    struct sigaction sa; 

    // Сброс маски режима создания файла

    umask(0);

    // Получить максимально возможный номер дескриптора файла

    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
        perror("невозможно получить максимально возможный номер дескриптора ");

/* согласно правилу 2, нужно вызвать fork() и завершить родительский процесс. Если демон запущен, как обычная команда оболочки, 
то завершив родительский процесс, мы заставим командную оболочку думать, что команда была выполнена.
Новый процесс после вызова setsid() (создания новой сессии) стал лидером новой сессии, лидером новой группы процессов и лишился управляющего терминала. */

    if ((pid = fork()) < 0)
        perror("ошибка fork ");
    else if (pid)
        exit(0);
    setsid(); 

    // Обеспечить невозможность обретения управляющего терминала

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0)
        perror("невозможно игнорировать сигнал SIGHUP ");

    // Назначение корневого каталога текущим рабочим каталогом

    if (chdir("/") < 0)
        perror("невозможно стать текущим рабочим каталогом ");

    // Закрыть все ненужные файловые дескрипторы

    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024;
    for (int i = 0; i < rl.rlim_max; i++)
        close(i);

    /* Присоединить файловые дескрипторы 0, 1 и 2 к /dev/null
 чтобы функции, читающие/пишущие в стандартный поток ввода/вывода не оказывали влияния на демона*/
  
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);

    // Инициализировать файл журнала

    openlog(cmd, LOG_CONS, LOG_DAEMON);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2)
    {
        syslog(LOG_ERR, "ошибочные файловые дескрипторы %d, %d, %d", fd0, fd1, fd2);
        exit(1);
    }
}

int main()
{
    daemonize("lab1daemon");

    if (already_running())
    {
        syslog(LOG_ERR, "А вот и нет! Я посчитал и больше не хочу!");
        exit(1);
    }

    while (1)
    {        
        syslog(LOG_INFO, "Заработало! 1 + 1 = 2");

        sleep(60);
    }    

    exit(0);
}
