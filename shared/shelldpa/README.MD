# libwintc-shelldpa
This directory contains the source-code for the Shell Display Protocol Abstraction library.

## Purpose
This library abstracts away differences in display protocols for APIs that require interacting with them. For instance, window management varies between X and Wayland - so this library attempts to use WNCK / xfce4windowing where it can.
