FROM python:3.12.0-slim
LABEL ru.senjun.image.authors="Shipilov Dmitry"

RUN python3 -m pip install mypy pympler objsize

RUN useradd -ms /bin/bash -u 1000 code_runner

WORKDIR /home/code_runner
COPY run.sh .

USER code_runner

ENTRYPOINT [ "bash" ]
