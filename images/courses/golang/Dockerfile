FROM golang:1.22.5
LABEL ru.senjun.image.authors="Shipilov Dmitry"

# change access wrtie for code_runner home directory to root
RUN useradd -ms /bin/bash code_runner \
    && chown root:code_runner /home/code_runner \
    && chmod 1775 /home/code_runner 

WORKDIR /home/code_runner
COPY run.sh .

USER code_runner
RUN mkdir user-code && cd user-code && go mod init senjun/user/user-code

USER root

# change access write for user-code to root
# permit to write code for launching
# forbid go.mod remove

RUN chown root:code_runner /home/code_runner/user-code \ 
    && chmod 1775 /home/code_runner/user-code \
    && chown root /home/code_runner/user-code/go.mod \
    && chmod -R o-w /usr/local
USER code_runner

ENTRYPOINT ["bash"]
