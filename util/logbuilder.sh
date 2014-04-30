#!/bin/sh

# Internal utility to generate changelogs for modules
#
# Usage: logbuilder <srcdir> <destdir>

if [ $# -ne 2 ];
then
    echo "Usage: $0 <srcdir> <destdir>"
    exit 1
fi

SRCDIR=$1
DESTDIR=$2

echo "Building changeslogs for modules in $SRCDIR to $DESTDIR"

for d in check fix format report transmit webauth
do
  for i in ${SRCDIR}/modules/$d/*
  do
    if [ -L $i/Makefile.in ];
    then
      # We're only interested in directories where the Makefile is a
      # symbolic link, since those directories are where scripted
      # (and therefore those lacking $Log tags) files are located

      m=`basename $i`

      rlog $i/$m | tail +13 > ${DESTDIR}/modules/$d/$m/changelog
    fi
  done
done
