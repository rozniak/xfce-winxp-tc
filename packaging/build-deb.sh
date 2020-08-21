#!/bin/bash

# Luna (Blue) Package
#
SOURCE_ROOT='../themes/luna/blue'
TARGET_ROOT='debian'

DEBIAN_ROOT="${TARGET_ROOT}/DEBIAN"
LUNA_BLUE_ROOT="${TARGET_ROOT}/usr/share/themes/Luna"

if [ -d $TARGET_ROOT ]
then
    rm -rf $TARGET_ROOT
fi

mkdir -p $DEBIAN_ROOT
mkdir -p $LUNA_BLUE_ROOT
mkdir -p "${LUNA_BLUE_ROOT}/gtk-3.0"

cp "${SOURCE_ROOT}/debian-control" "${TARGET_ROOT}/DEBIAN/control"

cp -r "${SOURCE_ROOT}/gtk-2.0" "${LUNA_BLUE_ROOT}/gtk-2.0"
cp -r "${SOURCE_ROOT}/Resources" "${LUNA_BLUE_ROOT}/Resources"
cp -r "${SOURCE_ROOT}/xfwm4" "${LUNA_BLUE_ROOT}/xfwm4"

scss "${SOURCE_ROOT}/gtk-3.0/main.scss" "${LUNA_BLUE_ROOT}/gtk-3.0/gtk.css" --sourcemap=none

fakeroot dpkg-deb -v --build $TARGET_ROOT

echo "Package has been built."