#!/bin/sh
set -eu

doas docker pull pspdev/pspdev:latest
doas docker run --rm \
  -v /home/builder/godot:/src \
  -w /src \
  pspdev/pspdev:latest \
  sh -lc '
    set -eu
    apk add --no-cache build-base git python3 py3-pip scons zlib-dev
    git clone --depth 1 https://github.com/pspdev/pspgl.git /tmp/pspgl
    make -C /tmp/pspgl -j4 install
    python3 misc/scripts/build_psp.py --configuration both --jobs 4
  '
doas chown -R builder:builder /home/builder/godot/bin
