#!/bin/bash

[ "$(/var/guardsarm-manager/bin/guardsarm-manager-control status | grep -E 'clusterd is running' | wc -l)" == 1 ] || exit 1
exit 0
