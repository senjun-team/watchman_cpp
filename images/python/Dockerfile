FROM python:3.10.6-slim
LABEL ru.senjun.image.authors="Shipilov Dmitry"

RUN useradd -ms /bin/bash code_runner
WORKDIR /home/code_runner
COPY run.sh .
RUN chmod +x run.sh

USER code_runner

ENTRYPOINT [ "bash" ]
