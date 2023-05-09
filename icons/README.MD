# Icons
This directory contains the source files used to generate icon themes.

## Structure
Within each directory (eg. `luna`) these are the contents:

`res/` - contains the PNG images for each icon, divided into subfolders based on the icon size

`index.theme` - the actual freedesktop theme config file (that gets deployed to `/usr/share/icons`)

`mappings` - since there are lots of Freedesktop icon names that map to the same icon, the mappings file maps those Freedesktop icon names to the icons found under `/res` (formatted as `(freedesktop icon name)-->(icon filename minus extension)`)

## Developing Icon Themes
All that you need to do to add icons (or change icons is):
- Place your icons in the appropriate directory under `/res`
- Update the `mappings` file as per the format mentioned above
