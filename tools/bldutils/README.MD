# Build Utilities
This directory contains tools used by build processes.

## Tools Available
Below describes each tool in this directory.

### bldmaps
`bldmaps.py` is used to create the symbolic links via a `mappings` file.

This script is used mainly for the cursors, icons, and sounds, so that there can be one 'dumping ground' (`res/` directory) and then the actual XDG filenames just point to the resources.

The mapping file is quite simply just a text file in the form:
```
symlink_name-->target_resource
```

### bldtheme
`bldtheme.py` is used to compose the graphics for the *Windows Classic* theme (either the GTK2 theme used by everything, or the actual *Windows Classic* theme for the GTK3 bits).

It scans an input directory looking for files that need composing and files that should just go straight to output:

For files that need composing, there must be a `*.src.png` file for the graphic, and a `*.mask.png` that will be used to apply the colour scheme to it.

For files that don't need composing (just copy to output), name the file `*.static.png`.

### compcurs
`compcurs.py` is used to compile the X11 cursor images from source graphics, and a configuration (which defines the hotspot of the cursor).

It is quite self explanitory, PNGs for graphics, and it will glob for `*.cfg` files to start compiling from.

## If you're stuck
For specific details on usages, you can pass `--help` to each script to get the parameter information. The above should give you enough context to understand how each tool is used, should you ever need them.
