FROM golang:1.22.5
LABEL ru.senjun.image.authors="Shipilov Dmitry"

# change access wrtie for code_runner home directory to root
RUN useradd -ms /bin/bash code_runner

WORKDIR /home/code_runner
COPY run.sh .

ENTRYPOINT ["bash"]
