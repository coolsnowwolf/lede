#! /bin/bash

yum install -y bind bind-chroot bind-utils
cp -f named.conf /etc/
cp -f test.com.zone /var/named/
chgrp named /etc/named.conf
chgrp named /var/named/test.com.zone
