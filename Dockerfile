# Instructions:
# 1. Build image: `docker build -t xfce-winxp-tc .`
# 2. Run container: `docker run --rm -v ./out:/out xfce-winxp-tc`
# 3. Package files will be copied to `./out` directory
# 4. For Arch Linux use `debtap` to convert packages to `pacman` format

FROM debian:11

RUN apt update && apt install -y \
	build-essential \
	git \
	cmake \
	fakeroot \
	gettext \
	pkg-config \
	libglib2.0-dev \
	libgtk-3-dev \
	libgarcon-1-dev \
	libgarcon-gtk3-1-dev \
	libxfce4panel-2.0-dev \
	rename x11-apps \
	ruby-sass \
	python3-venv

# Standalone build with git clone inside the container (instead of 'ADD')
# RUN git clone https://github.com/rozniak/xfce-winxp-tc.git --recurse-submodules /xfce-winxp-tc

# Relative build with files from local repository (instead of 'RUN git clone')
ADD . /xfce-winxp-tc

# Optionally log all messages to STDOUT for easy debugging
# RUN sed -i 's/>\+ "${log_path}" 2>&1//g' $(find /xfce-winxp-tc/packaging/deb -iname '*.sh')
# RUN sed -i 's/>\+ "${CMAKE_LOG_PATH}"//g' $(find /xfce-winxp-tc/packaging/deb -iname '*.sh')

WORKDIR /deb

# Compile and install included libraries used as dependencies for final components
RUN /xfce-winxp-tc/packaging/deb/libs/packlibs.sh comgtk && dpkg -i libcomgtk.deb
RUN /xfce-winxp-tc/packaging/deb/libs/packlibs.sh exec && dpkg -i libexec.deb
RUN /xfce-winxp-tc/packaging/deb/libs/packlibs.sh shllang && dpkg -i libshllang.deb

# Variable needed by dpkg to see installed packages
ENV PKG_CONFIG_PATH='/lib/x86_64-linux-gnu/pkgconfig'

# Compile and package all final components (comment out unnecessary components)
RUN /xfce-winxp-tc/packaging/deb/panel/packplug.sh shell/start shell/systray
RUN /xfce-winxp-tc/packaging/deb/programs/packprog.sh shell/run shell/winver
RUN /xfce-winxp-tc/packaging/deb/cursors/packcurs.sh no-shadow/standard with-shadow/standard
RUN /xfce-winxp-tc/packaging/deb/icons/packicon.sh luna
RUN /xfce-winxp-tc/packaging/deb/fonts/packfnts.sh
RUN /xfce-winxp-tc/packaging/deb/sounds/packsnds.sh

# Fix for unexpected character (SCSS compilation)
RUN sed -i "s/’/'/g" $(find /xfce-winxp-tc/themes -iname '*.scss')
RUN /xfce-winxp-tc/packaging/deb/themes/packthem.sh luna/blue native professional

# Copy all packages and logs to `/out` mounted directory
VOLUME /out
CMD cp /deb/* /out