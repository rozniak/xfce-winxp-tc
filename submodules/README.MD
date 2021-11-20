# Submodules / Forks
This directory contains submodules referenced by this repository. Mostly forks of existing software tweaked to patch things to fix certain problems.

## Note about cloning
If you simply `git clone`'d this repository these directories may be empty. If so, run `git submodule update --init --recursive` to make sure these submodules are also cloned.

## xfce-winxp-tc-panel
This is a fork of `xfce4-panel` which patches a few things such as removing borders and fixing external plugin backgrounds. It is recommended to install this so that the Start button looks correct.

To install this fork:
- `cd` into  `xfce-winxp-tc-panel/packaging` (it should already be on the `test-borders` branch, but you can check with `git status`)
- Build the packages by running these scripts:
  - `./pack-libxfce4panel-2.0.sh`
  - `./pack-xfce4-panel.sh`
- Install the generated packages with:
  - `sudo dpkg -i libxfce4panel-2.0-4.deb`
  - `sudo dpkg -i xfce4-panel.deb`

You must then restart the panel, logging off and back on is sufficient to do this.

(In future I will hopefully tidy up the fork and see if the patches can be merged with upstream)
