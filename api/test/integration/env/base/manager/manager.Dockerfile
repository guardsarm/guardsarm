FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive

RUN rm -f /var/lib/dpkg/statoverride && \
    rm -f /var/lib/dpkg/lock && \
    dpkg --configure -a && \
    apt-get -f install

RUN apt-get update && apt-get install supervisor wget git python3 gnupg2 gcc g++ curl make vim libc6-dev \
    policycoreutils automake autoconf libtool apt-transport-https lsb-release python3-cryptography sqlite3 cmake -y \
    --option=Dpkg::Options::=--force-confdef

RUN wget http://archive.ubuntu.com/ubuntu/pool/main/r/rtmpdump/librtmp1_2.4+20151223.gitfa8646d.1-2build4_amd64.deb && \
    dpkg -i librtmp1_2.4+20151223.gitfa8646d.1-2build4_amd64.deb && \
    rm librtmp1_2.4+20151223.gitfa8646d.1-2build4_amd64.deb && \
    rm -rf /var/lib/apt/lists/* && ldconfig

# INSTALL MANAGER
ARG GUARDSARM_BRANCH

ADD base/manager/supervisord.conf /etc/supervisor/conf.d/

RUN mkdir guardsarm && curl -sL https://github.com/guardsarm/guardsarm/tarball/${GUARDSARM_BRANCH} | tar zx --strip-components=1 -C guardsarm
COPY base/manager/preloaded-vars.conf /guardsarm/etc/preloaded-vars.conf
RUN /guardsarm/install.sh
RUN mkdir -p /var/guardsarm-manager/etc/certs && \
    touch /var/guardsarm-manager/etc/certs/root-ca.pem && \
    touch /var/guardsarm-manager/etc/certs/manager.pem && \
    touch /var/guardsarm-manager/etc/certs/manager-key.pem
COPY base/manager/entrypoint.sh /scripts/entrypoint.sh

# HEALTHCHECK
HEALTHCHECK --retries=900 --interval=1s --timeout=30s --start-period=30s CMD /var/guardsarm-manager/framework/python/bin/python3 /tmp_volume/healthcheck/healthcheck.py || exit 1
