import argparse

from pathlib import Path


def main():
    """Constructs symlinks from mappings for an XDG theme.
    """

    VALID_KINDS = ["cursors", "icons", "sounds"]
    
    parser = argparse.ArgumentParser(
        prog="XDG Theme Symlink Utility",
        description="Constructs symlinks from mappings for an XDG theme.",
        epilog="See `./README.MD` for details."
    )

    parser.add_argument("inputdir", help="the icon theme source directory")
    parser.add_argument("outputdir", help="the output directory to construct into")
    parser.add_argument("kind", help="the kind of XDG theme being mapped")

    args = parser.parse_args()

    input_dir  = Path(args.inputdir)
    output_dir = Path(args.outputdir).resolve()
    kind       = args.kind.lower()

    if not input_dir.is_dir():
        raise NotADirectoryError(f"{input_dir} is not a directory")

    if not output_dir.is_dir():
        output_dir.mkdir(parents=True)

    if not kind in VALID_KINDS:
        raise ValueError(f"{kind} is not a known XDG theme kind")

    # Set up paths
    #
    mappings_path = input_dir / "mappings"
    rel_res_path  = Path("res")
    res_path      = input_dir / rel_res_path

    if not mappings_path.is_file():
        raise FileNotFoundError(f"{mappings_path} could not be found")

    if not res_path.is_dir():
        raise NotADirectoryError(f"{res_path} could not be found")

    # Set up symlinks
    #
    if kind == "icons":
        # See what sizes we need
        #
        sizes = list()

        for sub_path in res_path.glob("*x*"):
            if not sub_path.is_dir():
                continue

            size_name = sub_path.stem

            size_path = output_dir / size_name

            if not size_path.is_dir():
                size_path.mkdir(parents=True)

            sizes.append(size_name)

        # Do we have symbolic icons?
        #
        sym_res_path = res_path / "svg"
        sym_out_path = output_dir / "symbolic"

        if sym_res_path.is_dir():
            if not sym_out_path.is_dir():
                sym_out_path.mkdir(parents=True)
        else:
            sym_res_path = None

        # Attempt to read through the mappings
        #
        res_path_rel_to_mapping = Path("../..") / rel_res_path

        with open(mappings_path) as mappings_file:
            for mapping in mappings_file:
                if mapping == "":
                    continue

                rel_icon_path, _, rel_res_name = mapping.partition("-->")

                rel_res_name = rel_res_name.strip()

                # Is this a symbolic icon?
                #
                if rel_icon_path.endswith("-symbolic"):
                    if sym_res_path is None:
                        raise NotADirectoryError(f"No SVGs for {rel_res_name}")

                    target_res_path = res_path_rel_to_mapping / "svg" / f"{rel_res_name}.svg"
                    theme_icon_path = output_dir / "symbolic" / f"{rel_icon_path}.svg"

                    theme_icon_path_dir = theme_icon_path.parent

                    if not theme_icon_path_dir.is_dir():
                        theme_icon_path_dir.mkdir(parents=True)

                    theme_icon_path.symlink_to(target_res_path)

                    continue

                # Deal with non-symbolic icons as usual
                #
                for size in sizes:
                    target_res_path = res_path_rel_to_mapping / size / f"{rel_res_name}.png"
                    theme_icon_path = output_dir / size / f"{rel_icon_path}.png"

                    theme_icon_path_dir = theme_icon_path.parent

                    if not theme_icon_path_dir.is_dir():
                        theme_icon_path_dir.mkdir(parents=True)

                    theme_icon_path.symlink_to(target_res_path)
    else:
        # Parse the mappings
        #
        with open(mappings_path) as mappings_file:
            for mapping in mappings_file:
                if mapping == "":
                    continue

                rel_link_name, _, rel_res_name = mapping.partition("-->")

                rel_res_name = rel_res_name.strip()

                target_res_path = rel_res_path / rel_res_name
                target_link_path = output_dir / rel_link_name

                # Sound themes have the .wav extension
                #
                if kind == "sounds":
                    target_res_path  = Path(f"{target_res_path}.wav")
                    target_link_path = Path(f"{target_link_path}.wav")

                target_link_path.symlink_to(target_res_path)


if __name__ == "__main__":
    main()
