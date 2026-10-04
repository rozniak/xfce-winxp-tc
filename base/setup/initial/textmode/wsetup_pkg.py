import configparser
import os
import shutil
import subprocess

wsetup_pkgpath = None

def wsetup_pkg_get_local_path(pkg, is_lib):
    pkgfmt         = os.environ.get("WSETUP_DIST_PKGFMT")
    pkgfmt_fileext = wsetup_pkg_get_pkgfmt_extension()
    pkg_src_dir    = wsetup_pkg_get_pkgpath()

    if is_lib:
        return f"{pkg_src_dir}/libwintc-{pkg}{pkgfmt_fileext}"
    else:
        return f"{pkg_src_dir}/wintc-{pkg}{pkgfmt_fileext}"

def wsetup_pkg_get_pkgfmt_extension():
    pkgfmt = os.environ.get("WSETUP_DIST_PKGFMT")

    if pkgfmt == "apk":
        return ".apk"
    elif pkgfmt == "archpkg":
        return ".pkg.tar.zst"
    elif pkgfmt == "bsdpkg":
        return ".pkg"
    elif pkgfmt == "deb":
        return ".deb"
    elif pkgfmt == "rpm":
        return ".rpm"
    elif pkgfmt == "xbps":
        return ".xbps"

    raise Exception(f"Unknown package format {pkgfmt}")

def wsetup_pkg_get_pkgnames_basesystem():
    pkgfmt     = os.environ.get("WSETUP_DIST_PKGFMT")
    setup_root = os.environ.get("SETUPROOT")

    # Read complist.ini to set up the stuff we need to install for phase 2
    #
    distpkgs_key = "DistPackages" + pkgfmt.title()

    complist = configparser.ConfigParser()
    complist.read(f"{setup_root}/setup/complist.ini")

    libs_arr     = complist["BaseSystem"]["Libs"].split(",")
    ourpkgs_arr  = complist["BaseSystem"]["OurPackages"].split(",")
    distpkgs_arr = complist["BaseSystem"][distpkgs_key].split(",")

    for i in range(len(libs_arr)):
        if libs_arr[i] == "":
            continue

        libs_arr[i] = wsetup_pkg_get_local_path(libs_arr[i], True)

    for i in range(len(ourpkgs_arr)):
        if ourpkgs_arr[i] == "":
            continue

        ourpkgs_arr[i] = wsetup_pkg_get_local_path(ourpkgs_arr[i], False)

    return (libs_arr + ourpkgs_arr + distpkgs_arr)

def wsetup_pkg_get_pkgpath():
    global wsetup_pkgpath

    if wsetup_pkgpath:
        return wsetup_pkgpath

    dir_setup_state = os.environ.get("WSETUP_STATE_ROOT")
    file_pkgpath = f"{dir_setup_state}/pkgpath"

    contents = None

    with open(file_pkgpath, "r") as f:
        contents = f.read()

    return contents

def wsetup_pkg_prepare_pkgpath():
    global wsetup_pkgpath

    setup_root = os.environ.get("SETUPROOT")

    # Construct the package source path
    #
    pkgfmt_arch = subprocess.Popen(
            "uname -m",
            shell=True,
            stdout=subprocess.PIPE
        ).stdout.read().decode('utf-8').strip()

    pkgfmt     = os.environ.get("WSETUP_DIST_PKGFMT")
    pkgfmt_ext = os.environ.get("WSETUP_DIST_PKGFMT_EXT", "std")

    dir_src = f"{setup_root}/{pkgfmt}/{pkgfmt_ext}/{pkgfmt_arch}"

    # Copy to the disk and set up pkgpath
    #
    dir_setup_state = os.environ.get("WSETUP_STATE_ROOT")
    dir_dst         = f"{dir_setup_state}/pkg"
    file_pkgpath    = f"{dir_setup_state}/pkgpath"

    shutil.copytree(dir_src, dir_dst)

    with open(file_pkgpath, "w") as f:
        f.write(dir_dst)

    wsetup_pkgpath = dir_dst
