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

# Setup and Installation

## Building / Installation
For AUR (Arch User Repository) there is an `xfce-winxp-tc-git` package maintained by [**@SelfRef**](https://github.com/SelfRef) [here](https://aur.archlinux.org/packages/xfce-winxp-tc-git).

Otherwise, please refer to `/packaging/README.MD` for build instructions. 😁

## Custom splash screen setup
Setting up the custom splash screen is distro specific and I recommend reading your distro's official documentations for setting it up. The splash screen provided by xfce-winxp-tc is called bootvid and is the one that you should set as your splash screen once you set up plymouth.

[Plymouth - Debian Wiki](https://wiki.debian.org/plymouth)

[Plymouth - ArchWiki](https://wiki.archlinux.org/title/Plymouth)

## Greeter setup
1. Open your terminal
2. Run the following command as root: `nano /etc/lightdm/lightdm.conf`
3. Add the following lines and save the file:

   ```
   [SeatDefaults]
   greeter-session=wintc-logonui
   ```
4. Run `systemctl restart lightdm` as root

## Visuals
1. Open up "Settings Manager"
2. Go to "Apperance"
3. Enable "Set matching Xfwm4 theme if there is one" and select "Windows XP style (blue)" from the list of themes

   ![appearance-style](https://github.com/user-attachments/assets/6459ccca-1a67-47fc-956d-d1e4cabd73e6)

4. Navigate to the "Icons" tab and select "Luna"

   ![appearance-icons](https://github.com/user-attachments/assets/e9251a63-2626-4f07-b92d-a92dcf84d613)
  
5. Navigate to the "Fonts" tab and set the following settings:

   ![appearance-fonts](https://github.com/user-attachments/assets/f1744d61-4ce7-4fbf-99c5-edc7c8c23097)


6. Go back to all settings and go to "Desktop"
7. In the background tab, click on "Folder", then "Other" and navigate to `/usr/share/backgrounds/wintc`. Here you can set any one of the default wallpapers that shipped with Windows XP.
8. Navigate to the "Icons tab and set the following parameters:

   ![desktop-icons](https://github.com/user-attachments/assets/1e044b4b-b7d2-435f-bdca-77482f4e1f34)

   You should disable trash icon because we will be creating a shortcut for it since there is no other way to rename the trash icon on XFCE.

9. Go back to all settings and go to "Window Manager" and set the "Title Alignment" to "Left"

   ![window-manager](https://github.com/user-attachments/assets/387c8d0e-6ab7-47b9-84fd-8badad3817f3)
    
11. Go back to all settings, go to "Window Manager Tweaks" and navigate to the "Compositor". Here you will have to disable all checkboxes that start with "Show Shadows"

    ![window-manager-tweaks-compositor](https://github.com/user-attachments/assets/4e7e7d6f-59eb-43a6-87cc-65584b1639d3)

12. Go back to all settings, go to "Mouse and Trackpad" and navigate to the "Theme" tab. Here you can either select "Windows XP Standard" or "Windows XP Standard (with pointer shadows)" based on whichever one you prefer

    ![mouse-theme](https://github.com/user-attachments/assets/1178ed5c-b18b-49e2-b879-841d7ff106b9)

13. Close the "Settings Manager", right click on the desktop, click on "Create Launcher" and create a launcher with the following parameters:

    ![recycle](https://github.com/user-attachments/assets/7ea352e4-ef46-4436-adf3-9c0509f9964c)

    The icon can be found at `/usr/share/icons/luna/32x32/places/user-trash.png`


## Panels / wintc-taskband
### This section will be focusing on setting up the Windows XP taskbar on your system

1. Open up settings manager, go to "Panel" and remove all existing panels
2. Create an empty panel and situate it at the bottom of the display
3. Set the following parameters:

   ![panel-display](https://github.com/user-attachments/assets/65f3564c-2240-4502-b4de-e260f0a13293)
   
   ![panel-apperance](https://github.com/user-attachments/assets/d496142e-4070-442c-b6b8-ff4c26b0bbe8)

This empty panel will make sure that maximised windows won't hide under the custom wintc-taskband panel

4. Go back to all settings, click on "Session and Startup", navigate to the Application Autostart tab and add an application with the following parameters:

   ![session](https://github.com/user-attachments/assets/1a015ce0-72ee-4262-88b8-518473e14b7d)

5. Log out, log in and voila! Your linux distro of choice looks exactly like Windows XP



