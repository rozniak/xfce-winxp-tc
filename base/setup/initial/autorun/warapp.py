import gi

gi.require_version("Gtk", "3.0")
from gi.repository import Gtk
from warwndar import WinTCAutorunWindow

class WinTCAutorunApplication(Gtk.Application):
    def __init__(self, *args, **kwargs):
        super().__init__(
            *args,
            application_id="uk.oddmatics.wintc.autorun",
            **kwargs
        )

        self.window_startup = None

    def do_activate(self):
        if not self.window_startup:
            self.window_startup = WinTCAutorunWindow(application=self)

        self.window_startup.present()
