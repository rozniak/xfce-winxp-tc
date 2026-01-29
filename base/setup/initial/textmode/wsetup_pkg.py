import configparser
import os
import subprocess

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

def wsetup_pkg_get_pkgpath():
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

    return f"{setup_root}/{pkgfmt}/{pkgfmt_ext}/{pkgfmt_arch}"

def wsetup_pkg_get_pkgnames_basesystem():
    setup_root = os.environ.get("SETUPROOT")

    pkgfmt         = os.environ.get("WSETUP_DIST_PKGFMT")
    pkgfmt_fileext = wsetup_pkg_get_pkgfmt_extension()
    pkg_src_dir    = wsetup_pkg_get_pkgpath()

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

        libs_arr[i] = f"{pkg_src_dir}/libwintc-{libs_arr[i]}{pkgfmt_fileext}"

    for i in range(len(ourpkgs_arr)):
        if ourpkgs_arr[i] == "":
            continue

        ourpkgs_arr[i] = f"{pkg_src_dir}/{ourpkgs_arr[i]}{pkgfmt_fileext}"

    return (libs_arr + ourpkgs_arr + distpkgs_arr)
