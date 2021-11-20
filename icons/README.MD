# Icons
This directory contains the source files used to generate icon themes.

## Under Construction
Icon themes are still under construction - checkout branch `feat-72` if you would like to be *cutting edge* and use the latest work (until that work is done and merged into `master`, at least).

## Structure
Within each directory (eg. `luna`) these are the contents:

`res/` - contains the PNG images for each icon, divided into subfolders based on the icon size

`debian-control` - the `control` file used in Debian packaging to provide metadata about the package

`index.theme` - the actual freedesktop theme config file (that gets deployed to `/usr/share/icons`)

`mappings` - since there are lots of Freedesktop icon names that map to the same icon, the mappings file maps those Freedesktop icon names to the icons found under `/res` (formatted as `(freedesktop icon name)-->(icon filename minus extension)`)

## Installation
If you would like to install the icon theme (bearing in mind it's incomplete!) you should:
- `git checkout feat-72` (to get yourself on the feature branch with the icons)
- `cd` over to the `/packaging/deb/icons` directory
- `./packicon.sh luna` (to build the Luna icon package)
- `sudo dpkg -i <the package>.deb` (to install the package)

Sorry this is very rough, it will be updated once I finish the `feat-72` branch. Which may be some time because there are a LOT of icons to get through...
