FROM rust:1.73.0-slim-buster
LABEL ru.senjun.image.authors="Shipilov Dmitry"

# change access wrtie for code_runner home directory to root
RUN useradd -ms /bin/bash code_runner \
    && chown root:code_runner /home/code_runner \
    && chmod 1775 /home/code_runner 

WORKDIR /home/code_runner
COPY run.sh .

USER code_runner
RUN cargo new user-code && rm user-code/src/main.rs

USER root

# change access wrtie for user-code to root
# permit to write code for launching
# forbid Cargo.toml remove
# forbid remove system cargo
RUN chown root:code_runner /home/code_runner/user-code \ 
    && chmod 1775 /home/code_runner/user-code \
    && chown root:code_runner /home/code_runner/user-code/src \
    && chmod 1775 /home/code_runner/user-code/src \
    && chown root /home/code_runner/user-code/Cargo.toml \
    && chmod -R o-w /usr/local
USER code_runner

ENTRYPOINT ["bash"]
