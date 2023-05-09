# Themes
This directory contains the source files used to generate desktop themes (aka *visual styles*).

## Structure
Each theme available in Windows XP lives in a subdirectory under here. There are also a couple of special directories used for development and building purposes.

The special directories are:

`adwaita-refactored/` - refactored reference source for the *Adwaita* theme, which is the upstream GTK 3 theme 'the only one' - used as a reference for any strange theme bugs so we know what the original does and can compare where things have gone pear shaped

`build-common/` - contains material used by all of these themes, that is common GTK2/GTK3 source, and CMake build instructions

For everything else, you should be able to figure out what the themes are based on their names. Here are the contents of a 'finished' (😉) theme:

`Resources/` - the graphics that the GTK3 theme uses (technically GTK2 parts get constructed into `Resources/composed/` at build time, but don't worry about that, it's automatic magic, aka voodoo)

`gtk-3.0/` - GTK3 theme SASS

`gtk-3.0/colors/` - manual colour constant overrides

`gtk-3.0/styling/` - *theme part* definitions

`gtk-3.0/main.scss.in` - SASS theme root document, it gets processed by CMake to plop in the location of `build-common/` to import `required.scss` and `main.scss` from

`xfwm4/` - XFWM4 theme, nothing fancy here

`scheme` - colour scheme constants that define the various colours used for *Windows Classic* (aka the GTK2 theme that gets automatically generated)

`theme` - theme name, description etc. constants used by the build process

## Developing a visual style
I don't have a great big guide yet on doing this, so you'll have to poke around for now. The general gist of it is that I have separated the 'core CSS' away from the themes (under `build-common/gtk-3.0-base/`), so that the themes can define things based on 'parts' akin to how Windows XP actually works. Instead of faffing around with CSS, you can just define things on parts (like checkboxes, toolbar buttons etc.) and the build process will take care of massaging all that into the right places in the CSS.

The GTK2 theme is automatically generated as a *Windows Classic* style theme using `scheme` for the colours.
