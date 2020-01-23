# source_guard

## ready
### lib
dnf install -y wget make bzip2 gcc openssl openssl-* php php-*

### get php code
mkdir /usr/local/src/php-dev

cd /usr/local/src/php-dev

wget -O php-7.2.11.tar.bz2 http://jp2.php.net/get/php-7.2.11.tar.bz2/from/this/mirror

tar xjf php-7.2.11.tar.bz2

cd /php-7.2.11/ext

### make skelton
./ext_skel --extname=source_guard

cd /source_guard

phpize

./configure --enable-source_guard

### exchange code
source_guard.c
php_source_guard.h

### set ini
/etc/php.d/ -> 80-source_guard.ini

## compile
make clean

make install

systemctl restart httpd

## check
php -d extension=modules/source_guard.so source_guard.php

php -m
