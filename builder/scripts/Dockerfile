FROM ubuntu:xenial

RUN apt-get update && apt-get -y upgrade

# OE-Core dependencies
RUN apt-get install -y chrpath diffstat gawk texinfo doxygen graphviz \
    python python3 wget unzip build-essential cpio \
    git-core

# Locale settings
RUN apt-get install -y locales
RUN sed -i -e 's/# en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/' /etc/locale.gen && \
        echo 'LANG="en_US.UTF-8"'>/etc/default/locale && \
        dpkg-reconfigure --frontend=noninteractive locales && \
        update-locale LANG=en_US.UTF-8

ENV LC_ALL en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US.UTF-8

RUN useradd --uid 1000 --create-home builder
RUN apt-get install -y sudo
RUN echo "builder ALL=(ALL) NOPASSWD: ALL" | tee -a /etc/sudoers

USER builder
WORKDIR /home/builder

# Prepare OE-Core
RUN cd /home/builder \
    && git clone git://git.openembedded.org/openembedded-core -b rocko oe-core \
    && cd oe-core \
    && git clone git://git.openembedded.org/bitbake -b 1.36

ENV OE_CORE_PATH /home/builder/oe-core

CMD "/bin/bash"