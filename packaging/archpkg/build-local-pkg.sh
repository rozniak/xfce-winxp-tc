#!/bin/sh
# This script is used to build the Arch Linux package using local repository.

if ! grep -q 'ID=arch' /etc/os-release; then
	echo "This script is only for Arch Linux only!"
	exit 1
fi

if [ ! -L ./src/xfce-winxp-tc ]; then
	rm -rf ./src
	mkdir ./src
	ln -sf ../../.. ./src/xfce-winxp-tc
fi

makepkg -sef