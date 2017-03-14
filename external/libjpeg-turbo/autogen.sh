#! /bin/sh

libtoolize --copy --force
aclocal
autoheader
autoconf
automake -a -c
