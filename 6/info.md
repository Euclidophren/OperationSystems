# Лабораторная работа №6
---

Ну давай разберем по буквам всё нами написанное.

- Заполняем поля структуры file\_system\_type. В поле owner ук азываем  модуль, владеющий структурой, в name -- имя файловой системы, в mount -- функцию, вызываемую при монтировании, в kill\_sb -- при размонтировании.


```c
static struct file_system_type myvfs_type = {
	.owner = THIS_MODULE,  
	.name = "myvfs",  
	.mount = myvfs_mount, 
	.kill_sb = kill_block_super,  
};
```
- При загрузке модуля мы регистрируем НАШУ файловую систему. 
В функцию `register_filesystem()` передаем структуру  `file_system_type`. Таким образом НАША файловая система добавляется в список файловых систем, распознаваемых ядром.

- Монтируем файловую систему с помощью  `myvfs_mount`. Внутри нее вызывается `mount_nodev`, которая заполнит поля структуры `super_block` и смонтирует НАШУ файловую систему в файл.

```c
sb->s_blocksize = PAGE_SIZE;        // размер в байтах
sb->s_blocksize_bits = PAGE_SHIFT;  // размер в битах
sb->s_magic = MYVFS_MAGIC_NUMBER;   // магическое число файловой системы, позволит определить, что под этим номером именно файловая система, а не что-то иное
sb->s_op = &myvfs_super_ops;        // операции над суперблоком
```

В поле `s_op`  передается структура `super_operations` с заполненными полями `put_super, statfs, drop_inode`.  В поле `put_super` --  функция удаления суперблока, `statfs` -- получения информации о файле, 
`drop_inode` -- для удаления дисковой копии `inode`.

---
Интермедия: функция `generic_delete_inode`

```
int generic_delete_inode(struct inode *inode)
{
	return 1;
}
```

Конец интермедии

---

- В `fill_sb` вызывается `myvfs_make_inode`, создающая `inode` и заполняющая поля структуры `struct inode`.

- При выгрузке модуля дерегистрируем НАШУ файловую систему с помощью `unregister_filesystem`