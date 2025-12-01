FROM quay.io/centos/centos:stream9 AS builder

# Install build dependencies
RUN yum upgrade -y && \
    yum install -y -q gcc g++ cmake make git wget boost-devel zlib-devel curl-devel

# Copy source code
WORKDIR /build
COPY . .

# Initialize and update git submodules
RUN git submodule update --init --recursive || true

# Build project
RUN cmake -B build/ && \
    cmake --build build/

# Runtime image
# FROM quay.io/centos/centos:stream9

# Install runtime dependencies
#RUN yum upgrade -y && \
#    yum install -y -q curl && \
#    yum clean all

# Install Docker CLI for Docker API access
#RUN yum install -y -q docker-cli || \
#    (curl -fsSL https://get.docker.com -o get-docker.sh && \
#     sh get-docker.sh && \
#     rm get-docker.sh) || true

WORKDIR /app

# Copy built binary and config
#COPY --from=builder /build/build/bin/watchman_cpp /app/
#COPY --from=builder /build/accomodation/etc/watchman_cpp_config.json /app/
RUN cp -r /build/build/bin/watchman_cpp /build/accomodation/etc/watchman_cpp_config.json /app/ 

# Expose port
EXPOSE 8000

# Run watchman_cpp
CMD ["./watchman_cpp"]

