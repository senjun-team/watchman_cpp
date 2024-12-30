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


# Directory for task launching
RUN mkdir /home/code_runner/task 
COPY run-task.sh /home/code_runner/task 
COPY CMakeLists.txt /home/code_runner/task 
COPY ut.hpp /home/code_runner/task

# Directory for playground
RUN mkdir /home/code_runner/playground 
COPY run-playground.sh /home/code_runner/playground

# change access write for code_runner home directory to root
RUN useradd -ms /bin/bash code_runner \
    && chown root:code_runner /home/code_runner \
    && chown root:code_runner /home/code_runner/task \
    && chown root:code_runner /home/code_runner/playground \
    && chmod 1775 /home/code_runner \
    && chmod 1775 /home/code_runner/task \
    && chmod 1775 /home/code_runner/playground 

USER code_runner

ENTRYPOINT ["bash"]
