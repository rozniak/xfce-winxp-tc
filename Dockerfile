FROM debian:11

RUN apt update && apt install build-essential git cmake fakeroot gettext pkg-config libglib2.0-dev libgtk-3-dev libgarcon-1-dev libgarcon-gtk3-1-dev libxfce4panel-2.0-dev -y

# Standalone build with git clone inside the container
# RUN git clone https://github.com/rozniak/xfce-winxp-tc.git --recurse-submodules /xfce-winxp-tc

# Relative build with files from local repository
ADD . /xfce-winxp-tc

# To print logs while build uncomment next line and use `docker build --progress=plain . -t xfce-winxp-tc`
# RUN sed -i 's/>\+ "${log_path}" 2>&1//g' $(find /xfce-winxp-tc/packaging/deb -iname '*.sh')

WORKDIR /deb

RUN /xfce-winxp-tc/packaging/deb/libs/packlibs.sh comgtk && dpkg -i libcomgtk.deb
RUN /xfce-winxp-tc/packaging/deb/libs/packlibs.sh exec && dpkg -i libexec.deb
RUN /xfce-winxp-tc/packaging/deb/libs/packlibs.sh shllang && dpkg -i libshllang.deb

ENV PKG_CONFIG_PATH='/lib/x86_64-linux-gnu/pkgconfig'

RUN /xfce-winxp-tc/packaging/deb/panel/packplug.sh shell/start shell/systray
RUN /xfce-winxp-tc/packaging/deb/programs/packprog.sh shell/run shell/winver
RUN /xfce-winxp-tc/packaging/deb/cursors/packcurs.sh no-shadow/standard shadow/standard
RUN /xfce-winxp-tc/packaging/deb/icons/packicon.sh luna
RUN /xfce-winxp-tc/packaging/deb/fonts/packfnts.sh
RUN /xfce-winxp-tc/packaging/deb/sounds/packsnds.sh
RUN /xfce-winxp-tc/packaging/deb/themes/packthem.sh

VOLUME /deb