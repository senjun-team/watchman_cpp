FROM python:3.12.0-slim
LABEL ru.senjun.image.authors="Shipilov Dmitry"

RUN python3 -m pip install mypy pympler

WORKDIR /home/code_runner

# Directory for task launching
RUN mkdir /home/code_runner/task 
COPY task/run-task.sh /home/code_runner/task

# Directory for playground
RUN mkdir /home/code_runner/playground 
COPY playground/run-playground.sh /home/code_runner/playground

# Directory for practice
RUN mkdir /home/code_runner/practice 
COPY practice/run-practice.sh /home/code_runner/practice

# Change access write for code_runner home directory to root recursively
RUN useradd -ms /bin/bash -u 1000 code_runner \
    && chown -R code_runner:code_runner /home/code_runner \
    && chmod -R 1750 /home/code_runner

USER code_runner

ENTRYPOINT [ "bash" ]
