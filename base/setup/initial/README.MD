# Setup (Initial Phase)
This directory contains the source-code for the initial phase of Setup.

## Purpose
The initial phase is responsible for text-mode setup as well as the autorun. If the user launches from a TTY, it will begin text-mode setup, otherwise it should start the autorun.

Text-mode setup will copy packages over to the disk under `/var/tmp/.wintc-setup`, necessary because setup can be executed off of a mounted disc which may not be re-mounted after a reboot. It will install the 'base system', aka the necessary packages that will allow the graphical phase to start.

'Base system' consists of all the WinTC libraries (because it's easier to just include them all), as well as some additional distro bits like `lightdm` and `xorg`.
