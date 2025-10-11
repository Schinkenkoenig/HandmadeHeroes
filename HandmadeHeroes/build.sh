#!/bin/bash
mkdir -p ../build
pushd ../build
c++ -DHANDMADE_SDL=1 ~/Work/Learn/HandmadeHeroes/HandmadeHeroes/sdl_handmade.cpp -o handmadehero -g -gsplit-dwarf `sdl2-config --cflags --libs`
popd

