struct dentry {

 atomic_t                 d_count;     /* счетчик использования */

 unsigned long            d_vfs_flags; /* флаги кэша объектов dentry */

 spinlock_t               d_lock; /* блокировка данного объекта dentry */

 struct inode             *d_inode; /* соответствующий файловый индекс */

 struct list_head         d_lru; /* список неиспользованных объектов */

 struct list_head         d_child;     /* список объектов у родительского

                                          экземпляра */

 struct list_head         d_subdirs;   /* подкаталоги */

 struct list_head         d_alias;     /* список альтернативных (alias)

                                          индексов */

 unsigned long            d_time;      /* время проверки правильности */

 struct dentry_operations *d_op;       /* таблица операций с элементом

                                          каталога */

 struct super_block       *d_sb;       /* связанный суперблок */

 unsigned int             d_flags;     /* флаги элемента каталога */

 int                      d_mounted;   /* является ли объект точкой

                                           монтирования */

 void                     *d_fsdata;   /* специфические данные

                                          файловой системы */

 struct rcu_head          d_rcu; /* блокировки RCU (read-copy update) */

 struct dcookie_struct    *d_cookie;    /* cookie-идентификатор */

 struct dentry            *d_parent;    /* объект dentry

                                           родительского каталога */

 struct qstr              d_name;       /* имя dentry */

 struct hlist_node        d_hash;       /* список хеширования */

 struct hlist_head        *d_bucket;    /* сегмент хеш-таблицы */

 unsigned char d_iname[DNAME_INLINE_LEN_MIN]; /* короткое имя файла */

};
