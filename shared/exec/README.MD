# libwintc-exec
This directory contains the source code for the Launcher library.

## Purpose
This library provides code for launching programs and utility methods to help with doing so (such as looking up MIME types and associated programs). Various parts of the shell are able to launch programs, so this library implements functionality that would be expected to support that.

The reason for an entire library instead of just using `g_spawn_async`:

- Support for URI schemes (eg. `http://example.org`)
- Support for UNC paths (eg. `\\MYPC\Stuff$` translates to `smb://MYPC/Stuff$`)
- Support for opening files & folders (detects MIME type for files)

Any other requirements related to launching programs are to be implemented in this library rather than in components themselves.
