#!/usr/bin/env bash

sed -i "s:<address>.*</address>:<address>$1</address>:g" /var/gsmsec/etc/gsmsec.conf
sed -i "s:agent.debug=0:agent.debug=2:g" /var/gsmsec/etc/internal_options.conf

sleep 1

/var/gsmsec/bin/guardsarm-control start

# Keep the container running
while true; do
    sleep 10
done
