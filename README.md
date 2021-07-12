# xfce-winxp-tc
This is my little chipping-away spot for a Windows XP Total Conversion for XFCE.

## What?
Essentially this repo is a 'project' to replicate the XP experience on XFCE / Linux in general. I'm working on themes at the moment (easier said than done), and potentially programs later down the line.

## Why?
I used to use Luna theme ports back when Windows 7 was still supported - it's no longer supported and customisability is non-existent on Windows 10 and whilst ports for XP themes do exist for Linux, I don't think they're particularly great. I want the theme to be close to pixel-perfect, limitations with GTK and XFCE notwithstanding.

The ports I've seen do a very minimal job leaving most of the UI looking very GNOME-ish, I want a clearly XP-style look. The themes here should be comprehensive.

## Building / Installation
Right now the build process targets Debian (as a `.deb`) - I have an issue open #35 to resolve this in future for other package managers.

For now - Debian users can use these steps to build and install:
```
cd packaging
./build-deb.sh
sudo dpkg -i debian.deb
```

## The theme(s) are buggy!
This is almost certainly true, especially seeing as [theming GTK isn't really a supported feature](https://stopthemingmy.app/). If you're using themes from this repository and programs look broken, you should file issues here rather than pestering the developers of said program.

I hope to cover theming for standard GTK widgets across the board, but there will always be potential bugs with subclassed widgets and the like - they'll have to be handled on a case-by-case basis.

The theme is now based directly from Adwaita to hopefully maximise compatibility and make it easier to fix theme bugs. A refactored form of Adwaita from the upstream GTK 3 sources exists in this repo to make it easier to base themes from/fix problems.

## Roadmap?
I don't have a fancy looking roadmap document for this repo - like I said I am just chipping away here and there. Where I remember to I have been creating issues for things I will or would like to do in future. Most of which is related to theming but it would be nice to have some programs / `xfce-panel` plugins one day as well.

## Screenshots?
It's still early days, whilst I do use the in-progress `Luna (Blue)` theme as my daily, it is by no means complete. It's usable, but not 'pretty'.

In any case, here are some screenshots of the current state (as of 2021-07-12):

![image](https://user-images.githubusercontent.com/13258281/125337252-acb9b680-e346-11eb-9822-685dda43af9c.png)
![image](https://user-images.githubusercontent.com/13258281/125337263-b04d3d80-e346-11eb-933f-1e7adcace129.png)
![image](https://user-images.githubusercontent.com/13258281/125337268-b2af9780-e346-11eb-8690-4b72c417da86.png)
![image](https://user-images.githubusercontent.com/13258281/125337491-f99d8d00-e346-11eb-8e4f-26e0c5783b6c.png)
