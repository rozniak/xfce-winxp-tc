# Session Manager
This directory contains the source-code for the session manager.

## State / Aim
The session manager is extremely early days and basically just boots up the shell and provides an implementation for the logout/shutdown stuff in the `org.xfce.SessionManager` DBus endpoint.

The aim is to eventually have the session manager behave akin to Windows - have everything set up on first run (themes etc.), play logon/logoff sounds, control the shell, that sort of thing.

Running the project under `xfce4-session` will always remain possible, hence implementing the XFSM APIs.

## Testing
By default `logonui` will stick to XFCE - `wintc` is provided as an alternative session. To boot into it from `logonui`, you should edit (or create) `/etc/wintc/logonui.ini` with the following:

```
[LogonUI]
session=wintc
```

Of course the greeter must be restarted for this change to take effect.
