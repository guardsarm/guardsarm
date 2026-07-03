FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

WORKDIR /tmp

RUN apt-get update && apt-get install -y \
    bash \
    wget \
    openssl \
    ca-certificates \
    tar \
 && rm -rf /var/lib/apt/lists/*

RUN wget https://packages.guardsarm.com/4.14/guardsarm-certs-tool.sh \
    && chmod +x guardsarm-certs-tool.sh

COPY generate_certs.sh /tmp/generate_certs.sh

RUN chmod +x /tmp/generate_certs.sh

ENTRYPOINT ["/tmp/generate_certs.sh"]
