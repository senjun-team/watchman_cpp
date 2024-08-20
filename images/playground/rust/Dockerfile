FROM rust:1.79.0-slim-buster
LABEL ru.senjun.image.authors="Shipilov Dmitry"

# change access wrtie for code_runner home directory to root
RUN useradd -ms /bin/bash code_runner \
    && chown root:code_runner /home/code_runner \
    && chmod 1775 /home/code_runner 

WORKDIR /home/code_runner
COPY run.sh .

USER code_runner

ENTRYPOINT ["bash"]
