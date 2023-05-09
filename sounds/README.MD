# Sounds
This directory contains the source files to generate the *Windows XP Default* Freedesktop.org sound theme.

## Structure
This is just a single sound theme, the structure of this tree is as follows:

`res/` - contains all the sounds in their original `.wav` format (for the core XP sounds, these are the higher bitrate versions from build 2481)

`index.theme` - the freedesktop theme config file

`mappings` - since there are lots of sound names that map to the same sound, this file describes those mappings so that each sound name can be symlinked to the real sound files provided in `sfx/`

## Developing the sound theme
It should be obvious with the above info on how to add additional sounds or make tweaks as appropriate by dropping the sound files in `sfx/` and then providing the mappings.

## Caveats
Please note that sound theme support varies by application. Sadly XFCE does not really make any use of sounds as far as I can tell, so it's quite silent. I may see if anything can be done about this in future. Things like Firefox or GNOME-developed applications most likely should support the system sound theme.
