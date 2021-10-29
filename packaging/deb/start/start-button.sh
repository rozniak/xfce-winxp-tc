#!/bin/bash

START_DIR="${PWD}/../../../shell/start"
BUILD_DIR="${START_DIR}/build"
WINXP_DIR="/usr/share/winxp"
REQUIRED_PACKAGES=( 'fakeroot' 'cmake' 'libgarcon-gtk3-1-dev' 'libxfce4panel-2.0-4' )

# Check for dependencies

echo "Checking for dependencies..."

for package in "${REQUIRED_PACKAGES[@]}"
do
	dpkg -s $package > /dev/null 2>&1

	if [[ $? -gt 0 ]]
	then
		echo "You are missing package: ${package}"
		exit 1
	fi
done


# Create winxp dir if it doesn't already exist
if [ ! -e $WINXP_DIR ]
then
	mkdir $WINXP_DIR
	mkdir -p "{$WINXP_DIR}/shell-res"
	echo "Directory ${WINXP_DIR} created."
else
	echo "Directory ${WINXP_DIR} exists."
fi

# Copy over necessary files
fakeroot cp -r "${START_DIR}/startbutton.desktop" "/usr/share/xfce4/panel/plugins/startbutton.desktop"
fakeroot cp -r "${START_DIR}/res/." "${WINXP_DIR}/shell-res/"

echo "Files copied."

# Compile start menu using cmake
echo "Compiling project..."

if [ ! -d $BUILD_DIR ]
then
	mkdir $BUILD_DIR
else
	echo "${BUILD_DIR} directory already exists."
fi

cd $BUILD_DIR
cmake ..
make

if [ -e "libstartbutton-plugin.so" ]
then
	fakeroot cp -r "libstartbutton-plugin.so" "/usr/lib/x86_64-linux-gnu/xfce4/panel/plugins/libstartbutton-plugin.so"
else
	echo "libstartbutton-plugin.so not found."
fi

echo "Done."

