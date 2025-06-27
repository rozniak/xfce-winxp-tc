#!/usr/bin/env sh

SETUPROOT=$(cd -P -- "$(dirname -- "$(command -v -- "$0")")" && pwd -P)
export SETUPROOT

#
# This is the bootstrap script for setup, nothing can be assumed about the
# system other than that it has a POSIX shell (to run this script)
#
# The bootstrap process will probe for bare-minimum dependencies, inspect the
# running distro and its init system before chaining onto the rest of setup
#

printf "%s\n" "Setup is inspecting your computer's hardware configuration...";

# Probe for Python as we cannot run binaries at this point
#
python_path=`which python 2>/dev/null`

if [ $? != 0 ]
then
    python_path=`which python3 2>/dev/null`

    if [ $? != 0 ]
    then
        printf "%s%s\n" \
            "Your system is missing the Python 3 interpreter, this is " \
            "required by setup. Setup will now exit.";
        exit 1;
    fi
fi

# Inspect the distro
#
. "$SETUPROOT/dal/detect.sh"
export DIST_ID
export DIST_ID_EXT

# Chain to either text-mode or the setup autorun depending on whether we're
# under a display manager
#
if [ -z "$DISPLAY" ] && [ -z "$WAYLAND_DISPLAY" ]
then
    $python_path textmode/main.py
else
    $python_path autorun/main.py
fi
