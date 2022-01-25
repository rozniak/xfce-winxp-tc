# Sound Theme Packaging
This directory contains the sound theme packaging script for Debian and derivatives (e.g. Ubuntu).

## Usage
Simply run the script itself - it requires no arguments as there is only one sound theme in the repo, which will be packaged.

```
./packsnds.sh
```

This will build and output the package for the sound theme, which can then be installed with:

```
sudo dpkg -i wintc-sound-theme-xp.deb
```

## Post-Install
This is *only* the sound theme itself, you will still need to set up your environment to make use of it once it is installed.

Suggestion is to install the `libcanberra` library which provides a simple API for playing sound themes, including via frameworks like GTK.

Package names may vary, but on Debian you should install these packages:
```
gnome-session-canberra
libcanberra-gtk3-0
libcanberra-gtk3-module
libcanberra-pulse
libcanberra0
```

For GTK applications to make use of the library, you must enable the module by specifying it in the `GTK_MODULES` environment variable. On my machine I simply added it to system-wide environment variables by adding the following to `/etc/environment`:
```
GTK_MODULES=canberra-gtk-module
```

And finally, for XFCE, you should enable sounds by going into the *Settings Editor*, and set these values:

`xsettings/Net/EnableEventSounds` ✔️

`xsettings/Net/EnableInputFeedbackSounds` ✔️

`xsettings/Net/SoundThemeName` *"Windows XP Default"*

That should be everything - bear in mind that sound themes are a bit of an afterthought on Linux so application support may vary.


## Any Problems?
If you have problems running the script, feel free to [submit an issue](https://github.com/rozniak/xfce-winxp-tc/issues) and include the generated log file. 😁

