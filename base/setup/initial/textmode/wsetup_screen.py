import curses
import math

from wsetup_brand import *

# Colours we need for setup
#
COLOR_PAIR_NORMAL_TEXT      = 1
COLOR_PAIR_BRIGHT_TEXT      = 2
COLOR_PAIR_HIGHLIGHT_OPTION = 3
COLOR_PAIR_INSTRUCTIONS     = 4
COLOR_PAIR_PROGRESS         = 5

# Coords for the working area
#
WSETUP_MAIN_Y = 4
WSETUP_MAIN_X = 3

# Dimensions for scaling based on Windows setup (NT setup uses 80x50 text mode)
#
WSETUP_NATIVE_W = 80
WSETUP_NATIVE_H = 50

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

    curses.init_pair(
        COLOR_PAIR_PROGRESS,
        curses.COLOR_YELLOW,
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

def wsetup_screen_write_status(stdscr, text):
    height, width = stdscr.getmaxyx()

    for i in range(width):
        stdscr.addch(
            height - 1, i,
            ' ',
            curses.color_pair(COLOR_PAIR_INSTRUCTIONS)
        )

    origin_x = width - len(text) - 1

    stdscr.addch(
        height - 1, origin_x -1,
        curses.ACS_VLINE,
        curses.color_pair(COLOR_PAIR_INSTRUCTIONS)
    )
    stdscr.addstr(
        height - 1, origin_x,
        text,
        curses.color_pair(COLOR_PAIR_INSTRUCTIONS)
    )

def wsetup_screen_write_direct(stdscr, y, x, text, attr):
    cur_y = y
    lines = text.split("\n")

    for line in lines:
        stdscr.addstr(cur_y, x, line, attr)
        cur_y += 1

def wsetup_screen_write_simple(stdscr, y, x, text, attr):
    wsetup_screen_write_direct(stdscr, WSETUP_MAIN_Y + y, WSETUP_MAIN_X + x, text, attr)

def wsetup_screen_draw_bar(stdscr, y, x, width):
    for i in range(width):
        stdscr.addch(
            y, x + i,
            curses.ACS_BLOCK,
            curses.color_pair(COLOR_PAIR_PROGRESS) | curses.A_BOLD
        )

def wsetup_screen_draw_box(stdscr, y, x, height, width):
    # Top left corner
    #
    stdscr.addch(
        y, x,
        curses.ACS_ULCORNER,
        curses.color_pair(COLOR_PAIR_NORMAL_TEXT)
    )

    # Top middle
    #
    for i in range(x + 1, x + width - 1):
        stdscr.addch(
            y, i,
            curses.ACS_HLINE,
            curses.color_pair(COLOR_PAIR_NORMAL_TEXT)
        )

    # Top right corner
    #
    stdscr.addch(
        y, x + width - 1,
        curses.ACS_URCORNER,
        curses.color_pair(COLOR_PAIR_NORMAL_TEXT)
    )

    # Middle
    #
    for i in range(y + 1, y + height - 1):
        stdscr.addch(
            i, x,
            curses.ACS_VLINE,
            curses.color_pair(COLOR_PAIR_NORMAL_TEXT)
        )
        stdscr.addch(
            i, x + width - 1,
            curses.ACS_VLINE,
            curses.color_pair(COLOR_PAIR_NORMAL_TEXT)
        )

    # Bottom left corner
    #
    stdscr.addch(
        y + height - 1, x,
        curses.ACS_LLCORNER,
        curses.color_pair(COLOR_PAIR_NORMAL_TEXT)
    )

    # Bottom middle
    #
    for i in range(x + 1, x + width - 1):
        stdscr.addch(
            y + height - 1, i,
            curses.ACS_HLINE,
            curses.color_pair(COLOR_PAIR_NORMAL_TEXT)
        )

    # Bottom right corner
    #
    stdscr.addch(
        y + height - 1, x + width - 1,
        curses.ACS_LRCORNER,
        curses.color_pair(COLOR_PAIR_NORMAL_TEXT)
    )
            
def wsetup_screen_get_scaled_x(stdscr, val):
    height, width = stdscr.getmaxyx()
    return math.floor((val / WSETUP_NATIVE_W) * width)

def wsetup_screen_get_scaled_y(stdscr, val):
    height, width = stdscr.getmaxyx()
    return math.floor((val / WSETUP_NATIVE_H) * height)
