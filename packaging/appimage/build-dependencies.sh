#!/bin/bash

# Halt on errors
set -e

# Be verbose
set -x

# Make sure the base dependencies are installed
apt-get -y install build-essential perl python git '^libxcb.*-dev' libx11-xcb-dev \
	libglu1-mesa-dev libxrender-dev libxi-dev flex bison gperf libicu-dev libxslt-dev ruby
apt-get -y install cmake3 wget tar bzip2 xz-utils libtool libfile-which-perl automake gcc-4.8 patch \
	g++-4.8 zlib1g-dev libglib2.0-dev libc6-dev libeigen3-dev libssl-dev \
	libcppunit-dev libstdc++-4.8-dev libfreetype6-dev libfontconfig1-dev liblcms2-dev \
	mesa-common-dev libaio-dev lzma liblzma-dev\
	libpulse-dev libsox-dev liblist-moreutils-perl libxml-parser-perl \
	libjack-dev autopoint language-pack-en-base

# Read in our parameters
#export BUILD_PREFIX=$1
#export KDENLIVE_SOURCES=$2
export BUILD_PREFIX=/build
export KDENLIVE_SOURCES=/kdenlive
export DEPS_INSTALL_PREFIX=/external

# qjsonparser, used to add metadata to the plugins needs to work in a en_US.UTF-8 environment.
# That's not always the case, so make sure it is
export LC_ALL=en_US.UTF-8
export LANG=en_us.UTF-8

# We want to use $prefix/deps/usr/ for all our dependencies
export DEPS_INSTALL_PREFIX=$BUILD_PREFIX/deps/usr
export DOWNLOADS_DIR=$BUILD_PREFIX/downloads

# Setup variables needed to help everything find what we build
export LD_LIBRARY_PATH=$DEPS_INSTALL_PREFIX/lib:$DEPS_INSTALL_PREFIX/openssl/lib:$LD_LIBRARY_PATH
export PATH=$DEPS_INSTALL_PREFIX/bin:$DEPS_INSTALL_PREFIX/openssl/bin:$PATH
export PKG_CONFIG_PATH=$DEPS_INSTALL_PREFIX/share/pkgconfig:$DEPS_INSTALL_PREFIX/lib/pkgconfig:$DEPS_INSTALL_PREFIX/openssl/lib/pkgconfig:/usr/lib/pkgconfig:$PKG_CONFIG_PATH

# A kdenlive build layout looks like this:
# kdenlive/ -- the source directory
# downloads/ -- downloads of the dependencies from files.kde.org
# deps-build/ -- build directory for the dependencies
# deps/ -- the location for the built dependencies
# build/ -- build directory for kdenlive itself
# kdenlive.appdir/ -- install directory for kdenlive and the dependencies

# Make sure our downloads directory exists
if [ ! -d $DOWNLOADS_DIR ] ; then
    mkdir -p $DOWNLOADS_DIR
fi

# Make sure our build directory exists
if [ ! -d $BUILD_PREFIX/deps-build/ ] ; then
    mkdir -p $BUILD_PREFIX/deps-build/
fi

# The 3rdparty dependency handling in Kdenlive also requires the install directory to be pre-created
if [ ! -d $DEPS_INSTALL_PREFIX ] ; then
    mkdir -p $DEPS_INSTALL_PREFIX
fi

# Switch to our build directory as we're basically ready to start building...
cd $BUILD_PREFIX/deps-build/

# Configure the dependencies for building
cmake $KDENLIVE_SOURCES/packaging/appimage/3rdparty -DCMAKE_INSTALL_PREFIX=$DEPS_INSTALL_PREFIX -DEXT_INSTALL_DIR=$DEPS_INSTALL_PREFIX -DEXT_DOWNLOAD_DIR=$DOWNLOADS_DIR

# Now start building everything we need, in the appropriate order
cmake --build . --target ext_iconv
cmake --build . --target ext_lzma
cmake --build . --target ext_xml
cmake --build . --target ext_gettext
cmake --build . --target ext_xslt
cmake --build . --target ext_png
# cmake --build . --target ext_jpeg #this causes build failures in Qt 5.10
cmake --build . --target ext_qt
cmake --build . --target ext_boost
cmake --build . --target ext_gpgme
cmake --build . --target ext_frameworks
cmake --build . --target ext_libsndfile
cmake --build . --target ext_libsamplerate
cmake --build . --target ext_nasm
cmake --build . --target ext_yasm
cmake --build . --target ext_alsa
cmake --build . --target ext_sdl2

