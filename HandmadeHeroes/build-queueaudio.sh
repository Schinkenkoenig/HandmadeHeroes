#!/bin/bash

SDL2BIN="sdl2-64/bin/sdl2-config"
SDL2RPATH='$ORIGIN/sdl2-64/lib'

mkdir -p ../../build
pushd ../../build
c++ -DHANDMADE_SDL=1 sdl_handmade_queueaudio.cpp -o handmadehero -g `$SDL2BIN --cflags --libs` -Wl,-rpath=$SDL2RPATH
popd

