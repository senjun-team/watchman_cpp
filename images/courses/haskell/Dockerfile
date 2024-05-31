FROM haskell:9.8.2-slim-buster

# change access wrtie for code_runner home directory to root
RUN apt-get -y update && apt-get -y upgrade \
    && apt-get -y install python3 \
    && useradd -ms /bin/bash code_runner \
    && chown root:code_runner /home/code_runner \
    && chmod 1775 /home/code_runner \
    && mkdir /home/code_runner/.stack && chown code_runner:code_runner /home/code_runner/.stack

WORKDIR /home/code_runner
COPY run.sh .
COPY package.yaml .
COPY config.yaml .


USER code_runner
RUN stack new user-code \
    && cp config.yaml .stack/ \
    && cp package.yaml user-code/ \
    && cd user-code \
    && stack build 

USER root

RUN chown root:code_runner /home/code_runner/user-code \ 
    && rm /home/code_runner/package.yaml \
    && rm /home/code_runner/config.yaml \
    && chmod 1775 /home/code_runner/user-code \
    && chown root:code_runner /home/code_runner/user-code/src \
    && chmod 1775 /home/code_runner/user-code/src \
    && chown root:code_runner /home/code_runner/user-code/stack.yaml \
    && chmod u=rw,g=r,o=r /home/code_runner/user-code/stack.yaml \
    && chmod -R o-w /usr/local

USER code_runner

ENTRYPOINT ["bash"]
