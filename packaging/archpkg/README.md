# Arch Linux package

## Build on Arch Linux

### Using AUR helper
The easiest method of installing a package is to use AUR helper like `yay`.

```
yay -S xfce-winxp-tc-git
```

All dependencies will be installer and build if needed.

### Manually from AUR
You can use `PKGBUILD` either from [`xfce-winxp-tc-git`](https://aur.archlinux.org/packages/xfce-winxp-tc-git) or from this repository.

```
# Run in directory with PKGBUILD file
makepkg -si
```

### Manually using local repository
For development and automation purposes the packages can be built using existing source. For that case use included script that creates symlink to local repo root directory and skips using `git`.

```
./build-local-pkg.sh
```

## Build on other distribution
For that case any kind of virtual machine or container with Arch Linux filesystem and programs can be used. The easiest to use is probably [Distrobox](https://distrobox.privatedns.org/).

1. Install Distrobox according to the [documentation](https://distrobox.privatedns.org/#installation)
2. `distrobox create arch --image archlinux` # create an Arch Linux container
3. `distrobox enter arch` # enter the container, `pwd` should remain the same
4. `sudo pacman -S base-devel` # install devel dependencies
5. `./build-local-pkg.sh` # use any method to build a packages like on Arch Linux
6. Package files should be created in this directory
7. `exit` # leave the container
8. `distrobox rm arch --force` # optionally remove the container