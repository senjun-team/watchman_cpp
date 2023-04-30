FROM rust:1.64.0-slim-buster
LABEL ru.senjun.image.authors="Shipilov Dmitry"

RUN useradd -ms /bin/bash code_runner
WORKDIR /home/code_runner

COPY run.sh .
RUN chmod +x run.sh

USER code_runner
RUN cargo new user-code && rm user-code/src/main.rs

ENTRYPOINT ["bash"]
