FROM ubuntu:18.04

RUN apt-get update && apt-get install -y curl apt-transport-https lsb-release gnupg2
RUN curl -s https://packages.guardsarmsiem.com/key/GPG-KEY-GUARDSARM | apt-key add - && \
    echo "deb https://packages.guardsarmsiem.com/4.x/apt/ stable main" | tee /etc/apt/sources.list.d/guardsarm.list && \
    apt-get update && apt-get install guardsarm-agent=4.14.1-1 -y
