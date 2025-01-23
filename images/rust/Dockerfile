FROM rust:1.79.0-slim-buster
LABEL ru.senjun.image.authors="Shipilov Dmitry"

WORKDIR /home/code_runner

# Directory for task launching
RUN mkdir /home/code_runner/task 
COPY task/run-task.sh /home/code_runner/task

# Directory for playground
RUN mkdir /home/code_runner/playground 
COPY playground/run-playground.sh /home/code_runner/playground

RUN cd task && cargo new user-code && rm user-code/src/main.rs

# Change access write for code_runner home directory to root recursively
RUN useradd -ms /bin/bash -u 1000 code_runner \
    && chown -R code_runner:code_runner /home/code_runner \
    && chmod -R 1750 /home/code_runner

ENTRYPOINT ["bash"]
