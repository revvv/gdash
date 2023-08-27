#!/bin/sh
SUBDIRS=$(find . -type d | sed 's!^./!!' | grep -v "\.$")

echo "# Automatically generated Makefile.am! Check $0" >Makefile.am
echo >>Makefile.am

for a in $SUBDIRS; do
    CAVES=$(find $a -maxdepth 1 -type f)
    echo ${a/\//_}dir = '$(pkgdatadir)'/caves/${a/\//_} >> Makefile.am
    echo ${a/\//_}_CAVES = $CAVES >>Makefile.am
    DIST=$DIST\ \$\(${a/\//_}_CAVES\)
    echo ${a/\//_}_DATA = \$\(${a/\//_}_CAVES\) >>Makefile.am
    echo >>Makefile.am
done
echo EXTRA_DIST = $(basename $0) $DIST >>Makefile.am

