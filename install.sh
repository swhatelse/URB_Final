#!/bin/bash

BASE=$PWD
LIB_SRC_PATH=$BASE/lib
BIN_PATH=$BASE/bin
LIB_BIN_PATH=$BIN_PATH/lib
GLIB_SRC=$LIB_SRC_PATH/glib-2.46.2
GLIB_BIN=$LIB_BIN_PATH/glib

# Create directories
mkdir -p $BIN_PATH
mkdir -p $LIB_SRC_PATH
mkdir -p $LIB_BIN_PATH

# Compile & install GLib
echo "======================== Retrieving the library GLib ====================="
wget http://ftp.gnome.org/pub/gnome/sources/glib/2.46/glib-2.46.2.tar.xz
cd $LIB_SRC_PATH
tar Jxvf ../glib-2.46.2.tar.xz
cd $GLIB_SRC
./autogen.sh
./configure --prefix=$GLIB_BIN
make -j5
make install

# Compile URB program 
export PKG_CONFIG_PATH=$BASE/bin/lib/glib/lib/pkgconfig/
cd $BASE
make
