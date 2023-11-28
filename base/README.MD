# Base Components
This directory contains components that form parts of the base system itself.

## Structure
Each directory contains a generally standard CMake project, however due to the nature that each component targets different areas of the system, there is no absolutely standard structure like there is for typical programs.

## What's in here?
Things that are either build related (in the case of `bldtag`), or target key system services (Plymouth boot screen in `bootvid`, or the LightDM greeter in `logonui`).
