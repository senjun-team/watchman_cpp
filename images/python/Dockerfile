FROM python:3.12.0-slim
LABEL ru.senjun.image.authors="Shipilov Dmitry"

RUN python3 -m pip install mypy

RUN useradd -ms /bin/bash code_runner \
    && chown root:code_runner /home/code_runner\
    && chmod 1775 /home/code_runner

WORKDIR /home/code_runner
COPY run.sh .

USER code_runner

ENTRYPOINT [ "bash" ]
