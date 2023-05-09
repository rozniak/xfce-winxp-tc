# Shared Libraries
This directory contains source code for the shared libraries used by various components.

## Structure
Each library has its own directory in here. CMake is used as the build process, and there are a few input files dotted around that are filled in by variables set in the CMakeLists.txt files.

Inside each directory are the following things:

`public/` - contains the public API header(s)

`src/` - contains the source code for the library

## Developing a library
The needs for library APIs and/or new libraries altogether should be discussed in an issue beforehand. Forward planning to decide what common APIs will be needed and how best to implement them in future has to be thought out.

As for writing a library, it is just a case of working with the codebase as they are written in C and use CMake for building. There isn't much reason to make a start from library code - these libraries are developed when it makes sense based on other features. You should only really need to poke around in here to build, or if directed by an issue.
