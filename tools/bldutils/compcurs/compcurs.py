import argparse
import subprocess

from pathlib import Path


def main():
    """Compiled X11 cursors from source images.
    """

    parser = argparse.ArgumentParser(
        prog="X11 Cusor Compiler Utility",
        description="Compiles X11 cursors from source images.",
        epilog="See `./README.MD` for details."
    )

    parser.add_argument("cfgdir", help="the cursor configs source directory")
    parser.add_argument("imgdir", help="the cursor images source directory")
    parser.add_argument("outputdir", help="the output directory for the compiled cursors")

    args = parser.parse_args()

    config_dir = Path(args.cfgdir)
    image_dir  = Path(args.imgdir)
    output_dir = Path(args.outputdir)

    if not config_dir.is_dir():
        raise NotADirectoryError(f"{config_dir} is not a directory")

    if not image_dir.is_dir():
        raise NotADirectoryError(f"{image_dir} is not a directory")

    if not output_dir.is_dir():
        output_dir.mkdir(parents=True)

    # Generate the cursors
    #
    for src_file in config_dir.glob("*.cfg"):
        cur_name = src_file.stem
        output_file_path = output_dir / cur_name

        subprocess.run(["xcursorgen", "--prefix", image_dir, src_file, output_file_path], check=True)


if __name__ == "__main__":
    main()
