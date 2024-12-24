FROM silkeh/clang:19
LABEL ru.senjun.image.authors="Shipilov Dmitry"

RUN apt-get update \
    && apt install -y ninja-build \
    && apt-get purge -y cmake

ARG CMAKE_VERSION=3.31.2

RUN wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-Linux-x86_64.sh \
    -q -O /tmp/cmake-install.sh \
    && chmod u+x /tmp/cmake-install.sh \
    && mkdir /usr/bin/cmake \
    && /tmp/cmake-install.sh --skip-license --prefix=/usr/bin/cmake \
    && rm /tmp/cmake-install.sh

ENV PATH="/usr/bin/cmake/bin:${PATH}"

WORKDIR /home/code_runner

# Build catch2 from sources. It is too old from the repository
RUN wget https://github.com/catchorg/Catch2/archive/refs/tags/v3.7.1.tar.gz \
    && tar -xvf /home/code_runner/v3.7.1.tar.gz \
    && cmake -S /home/code_runner/Catch2-3.7.1 -B/home/code_runner/Catch2-3.7.1/build \
    && cmake --build /home/code_runner/Catch2-3.7.1/build -- -j12 \
    && cmake --install /home/code_runner/Catch2-3.7.1/build \
    && rm -rf /home/code_runner/Catch2-3.7.1 \
    && rm -rf /home/code_runner/v3.7.1.tar.gz

# change access wrtie for code_runner home directory to root
RUN useradd -ms /bin/bash code_runner \
    && chown root:code_runner /home/code_runner \
    && chmod 1775 /home/code_runner

COPY run.sh .
COPY CMakeLists.txt .

USER code_runner

ENTRYPOINT ["bash"]
