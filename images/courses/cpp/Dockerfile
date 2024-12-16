FROM gcc:14.1.0-bookworm
LABEL ru.senjun.image.authors="Shipilov Dmitry"

# change access wrtie for code_runner home directory to root
RUN useradd -ms /bin/bash code_runner \
    && chown root:code_runner /home/code_runner \
    && chmod 1775 /home/code_runner

RUN apt-get update \
    && apt install -y cmake=3.25.1-1

WORKDIR /home/code_runner

# Build catch2 from sources. It is too old from the repository
RUN wget https://github.com/catchorg/Catch2/archive/refs/tags/v3.7.1.tar.gz \
    && tar -xvf /home/code_runner/v3.7.1.tar.gz \
    && cmake -S/home/code_runner/Catch2-3.7.1 -B/home/code_runner/Catch2-3.7.1/build \
    && cmake --build /home/code_runner/Catch2-3.7.1/build -- -j12 \
    && cmake --install /home/code_runner/Catch2-3.7.1/build \
    && rm -rf /home/code_runner/Catch2-3.7.1 \
    && rm -rf /home/code_runner/v3.7.1.tar.gz

COPY run.sh .

USER code_runner

ENTRYPOINT ["bash"]
