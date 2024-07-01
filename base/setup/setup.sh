#!/usr/bin/env sh

#
# Setup bootstrapper test...
#

# Initially, we need to probe for stuff to be able to bootstrap into Python
# and export some environment variables about the system for the rest of setup
# to use
#
printf "%s\n" "Setup is inspecting your computer's hardware configuration...";

python_path=`which python`

if [ $? != 0 ]
then
    printf "%s%s\n" \
        "Your system is missing the Python 3 interpreter, this is required " \
        "by setup. Setup will now exit.";
    exit 1;
fi


if [ -z "$DISPLAY" ] && [ -z "$WAYLAND_DISPLAY" ]
then
    $python_path textmode/main.py;
else
    printf "%s\n" "GUI mode setup is not yet implemented. Sorry!"
    exit 1
fi
