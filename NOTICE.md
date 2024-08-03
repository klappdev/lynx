**NOTICE**

**Network setup**
## Show list opened ports
```bash
$ lsof -i -P -n | grep LISTEN
$ netstat -an | grep "LISTEN "
```

**Database setup**
## Disable mysql from autostart
```bash
$ systemctl disable mysql
```
## Disable mysql from autostart
```bash
$ service mysql stop
$ service mysql start
```

## Start mysql client:
```bash
$ mysql -u user -p
```

## Create database:
```bash
> DROP DATABASE IF EXISTS dictionary;
> CREATE DATABASE dictionary;
```

## Show databases:
```bash
> SHOW DATABASES;
> USE dictionary;
```

## Show tables:
```bash
> SHOW TABLES;
```

## Show info tables:
```bash
> DESCRIBE word;
> DESCRIBE word_image;
```

## Show users and tables:
```bash
> SELECT user FROM mysql.user;
> SELECT * FROM word;
> SELECT * FROM word_image;
```

**Debugging**
## Enable core dump
```bash
$ ulimit -c unlimited
$ ulimit -c
```

## Start crash reporter
```bash
$ sudo service apport start
$ sudo service apport status
```

## Run debugger with core dump
```bash
$ gdb cmake-build-debug/lynx/lynx_test /var/lib/apport/coredump/<coredump_file>
  bt full
```

## Run debugger with app
```bash
$ gdb cmake-build-debug/lynx/lynx_test
  run   # OR r
  bt full
  quit  # OR q
```





