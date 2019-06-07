#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/init_task.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Fortune Kernel Module");
#define COOKIE_BUF_SIZE PAGE_SIZE
#define TEMP_BUF_SIZE 256

// прототипы своих собственных системных вызовов read/write согласно спецификациям
static ssize_t fortune_read(struct file *f, char __user *buf, size_t size, loff_t *offset);
static ssize_t fortune_write(struct file *f, const char *buf, size_t size, loff_t *offset);

int fortune_init(void);
void fortune_exit(void);

// структура, определяющая операции над открытими файлами
// хранится непосредственно в структуре proc_dir_entry как поле
struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = fortune_read,
    .write = fortune_write,
};

static char *cookie_buf; // буфер пространства ядра
static struct proc_dir_entry *proc_entry; // структура proc_dir_entry для создаваемого файла fortune
// для которого переопределяются операции read/write
static unsigned read_index; // индекс следующего байта для чтения в буфер cookie_buf
static unsigned write_index; // индекс следующего байта для записи из буфера cookie_buf

char temp[TEMP_BUF_SIZE];

struct task_struct *task = &init_task; // структура task_struct для обхода в цикле кольцевого списка всех структур task_struct
// структура task_struct описывает процесс в системе (т. е. это дескриптор процесса, существует для каждого процесса)
// все структуры task_struct динамические, то есть не хранятся на диске, а создаются при обращении к ним
// кроме init_task - структуры, описывающей процесс, запискающий систему, она хранится на диске
// список начинается со структуры init_task и заканчивается на ней (то есть он циклический)

int len, t_len;

// собственная функция read
static ssize_t fortune_read(struct file *f, char __user *buf, size_t size, loff_t *offset)
{
	// если смещение больше 0, то есть читаем не сначала файла, то выйти из функции
	// видимо это для того, чтобы не возникало таких ситуаций, когда файл считывается не полностью, 
	// так как нам надо вывести все его содержимое (которое мы туда записывали, если, конечно, записывали)
    if (*offset > 0)
    {
        return 0;
    }

	// если позиция чтения не меньше позиции записи, то читаем с начала буфера
    if (read_index >= write_index)
    {
        read_index = 0;
    }

    len = 0;
    t_len = 0;

	// если в файл что-то было записано ранее, то считываем содержимое из буфера пространства ядра начиная с read_index
	// иначе ничего, потому что ничего в буфере нету
    if (write_index > 0)
    {
		// считали из буфера пространства ядра в буфер пространства пользователя
        len = sprintf(buf, "%s\n", &cookie_buf[read_index]);
		// передвинули указатель буфера пользователя на длину считанных данных для 
		// дальнейшей записи информации о процессах
        buf += len;
		// сместили read_index на длину счытанных данных
        read_index += len;
    }

	// пока task не будет равно init_task, то есть пока не пройдем весь список структур процессов и не вернемся в начало
    do
    {
		// вывести для каждого процесса его имя, его ID, имя его родителя и ID родителя
        t_len = sprintf(buf, "* Name =  %s; pid = [%d];\t\tParent name = %s; Parent pid = [%d]\n",
                        task->comm, task->pid, task->parent->comm, task->parent->pid);
		// сдвинуть указатель для следующей записи
        buf += t_len;
		// увеличить длинну записанных данных на длину только что записанной в пространство пользователя строки
        len += t_len;
    } while((task = next_task(task)) != &init_task);

	// вывести для данного процесса его имя и его ID
    t_len = sprintf(buf, "* Current task name = %s; Current task pid = [%d]\n",
                    current->comm, current->pid);
	// сдвинуть указатель для следующей записи (это тут не нужно, мыж ничего больше не пишем, но я забил)
    buf += t_len;
	// увеличить длинну записанных данных на длину только что записанной в пространство пользователя строки
    len += t_len;
	// увеличить значение позиции в файле на длинну считанных данных (как обычно работает чтение из файла - читаем, указатель в файле двигается)
    *offset += len;

	// возврат общей длинны считанных данных в байтах (так нужно согласно спецификациям)
    return len;
}

// собственная функция write
static ssize_t fortune_write(struct file *f, const char __user *buf, size_t size, loff_t *offset)
{
	// считаем свободное место в буфере пространства ядра
    int space_available = (COOKIE_BUF_SIZE - write_index) + 1;

	// если места не хватает, завершаемся с ошибкой
    if (size > space_available)
    {
        printk(KERN_INFO "* buffer is full\n");
        return -ENOSPC;
    }

	// считываем записываемые данных из пространства пользователя в пространство ядра
	// тут же проверяем на ошибки
    if (copy_from_user(&cookie_buf[write_index], buf, size))
    {
        return -EFAULT;
    }

	// сдвигаем индекс для записи на длину записанных данных
    write_index += size;
	// зануляем последний байт записанной строки, так как строки в си нультерминированные
	// чтобы при печати можно было найти конец
    cookie_buf[write_index - 1] = 0;

	// возвращаем число записанных байт
    return size;
}

int fortune_init(void)
{
	// выделяем 4 страницы для буфера пространства ядра
	// с помощью функции пространства ядра vmalloc (по аналогии с malloc в пространстве пользователя)
    cookie_buf = (char *) vmalloc(COOKIE_BUF_SIZE * 4);

	// проверяем, выделилась ли память
    if (!cookie_buf)
    {
        printk(KERN_INFO "* not enough memory for the cookie pot\n");
        return -ENOMEM;
    }

	// инициализируем выделенную память нулями
    memset(cookie_buf, 0, COOKIE_BUF_SIZE);
	// создаем в виртуальной файловой системе /proc файл fortune с определенными ранее операциями read/write,
	// которые указаны в структуре fops
	// с этим файлом и будем далее работать (писать и читать из него)
    proc_entry = proc_create("fortune", 0666, NULL, &fops);

	// если файл не создался, очистим буфер пространства ядра и завершимся с ошибкой
    if (!proc_entry)
    {
        vfree(cookie_buf);
        printk(KERN_INFO "* Couldn't create proc entry\n");
        return -ENOMEM;
    }

	// устанавливаем начальные позиции для чтения и записи в 0
    read_index = 0;
    write_index = 0;

	// создаем директорию в ФС /proc (прост)
    proc_mkdir("my_dir", NULL);
	// создаем символическую ссылку в ФС /proc (прост)
    proc_symlink("my_symbolic_link", NULL, "/proc/fortune");

    printk(KERN_INFO "* fortune module loaded\n");
    printk(KERN_INFO "* current process ID = %i\n", current->pid);
    return 0;
}

void fortune_exit(void)
{
	// удаляем файл fortune, директорию и ссылку
    remove_proc_entry("fortune", NULL);
    remove_proc_entry("my_dir", NULL);
    remove_proc_entry("my_symbolic_link", NULL);

	// очищаем память по буфер пространства ядра, если она была выделена
    if (cookie_buf)
    {
        vfree(cookie_buf);
    }

    printk(KERN_INFO "* fortune module unloaded.\n");
}

module_init(fortune_init);
module_exit(fortune_exit);
