FROM quay.io/centos/centos:stream9 AS builder

RUN --mount=type=cache,target=/var/cache/yum \
    yum upgrade -y && \
    yum install -y -q gcc g++ cmake make git wget boost-devel zlib-devel curl-devel


WORKDIR /build
COPY . .

RUN git submodule update --init --recursive || true

RUN cmake -B build/ && \
    cmake --build build/ -j

FROM quay.io/centos/centos:stream9

RUN --mount=type=cache,target=/var/cache/yum \
    yum upgrade -y && \
    yum install -y -q yum-utils && \
    yum-config-manager --add-repo https://download.docker.com/linux/centos/docker-ce.repo && \
    yum install -y -q docker-ce-cli

WORKDIR /app

COPY --from=builder /build/build/bin/watchman_cpp /app/
COPY --from=builder /build/accomodation/etc/watchman_cpp_config.json /app/ 

EXPOSE 8000

CMD ["./watchman_cpp"]

