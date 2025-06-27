import sys
import gi

gi.require_version("Gtk", "3.0")
from gi.repository import Gtk
from warapp import WinTCAutorunApplication

if __name__ == "__main__":
    app = WinTCAutorunApplication()
    app.run(sys.argv)
