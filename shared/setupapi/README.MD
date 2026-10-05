# libwintc-setupapi
This directory contains the source code for the setup functions library.

## Purpose
This library abstracts away the system package manager and provides a simple object for installing packages with a signal for reporting progress.

## Technical Info
This component builds both the library and an executable `wintc-setupapi-exec` which is responsible for the actual communication with the system
package manager. The reason for this separation is due to privileges, as `setupapi` will likely be linked to a program that runs in the user's
context. `wintc-setupapi-exec` therefore runs with elevated privileges via `pkexec` when launched via `setupapi`.

It is necessary for the process to be privileged because not all systems provide a package manager that is scriptable. `pacman` is not intended
to be scripted and instead `libalpm` must be used, so the linked program must be privileged.
