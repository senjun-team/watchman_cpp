FROM haskell:9.8.2-slim-buster

RUN apt-get -y update && apt-get -y upgrade \
    && apt-get -y install python3

WORKDIR /home/code_runner

# Directory for task launching
RUN mkdir /home/code_runner/task 
COPY task/run-task.sh /home/code_runner/task

COPY package.yaml /home/code_runner
COPY config.yaml /home/code_runner

# Directory for task launching
RUN mkdir /home/code_runner/playground 
COPY playground/run-playground.sh /home/code_runner/playground

# Change access write for code_runner home directory to root recursively
RUN useradd -ms /bin/bash -u 1000 code_runner \
    && mkdir /home/code_runner/.stack \
    && chown -R code_runner:code_runner /home/code_runner \
    && chmod -R 1750 /home/code_runner 

USER code_runner

RUN stack new user-code \
    && cp /home/code_runner/config.yaml /home/code_runner/.stack/ \
    && cp /home/code_runner/package.yaml /home/code_runner/user-code/

RUN cd /home/code_runner/user-code/ && stack build

ENTRYPOINT ["bash"]
