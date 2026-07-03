FROM guardsarm-base

COPY preloaded-vars.conf /guardsarm/etc/preloaded-vars.conf
RUN THREADS=$(nproc) /guardsarm/install.sh

ADD entrypoint.sh /scripts/entrypoint.sh

HEALTHCHECK --interval=5s --timeout=30s --start-period=5s --retries=35 CMD grep "INFO: (4102): Connected to the server" /var/ossec/logs/ossec.log || exit 1
