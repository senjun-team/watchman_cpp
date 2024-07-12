FROM gcc:14.1.0-bookworm
LABEL ru.senjun.image.authors="Shipilov Dmitry"

# change access wrtie for code_runner home directory to root
RUN useradd -ms /bin/bash code_runner \
    && chown root:code_runner /home/code_runner \
    && chmod 1775 /home/code_runner

RUN apt-get update \
    && apt install -y cmake=3.25.1-1

WORKDIR /home/code_runner
COPY run.sh .

USER code_runner

ENTRYPOINT ["bash"]
