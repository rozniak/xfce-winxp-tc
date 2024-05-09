# regsvc
This directory contains the source-code for the registry service/daemon.

## Details
Essentially `regsvc` is a very billy-basic daemon that provides the backend for the simplified 'registry' interface defined by `libwintc-registry`.
- The registry is implemented as a daemon for DBus-based IPC, so things like Control Panel applets can configure settings and the recipient programs (like the desktop) can be notified
- It's not a full clone of the Windows Registry, just a very simple form
- The backend uses SQLite3 for storage in the user profile
