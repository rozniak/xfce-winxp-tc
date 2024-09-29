import argparse
import pefile

from io import BytesIO
from pathlib import Path
from PIL import Image

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
        # Based on resource string example in pefile:
        # https://github.com/erocarrera/pefile/blob/wiki/ReadingResourceStrings.md
        #
        # Just testing dumping out all bitmaps for now
        #
        # NOTE: Appears pillow does not handle 32bpp bitmaps - see the radio
        #       button output, should have an alpha channel
        #
        data_rva = entry.directory.entries[0].data.struct.OffsetToData
        size = entry.directory.entries[0].data.struct.Size

        print(entry.name)

        data = pe.get_memory_mapped_image()[data_rva:data_rva+size]

        datafile = BytesIO(data)

        img = Image.open(datafile)
        img.save(str(entry.name) + ".png")


if __name__ == "__main__":
    main()
