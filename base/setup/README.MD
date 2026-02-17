# Setup
This directory contains the components that make up the setup process.

## Structure
Each sub-directory is a phase of setup, as explained:
- `initial` - the very first phase which the user executes, starting from `setup.sh`, which will load straight into text-mode setup or the autorun depending on if started within a TTY or graphical session -- responsible for installing all the WinTC libraries and additional distro packages for booting into graphical mode setup
- `guiphase` - the second phase, graphical mode setup, where the user runs through a couple wizards and all the requested packages are installed
- `oobe` - the final phase, OOBE (Out-of-Box Experience), where the user completes a wizard for setting up users and networking
