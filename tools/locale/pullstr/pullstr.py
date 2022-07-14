import os
import sys

DS_SETFONT = 0x40
MF_POPUP = 0x00000010

DIALOG_ID_OFFSET = 10000000
MENU_ID_OFFSET = 20000000

RESOURCE_ID_OFFSET = 0

def main():
    global RESOURCE_ID_OFFSET # Nasty but I am lazy

    if len(sys.argv) != 3:
        print("Usage: python3 pullstr.py <DIALOG|MENU> <file>")
        sys.exit(1)

    resource_type = sys.argv[1]
    file_name = sys.argv[2]

    RESOURCE_ID_OFFSET = int(os.path.basename(file_name)) * 100

    try:
        resource_file = open(file_name, "rb")
    except Exception as e:
        print(e)
        sys.exit(1)

    if resource_type == "DIALOG":
        parse_dialog(resource_file)
    elif resource_type == "MENU":
        parse_menu(resource_file)
    else:
        print("Invalid resource type, must be DIALOG or MENU.")
        sys.exit(1)

    resource_file.close()

def parse_dialog(resource):
    resource.seek(2, 0)

    dlgtemplate_signature = read_word(resource)

    resource.seek(0, 0);

    if dlgtemplate_signature == 0xFFFF:
        parse_extended_dialog(resource)
    else:
        parse_standard_dialog(resource)

def parse_extended_dialog(resource):
    # Parse DLGTEMPLATEEX
    #
    dlg_dlgVer = read_word(resource)
    dlg_dlgSignature = read_word(resource)
    dlg_helpID = read_dword(resource)
    dlg_exStyle = read_dword(resource)
    dlg_style = read_dword(resource)
    dlg_cDlgItems = read_word(resource)
    dlg_x = read_word(resource)
    dlg_y = read_word(resource)
    dlg_cx = read_word(resource)
    dlg_cy = read_word(resource)
    dlg_menu = read_sz_or_ordinal(resource)
    dlg_windowClass = read_sz_or_ordinal(resource)
    dlg_title = read_utf16le(resource)
    dlg_pointsize = read_word(resource)
    dlg_weight = read_word(resource)
    dlg_italic = read_byte(resource)
    dlg_charset = read_byte(resource)
    dlg_typeface = read_utf16le(resource)

    print_not_empty(dlg_title, DIALOG_ID_OFFSET + RESOURCE_ID_OFFSET)

    # Parse DLGTEMPLATEITEMEX structs
    #
    for i in range(dlg_cDlgItems):
        seek_align_to_dword(resource)

        ctl_helpId = read_dword(resource)
        ctl_exStyle = read_dword(resource)
        ctl_style = read_dword(resource)
        ctl_x = read_word(resource)
        ctl_y = read_word(resource)
        ctl_cx = read_word(resource)
        ctl_cy = read_word(resource)
        ctl_id = read_dword(resource)
        ctl_windowClass = read_sz_or_ordinal(resource)
        ctl_title = read_sz_or_ordinal(resource)
        ctl_extraCount = read_word(resource)

        print_not_empty(ctl_title, DIALOG_ID_OFFSET + RESOURCE_ID_OFFSET + i + 1)

def parse_standard_dialog(resource):
    # Parse DLGTEMPLATE
    #
    dlg_style = read_dword(resource)
    dlg_dwExtendedStyle = read_dword(resource)
    dlg_cdit = read_word(resource)
    dlg_x = read_word(resource)
    dlg_y = read_word(resource)
    dlg_cx = read_word(resource)
    dlg_cy = read_word(resource)
    dlg_menu = read_sz_or_ordinal(resource)
    dlg_windowClass = read_sz_or_ordinal(resource)
    dlg_title = read_utf16le(resource)

    if dlg_style & DS_SETFONT > 0:
        dlg_wPointSize = read_word(resource)
        dlg_szFontName = read_utf16le(resource)

    print_not_empty(dlg_title, DIALOG_ID_OFFSET + RESOURCE_ID_OFFSET)

    # Parse DLGITEMTEMPLATE structs
    #
    for i in range(dlg_cdit):
        seek_align_to_dword(resource)

        ctl_style = read_dword(resource)
        ctl_dwExtendedStyle = read_dword(resource)
        ctl_x = read_word(resource)
        ctl_y = read_word(resource)
        ctl_cx = read_word(resource)
        ctl_cy = read_word(resource)
        ctl_id = read_word(resource)
        ctl_windowClass = read_sz_or_ordinal(resource)
        ctl_title = read_sz_or_ordinal(resource)
        ctl_extraCount = read_word(resource)

        print_not_empty(ctl_title, DIALOG_ID_OFFSET + RESOURCE_ID_OFFSET + i + 1)

def parse_menu(resource):
    menutemplate_version = read_word(resource)

    resource.seek(0, 0);

    if menutemplate_version == 1:
        parse_extended_menu(resource)
    else:
        parse_standard_menu(resource)

def parse_extended_menu(resource):
    # Parse MENUEX_TEMPLATE_HEADER
    #
    header_wVersion = read_word(resource)
    header_wOffset = read_word(resource)
    header_dwHelpId = read_dword(resource)

    # Seek to the start of menu items, rewind a DWORD and add the offset
    #
    resource.seek(-4, 1)
    resource.seek(header_wOffset, 1)

    # Parse MENUEX_TEMPLATE_ITEM
    #
    i = 0
    while not test_eof(resource):
        i += 1

        menuitem_dwType = read_dword(resource)
        menuitem_dwState = read_dword(resource)
        menuitem_uId = read_dword(resource)
        menuitem_wFlags = read_word(resource)
        menuitem_szText = read_utf16le(resource)

        print_not_empty(menuitem_szText, MENU_ID_OFFSET + RESOURCE_ID_OFFSET + i + 1)

        seek_align_to_dword(resource)

        # I don't see any documentation for this, but if the menu item has a child
        # menu, then there's an extra DWORD on the end, wtf?
        #
        if menuitem_wFlags & 0x0001 > 0:
            resource.seek(4, 1)

def parse_standard_menu(resource):
    # Parse MENUHEADER
    #
    header_wVersion = read_word(resource)
    header_cbHeaderSize = read_word(resource)

    # Parse menu items
    #
    i = 0

    while not test_eof(resource):
        i += 1

        menuitem_fItemFlags = read_word(resource)

        # NORMALMENUITEM has an extra field to read...
        #
        if menuitem_fItemFlags & MF_POPUP == 0:
            menuitem_wMenuID = read_word(resource)

        menuitem_szItemText = read_utf16le(resource)

        print_not_empty(menuitem_szItemText, MENU_ID_OFFSET + RESOURCE_ID_OFFSET + i + 1)

def print_not_empty(string, index):
    if isinstance(string, int):
        return

    safe_string = string
    safe_string = safe_string.replace("\n", "\\\\n")
    safe_string = safe_string.replace("\"", "\\\"")
    safe_string = safe_string.rstrip("\x00")

    if len(safe_string) > 0:
        print(str(index) + "\t" + safe_string)

def read_bytes_le(resource, num_bytes):
    return int.from_bytes(resource.read(num_bytes), "little")

def read_byte(resource):
    return read_bytes_le(resource, 1)

def read_word(resource):
    return read_bytes_le(resource, 2)

def read_dword(resource):
    return read_bytes_le(resource, 4)

def read_utf16le(resource):
    utf_string = bytearray()
    last_char = 0xFFFF

    while last_char != 0x0000:
        next_char = resource.read(2)
        utf_string.extend(next_char)

        last_char = int.from_bytes(next_char, "little")

    return utf_string.decode("utf-16le")

def read_sz_or_ordinal(resource):
    first_word = read_word(resource)

    if first_word == 0x0000:
        return 0
    elif first_word == 0xFFFF:
        return read_word(resource) # Ordinal
    else:
        resource.seek(-2, 1)
        return read_utf16le(resource)

def seek_align_to_dword(resource):
    alignment = resource.tell() % 4
    
    if alignment > 0:
        resource.seek(4 - alignment, 1)

def test_eof(resource):
    test = resource.read(1)
    
    if test == b'':
        return True

    resource.seek(-1, 1)
    return False

if __name__ == "__main__":
    main()
