# launch
This directory contains the source-code for launching WinTC sessions.

## Purpose
Due to the fact that multiple sessions are used by the project (smss, and the phases of setup) it made sense to pull out the resources responsible for launching them into a separate package.

This package only carries resources such as the main script for launching into a session (`startwintc`) as well as supporting resources like the systemd unit (`wintc-launch.service`) and xinit script (`xinitrc`).
