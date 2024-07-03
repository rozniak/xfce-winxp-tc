import curses

from wsetup_brand import *

# Colours we need for setup
#
COLOR_BRIGHT_WHITE = 15

COLOR_PAIR_NORMAL_TEXT      = 1
COLOR_PAIR_BRIGHT_TEXT      = 2
COLOR_PAIR_HIGHLIGHT_OPTION = 3
COLOR_PAIR_INSTRUCTIONS     = 4

# Coords for the working area
#
WSETUP_MAIN_Y = 4
WSETUP_MAIN_X = 3

def wsetup_screen_init(stdscr):
    # Set up our colours
    #
    # Use a try-catch for the bright color, for some stupid reason even if we
    # ask whether the terminal has 256color, it says 'yes' and then dies anyway
    #
    curses.init_pair(
        COLOR_PAIR_NORMAL_TEXT,
        curses.COLOR_WHITE,
        curses.COLOR_BLUE
    )
    curses.init_pair(
        COLOR_PAIR_HIGHLIGHT_OPTION,
        curses.COLOR_BLUE,
        curses.COLOR_WHITE
    )
    curses.init_pair(
        COLOR_PAIR_INSTRUCTIONS,
        curses.COLOR_BLACK,
        curses.COLOR_WHITE
    )

    try:
        curses.init_pair(
            COLOR_PAIR_BRIGHT_TEXT,
            COLOR_BRIGHT_WHITE,
            curses.COLOR_BLUE
        )
    except:
        curses.init_pair(
            COLOR_PAIR_BRIGHT_TEXT,
            curses.COLOR_WHITE,
            curses.COLOR_BLUE
        )

    # Init the screen
    #
    height, width = stdscr.getmaxyx()

    stdscr.bkgd(' ', curses.color_pair(COLOR_PAIR_NORMAL_TEXT))
    stdscr.clear()

    wsetup_screen_set_title(
        stdscr,
        WSETUP_BRAND_PRODUCT_NAME + " Setup"
    )

    wsetup_screen_write_instructions(stdscr, [])

    stdscr.refresh()

def wsetup_screen_clear(stdscr):
    height, width = stdscr.getmaxyx()

    for y in range(WSETUP_MAIN_Y, height - 1):
        for x in range(width):
            stdscr.insch(y, x, ' ')

def wsetup_screen_set_title(stdscr, title):
    height, width = stdscr.getmaxyx()

    for y in range(3):
        for x in range(width):
            stdscr.insch(y, x, ' ');

    # Write the new title
    #
    title_len = len(title)

    stdscr.addstr(
        1, 1,
        title,
        curses.color_pair(COLOR_PAIR_NORMAL_TEXT)
    )
    stdscr.addstr(
        2, 0,
        "".ljust(title_len + 3, "="),
        curses.color_pair(COLOR_PAIR_NORMAL_TEXT)
    )

def wsetup_screen_write_instructions(stdscr, arr):
    height, width = stdscr.getmaxyx()

    cur_x = 2

    for i in range(width):
        stdscr.insch(
            height - 1, i,
            ' ',
            curses.color_pair(COLOR_PAIR_INSTRUCTIONS)
        )

    for instruction in arr:
        stdscr.addstr(
            height - 1, cur_x,
            instruction,
            curses.color_pair(COLOR_PAIR_INSTRUCTIONS)
        )

        cur_x += len(instruction) + 2

def wsetup_screen_write_simple(stdscr, y, x, text, attr):
    cur_y = y
    lines = text.split("\n")

    for line in lines:
        stdscr.addstr(WSETUP_MAIN_Y + cur_y, WSETUP_MAIN_X + x, line, attr)
        cur_y += 1
