FROM ubuntu:bionic

RUN apt update && \
    DEBIAN_FRONTEND=noninteractive apt install -y \
    sudo software-properties-common wget

RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null && \
    apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main'
    
RUN apt update && \
    DEBIAN_FRONTEND=noninteractive apt install -y \
    openjdk-11-jdk cmake git-core python python3 python3.7 python3-pip unzip emacs-nox mingw-w64 \
    libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-bad1.0-dev \
    gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly \
    gstreamer1.0-libav gstreamer1.0-doc gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl \
    gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio git-lfs

RUN git lfs install --system --skip-repo

RUN update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.6 1 && \
    update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.7 2

RUN pip3 install -U pip

RUN pip3 install conan

RUN useradd -l --uid 1000 --create-home conan
RUN echo "conan ALL=(ALL) NOPASSWD: ALL" | tee -a /etc/sudoers

USER conan
WORKDIR /home/conan

RUN conan profile new default --detect && \
    conan profile update settings.compiler.libcxx=libstdc++11 default