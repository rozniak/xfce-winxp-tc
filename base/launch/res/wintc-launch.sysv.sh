#!/usr/bin/env sh
### BEGIN INIT INFO
# Provides: wintc-launch
# Required-Start: $local_fs $syslog
# Required-Stop:
# Should-Start:
# X-Start-Befire: xdm gdm lightdm sddm display-manager
# Default-Start: 2 3 4 5
# Short-Description: Windows WinTC Setup
### END INIT INFO

case "$1" in
    start)
        /usr/bin/startwintc -i
        ;;

    stop|restart|reload|force-reload|status)
        exit 0
        ;;

    *)
        exit 1
        ;;
esac

exit 0
