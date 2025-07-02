import curses
import os
import subprocess

from wsetup_brand  import *
from wsetup_pkg    import *
from wsetup_screen import *

def wsetup_step_init(stdscr):
    wsetup_screen_clear(stdscr)

    # Check the current distro is one we know about
    #
    dist_pkgfmt = os.environ.get("WSETUP_DIST_PKGFMT", "unsupported")

    if dist_pkgfmt == "unsupported":
        wsetup_screen_write_simple(
            stdscr,
            0, 0,
            "The installed OS distribution on this computer is not supported." +
            "\n\n" +
            "Setup cannot continue. Press any key to exit.",
            curses.color_pair(COLOR_PAIR_NORMAL_TEXT)
        )
        stdscr.getch()
        return 0

    return 2 # All good

def wsetup_step_beta_notice(stdscr):
    wsetup_screen_clear(stdscr)

    wsetup_screen_write_simple(
        stdscr,
        0, 0,
        "Setup Notification:",
        curses.color_pair(COLOR_PAIR_BRIGHT_TEXT)
    )

    wsetup_screen_write_simple(
        stdscr,
        2, 0,
        "You are about to install a pre-release version of " +
        WSETUP_BRAND_PRODUCT_NAME + "\n" +
        "operating environment for testing purposes only.",
        curses.color_pair(COLOR_PAIR_NORMAL_TEXT)
    )

    wsetup_screen_write_simple(
        stdscr,
        5, 3,
        "> To continue, press ENTER.\n\n" +
        "> To quit Setup without installing " +
        WSETUP_BRAND_PRODUCT_NAME + ", press F3.",
        curses.color_pair(COLOR_PAIR_NORMAL_TEXT)
    )

    wsetup_screen_write_instructions(
        stdscr,
        [
            "ENTER=Continue",
            "F3=Quit"
        ]
    )

    # Input
    #
    while True:
        user_option = stdscr.getch()

        if user_option == curses.KEY_F0 + 3:
            return 0
        elif user_option == curses.KEY_ENTER or \
             user_option == ord("\n")        or \
             user_option == ord("\r"):
            return 3

def wsetup_step_welcome(stdscr):
    wsetup_screen_clear(stdscr)

    wsetup_screen_write_simple(
        stdscr,
        0, 0,
        "Welcome to Setup.",
        curses.color_pair(COLOR_PAIR_BRIGHT_TEXT)
    )

    wsetup_screen_write_simple(
        stdscr,
        2, 0,
        "This portion of the Setup program prepares " +
        WSETUP_BRAND_PRODUCT_NAME + "\n"
        "to run on your computer.",
        curses.color_pair(COLOR_PAIR_NORMAL_TEXT)
    )

    # TODO: We don't have any 'Recovery Console', left off for now, deal with
    #       in future perhaps
    #
    wsetup_screen_write_simple(
        stdscr,
        5, 3,
        "> To set up " + WSETUP_BRAND_PRODUCT_NAME + " now, press ENTER.\n\n" +
        "> To quit Setup without installing " +
        WSETUP_BRAND_PRODUCT_NAME + ", press F3.",
        curses.color_pair(COLOR_PAIR_NORMAL_TEXT)
    )

    wsetup_screen_write_instructions(
        stdscr,
        [
            "ENTER=Continue",
            "F3=Quit"
        ]
    )

    # Input
    #
    while True:
        user_option = stdscr.getch()

        if user_option == curses.KEY_F0 + 3:
            return 0
        elif user_option == curses.KEY_ENTER or \
             user_option == ord("\n")        or \
             user_option == ord("\r"):
            return 4

def wsetup_step_eula(stdscr):
    wsetup_screen_clear(stdscr)

    #
    # TODO: Dunno, currently not reading a eula.txt that's for sure
    #

    wsetup_screen_set_title(
        stdscr,
        WSETUP_BRAND_PRODUCT_NAME + " License Agreement"
    )

    wsetup_screen_write_simple(
        stdscr,
        0, 0,
        "END USER LICENSE AGREEMENT\n\n" +
        "You don't need a license to drive a sandwich.",
        curses.color_pair(COLOR_PAIR_NORMAL_TEXT)
    )

    wsetup_screen_write_instructions(
        stdscr,
        [
            "F8=I agree",
            "Q=I do not agree" # Use Q because Esc is funny in curses
        ]
    )

    # Input
    #
    while True:
        user_option = stdscr.getch()

        if user_option == ord("q"):
            return 0 # TODO: I do not agree...
        elif user_option == curses.KEY_F0 + 8:
            wsetup_screen_set_title(
                stdscr,
                WSETUP_BRAND_PRODUCT_NAME + " Setup"
            )
            return 5

def wsetup_step_confirm_system(stdscr):
    # TUI update
    #
    wsetup_screen_clear(stdscr)

    wsetup_screen_write_simple(
        stdscr,
        0, 0,
        "Setup has determined that the following OS distribution is " +
        "installed on \nyour computer.",
        curses.color_pair(COLOR_PAIR_NORMAL_TEXT)
    )
    wsetup_screen_write_simple(
        stdscr,
        3, 4,
        os.environ.get("WSETUP_DIST_NAME"),
        curses.color_pair(COLOR_PAIR_NORMAL_TEXT)
    )
    wsetup_screen_write_simple(
        stdscr,
        5, 0,
        "If the above OS distribution is accurate, press ENTER to install \n" +
        WSETUP_BRAND_PRODUCT_NAME + ". If this is not accurate, you should " +
        "not continue and\npress F3 to exit Setup.",
        curses.color_pair(COLOR_PAIR_NORMAL_TEXT)
    )

    wsetup_screen_write_instructions(
        stdscr,
        [
            "ENTER=Continue",
            "F3=Quit"
        ]
    )

    # Input
    #
    while True:
        user_option = stdscr.getch()

        if user_option == curses.KEY_F0 + 3:
            return 0
        elif user_option == curses.KEY_ENTER or \
             user_option == ord("\n")        or \
             user_option == ord("\r"):
            return 6

def wsetup_step_install_base(stdscr):
    wsetup_screen_clear(stdscr)

    wsetup_screen_write_direct(
        stdscr,
        wsetup_screen_get_scaled_y(stdscr, WSETUP_MAIN_Y + 3),
        wsetup_screen_get_scaled_x(stdscr, 40) - 22,
        "    Please wait while Setup copies files    \n" +
        "    to the Windows installation folders.    \n" +
        "This might take several minutes to complete.",
        curses.color_pair(COLOR_PAIR_NORMAL_TEXT)
    )

    # Main box
    #
    box_h = 7
    box_w = 68
    box_y = wsetup_screen_get_scaled_y(stdscr, 20)
    box_x = wsetup_screen_get_scaled_x(stdscr, 40) - int(box_w / 2)

    wsetup_screen_draw_box(
        stdscr,
        box_y,
        box_x,
        box_h,
        box_w
    )

    wsetup_screen_write_direct(
        stdscr,
        box_y + 1,
        box_x + 2,
        "Setup is copying files...",
        curses.color_pair(COLOR_PAIR_NORMAL_TEXT)
    )

    # Progress box
    #
    progbox_h = 3
    progbox_w = box_w - 10
    progbox_y = box_y + 3
    progbox_x = box_x + 5

    wsetup_screen_draw_box(
        stdscr,
        progbox_y,
        progbox_x,
        progbox_h,
        progbox_w
    )

    wsetup_screen_write_instructions(
        stdscr,
        [
            "ENTER=Continue"
        ]
    )

    # Install the base packages to get to phase 2
    #
    pkgcmd       = ""
    pkgfmt       = os.environ.get("WSETUP_DIST_PKGFMT")
    pkgnames_arr = wsetup_pkg_get_pkgnames_basesystem()
    pkgnames     = " ".join(pkgnames_arr)

    if pkgfmt == "deb":
        pkgcmd = f"apt-get install -y -o APT::Status-Fd=1 {pkgnames}"
    else:
        raise Exception(f"No install command for format {pkgfmt}")

    process = subprocess.Popen(
            pkgcmd.split(),
            bufsize=1,
            stderr=subprocess.PIPE,
            stdout=subprocess.PIPE,
            universal_newlines=True
        )

    while True:
        cmd_out = process.stdout.readline()

        if process.poll() is not None:
            break

        if cmd_out:
            if pkgfmt == "deb":
                # Parse apt-get status output
                #
                if not cmd_out.startswith("pmstatus"):
                    continue

                progress = str(int(float(cmd_out.split(":")[2]))) + "%"

                wsetup_screen_write_simple(
                    stdscr,
                    box_y + 2,
                    wsetup_screen_get_scaled_x(stdscr, 40) - 3,
                    progress,
                    curses.color_pair(COLOR_PAIR_NORMAL_TEXT)
                )
                wsetup_screen_draw_bar(
                    stdscr,
                    progbox_y + 1,
                    progbox_x + 1,
                    progbox_w - 2
                )

                stdscr.refresh()

    wsetup_screen_write_simple(
        stdscr,
        1, 0,
        "Finito",
        curses.color_pair(COLOR_PAIR_NORMAL_TEXT)
    )

    # Input
    #
    while True:
        user_option = stdscr.getch()

        if user_option == curses.KEY_F0 + 3:
            return 0
