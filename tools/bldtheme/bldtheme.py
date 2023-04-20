import argparse
import fnmatch
import os
import sys

from PIL import Image, ImageDraw


def main():
    parser = argparse.ArgumentParser(prog="Theme Part Composer Utility", description="Composes theme part graphics from source files.", epilog="See README.MD if you want to know the details.")

    parser.add_argument("inputdir", help="the input directory to scan for files")
    parser.add_argument("outputdir", help="the output directory to place composed files")

    args = parser.parse_args()

    for filename in os.listdir(args.inputdir):
        if fnmatch.fnmatch(filename, "*.src.png"):
            basename = filename[0:filename.find(".src.png")]
            maskname = f"{basename}.mask.png"
            pngname = filename

            pngRGB = Image.open(os.path.join(args.inputdir, pngname))

            maskRGB = Image.open(os.path.join(args.inputdir, maskname))
            maskMono = maskRGB.convert("1")

            newImage = Image.new("RGBA", pngRGB.size)

            drawing = ImageDraw.Draw(newImage)

            drawing.bitmap((0, 0), maskMono, "rgb(128,128,128)")
            
            composed = Image.alpha_composite(newImage, pngRGB)

            composed.save(os.path.join(args.outputdir, f"{basename}.png"))

if __name__ == "__main__":
    main()
