# Taskband network connections on FreeBSD

FreeBSD has no NetworkManager, so on `bsdpkg` builds the taskband's network
icons talk to the kernel (`getifaddrs(3)`, the routing socket, network ioctls)
and to `wpa_supplicant(8)` through its control socket instead of libnm. The UI
is the same as on Linux: one tray icon per adapter, the wireless network list,
the status window and so on.

## Plug and play

Like NetworkManager on Linux, adapters work without any configuration:

- Ethernet adapters are switched on, and get an address by DHCP when a cable
  is plugged in
- Wireless adapters get a `wlan(4)` interface with `wpa_supplicant` on it, so
  networks can be picked from the list - those connected to are saved in
  `/var/db/wintc/wpa_supplicant.conf`, which only root can read
- The user at the machine can enable, disable and repair adapters without a
  password, as with NetworkManager's default polkit policy

This also covers adapters plugged in later, and works from boot. Adapters that
have something set up in `rc.conf(5)` (`ifconfig_<interface>`,
`ifconfig_DEFAULT` or `wlans_<device>`) are left entirely to the system - in
the same way NetworkManager leaves alone the ones in
`/etc/network/interfaces`.

How it works:

- `etc/devd/wintc-net.conf` hands adapters appearing and links coming up to
  `libexec/wintc/wintc-net-devd`. These rules take over the stock ones in
  `/etc/devd.conf` and `/etc/devd/dhclient.conf` (devd only runs one rule per
  event), and the helper always runs the stock action first.
- Enabling, disabling and repairing go through `pkexec(1)` and
  `libexec/wintc/wintc-netctl`, with the polkit action
  `uk.co.oddmatics.wintc.taskband.network-control` (`allow_active` is `yes`).

## Requirements

- `devd` has to be restarted once after the package is installed (or the
  machine rebooted): `service devd restart`
- The system D-Bus has to run for polkit: `dbus_enable="YES"` in `rc.conf`
- The session has to be registered with ConsoleKit as the active one, as a
  display manager such as LightDM does - otherwise polkit asks for the
  administrator password, which needs an authentication agent in the session
- Scanning and connecting needs access to the `wpa_supplicant` control
  socket, which is given to the `wheel` group (as in the
  `wpa_supplicant.conf(5)` examples)

## rc.conf(5) variables

    wintc_net_enable="NO"     # leave all adapters to rc.conf
    wintc_net_group="network" # group allowed to scan and connect

## Setting up wireless in rc.conf instead

The usual FreeBSD setup works as well (replace `iwm0` as needed):

    wlans_iwm0="wlan0"
    ifconfig_wlan0="WPA DHCP"
    wpa_supplicant_umask="077"

The umask matters: `wpa_supplicant` rewrites its configuration file when a
network is saved, and with the default umask the new file would be readable by
everyone, network keys included. For the taskband to be able to scan and
connect, `/etc/wpa_supplicant.conf` needs:

    ctrl_interface=/var/run/wpa_supplicant
    ctrl_interface_group=wheel
    update_config=1

## Limitations

- Only adapters FreeBSD has a driver for will show up - USB sticks driven by
  `rtwn(4)` or `run(4)` work, for example, while `rtw88` only supports PCIe
  cards
- WPA-Enterprise and hidden networks have to be set up in
  `wpa_supplicant.conf` by hand, the XP network key dialog can't create them
- An adapter that was disabled comes back on after a reboot
- FreeBSD has one default route, so the status window only shows a default
  gateway for the adapter that has it
- 'Open Network Connections' and the firewall settings are greyed out, as the
  tools they open aren't available on FreeBSD
