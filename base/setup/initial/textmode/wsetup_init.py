import os
import subprocess

from pathlib import Path
from enum    import Enum

class WSetupInitSys(Enum):
    UNKNOWN  = 0
    SYSTEMD  = 1
    RUNIT    = 2
    UPSTART  = 3
    SYSVINIT = 4
    OPENRC   = 5

#
# wsetup_get_init_sys()
#
wsetup_init_sys = None

def _wsetup_get_init_sys():
    # Simple path checks
    #
    if Path("/run/systemd/system").is_dir():
        return WSetupInitSys.SYSTEMD

    if Path("/run/openrc").is_dir():
        return WSetupInitSys.OPENRC

    if Path("/run/upstart").is_dir():
        return WSetupInitSys.UPSTART

    # Check PID 1
    #
    pid1_path = Path("/proc/1/comm")

    if pid1_path.is_file():
        try:
            pid1_cmd = pid1_path.read_text().strip()

            if pid1_cmd == "runit-init":
                return WSetupInitSys.RUNIT
            elif pid1_cmd == "init":
                return WSetupInitSys.SYSVINIT
        except IOError:
            pass

    return WSetupInitSys.UNKNOWN

def wsetup_get_init_sys():
    global wsetup_init_sys

    if wsetup_init_sys != None:
        return wsetup_init_sys

    wsetup_init_sys = _wsetup_get_init_sys()

    return wsetup_init_sys

#
# wsetup_reboot()
#
def wsetup_reboot():
    init_sys = wsetup_get_init_sys()

    if init_sys == WSetupInitSys.SYSTEMD:
        subprocess.run(
            [ "reboot", "now" ],
            capture_output=True,
            check=True
        )
    elif init_sys == WSetupInitSys.SYSVINIT:
        subprocess.run(
            [ "reboot" ],
            capture_output=True,
            check=True
        )
    else:
        raise Exception("No reboot mechanism known for the current init.")
