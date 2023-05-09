# xfce-winxp-tc
This is my little chipping-away spot for a Windows XP Total Conversion for XFCE.

![luna-blue-promo](https://user-images.githubusercontent.com/13258281/234408181-dc8222c1-81ac-4759-b6ac-84a515b0cb13.png)
![professional-promo](https://user-images.githubusercontent.com/13258281/234408192-3b06b634-cff7-4d24-ba0d-d64b949272e8.png)
![classic-promo](https://user-images.githubusercontent.com/13258281/234408198-ac3232c2-d00c-4eaf-8a0d-37d7e239293b.png)

## What?
Essentially this repo is a 'project' to replicate the XP experience on XFCE / Linux in general. This includes everything from desktop themes, icons, cursors, all the way to programs and the shell itself.

**Please be aware of the following:**
- This project is **NOT** for installing on your parents/grandparents/whoever's computer to 'ease them into Linux' or something, I share this project for the interest of Windows/Linux enthusiasts
- There is no attempt to pretend the system is not Linux - consider this as 'Windows XP on the Linux kernel' (ie. you cannot expect there to be a `C:` drive under *My Computer*)
- Everything is massively under construction, and I am one person, so please don't whinge to me about how x/y/z doesn't look 100% like Windows XP, or that I haven't made program a/b/c

## Why?
I used to use Luna theme ports on Windows 7, which has now lost support, and customisability is non-existent/blown away by WU on Windows 10 - switching to Linux seemed like the best choice.

There are themes that aim to replicate the Windows XP visual styles already, however as anyone who has tried this stuff knows, themes are either lacking or only go so far. This project differs in that I aim for as close to pixel-perfect as possible, and write programs to recreate the complete Windows XP environment (themes alone cannot reproduce the Start menu in the screenshots above).

## Building / Installation
Make sure to clone this repository recursively so that `/submodules` is populated! (use `git clone <url> --recurse-submodules`)

This project is componentised, so you can compile/install each thing individually (each cursor theme, desktop theme, program, library, etc.). The components are built using CMake (so for developing you can `make`, or if you're on an unsupported distro and desperate, `make install`).

The typical install process involves packaging for each component, and then installing those packages. At the moment, I have only implemented `.deb` packaging, though I have [issue #35](https://github.com/rozniak/xfce-winxp-tc/issues/35) open to sort out other package managers eventually.

The packaging [lives under `/packaging`](https://github.com/rozniak/xfce-winxp-tc/tree/master/packaging) per format (except for `/submodules`). In general, have a poke around in this repository, there are `README`s in each directory that will explain what things are, and how to install them. 😁

*Please note, if you came from YouTube, the packaging process differs now in that it has been simplified to a single script rather than a script per type of component.*

## The theme(s) are buggy!
This is almost certainly true, especially seeing as [theming GTK isn't really a supported feature](https://stopthemingmy.app/). If you're using themes from this repository and programs look broken, you should file issues here rather than pestering the developers of said program.

I hope to cover theming for standard GTK widgets across the board, but there will always be potential bugs with subclassed widgets and the like - they'll have to be handled on a case-by-case basis.

The theme is now based directly from Adwaita to hopefully maximise compatibility and make it easier to fix theme bugs. A refactored form of Adwaita from the upstream GTK 3 sources exists in this repo to make it easier to base themes from/fix problems.

## Licence
The source code in this repository, essentially any *text* files, such as SASS, C, Bash script source code, are licensed under GPL 2.0.

This licence obviously does not cover the assets from Windows/Office (images, sounds, fonts etc.) - those are still Microsoft's copyright (packaging will mark components using these as `non-free`). They're in this repo because I am lazy. 😛

## Roadmap?
I don't have a fancy looking roadmap document for this repo - there's too much to list really. Essentially, if something was in Windows XP, it's on my mind.

As part of that, user-friendliness is always a target - besides themes and programs, I aim to one day have a nice setup application/process akin to XP's. And perhaps an OOBE if I can figure that out (mostly for the nostalgic music).
