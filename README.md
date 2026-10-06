# xfce-winxp-tc
This is my little chipping-away spot for a Windows XP Total Conversion for XFCE.

<img width="800" height="600" alt="Image" src="https://github.com/user-attachments/assets/e794e3e5-26a0-4159-9e1b-3bac0059c146" />
<img width="800" height="600" alt="Image" src="https://github.com/user-attachments/assets/bd64b4d5-b651-4bb2-8277-a5612aa6c991" />
<img width="800" height="600" alt="Image" src="https://github.com/user-attachments/assets/ae5c289b-8b56-44ba-95c6-536ce4d7bb2f" />
<img width="800" height="600" alt="Image" src="https://github.com/user-attachments/assets/7dc1702d-7d2f-4e15-ae75-ef9f5ed2f8e3" />

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
Please see the *Installation* section of the Wiki here: https://github.com/rozniak/xfce-winxp-tc/wiki/Installation 😁

## The theme(s) are buggy!
Themes in GTK3 are not supported by upstream and this project is still under development, so they can potentially look broken in certain programs. If you're using themes from this repository and programs look broken, you should file issues here rather than pestering the developers of said program.

I hope to cover theming for standard GTK widgets across the board, but there will always be potential bugs with subclassed widgets and the like - they'll have to be handled on a case-by-case basis.

The theme is now based directly from Adwaita to hopefully maximise compatibility and make it easier to fix theme bugs. A refactored form of Adwaita from the upstream GTK 3 sources exists in this repo to make it easier to base themes from/fix problems.

## Licence
The source code in this repository, essentially any *text* files, such as SASS, C, Bash script source code, are licensed under GPL 2.0.

This licence obviously does not cover the assets from Windows/Office (images, sounds, fonts etc.) - those are still Microsoft's copyright (packaging will mark components using these as `non-free`). They're in this repo because I am lazy. 😛

## Roadmap?
I don't have a fancy looking roadmap document for this repo - there's too much to list really. Essentially, if something was in Windows XP, it's on my mind.

As part of that, user-friendliness is always a target - besides themes and programs, I aim to one day have a nice setup application/process akin to XP's. And perhaps an OOBE if I can figure that out (mostly for the nostalgic music).

If you're interested in the current completion state of the project, see this Wiki page: https://github.com/rozniak/xfce-winxp-tc/wiki/Project-progress-and-status
