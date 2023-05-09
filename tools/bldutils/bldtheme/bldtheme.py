import argparse
import fnmatch
import shutil

from pathlib import Path
from PIL import Image, ImageDraw


def main():
    """Composes theme part graphics from source files.

    Parses command line arguments for the path to the scheme file,
    the input directory to scan for files, and the output directory to place
    composed files. It then reads the scheme file and uses it to compose theme part
    graphics from source files.
    """
    parser = argparse.ArgumentParser(
        prog="Theme Part Composer Utility",
        description="Composes theme part graphics from source files.",
        epilog="See `./README.MD` for details.",
    )

    parser.add_argument("scheme", help="path to the scheme file")
    parser.add_argument("inputdir", help="the input directory to scan for files")
    parser.add_argument("outputdir", help="the output directory to place composed files")

    args = parser.parse_args()

    scheme = parse_scheme(Path(args.scheme))
    input_dir = Path(args.inputdir)
    output_dir = Path(args.outputdir)

    if not input_dir.is_dir():
        raise NotADirectoryError(f"{input_dir} is not a directory")

    if not output_dir.is_dir():
        output_dir.mkdir(parents=True)

    for file_path in input_dir.glob("*.*.png"):
        suffix, _ = file_path.suffixes # Discard png suffix
        if suffix == ".src":
            basename = get_basename(file_path)
            maskname = f"{basename}.mask.png"

            pngRGB = Image.open(file_path) # Also `input_dir / pngname`

            maskRGB = Image.open(file_path.with_name(maskname))
            maskMono = maskRGB.convert("1")

            newImage = Image.new("RGBA", pngRGB.size)

            drawing = ImageDraw.Draw(newImage)

            if "progress" in basename:
                drawing.bitmap((0, 0), maskMono, scheme["ACTIVE_TITLE_BAR_BG1"])
            else:
                drawing.bitmap((0, 0), maskMono, scheme["THREED_OBJECTS_BG"])

            composed = Image.alpha_composite(newImage, pngRGB)

            composed.save(output_dir / f"{basename}.png")
        elif suffix == ".static":
            basename = get_basename(file_path)

            shutil.copyfile(file_path, output_dir / f"{basename}.png")


def get_basename(filename: Path) -> str:
    """
    Get the basename of a file without the extension.

    Examples:
        >>> get_basename(Path('example.tar.gz'))
        'example'
    """
    return filename.stem.partition('.')[0]

def parse_scheme(path: Path) -> dict:
    """Parse a scheme file into a dictionary."""
    scheme = {}

    with open(path) as schemefile:
        for line in schemefile:
            if line == "":
                continue

            scheme_id, _, scheme_value = line.partition("=")

            scheme[scheme_id] = scheme_value.strip()

    return scheme


if __name__ == "__main__":
    main()
