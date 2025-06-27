import gi

gi.require_version("Gtk", "3.0")
from gi.repository import Gtk

class WinTCAutorunWindow(Gtk.ApplicationWindow):
    def __init__(self, *args, **kwargs):
        super().__init__(
            *args,
            title="Windows Setup",
            **kwargs
        )

        self.testlabel = Gtk.Label(label="Welcome to Windows WinTC")
        self.add(self.testlabel)
        self.show_all()
