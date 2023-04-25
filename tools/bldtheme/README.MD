# Theme Composition
This directory contains the `bldtheme` tool for composing Windows Classic graphics.

## Tools Available
`bldtheme.py` is used to compose Windows Classic graphics. It is intended as a tool for the theme build process, however you can run it manually if you wish (for debugging?).

It takes three parameters:
`bldtheme.py --scheme=<schemefile> --inputdir=<input directory> --outputdir=<output directory>`

`<schemefile>` is the `scheme` file that defines constants for the Windows Classic colour scheme

`<inputdir>` specifies the path to search for images to compose

`<outputdir>` specifies the path that the composed images will be output

Very simply, this script will:
- Import the constants from `<schemefile>`
- Iterate over `.png` files inside `<inputdir>`
  - Files ending with `.src.png` will be composed, that is, they will be combined with the relevant colour from the colour scheme, using the corresponding `.mask.png` file
  - Files ending with `.static.png` will simply be copied to `<outputdir>` as-is

## Notes on the Future
I don't intend this tool to last forever. It exists now so that we can build themes into the GTK2/GTK3 directory structures as they currently work.

In future I intend to have themes work differently. As the user is able to pick custom colours in Windows XP, this process needs to be able to build a theme post-install:
- Theme files supporting the build should be installed somewhere like `/var/lib/wintc/themes`
- When selecting the theme, we do a build and output into their home directory (ie. places like `~/.themes`) and then select that newly created theme
- This will need to be done via our own `display.cpl` implementation and theme library code
- Standard `/usr/share/themes` will be dropped, themes will have to be selected from `display.cpl` and not something like `xfce4-settings`
