FROM python:3.10.6-slim
LABEL ru.senjun.image.authors="Shipilov Dmitry"

RUN useradd -ms /bin/bash code_runner \
    && chown root:code_runner /home/code_runner\
    && chmod 1775 /home/code_runner

WORKDIR /home/code_runner
COPY run.sh .

USER code_runner

ENTRYPOINT [ "bash" ]
