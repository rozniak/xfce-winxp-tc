import curses
import os

from wsetup_brand  import *
from wsetup_screen import *

def wsetup_step_init(stdscr):
    wsetup_screen_clear(stdscr)

    # Check the current distro is one we know about
    #
    dist_id = os.environ.get("DIST_ID")

    if dist_id == None or dist_id == "unsupported":
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
    # Ident the distro we found to the user
    #
    env_dist_id = os.environ.get("DIST_ID")
    env_dist_id_ext = os.environ.get("DIST_ID_EXT")

    dist_name = "Unknown distribution"

    if env_dist_id == "archpkg":
        dist_name = "Arch Linux or derivative"
    elif env_dist_id == "apk":
        dist_name = "Alpine Linux"
    elif env_dist_id == "bsdpkg":
        dist_name = "FreeBSD"
    elif env_dist_id == "deb":
        dist_name = "Debian or derivative"
    elif env_dist_id == "rpm":
        dist_name = "Red Hat, Fedora, or other RPM-based distribution"
    elif env_dist_id == "xbps":
        if env_dist_id_ext == "glibc":
            dist_name = "Void Linux (glibc)"
        elif env_dist_id_ext == "musl":
            dist_name = "Void Linux (musl)"

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
        dist_name,
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
            return 0 # TODO: Continue to install
