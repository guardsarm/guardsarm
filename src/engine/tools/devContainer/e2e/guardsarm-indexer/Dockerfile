FROM ubuntu:24.04

# Update and install dependencies
RUN apt-get update && apt-get install -y locales && rm -rf /var/lib/apt/lists/* \
    && localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8
ENV LANG=en_US.utf8

# Copy all files to a temporary location for installation
WORKDIR /tmp/guardsarm-install
COPY . .

# Install guardsarm-indexer
RUN apt install -y /tmp/guardsarm-install/guardsarm-indexer_5.0.0-latest_amd64.deb


# Add Java security policy permissions for cgroup access
RUN cat <<'EOF' >> /etc/guardsarm-indexer/jvm.options
-Djava.security.policy=all.policy
-Dpermission.java.io.FilePermission=/sys/fs/cgroup/-,read
EOF

# Change jvm options to use more memory for testing from the default 1g to 4g for Xms and 4g for Xmx
RUN sed -i 's/-Xms1g/-Xms4g/g' /etc/guardsarm-indexer/jvm.options
RUN sed -i 's/-Xmx1g/-Xmx4g/g' /etc/guardsarm-indexer/jvm.options

# Create certs directory (will be populated by volume mount at runtime)
RUN mkdir -p /etc/guardsarm-indexer/certs/

# Clean up installation files
RUN rm -rf /tmp/guardsarm-install

# Set working directory
WORKDIR /root

# Wait for interactive debugging
#CMD [ "sleep", "infinity" ]
