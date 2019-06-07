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

#define COOKIE_BUF_SIZE PAGE_SIZE
#define TEMP_BUF_SIZE 256

ssize_t fortune_read(struct file *file, char *buf, size_t count, loff_t *f_pos);
ssize_t fortune_write(struct file *file, const char *buf, size_t count, loff_t *f_pos);

int fortune_init(void);
void fortune_exit(void);

struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = fortune_read,
    .write = fortune_write,
};

static char *cookie_buf;// буфер пространства ядра
static struct proc_dir_entry *proc_entry;
static unsigned read_index;// индекс следующего байта для чтения в буфер cookie_buf
static unsigned write_index;// индекс следующего байта для записи в буфер cookie_buf

char temp[TEMP_BUF_SIZE];

struct task_struct *task = &init_task;

int len, t_len;

ssize_t fortune_read(struct file *file, char *buf, size_t count, loff_t *f_pos)
{
    if (*f_pos > 0)
        return 0;

// если позиция чтения не меньше позиции записи, то читаем с начала буфера
    if (read_index >= write_index)
        read_index = 0;

    len = 0;
    t_len = 0;

    do
    {
        t_len = sprintf(temp, "fort_mod\tname: %20s [%5d]\tparent name: %20s [%5d]\n", task->comm, task->pid, task->parent->comm, task->parent->pid);

        copy_to_user(buf, temp, t_len);
        buf += t_len;
        len += t_len;
    }
    while((task = next_task(task)) != &init_task);

    t_len = sprintf(temp, "fort_mod\tCurrent task is %s [%d]\n", current->comm, current->pid);
    copy_to_user(buf, temp, t_len);
    buf += t_len;
    len += t_len;

    if (write_index > 0)
    {
        t_len = sprintf(temp, "%s\n", &cookie_buf[read_index]);

        copy_to_user(buf, temp, t_len);
        buf += t_len;
        len += t_len;
        read_index += t_len;
    }

    *f_pos += len;

    return len;
}

ssize_t fortune_write(struct file *file, const char *buf, size_t count, loff_t *f_pos)
{
  // считаем свободное место в буфере пространства ядра
    int space_available = (COOKIE_BUF_SIZE - write_index) + 1;
  // если места не хватает, завершаемся с ошибкой
    if (count > space_available)
    {
        printk(KERN_INFO "fort_mod\tCookie pot is full\n");
        return -ENOSPC;
    }
  // считываем записываемые данных из пространства пользователя в пространство ядра
  // тут же проверяем на ошибки
    if (copy_from_user(&cookie_buf[write_index], buf, count))
        return -EFAULT;
  // сдвигаем индекс для записи на длину записанных данных
    write_index += count;
  // зануляем последний байт записанной строки, так как строки в си нультерминированные
  // чтобы при печати можно было найти конец
    cookie_buf[write_index - 1] = 0;

    return count;
}

int fortune_init(void)
{
    cookie_buf = (char *) vmalloc(COOKIE_BUF_SIZE * 4);

    if (!cookie_buf)
    {
        printk(KERN_INFO "fort_mod\tNot enough memory for the cookie pot\n");

        return -ENOMEM;
    }

    memset(cookie_buf, 0, COOKIE_BUF_SIZE * 4);
    proc_entry = proc_create("fortune", 0666, NULL, &fops);

    if (!proc_entry)
    {
        vfree(cookie_buf);
        printk(KERN_INFO "fort_mod\tCouldn't create proc entry\n");

        return -ENOMEM;
    }

    read_index = 0;
    write_index = 0;

    proc_mkdir("my_dir_in_proc", NULL);
    proc_symlink("my_symbolic_in_proc", NULL, "/proc/fortune");

    printk(KERN_INFO "fort_mod\tfortune module loaded.\n");

    return 0;
}

void fortune_exit(void)
{
    remove_proc_entry("fortune", NULL);
    remove_proc_entry("my_dir", NULL);
    remove_proc_entry("my_symbolic_link", NULL);

    if (cookie_buf)
        vfree(cookie_buf);

    printk(KERN_INFO "fort_mod\tfortune module unloaded.\n");
}

module_init(fortune_init);
module_exit(fortune_exit);
