FROM almalinux:9 AS builder

RUN dnf clean all && \
    dnf update -y && \
    dnf install -y -q gcc gcc-c++ cmake make git wget boost-devel zlib-devel libcurl-devel

WORKDIR /build
COPY . .

RUN git submodule update --init --recursive || true

RUN cmake -B build/ && \
    cmake --build build/ -j

FROM almalinux:9

RUN dnf update -y && \
    dnf install -y -q dnf-plugins-core && \
    dnf config-manager --add-repo https://download.docker.com/linux/centos/docker-ce.repo && \
    dnf install -y -q docker-ce-cli

WORKDIR /app

COPY --from=builder /build/build/bin/watchman_cpp /app/
COPY --from=builder /build/accomodation/etc/watchman_cpp_config.json /app/ 

EXPOSE 8000

CMD ["./watchman_cpp"]