# GTK 3 Base Source
This directory contains the SASS source for defining the core theme, and various functions/constants used by the GTK 3 themes.

## Structure
The contents are as follows:

`colors-public/`, `colors/`, `common/`, `drawing/`, `override/` - these are all part of the 'core CSS', essentially what actually builds the theme - it is a modified version of Adwaita that applies the *theme parts* definitions in the correct places to construct the final theme

`init/` - defines the *theme parts* constants

`styling/` - defines defaults for the theme, this produces *Windows Classic* - the other themes then build from this base (aside from `native`, which literally just uses these defaults of course)

`tc-mixins/` - defines mixins/functions used by this project

`main.scss` - the main document that will create the resultant theme

`required.scss` - the prerequisite document that will draw in the *theme parts* constants and function definitions

`scheme.scss.in` - the colour scheme document that will get processed by CMake as part of the build to apply the constant values from `scheme`
