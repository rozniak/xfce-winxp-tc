import argparse
import fnmatch
import os
import shutil
import sys

from PIL import Image, ImageDraw


def main():
    parser = argparse.ArgumentParser(prog="Theme Part Composer Utility", description="Composes theme part graphics from source files.", epilog="See README.MD if you want to know the details.")

    parser.add_argument("scheme", help="path to the scheme file")
    parser.add_argument("inputdir", help="the input directory to scan for files")
    parser.add_argument("outputdir", help="the output directory to place composed files")

    args = parser.parse_args()

    scheme = parse_scheme(args.scheme)

    for filename in os.listdir(args.inputdir):
        if fnmatch.fnmatch(filename, "*.src.png"):
            basename = get_basename(filename)
            maskname = f"{basename}.mask.png"
            pngname = filename

            pngRGB = Image.open(os.path.join(args.inputdir, pngname))

            maskRGB = Image.open(os.path.join(args.inputdir, maskname))
            maskMono = maskRGB.convert("1")

            newImage = Image.new("RGBA", pngRGB.size)

            drawing = ImageDraw.Draw(newImage)

            if "progress" in basename:
                drawing.bitmap((0, 0), maskMono, scheme["ACTIVE_TITLE_BAR_BG1"])
            else:
                drawing.bitmap((0, 0), maskMono, scheme["THREED_OBJECTS_BG"])

            composed = Image.alpha_composite(newImage, pngRGB)

            composed.save(os.path.join(args.outputdir, f"{basename}.png"))
        elif fnmatch.fnmatch(filename, "*.static.png"):
            basename = get_basename(filename)

            shutil.copyfile(os.path.join(args.inputdir, filename), os.path.join(args.outputdir, f"{basename}.png"))

def get_basename(filename):
    return filename[0:filename.find(".")]

def parse_scheme(path):
    scheme = dict()

    with open(path) as schemefile:
        for line in schemefile:
            if line == "":
                continue

            equals_pos = line.find("=")

            scheme_id = line[0:equals_pos]
            scheme_value = line[equals_pos + 1:len(line)]

            scheme[scheme_id] = scheme_value

    return scheme

if __name__ == "__main__":
    main()
