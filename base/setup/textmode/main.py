import curses

def main(stdscr):
    stdscr.clear()

    stdscr.addstr(0, 0, "This is a test.")

    stdscr.refresh()
    stdscr.getkey()

curses.wrapper(main)
