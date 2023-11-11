to compile:
`emcc mygame.c -s WASM=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' --preload-file assets -o index.js`
to run you can do something like:
`python -m http.server`
