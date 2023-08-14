FROM mcr.microsoft.com/devcontainers/base:focal

# download libirimager
RUN wget http://ftp.evocortex.com/libirimager-4.1.1-amd64.deb -O /tmp/libirimager-4.1.1-amd64.deb

# install libirimager dependencies
RUN DEBIAN_FRONTEND=noninteractive apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install --yes \
        'libudev-dev' \
        'libusb-1.0-0-dev' \
        'gcc' \
        'pkg-config' \
        'build-essential' \
        'cmake'

# install libirmager, deleting the broken libirimager.postinst script first
RUN dpkg --unpack /tmp/libirimager-4.1.1-amd64.deb \
    && rm /var/lib/dpkg/info/libirimager.postinst \
    && dpkg --configure libirimager \
    && DEBIAN_FRONTEND=noninteractive apt-get install --yes --fix-broken

# install Python and Pip
RUN DEBIAN_FRONTEND=noninteractive apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install --yes \
        python3-pip \
        python3.9 \
        python-is-python3

# install pdm
RUN pip3 install pdm

# update the virtualenv with the latest version of pip that has been installed, required for PEP 621 support
RUN virtualenv --upgrade-embed-wheels
