FROM rockylinux:8

RUN dnf -y update \
 && dnf -y install epel-release \
 && dnf -y install ca-certificates procps-ng hostname tini \
 && dnf clean all \
 && rm -rf /var/cache/dnf

COPY pkgs/guardsarm-agent-5*.x86_64.rpm /tmp/guardsarm-agent.rpm
RUN dnf -y install /tmp/guardsarm-agent.rpm \
 && rm -f /tmp/guardsarm-agent.rpm \
 && dnf clean all \
 && rm -rf /var/cache/dnf

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/usr/bin/tini","--","/entrypoint.sh"]
