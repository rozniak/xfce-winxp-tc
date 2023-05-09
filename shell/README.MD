# Shell Components
This directory contains source code for the shell components.

## Structure
Generally the structure under each subdirectory is that of a typical CMake project. In addition there are the following files:

`<name>.desktop` - for programs this file is optional, if included it will be added to `/usr/share/applications` to appear in the application launcher (which isn't relevant for all programs, like `run`). For panel plugins this is required and will be deployed to `/usr/share/xfce4/panel/plugins` so that the plugin is available to use.

## Developing Shell Components
If you're planning to contribute by developing a program or plugin, then you should consider the following:
- If you plan on writing a program, check that it is a shell component - things considered shell components are those that are typically part of explorer (including explorer itself) that are not really a program in their own right
  - This may sound vague, examples of this is: the run dialog, the autorun dialog, `winver`, explorer file browser, control panel (as part of explorer), etc.
  - If unsure, [check the issues page](https://github.com/rozniak/xfce-winxp-tc/issues?q=is%3Aissue+is%3Aopen+label%3Aadditional-program) for `additional-programs` tag to see if what you want to work on is there, if not, open a new issue and it can be discussed where it belongs
- If you plan on writing a panel plugin, it is always considered a shell component, as the panel is part of the shell
- If the program is not a shell component, it belongs under `/windows` rather than `/shell`
