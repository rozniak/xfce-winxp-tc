import argparse

from pathlib import Path

def main():
    VALID_DISTROS=["apk", "archpkg", "bsdpkg", "deb", "rpm", "xbps"]

    parser = argparse.ArgumentParser(
        prog="Dependency Mapper Utility",
        description="Maps dependency names to their distro package names.",
        epilog="See ./README.MD for details."
    )

    parser.add_argument("depsfile", help="the 'deps' file with names to map")
    parser.add_argument("distro", help="the distro ID to map to")

    args = parser.parse_args()

    depsfile = Path(args.depsfile)
    distro   = args.distro

    if not distro in VALID_DISTROS:
        raise KeyError(f"{distro} not a known distro")

    # Read mappings
    #
    # Maps in the form: [nice name]-->[bt|rt|bt,rt]-->[pkg name]
    #
    distro_maps = dict()
    scriptdir   = Path(__file__).parent.absolute()

    maps_file = open(scriptdir / f"{distro}-maps")
    maps_txt  = maps_file.readlines()

    maps_file.close()

    for mapping in maps_txt:
        map_split = mapping.split("-->")

        if len(map_split) != 3:
            raise ValueError(f"Invalid mapping: {mapping}")

        nice_name = map_split[0]
        dep_stage = map_split[1]
        pkg_name  = map_split[2].rstrip()

        if dep_stage == "bt" or dep_stage == "bt,rt":
            distro_maps[f"bt-{nice_name}"] = pkg_name

        if dep_stage == "rt" or dep_stage == "bt,rt":
            distro_maps[f"rt-{nice_name}"] = pkg_name

    # Perform mappings
    #
    # Maps in the form: [bt|rt|bt,rt]:[nice name]
    #
    deps_file = open(depsfile)
    deps_txt  = deps_file.readlines()

    deps_file.close()

    for dep in deps_txt:
        dep_split = dep.split(":")

        if len(dep_split) != 2:
            raise ValueError(f"Invalid dep: {dep}")

        dep_stage = dep_split[0]
        dep_name  = dep_split[1].rstrip()

        if dep_stage == "bt" or dep_stage == "bt,rt":
            if not f"bt-{dep_name}" in distro_maps:
                raise ValueError(f"No build-time mapping for dep: {dep_name}")

            if distro_maps[f"bt-{dep_name}"] != "NULL":
                print("bt:" + distro_maps[f"bt-{dep_name}"])

        if dep_stage == "rt" or dep_stage == "bt,rt":
            if not f"rt-{dep_name}" in distro_maps:
                raise ValueError(f"No run-time mapping for dep: {dep_name}")

            if distro_maps[f"rt-{dep_name}"] != "NULL":
                print("rt:" + distro_maps[f"rt-{dep_name}"])


if __name__ == "__main__":
    main()
