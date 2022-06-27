FROM ubuntu:focal

RUN apt update && \
    DEBIAN_FRONTEND=noninteractive apt install -y \
    sudo cmake git-core python python3 pip wget unzip emacs-nox mingw-w64 \
    libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-bad1.0-dev \
    gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly \
    gstreamer1.0-libav gstreamer1.0-doc gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl \
    gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio git-lfs

RUN git lfs install --system --skip-repo

RUN pip install -U pip

RUN pip install conan

RUN useradd -l --uid 1000 --create-home conan
RUN echo "conan ALL=(ALL) NOPASSWD: ALL" | tee -a /etc/sudoers

USER conan
WORKDIR /home/conan

RUN conan profile new default --detect && \
    conan profile update settings.compiler.libcxx=libstdc++11 default