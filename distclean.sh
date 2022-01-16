#!/usr/bin/env bash

docker run -it -v $(pwd):/chipbox-kernel stulluk/chipbox-docker-builder \
/bin/bash -c "cd /chipbox-kernel \
&& make distclean"
