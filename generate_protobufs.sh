#!/bin/bash

mkdir -p generated
cd protobufs
python3 ../.pio/libdeps/cubecell/Nanopb/generator/protoc \
    --experimental_allow_proto3_optional "--nanopb_out=-S.cpp:../generated/" -I=. meshtastic/*.proto