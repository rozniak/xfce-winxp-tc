# xfce-winxp-tc
This is my little chipping-away spot for a Windows XP Total Conversion for XFCE.

![desktop](https://user-images.githubusercontent.com/13258281/142736910-7393327e-5361-4347-a06e-783d23ab936c.png)


## What?
Essentially this repo is a 'project' to replicate the XP experience on XFCE / Linux in general. This includes everything from desktop themes, icons, cursors, all the way to programs and the shell itself.

## Why?
I used to use Luna theme ports back when Windows 7 was still supported - it's no longer supported and customisability is non-existent on Windows 10. Whilst ports for XP themes do exist for Linux, I don't think they're particularly great. I want to replicate the Windows XP experience, as close to pixel perfect as possible.

As part of my aim this includes writing programs and panel plugins to achieve this goal, such as the Start menu shown in the screenshot above.

## Building / Installation
Make sure to clone this repository recursively so that `/submodules` is populated! (use `git clone <url> --recurse-submodules`)

The build process involves packaging for each component, and then installing those packages. At the moment, I have only implemented `.deb` packaging, though I have [issue #35](https://github.com/rozniak/xfce-winxp-tc/issues/35) open to sort out other package managers eventually.

The packaging is, of course, [found under `/packaging`](https://github.com/rozniak/xfce-winxp-tc/tree/master/packaging) (except for `/submodules`), with each directory explaining how to use the relevant script. In general, have a poke around in this repository, there are `README`s in each directory that will explain what things are, and how to install them. 😁

## The theme(s) are buggy!
This is almost certainly true, especially seeing as [theming GTK isn't really a supported feature](https://stopthemingmy.app/). If you're using themes from this repository and programs look broken, you should file issues here rather than pestering the developers of said program.

I hope to cover theming for standard GTK widgets across the board, but there will always be potential bugs with subclassed widgets and the like - they'll have to be handled on a case-by-case basis.

The theme is now based directly from Adwaita to hopefully maximise compatibility and make it easier to fix theme bugs. A refactored form of Adwaita from the upstream GTK 3 sources exists in this repo to make it easier to base themes from/fix problems.

## Licence
The source code in this repository, essentially any *text* files, such as SASS, C, Bash script source code, are licensed under GPL 2.0.

This licence obviously does not cover the assets from Windows/Office (images, sounds, fonts etc.) - I am assuming you own genuine copies of Windows XP and Office 2007 in your retro software collection.

## Roadmap?
I don't have a fancy looking roadmap document for this repo - there's too much to list really. Essentially, if something was in Windows XP, it's on my mind.

As part of that, user-friendliness is always a target - besides themes and programs, I aim to one day have a nice setup application/process akin to XP's. And perhaps an OOBE if I can figure that out (mostly for the nostalgic music).

## Screenshots?
It's still early days, whilst I do use the in-progress `Luna (Blue)` theme as my daily, it is by no means complete. It's usable, but not 'pretty'.

These screenshots are quite old, but generally reflect the current status of the desktop theme (due to working on things like the Start menu). I will be resuming the theming work shortly, so keep an eye out. 😉

In any case, here are some screenshots of the current state (as of 2021-07-19):

![image](https://user-images.githubusercontent.com/13258281/126234957-171b12a4-36c7-43ad-ba85-ddbfe4758acb.png)
![image](https://user-images.githubusercontent.com/13258281/126234968-c641bd9f-2109-4777-bb79-63a26bd30609.png)
![image](https://user-images.githubusercontent.com/13258281/126234983-1b1c0340-400e-4d98-9ea9-87fccde318cf.png)
![image](https://user-images.githubusercontent.com/13258281/126234993-911cc8fa-623d-4901-80fc-ac581c0abfe6.png)
