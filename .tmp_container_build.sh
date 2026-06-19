#!/bin/sh
set -eux

apk add --no-cache build-base git python3 py3-pip scons zlib-dev
rm -rf /tmp/pspgl
git clone --depth 1 https://github.com/pspdev/pspgl.git /tmp/pspgl
make -C /tmp/pspgl -j4 install
python3 misc/scripts/build_psp.py --configuration both --jobs 4
