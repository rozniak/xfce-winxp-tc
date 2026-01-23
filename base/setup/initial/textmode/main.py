import curses
import os

from wsetup_screen import *
from wsetup_step   import *

def main(stdscr):
    steps = [
        None,
        wsetup_step_init,
        wsetup_step_beta_notice,
        wsetup_step_welcome,
        wsetup_step_eula,
        wsetup_step_confirm_system,
        wsetup_step_prep_install,
        wsetup_step_install_base,
        wsetup_step_prepare_chain_to_gui
    ]

    os.environ["ASAN_OPTIONS"] = "exitcode=0"

    current_step = 1

    wsetup_screen_init(stdscr)

    while current_step != 0:
        current_step = steps[current_step](stdscr)

curses.wrapper(main)
