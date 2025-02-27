import argparse
import pefile
import struct

from io import BytesIO
from pathlib import Path
#from PIL import Image
from PIL import Image, _binary, BmpImagePlugin

def main():
    parser = argparse.ArgumentParser(
        prog="Windows Theme Test",
        description="Theme test tool.",
        epilog="Internal only atm.",
    )

    parser.add_argument("file", help="path to windows pe")

    args = parser.parse_args()

    # Important stuff begins here
    #
    file_path = Path(args.file)
    pe = pefile.PE(file_path, fast_load=True)

    pe.parse_data_directories(
        directories=[
            pefile.DIRECTORY_ENTRY['IMAGE_DIRECTORY_ENTRY_RESOURCE']
        ]
    )

    rt_bitmap_idx = [entry.id for entry in pe.DIRECTORY_ENTRY_RESOURCE.entries].index(pefile.RESOURCE_TYPE['RT_BITMAP'])
    rt_bitmap_entry = pe.DIRECTORY_ENTRY_RESOURCE.entries[rt_bitmap_idx]

    for entry in rt_bitmap_entry.directory.entries:
        print(entry.name)

        # Based on resource string example in pefile:
        # https://github.com/erocarrera/pefile/blob/wiki/ReadingResourceStrings.md
        #
        # Just testing dumping out all bitmaps for now
        #
        data_rva = entry.directory.entries[0].data.struct.OffsetToData
        size = entry.directory.entries[0].data.struct.Size
        data = pe.get_memory_mapped_image()[data_rva:data_rva+size]

        filename = "out/" + str(entry.name) + ".bmp"

        with open(filename, "wb") as myFile:
              myFile.write(data)

        # Convert the 32bpp to output a PNG - normally Pillow doesn't support
        # reading the alpha channel, because MS' documentation for BMP/DIB
        # files says its unused
        #
        # Obviously Windows itself uses it, so we need it - a helpful developer
        # from the Pillow project supplied the below snippet to read the alpha
        # channel
        # https://github.com/python-pillow/Pillow/issues/8594#issuecomment-2541337200
        #
        im = Image.open(filename)

        if (
            im.format == "DIB" and
            im.info["compression"] ==
                BmpImagePlugin.BmpImageFile.COMPRESSIONS["RAW"]
        ):
            bits = None
            with open(filename, "rb") as fp:
                header_size = _binary.i32le(fp.read(4))
                if header_size == 40:
                    header_data = fp.read(header_size - 4)
                    bits = _binary.i16le(header_data, 10)
            if bits == 32:
                im._mode = "RGBA"
                args = list(im.tile[0].args)
                args[0] = "BGRA"
                im.tile = [im.tile[0]._replace(args=tuple(args))]

        try:
            im.save("out/" + str(entry.name) + ".png")
        except Exception as e:
            print("Unhappy with " + str(entry.name))
            print(e)


if __name__ == "__main__":
    main()
