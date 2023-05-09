# Common Build Stuff for Themes
This directory contains various bits used by the build process of themes.

## Structure
This is the core stuff in here, the contents are as follows:

`cmake/` - the common CMake build instructions used by all themes

`gtk-2.0-base/` - the GTK2 *Windows Classic* style that is used by all themes

`gtk-3.0-base/` - the core GTK3 SASS that gets transpiled against, this directory also defines the various mixins/functions as well as initialises the theme parts

`native-resources/` - the base graphics for *Windows Classic*, these get processed by CMake using the `bldtheme` tool to compose the final graphics based on the colour scheme
