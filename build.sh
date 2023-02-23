mkdir -p build
g++ -I./glad -I./stb main.cpp -o build/Boat $(sdl2-config --cflags --libs)
