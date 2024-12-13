FROM gcc:14.2.0
LABEL ru.senjun.image.authors="Shipilov Dmitry"

ARG CMAKE_VERSION=3.31.2

RUN apt-get update \
    && apt install -y ninja-build vim

RUN wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-Linux-x86_64.sh \
    -q -O /tmp/cmake-install.sh \
    && chmod u+x /tmp/cmake-install.sh \
    && mkdir /usr/bin/cmake \
    && /tmp/cmake-install.sh --skip-license --prefix=/usr/bin/cmake \
    && rm /tmp/cmake-install.sh

ENV PATH="/usr/bin/cmake/bin:${PATH}"

# change access wrtie for code_runner home directory to root
RUN useradd -ms /bin/bash code_runner \
    && chown root:code_runner /home/code_runner \
    && chmod 1775 /home/code_runner

WORKDIR /home/code_runner
COPY run.sh .

USER code_runner

ENTRYPOINT ["bash"]
