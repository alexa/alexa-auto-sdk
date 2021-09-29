FROM ubuntu:xenial

RUN apt-get update && apt-get install -y \
    chrpath diffstat gawk texinfo doxygen graphviz \
    python python3 wget unzip build-essential cpio \
    git-core default-jdk ninja-build \
    sudo locales

# Use openssl 1.1 rather than an older version included in apt-get
ARG AAC_OPENSSL_VERSION

RUN if [ -z $AAC_OPENSSL_VERSION ]; then echo "No openssl version defined in the build command for this dockerfile. Please specify it in AAC_OPENSSL_VERSION. E.g. AAC_OPENSSL_VERSION=1.1.1k"; exit 1; fi \
    && cd /root \
    && wget https://www.openssl.org/source/openssl-${AAC_OPENSSL_VERSION}.tar.gz || wget https://www.openssl.org/source/old/${AAC_OPENSSL_VERSION:0:5}/openssl-${AAC_OPENSSL_VERSION}.tar.gz \
    && apt-get -y remove openssl \
    && apt-get -y install ca-certificates \
    && tar xvzf openssl-${AAC_OPENSSL_VERSION}.tar.gz \
    && cd openssl-${AAC_OPENSSL_VERSION} \
    && ./config -Wl,--enable-new-dtags,-rpath,'$(LIBRPATH)' --prefix=/usr/ --openssldir=/usr/ \
    && make -j8 \
    && make install

ENV LD_LIBRARY_PATH="/usr/local/lib:${LD_LIBRARY_PATH}"
ENV PKG_CONFIG_PATH="/usr/local/lib/pkgconfig"
ENV CPATH="/usr/local/include:${CPATH}"

# Locale settings
RUN sed -i -e 's/# en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/' /etc/locale.gen && \
        echo 'LANG="en_US.UTF-8"'>/etc/default/locale && \
        dpkg-reconfigure --frontend=noninteractive locales && \
        update-locale LANG=en_US.UTF-8

ENV LC_ALL en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US.UTF-8

ARG USER_ID
ARG GROUP_ID
RUN [ $(getent group ${GROUP_ID}) ] || addgroup --gid $GROUP_ID builder
RUN [ $(getent passwd builder) ] || useradd -l --uid $USER_ID --gid $GROUP_ID --create-home builder
RUN echo "builder ALL=(ALL) NOPASSWD: ALL" | tee -a /etc/sudoers

USER builder
WORKDIR /home/builder

# Prepare OE-Core
RUN cd /home/builder \
    && git clone git://git.openembedded.org/openembedded-core -b rocko oe-core \
    && cd oe-core \
    && git checkout 1b18cdf6b8bdb00ff5df165b9ac7bc2b10c87d57 \
    && git clone git://git.openembedded.org/bitbake -b 1.36

ENV OE_CORE_PATH /home/builder/oe-core

# Additional packages
RUN sudo apt-get update && sudo apt-get install -y quilt \
    libsqlite3-dev libarchive-dev python3-dev \
    libdb-dev libpopt-dev

# Audio dependencies
RUN sudo apt-get update && sudo apt-get install -y \
    libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev

# Coverage support
RUN sudo apt-get update && sudo apt-get install -y lcov

CMD "/bin/bash"
